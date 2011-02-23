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

enum
{
  PROTOCOL_ADDED,
  PROTOCOL_REMOVED,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };
VinagrePluginsEngine *default_engine = NULL;

static void
vinagre_plugins_engine_load_extensions (VinagrePluginsEngine *engine)
{
 GStrv plugins;

  g_object_get (vinagre_prefs_get_default (),
		"active-plugins", &plugins,
		NULL);

  engine->priv->loading_plugin_list = TRUE;
  peas_engine_set_loaded_plugins (PEAS_ENGINE (engine), (const gchar **)plugins);
  engine->priv->loading_plugin_list = FALSE;
  g_strfreev (plugins);
}

static void
vinagre_plugins_engine_extension_added (PeasExtensionSet     *extensions,
					PeasPluginInfo	     *info,
					PeasExtension	     *exten,
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
  g_signal_emit (engine, signals[PROTOCOL_ADDED], 0, exten);
}

static void
vinagre_plugins_engine_extension_removed (PeasExtensionSet     *extensions,
					  PeasPluginInfo       *info,
					  PeasExtension	       *exten,
					  VinagrePluginsEngine *engine)
{
  const gchar *protocol = NULL;

  peas_extension_call (exten, "get_protocol", &protocol);

  g_hash_table_remove (engine->priv->protocols, (gpointer)protocol);
  g_signal_emit (engine, signals[PROTOCOL_REMOVED], 0, exten);
}

static void
vinagre_plugins_engine_constructed (GObject *object)
{
  VinagrePluginsEngine *engine;

  if (G_OBJECT_CLASS (vinagre_plugins_engine_parent_class)->constructed)
    G_OBJECT_CLASS (vinagre_plugins_engine_parent_class)->constructed (object);

  engine = VINAGRE_PLUGINS_ENGINE (object);
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

  vinagre_plugins_engine_load_extensions (engine);
}

static void
vinagre_plugins_engine_init (VinagrePluginsEngine *engine)
{
  gchar *tmp, *typelib_dir;
  GError *error = NULL;
  PeasEngine *p_engine = PEAS_ENGINE (engine);

  engine->priv = G_TYPE_INSTANCE_GET_PRIVATE (engine,
					      VINAGRE_TYPE_PLUGINS_ENGINE,
					      VinagrePluginsEnginePrivate);

  engine->priv->loading_plugin_list = FALSE;
  engine->priv->protocols = g_hash_table_new (g_str_hash, g_str_equal);

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
  g_irepository_require_private (g_irepository_get_default (),
				 typelib_dir, "Vinagre", "3.0", 0, &error);
  g_free (typelib_dir);
  g_free (tmp);
  if (error)
    {
      g_print ("error registering vinagre typelib: %s\n", error->message);
      g_error_free (error);
    }

  tmp = vinagre_dirs_get_user_plugins_dir ();
  peas_engine_add_search_path (p_engine, tmp, tmp);
  g_free (tmp);

  tmp = vinagre_dirs_get_vinagre_plugins_dir ();
  peas_engine_add_search_path (p_engine, tmp, tmp);
  g_free (tmp);
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

  loaded_plugins = peas_engine_get_loaded_plugins (PEAS_ENGINE (engine));

  g_object_set (vinagre_prefs_get_default (),
		"active-plugins", loaded_plugins,
		NULL);

  g_strfreev (loaded_plugins);
}

static void
vinagre_plugins_engine_load_plugin (PeasEngine	   *engine,
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
  object_class->constructed = vinagre_plugins_engine_constructed;

  engine_class->load_plugin = vinagre_plugins_engine_load_plugin;
  engine_class->unload_plugin = vinagre_plugins_engine_unload_plugin;

  signals[PROTOCOL_ADDED] =
		g_signal_new ("protocol-added",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      0,
			      NULL, NULL,
			      g_cclosure_marshal_VOID__OBJECT,
			      G_TYPE_NONE,
			      1,
			      PEAS_TYPE_EXTENSION);

  signals[PROTOCOL_REMOVED] =
		g_signal_new ("protocol-removed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      0,
			      NULL, NULL,
			      g_cclosure_marshal_VOID__OBJECT,
			      G_TYPE_NONE,
			      1,
			      PEAS_TYPE_EXTENSION);

  g_type_class_add_private (klass, sizeof (VinagrePluginsEnginePrivate));
}

/**
 * vinagre_plugins_engine_get_default:
 *
 * Return value: (transfer none):
 */
VinagrePluginsEngine *
vinagre_plugins_engine_get_default (void)
{
  if (!default_engine)
    {
      default_engine = VINAGRE_PLUGINS_ENGINE (g_object_new (VINAGRE_TYPE_PLUGINS_ENGINE, NULL));
      g_object_add_weak_pointer (G_OBJECT (default_engine),
				 (gpointer) &default_engine);
    }

  return default_engine;
}

/**
 * vinagre_plugins_engine_get_plugin_by_protocol:
 *
 * Return value: (allow-none) (transfer none):
 */
VinagreProtocol *
vinagre_plugins_engine_get_plugin_by_protocol (VinagrePluginsEngine *engine,
					       const gchar	    *protocol)
{
  g_return_val_if_fail (VINAGRE_IS_PLUGINS_ENGINE (engine), NULL);

  return g_hash_table_lookup (engine->priv->protocols, (gconstpointer) protocol);
}

/**
 * vinagre_plugins_engine_get_plugins_by_protocol:
 *
 * Return value: (element-type utf8 PeasExtension) (transfer none):
 */
GHashTable *
vinagre_plugins_engine_get_plugins_by_protocol (VinagrePluginsEngine *engine)
{
  g_return_val_if_fail (VINAGRE_IS_PLUGINS_ENGINE (engine), NULL);

  return engine->priv->protocols;
}

/* ex:set ts=8 noet: */
