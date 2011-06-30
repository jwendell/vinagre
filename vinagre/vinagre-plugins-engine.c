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

#include "vinagre-plugins-engine.h"
#include "vinagre-debug.h"
#include "vinagre-protocol.h"
#include "vinagre-prefs.h"
#include "vinagre-static-extension.h"

G_DEFINE_TYPE (VinagrePluginsEngine, vinagre_plugins_engine, G_TYPE_OBJECT)

struct _VinagrePluginsEnginePrivate
{
  gboolean loading_plugin_list : 1;
  GHashTable *protocols;
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
vinagre_plugins_engine_add_protocol (VinagrePluginsEngine *engine,
    const gchar *name, GObject *protocol)
{
    GObject *previous_ext = g_hash_table_lookup (engine->priv->protocols, name);

    if (previous_ext)
    {
        g_warning ("The protocol %s was already registered", name);
        return;
    }

    g_hash_table_insert (engine->priv->protocols, (gpointer)name, protocol);
    g_signal_emit (engine, signals[PROTOCOL_ADDED], 0, protocol);
}

gboolean
vinagre_plugins_engine_load_extension (VinagrePluginsEngine *engine,
    const gchar *name)
{
    g_warn_if_reached ();
    return FALSE;
}

static void
vinagre_plugins_engine_init (VinagrePluginsEngine *engine)
{
  guint n_children;
  GType child, *children;
  GObject *object;
  const gchar *protocol;

  engine->priv = G_TYPE_INSTANCE_GET_PRIVATE (engine,
					      VINAGRE_TYPE_PLUGINS_ENGINE,
					      VinagrePluginsEnginePrivate);

  engine->priv->loading_plugin_list = FALSE;
  engine->priv->protocols = g_hash_table_new (g_str_hash, g_str_equal);

  children = g_type_children (VINAGRE_TYPE_STATIC_EXTENSION, &n_children);
  while (n_children > 0 )
  {
    n_children--;
    child = children[n_children];
    object = g_object_new (child, NULL);
    protocol = vinagre_protocol_get_protocol (VINAGRE_PROTOCOL (object));
    vinagre_plugins_engine_add_protocol (engine, protocol, object);
  }

  g_free (children);
}

static void
vinagre_plugins_engine_finalize (GObject *object)
{
  VinagrePluginsEngine *engine = VINAGRE_PLUGINS_ENGINE (object);

  g_hash_table_destroy (engine->priv->protocols);

  G_OBJECT_CLASS (vinagre_plugins_engine_parent_class)->finalize (object);
}

static void
vinagre_plugins_engine_class_init (VinagrePluginsEngineClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = vinagre_plugins_engine_finalize;

  signals[PROTOCOL_ADDED] =
		g_signal_new ("protocol-added",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      0,
			      NULL, NULL,
			      g_cclosure_marshal_VOID__OBJECT,
			      G_TYPE_NONE,
			      1,
			      VINAGRE_TYPE_PROTOCOL);

  signals[PROTOCOL_REMOVED] =
		g_signal_new ("protocol-removed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      0,
			      NULL, NULL,
			      g_cclosure_marshal_VOID__OBJECT,
			      G_TYPE_NONE,
			      1,
			      VINAGRE_TYPE_PROTOCOL);

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
 * Return value: (element-type utf8 GObject) (transfer none):
 */
GHashTable *
vinagre_plugins_engine_get_plugins_by_protocol (VinagrePluginsEngine *engine)
{
  g_return_val_if_fail (VINAGRE_IS_PLUGINS_ENGINE (engine), NULL);

  return engine->priv->protocols;
}

/* ex:set ts=8 noet: */
