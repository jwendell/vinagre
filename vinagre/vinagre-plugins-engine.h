/*
 * vinagre-plugins-engine.h
 * This file is part of vinagre
 *
 * Copyright (C) 2010 Jonh Wendell <wendell@bani.com.br>
 *
 * vinagre-plugins-engine.h is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * vinagre-plugins-engine.h is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __VINAGRE_PLUGINS_ENGINE_H__
#define __VINAGRE_PLUGINS_ENGINE_H__

#include <glib-object.h>

#include "vinagre-protocol.h"

G_BEGIN_DECLS

#define VINAGRE_TYPE_PLUGINS_ENGINE              (vinagre_plugins_engine_get_type ())
#define VINAGRE_PLUGINS_ENGINE(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), VINAGRE_TYPE_PLUGINS_ENGINE, VinagrePluginsEngine))
#define VINAGRE_PLUGINS_ENGINE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), VINAGRE_TYPE_PLUGINS_ENGINE, VinagrePluginsEngineClass))
#define VINAGRE_IS_PLUGINS_ENGINE(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), VINAGRE_TYPE_PLUGINS_ENGINE))
#define VINAGRE_IS_PLUGINS_ENGINE_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), VINAGRE_TYPE_PLUGINS_ENGINE))
#define VINAGRE_PLUGINS_ENGINE_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), VINAGRE_TYPE_PLUGINS_ENGINE, VinagrePluginsEngineClass))

typedef struct _VinagrePluginsEngine		VinagrePluginsEngine;
typedef struct _VinagrePluginsEnginePrivate	VinagrePluginsEnginePrivate;
typedef struct _VinagrePluginsEngineClass	VinagrePluginsEngineClass;

struct _VinagrePluginsEngine
{
  GObject parent;
  VinagrePluginsEnginePrivate *priv;
};

struct _VinagrePluginsEngineClass
{
  GObjectClass parent_class;
};

GType			 vinagre_plugins_engine_get_type	(void) G_GNUC_CONST;

VinagrePluginsEngine	*vinagre_plugins_engine_get_default	(void);
VinagreProtocol		*vinagre_plugins_engine_get_plugin_by_protocol	(VinagrePluginsEngine *engine,
									 const gchar          *protocol);
GHashTable		*vinagre_plugins_engine_get_plugins_by_protocol	(VinagrePluginsEngine *engine);
gboolean                 vinagre_plugins_engine_load_extension  (VinagrePluginsEngine *engine,
                                                                 const gchar *name);
G_END_DECLS

#endif  /* __VINAGRE_PLUGINS_ENGINE_H__ */

/* vim: set ts=8: */
