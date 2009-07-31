/*
 * vinagre-plugins-engine.c
 * This file is part of vinagre
 *
 * Based on gedit plugin system
 * Copyright (C) 2002-2005 Paolo Maggi
 * Copyright (C) 2009 Jonh Wendell <wendell@bani.com.br>s
 * 
 * vinagre-plugins-engine.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * vinagre-plugins-engine.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <glib/gi18n.h>

#include "vinagre-plugins-engine.h"
#include "vinagre-plugin-info-priv.h"
#include "vinagre-plugin.h"
#include "vinagre-debug.h"
#include "vinagre-app.h"
#include "vinagre-prefs.h"
#include "vinagre-plugin-loader.h"
#include "vinagre-object-module.h"
#include "vinagre-dirs.h"

#define VINAGRE_PLUGINS_ENGINE_BASE_KEY "/apps/vinagre/plugins"
#define VINAGRE_PLUGINS_ENGINE_KEY VINAGRE_PLUGINS_ENGINE_BASE_KEY "/active-plugins"

#define PLUGIN_EXT	".vinagre-plugin"
#define LOADER_EXT	G_MODULE_SUFFIX

typedef struct
{
  VinagrePluginLoader *loader;
  VinagreObjectModule *module;
} LoaderInfo;

/* Signals */
enum
{
  ACTIVATE_PLUGIN,
  DEACTIVATE_PLUGIN,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

G_DEFINE_TYPE(VinagrePluginsEngine, vinagre_plugins_engine, G_TYPE_OBJECT)

struct _VinagrePluginsEnginePrivate
{
  GSList *plugin_list;
  GHashTable *loaders;
  GHashTable *protocols;

  gboolean activate_from_prefs;
};

VinagrePluginsEngine *default_engine = NULL;

static void	vinagre_plugins_engine_activate_plugin_real (VinagrePluginsEngine *engine,
							     VinagrePluginInfo    *info);
static void	vinagre_plugins_engine_deactivate_plugin_real (VinagrePluginsEngine *engine,
							       VinagrePluginInfo    *info);

typedef gboolean (*LoadDirCallback)(VinagrePluginsEngine *engine, const gchar *filename, gpointer userdata);

static gboolean
load_dir_real (VinagrePluginsEngine *engine,
	       const gchar          *dir,
	       const gchar          *suffix,
	       LoadDirCallback      callback,
	       gpointer             userdata)
{
	GError *error = NULL;
	GDir *d;
	const gchar *dirent;
	gboolean ret = TRUE;

	g_return_val_if_fail (dir != NULL, TRUE);

	vinagre_debug_message (DEBUG_PLUGINS, "DIR: %s", dir);

	d = g_dir_open (dir, 0, &error);
	if (!d)
	{
		g_warning ("%s", error->message);
		g_error_free (error);
		return TRUE;
	}
	
	while ((dirent = g_dir_read_name (d)))
	{
		gchar *filename;

		if (!g_str_has_suffix (dirent, suffix))
			continue;

		filename = g_build_filename (dir, dirent, NULL);

		ret = callback (engine, filename, userdata);

		g_free (filename);

		if (!ret)
			break;
	}

	g_dir_close (d);
	return ret;
}

static gboolean
load_plugin_info (VinagrePluginsEngine *engine,
            		  const gchar        *filename,
            		  gpointer            userdata)
{
	VinagrePluginInfo *info;
	
	info = _vinagre_plugin_info_new (filename);

	if (info == NULL)
		return TRUE;

	/* If a plugin with this name has already been loaded
	 * drop this one (user plugins override system plugins) */
	if (vinagre_plugins_engine_get_plugin_info (engine, vinagre_plugin_info_get_module_name (info)) != NULL)
	{
		vinagre_debug_message (DEBUG_PLUGINS, "Two or more plugins named '%s'. "
				     "Only the first will be considered.\n",
				     vinagre_plugin_info_get_module_name (info));

		_vinagre_plugin_info_unref (info);

		return TRUE;
	}

	engine->priv->plugin_list = g_slist_prepend (engine->priv->plugin_list, info);

	vinagre_debug_message (DEBUG_PLUGINS, "Plugin %s loaded", info->name);
	return TRUE;
}

static void
load_all_plugins (VinagrePluginsEngine *engine)
{
	gchar *plugin_dir;
	const gchar *pdirs_env = NULL;

	/* load user plugins */
	plugin_dir = vinagre_dirs_get_user_plugins_dir ();
	if (g_file_test (plugin_dir, G_FILE_TEST_IS_DIR))
	{
		load_dir_real (engine,
			       plugin_dir,
			       PLUGIN_EXT,
			       load_plugin_info,
			       NULL);

	}
	g_free (plugin_dir);

	/* load system plugins */
	pdirs_env = g_getenv ("VINAGRE_PLUGINS_PATH");

	vinagre_debug_message (DEBUG_PLUGINS, "VINAGRE_PLUGINS_PATH=%s", pdirs_env);

	if (pdirs_env != NULL)
	{
		gchar **pdirs;
		gint i;

		pdirs = g_strsplit (pdirs_env, G_SEARCHPATH_SEPARATOR_S, 0);

		for (i = 0; pdirs[i] != NULL; i++)
		{
			if (!load_dir_real (engine,
					    pdirs[i],
					    PLUGIN_EXT,
					    load_plugin_info,
					    NULL))
			{
				break;
			}
		}

		g_strfreev (pdirs);
	}
	else
	{
		plugin_dir = vinagre_dirs_get_vinagre_plugins_dir ();

		load_dir_real (engine,
			       plugin_dir,
			       PLUGIN_EXT,
			       load_plugin_info,
			       NULL);

		g_free (plugin_dir);
	}
}

static guint
hash_lowercase (gconstpointer data)
{
	gchar *lowercase;
	guint ret;
	
	lowercase = g_ascii_strdown ((const gchar *)data, -1);
	ret = g_str_hash (lowercase);
	g_free (lowercase);
	
	return ret;
}

static gboolean
equal_lowercase (gconstpointer a, gconstpointer b)
{
	return g_ascii_strcasecmp ((const gchar *)a, (const gchar *)b) == 0;
}

static void
loader_destroy (LoaderInfo *info)
{
	if (!info)
		return;
	
	if (info->loader)
		g_object_unref (info->loader);
	
	g_free (info);
}

static void
add_loader (VinagrePluginsEngine *engine,
      	    const gchar        *loader_id,
      	    VinagreObjectModule  *module)
{
	LoaderInfo *info;

	info = g_new (LoaderInfo, 1);
	info->loader = NULL;
	info->module = module;

	g_hash_table_insert (engine->priv->loaders, g_strdup (loader_id), info);
}

static void
activate_engine_plugins (VinagrePluginsEngine *engine)
{
  GSList *active_plugins, *l;

  vinagre_debug_message (DEBUG_PLUGINS, "Activating engine plugins");

  g_object_get (vinagre_prefs_get_default (),
		"active-plugins", &active_plugins,
		NULL);

  for (l = engine->priv->plugin_list; l; l = l->next)
    {
      VinagrePluginInfo *info = (VinagrePluginInfo*)l->data;
		
      if (g_slist_find_custom (active_plugins,
			       vinagre_plugin_info_get_module_name (info),
			       (GCompareFunc)strcmp) == NULL)
	continue;
		
      if (vinagre_plugin_info_is_engine (info))
	vinagre_plugins_engine_activate_plugin (engine, info);
    }
}

static void
vinagre_plugins_engine_init (VinagrePluginsEngine *engine)
{
  vinagre_debug (DEBUG_PLUGINS);

  if (!g_module_supported ())
    {
      g_warning ("vinagre is not able to initialize the plugins engine.");
      return;
    }

  engine->priv = G_TYPE_INSTANCE_GET_PRIVATE (engine,
					      VINAGRE_TYPE_PLUGINS_ENGINE,
					      VinagrePluginsEnginePrivate);

  load_all_plugins (engine);

  /* make sure that the first reactivation will read active plugins
     from the prefs */
  engine->priv->activate_from_prefs = TRUE;

  /* mapping from loadername -> loader object */
  engine->priv->loaders = g_hash_table_new_full (hash_lowercase,
						 equal_lowercase,
						 (GDestroyNotify)g_free,
						 (GDestroyNotify)loader_destroy);

  engine->priv->protocols = g_hash_table_new (g_str_hash, g_str_equal);
  activate_engine_plugins (engine);
}

static void
loader_garbage_collect (const char *id, 
                        LoaderInfo *info)
{
	if (info->loader)
		vinagre_plugin_loader_garbage_collect (info->loader);
}

void
vinagre_plugins_engine_garbage_collect (VinagrePluginsEngine *engine)
{
	g_hash_table_foreach (engine->priv->loaders,
			      (GHFunc) loader_garbage_collect,
			      NULL);
}

static void
vinagre_plugins_engine_finalize (GObject *object)
{
	VinagrePluginsEngine *engine = VINAGRE_PLUGINS_ENGINE (object);
	GSList *item;
	
	vinagre_debug (DEBUG_PLUGINS);

	/* Firs deactivate all plugins */
	for (item = engine->priv->plugin_list; item; item = item->next)
	{
		VinagrePluginInfo *info = VINAGRE_PLUGIN_INFO (item->data);
		
		if (vinagre_plugin_info_is_active (info))
			vinagre_plugins_engine_deactivate_plugin_real (engine, info);
	}
	
	/* unref the loaders */	
	g_hash_table_destroy (engine->priv->loaders);

	g_hash_table_destroy (engine->priv->protocols);

	/* and finally free the infos */
	for (item = engine->priv->plugin_list; item; item = item->next)
	{
		VinagrePluginInfo *info = VINAGRE_PLUGIN_INFO (item->data);

		_vinagre_plugin_info_unref (info);
	}

	g_slist_free (engine->priv->plugin_list);

	G_OBJECT_CLASS (vinagre_plugins_engine_parent_class)->finalize (object);
}

static void
vinagre_plugins_engine_class_init (VinagrePluginsEngineClass *klass)
{
	GType the_type = G_TYPE_FROM_CLASS (klass);
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = vinagre_plugins_engine_finalize;
	klass->activate_plugin = vinagre_plugins_engine_activate_plugin_real;
	klass->deactivate_plugin = vinagre_plugins_engine_deactivate_plugin_real;

	signals[ACTIVATE_PLUGIN] =
		g_signal_new ("activate-plugin",
			      the_type,
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (VinagrePluginsEngineClass, activate_plugin),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__BOXED,
			      G_TYPE_NONE,
			      1,
			      VINAGRE_TYPE_PLUGIN_INFO | G_SIGNAL_TYPE_STATIC_SCOPE);

	signals[DEACTIVATE_PLUGIN] =
		g_signal_new ("deactivate-plugin",
			      the_type,
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (VinagrePluginsEngineClass, deactivate_plugin),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__BOXED,
			      G_TYPE_NONE,
			      1,
			      VINAGRE_TYPE_PLUGIN_INFO | G_SIGNAL_TYPE_STATIC_SCOPE);

	g_type_class_add_private (klass, sizeof (VinagrePluginsEnginePrivate));
}

static gboolean
load_loader (VinagrePluginsEngine *engine,
      	     const gchar        *filename,
	           gpointer		 data)
{
	VinagreObjectModule *module;
	gchar *base;
	gchar *path;
	const gchar *id;
	GType type;
	
	/* try to load in the module */
	path = g_path_get_dirname (filename);
	base = g_path_get_basename (filename);

	/* for now they are all resident */
	module = vinagre_object_module_new (base,
					  path,
					  "register_vinagre_plugin_loader",
					  TRUE);

	g_free (base);
	g_free (path);

	/* make sure to load the type definition */
	if (!g_type_module_use (G_TYPE_MODULE (module)))
	{
		g_object_unref (module);
		g_warning ("Plugin loader module `%s' could not be loaded", filename);

		return TRUE;
	}

	/* get the exported type and check the name as exported by the 
	 * loader interface */
	type = vinagre_object_module_get_object_type (module);
	id = vinagre_plugin_loader_type_get_id (type);
	
	add_loader (engine, id, module);	
	g_type_module_unuse (G_TYPE_MODULE (module));

	return TRUE;
}

static void
ensure_loader (LoaderInfo *info)
{
	if (info->loader == NULL && info->module != NULL)
	{
		/* create a new loader object */
		VinagrePluginLoader *loader;
		loader = (VinagrePluginLoader *)vinagre_object_module_new_object (info->module, NULL);
	
		if (loader == NULL || !VINAGRE_IS_PLUGIN_LOADER (loader))
		{
			g_warning ("Loader object is not a valid VinagrePluginLoader instance");
		
			if (loader != NULL && G_IS_OBJECT (loader))
				g_object_unref (loader);
		}
		else
		{
			info->loader = loader;
		}
	}
}

static VinagrePluginLoader *
get_plugin_loader (VinagrePluginsEngine *engine, 
                   VinagrePluginInfo *info)
{
	const gchar *loader_id;
	LoaderInfo *loader_info;

	loader_id = info->loader;

	loader_info = (LoaderInfo *)g_hash_table_lookup (
			engine->priv->loaders, 
			loader_id);

	if (loader_info == NULL)
	{
		gchar *loader_dir;

		loader_dir = vinagre_dirs_get_vinagre_plugin_loaders_dir ();

		/* loader could not be found in the hash, try to find it by 
		   scanning */
		load_dir_real (engine, 
			       loader_dir,
			       LOADER_EXT,
			       (LoadDirCallback)load_loader,
			       NULL);
		g_free (loader_dir);
		
		loader_info = (LoaderInfo *)g_hash_table_lookup (
				engine->priv->loaders, 
				loader_id);
	}

	if (loader_info == NULL)
	{
		/* cache non-existent so we don't scan again */
		add_loader (engine, loader_id, NULL);
		return NULL;
	}
	
	ensure_loader (loader_info);
	return loader_info->loader;
}

VinagrePluginsEngine *
vinagre_plugins_engine_get_default (void)
{
  if (default_engine != NULL)
    return default_engine;

  default_engine = VINAGRE_PLUGINS_ENGINE (g_object_new (VINAGRE_TYPE_PLUGINS_ENGINE, NULL));
  g_object_add_weak_pointer (G_OBJECT (default_engine),
			     (gpointer) &default_engine);
  return default_engine;
}

const GSList *
vinagre_plugins_engine_get_plugin_list (VinagrePluginsEngine *engine)
{
  vinagre_debug (DEBUG_PLUGINS);
  return engine->priv->plugin_list;
}

static gint
compare_plugin_info_and_name (VinagrePluginInfo *info,
			      const gchar *module_name)
{
  return strcmp (vinagre_plugin_info_get_module_name (info), module_name);
}

VinagrePluginInfo *
vinagre_plugins_engine_get_plugin_info (VinagrePluginsEngine *engine,
					const gchar          *name)
{
  GSList *l = g_slist_find_custom (engine->priv->plugin_list,
				   name,
				   (GCompareFunc) compare_plugin_info_and_name);

  return l == NULL ? NULL : (VinagrePluginInfo *) l->data;
}

static void
save_active_plugin_list (VinagrePluginsEngine *engine)
{
  GSList *l, *active_plugins = NULL;

  for (l = engine->priv->plugin_list; l != NULL; l = l->next)
    {
      VinagrePluginInfo *info = (VinagrePluginInfo *) l->data;

      if (vinagre_plugin_info_is_active (info))
	active_plugins = g_slist_prepend (active_plugins,
					  (gpointer)vinagre_plugin_info_get_module_name (info));
    }

  g_object_set (vinagre_prefs_get_default (),
		"active-plugins", active_plugins,
		NULL);

  g_slist_free (active_plugins);
}

static gboolean
load_plugin (VinagrePluginsEngine *engine,
      	     VinagrePluginInfo    *info)
{
	VinagrePluginLoader *loader;
	gchar *path;

	if (vinagre_plugin_info_is_active (info))
		return TRUE;
	
	if (!vinagre_plugin_info_is_available (info))
		return FALSE;

	loader = get_plugin_loader (engine, info);
	
	if (loader == NULL)
	{
		g_warning ("Could not find loader `%s' for plugin `%s'", info->loader, info->name);
		info->available = FALSE;
		return FALSE;
	}
	
	path = g_path_get_dirname (info->file);
	g_return_val_if_fail (path != NULL, FALSE);

	info->plugin = vinagre_plugin_loader_load (loader, info, path);
	
	g_free (path);
	
	if (info->plugin == NULL)
	{
		g_warning ("Error loading plugin '%s'", info->name);
		info->available = FALSE;
		return FALSE;
	}

	g_object_set_data (G_OBJECT (info->plugin), "info", info);

	return TRUE;
}

static void
vinagre_plugins_engine_activate_plugin_real (VinagrePluginsEngine *engine,
					     VinagrePluginInfo    *info)
{
  const GList *wins;
  const gchar *protocol;
  VinagrePluginInfo *plugin_protocol;

  if (!load_plugin (engine, info))
    return;

  if (vinagre_plugin_info_is_engine (info))
    {
      protocol = vinagre_plugin_get_protocol (info->plugin);
      plugin_protocol = g_hash_table_lookup (engine->priv->protocols, protocol);
      if (plugin_protocol)
	{
	  g_warning ("The protocol %s was already registered by the plugin %s",
		     protocol,
		     vinagre_plugin_info_get_name (plugin_protocol));
	  return;
	}

      vinagre_plugin_activate (info->plugin, NULL);
      g_hash_table_insert (engine->priv->protocols, (gpointer)protocol, info->plugin);
      return;
    }

  /* activate plugin for all windows */
  wins = vinagre_app_get_windows (vinagre_app_get_default ());
  for (; wins != NULL; wins = wins->next)
    vinagre_plugin_activate (info->plugin, VINAGRE_WINDOW (wins->data));
}

gboolean
vinagre_plugins_engine_activate_plugin (VinagrePluginsEngine *engine,
					VinagrePluginInfo    *info)
{
  vinagre_debug (DEBUG_PLUGINS);

  g_return_val_if_fail (info != NULL, FALSE);

  if (!vinagre_plugin_info_is_available (info))
    return FALSE;
		
  if (vinagre_plugin_info_is_active (info))
    return TRUE;

  g_signal_emit (engine, signals[ACTIVATE_PLUGIN], 0, info);

  if (vinagre_plugin_info_is_active (info))
    save_active_plugin_list (engine);

  return vinagre_plugin_info_is_active (info);
}

static void
call_plugin_deactivate (VinagrePlugin *plugin, 
			VinagreWindow *window)
{
  vinagre_plugin_deactivate (plugin, window);

  /* ensure update of ui manager, because we suspect it does something
     with expected static strings in the type module (when unloaded the
     strings don't exist anymore, and ui manager updates in an idle
     func) */
  gtk_ui_manager_ensure_update (vinagre_window_get_ui_manager (window));
}

static void
vinagre_plugins_engine_deactivate_plugin_real (VinagrePluginsEngine *engine,
					       VinagrePluginInfo    *info)
{
  const GList *wins;
  VinagrePluginLoader *loader;

  if (!vinagre_plugin_info_is_active (info) || 
      !vinagre_plugin_info_is_available (info))
    return;

  if (vinagre_plugin_info_is_engine (info))
    {
      g_hash_table_remove (engine->priv->protocols,
			   vinagre_plugin_get_protocol (info->plugin));
      vinagre_plugin_deactivate (info->plugin, NULL);
    }
  else
    {
      wins = vinagre_app_get_windows (vinagre_app_get_default ());
      for (; wins != NULL; wins = wins->next)
	call_plugin_deactivate (info->plugin, VINAGRE_WINDOW (wins->data));
    }

  /* first unref the plugin (the loader still has one) */
  g_object_unref (info->plugin);
	
  /* find the loader and tell it to gc and unload the plugin */
  loader = get_plugin_loader (engine, info);
	
  vinagre_plugin_loader_garbage_collect (loader);
  vinagre_plugin_loader_unload (loader, info);
	
  info->plugin = NULL;
}

gboolean
vinagre_plugins_engine_deactivate_plugin (VinagrePluginsEngine *engine,
					  VinagrePluginInfo    *info)
{
  vinagre_debug (DEBUG_PLUGINS);

  g_return_val_if_fail (info != NULL, FALSE);

  if (!vinagre_plugin_info_is_active (info))
    return TRUE;

  g_signal_emit (engine, signals[DEACTIVATE_PLUGIN], 0, info);
  if (!vinagre_plugin_info_is_active (info))
    save_active_plugin_list (engine);

  return !vinagre_plugin_info_is_active (info);
}

void
vinagre_plugins_engine_activate_plugins (VinagrePluginsEngine *engine,
					 VinagreWindow        *window)
{
  GSList *pl, *active_plugins = NULL;

  vinagre_debug (DEBUG_PLUGINS);

  g_return_if_fail (VINAGRE_IS_PLUGINS_ENGINE (engine));
  g_return_if_fail (VINAGRE_IS_WINDOW (window));

  /* the first time, we get the 'active' plugins from gconf */
  if (engine->priv->activate_from_prefs)
    {
      g_object_get (vinagre_prefs_get_default (),
		    "active-plugins", &active_plugins,
		    NULL);
    }

  for (pl = engine->priv->plugin_list; pl; pl = pl->next)
    {
      VinagrePluginInfo *info = (VinagrePluginInfo*)pl->data;
		
      if (engine->priv->activate_from_prefs && 
	  g_slist_find_custom (active_plugins,
			       vinagre_plugin_info_get_module_name (info),
			       (GCompareFunc)strcmp) == NULL)
	continue;
		
      /* If plugin is not active, don't try to activate/load it */
      if (!engine->priv->activate_from_prefs && 
	  !vinagre_plugin_info_is_active (info))
	continue;

      if (load_plugin (engine, info))
	vinagre_plugin_activate (info->plugin, window);
    }
	
  if (engine->priv->activate_from_prefs)
    {
      g_slist_foreach (active_plugins, (GFunc) g_free, NULL);
      g_slist_free (active_plugins);
      engine->priv->activate_from_prefs = FALSE;
    }
	
  vinagre_debug_message (DEBUG_PLUGINS, "End");

  /* also call update_ui after activation */
  vinagre_plugins_engine_update_plugins_ui (engine, window);
}

void
vinagre_plugins_engine_deactivate_plugins (VinagrePluginsEngine *engine,
                              					   VinagreWindow        *window)
{
	GSList *pl;
	
	vinagre_debug (DEBUG_PLUGINS);

	g_return_if_fail (VINAGRE_IS_PLUGINS_ENGINE (engine));
	g_return_if_fail (VINAGRE_IS_WINDOW (window));
	
	for (pl = engine->priv->plugin_list; pl; pl = pl->next)
	{
		VinagrePluginInfo *info = (VinagrePluginInfo*)pl->data;
		
		/* check if the plugin is actually active */
		if (!vinagre_plugin_info_is_active (info))
			continue;
		
		/* call deactivate for the plugin for this window */
		vinagre_plugin_deactivate (info->plugin, window);
	}
	
	vinagre_debug_message (DEBUG_PLUGINS, "End");
}

void
vinagre_plugins_engine_update_plugins_ui (VinagrePluginsEngine *engine,
                                          VinagreWindow        *window)
{
	GSList *pl;

	vinagre_debug (DEBUG_PLUGINS);

	g_return_if_fail (VINAGRE_IS_PLUGINS_ENGINE (engine));
	g_return_if_fail (VINAGRE_IS_WINDOW (window));

	/* call update_ui for all active plugins */
	for (pl = engine->priv->plugin_list; pl; pl = pl->next)
	{
		VinagrePluginInfo *info = (VinagrePluginInfo*)pl->data;

		if (!vinagre_plugin_info_is_active (info))
			continue;
			
	       	vinagre_debug_message (DEBUG_PLUGINS, "Updating UI of %s", info->name);
		vinagre_plugin_update_ui (info->plugin, window);
	}
}

void 	 
vinagre_plugins_engine_configure_plugin (VinagrePluginsEngine *engine,
                          				       VinagrePluginInfo    *info,
                          				       GtkWindow          *parent)
{
	GtkWidget *conf_dlg;
	
	GtkWindowGroup *wg;
	
	vinagre_debug (DEBUG_PLUGINS);

	g_return_if_fail (info != NULL);

	conf_dlg = vinagre_plugin_create_configure_dialog (info->plugin);
	g_return_if_fail (conf_dlg != NULL);
	gtk_window_set_transient_for (GTK_WINDOW (conf_dlg),
				      parent);

	wg = parent->group;		      
	if (wg == NULL)
	{
		wg = gtk_window_group_new ();
		gtk_window_group_add_window (wg, parent);
	}
			
	gtk_window_group_add_window (wg,
				     GTK_WINDOW (conf_dlg));
		
	gtk_window_set_modal (GTK_WINDOW (conf_dlg), TRUE);		     
	gtk_widget_show (conf_dlg);
}

void 
vinagre_plugins_engine_active_plugins_changed (VinagrePluginsEngine *engine)
{
	gboolean to_activate;
	GSList *pl, *active_plugins;

	vinagre_debug (DEBUG_PLUGINS);

	g_object_get (vinagre_prefs_get_default (),
		      "active-plugins", &active_plugins,
		      NULL);

	for (pl = engine->priv->plugin_list; pl; pl = pl->next)
	{
		VinagrePluginInfo *info = (VinagrePluginInfo*)pl->data;

		if (!vinagre_plugin_info_is_available (info))
			continue;

		to_activate = (g_slist_find_custom (active_plugins,
						    vinagre_plugin_info_get_module_name (info),
						    (GCompareFunc)strcmp) != NULL);

		if (!vinagre_plugin_info_is_active (info) && to_activate)
			g_signal_emit (engine, signals[ACTIVATE_PLUGIN], 0, info);
		else if (vinagre_plugin_info_is_active (info) && !to_activate)
			g_signal_emit (engine, signals[DEACTIVATE_PLUGIN], 0, info);
	}

	g_slist_foreach (active_plugins, (GFunc) g_free, NULL);
	g_slist_free (active_plugins);
}

void
vinagre_plugins_engine_rescan_plugins (VinagrePluginsEngine *engine)
{
	vinagre_debug (DEBUG_PLUGINS);
	
	load_all_plugins (engine);
}

GHashTable *
vinagre_plugin_engine_get_plugins_by_protocol (VinagrePluginsEngine *engine)
{
  return engine->priv->protocols;
}

VinagrePlugin *
vinagre_plugins_engine_get_plugin_by_protocol (VinagrePluginsEngine *engine,
					      const gchar          *protocol)
{
  g_return_val_if_fail (VINAGRE_IS_PLUGINS_ENGINE (engine), NULL);

  return g_hash_table_lookup (engine->priv->protocols, (gconstpointer) protocol);
}

/* vim: set ts=8: */
