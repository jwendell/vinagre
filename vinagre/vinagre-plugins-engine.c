/*
 * vinagre-plugins-engine.c
 * This file is part of vinagre
 *
 * Copyright (C) 2010 Jonh Wendell <wendell@bani.com.br>
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

#include <girepository.h>

#include "vinagre-plugins-engine.h"
#include "vinagre-dirs.h"
#include "vinagre-debug.h"
#include "vinagre-protocol.h"
#include "vinagre-prefs.h"

G_DEFINE_TYPE (VinagrePluginsEngine, vinagre_plugins_engine, PEAS_TYPE_ENGINE)

struct _VinagrePluginsEnginePrivate
{
  gboolean loading_plugin_list : 1;
  GHashTable *protocols;
  PeasExtensionSet *extensions;
};

VinagrePluginsEngine *default_engine = NULL;

static void
vinagre_plugins_engine_extension_added (PeasExtensionSet     *extensions,
					PeasPluginInfo       *info,
					PeasExtension        *exten,
					VinagrePluginsEngine *engine)
{
  PeasExtension *previous_ext;
  const gchar *protocol = NULL;

  peas_extension_call (exten, "get_protocol", &protocol);

  previous_ext = g_hash_table_lookup (engine->priv->protocols, protocol);

  if (previous_ext)
    {
      g_warning ("The protocol %s was already registered by the plugin %s",
		 protocol,
		 peas_plugin_info_get_name (info));
      return;
    }

  g_hash_table_insert (engine->priv->protocols, (gpointer)protocol, exten);
}

static void
vinagre_plugins_engine_extension_removed (PeasExtensionSet    *extensions,
					  PeasPluginInfo      *info,
					  PeasExtension       *exten,
					 VinagrePluginsEngine *engine)
{
  const gchar *protocol = NULL;

  peas_extension_call (exten, "get_protocol", &protocol);

  g_hash_table_remove (engine->priv->protocols, (gpointer)protocol);
}

static void
vinagre_plugins_engine_init (VinagrePluginsEngine *engine)
{
  engine->priv = G_TYPE_INSTANCE_GET_PRIVATE (engine,
					      VINAGRE_TYPE_PLUGINS_ENGINE,
					      VinagrePluginsEnginePrivate);

  engine->priv->loading_plugin_list = FALSE;
  engine->priv->protocols = g_hash_table_new (g_str_hash, g_str_equal);

  engine->priv->extensions = peas_extension_set_new (PEAS_ENGINE (engine),
						     VINAGRE_TYPE_PROTOCOL,
						     NULL);
  g_signal_connect (engine->priv->extensions,
		    "extension-added",
		    G_CALLBACK (vinagre_plugins_engine_extension_added),
		    engine);
  g_signal_connect (engine->priv->extensions,
		    "extension-removed",
		    G_CALLBACK (vinagre_plugins_engine_extension_removed),
		    engine);
}

static void
vinagre_plugins_engine_finalize (GObject *object)
{
  VinagrePluginsEngine *engine = VINAGRE_PLUGINS_ENGINE (object);

  g_hash_table_destroy (engine->priv->protocols);

  G_OBJECT_CLASS (vinagre_plugins_engine_parent_class)->finalize (object);
}

static void
save_plugin_list (VinagrePluginsEngine *engine)
{
  gchar **loaded_plugins;
  GSList *plugins = NULL;
  gint i;

  loaded_plugins = peas_engine_get_loaded_plugins (PEAS_ENGINE (engine));
  for (i = 0; loaded_plugins[i]; i++)
    plugins = g_slist_prepend (plugins, loaded_plugins[i]);

  g_object_set (vinagre_prefs_get_default (),
		"active-plugins", plugins,
		NULL);

  g_strfreev (loaded_plugins);
  g_slist_free (plugins);
}

static void
vinagre_plugins_engine_load_plugin (PeasEngine     *engine,
				    PeasPluginInfo *info)
{
  VinagrePluginsEngine *vengine = VINAGRE_PLUGINS_ENGINE (engine);

  PEAS_ENGINE_CLASS (vinagre_plugins_engine_parent_class)->load_plugin (engine, info);

  /* We won't save the plugin list if we are currently loading the
   * plugins from the saved list */
  if (!vengine->priv->loading_plugin_list && peas_plugin_info_is_loaded (info))
    save_plugin_list (vengine);
}

static void
vinagre_plugins_engine_unload_plugin (PeasEngine     *engine,
				      PeasPluginInfo *info)
{
  VinagrePluginsEngine *vengine = VINAGRE_PLUGINS_ENGINE (engine);

  PEAS_ENGINE_CLASS (vinagre_plugins_engine_parent_class)->unload_plugin (engine, info);

  /* We won't save the plugin list if we are currently unloading the
   * plugins from the saved list */
  if (!vengine->priv->loading_plugin_list && !peas_plugin_info_is_loaded (info))
    save_plugin_list (vengine);
}

static void
vinagre_plugins_engine_class_init (VinagrePluginsEngineClass *klass)
{
  PeasEngineClass *engine_class = PEAS_ENGINE_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = vinagre_plugins_engine_finalize;

  engine_class->load_plugin = vinagre_plugins_engine_load_plugin;
  engine_class->unload_plugin = vinagre_plugins_engine_unload_plugin;

  g_type_class_add_private (klass, sizeof (VinagrePluginsEnginePrivate));
}

static void
vinagre_plugins_engine_active_plugins_changed (VinagrePluginsEngine *engine)
{
  GSList *plugins, *l;
  gchar **loaded_plugins;
  gint i;

  g_object_get (vinagre_prefs_get_default (),
		"active-plugins", &plugins,
		NULL);

  loaded_plugins = g_new0 (gchar *, g_slist_length (plugins) + 1);
  i = 0;
  for (l = plugins; l; l = l->next)
    loaded_plugins[i++] = (l->data);

  engine->priv->loading_plugin_list = TRUE;
  peas_engine_set_loaded_plugins (PEAS_ENGINE (engine),
				  (const gchar **) loaded_plugins);
  engine->priv->loading_plugin_list = FALSE;
  g_strfreev (loaded_plugins);
  g_slist_free (plugins);
}

VinagrePluginsEngine *
vinagre_plugins_engine_get_default (void)
{
  gchar *tmp, *typelib_dir, **search_paths;
  GError *error;

  if (default_engine != NULL)
    return default_engine;

  /* This should be moved to libpeas */
  g_irepository_require (g_irepository_get_default (),
			 "Peas", "1.0", 0, NULL);
  g_irepository_require (g_irepository_get_default (),
			 "PeasUI", "1.0", 0, NULL);

  /* Require vinagre's typelib. */
  tmp = vinagre_dirs_get_vinagre_lib_dir ();
  typelib_dir = g_build_filename (tmp,
				  "girepository-1.0",
				  NULL);
  error = NULL;
  g_irepository_require_private (g_irepository_get_default (),
				 typelib_dir, "Vinagre", "3.0", 0, &error);
  g_free (typelib_dir);
  g_free (tmp);
  if (error)
    {
      g_print ("error registering vinagre typelib: %s\n", error->message);
      g_error_free (error);
    }

  search_paths = g_new (gchar *, 5);
  /* Add the user plugins dir in ~ */
  search_paths[0] = vinagre_dirs_get_user_plugins_dir ();
  search_paths[1] = vinagre_dirs_get_user_plugins_dir ();
  /* Add the system plugins dir */
  search_paths[2] = vinagre_dirs_get_vinagre_plugins_dir ();
  search_paths[3] = vinagre_dirs_get_vinagre_plugins_dir ();
  /* Add the trailing NULL */
  search_paths[4] = NULL;

  default_engine = VINAGRE_PLUGINS_ENGINE (g_object_new (VINAGRE_TYPE_PLUGINS_ENGINE,
							 "app-name", "Vinagre",
							 "search-paths", search_paths,
							 NULL));

  g_strfreev (search_paths);

  g_object_add_weak_pointer (G_OBJECT (default_engine),
			     (gpointer) &default_engine);

  vinagre_plugins_engine_active_plugins_changed (default_engine);

  return default_engine;
}


VinagreProtocolExt *
vinagre_plugins_engine_get_plugin_by_protocol (VinagrePluginsEngine *engine,
					       const gchar          *protocol)
{
  g_return_val_if_fail (VINAGRE_IS_PLUGINS_ENGINE (engine), NULL);

  return g_hash_table_lookup (engine->priv->protocols, (gconstpointer) protocol);
}

GHashTable *
vinagre_plugins_engine_get_plugins_by_protocol (VinagrePluginsEngine *engine)
{
  g_return_val_if_fail (VINAGRE_IS_PLUGINS_ENGINE (engine), NULL);

  return engine->priv->protocols;
}

/* ex:set ts=8 noet: */
