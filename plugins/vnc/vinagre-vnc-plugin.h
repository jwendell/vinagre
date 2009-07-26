/*
 * vinagre-vnc-plugin.h
 * This file is part of vinagre
 *
 * Copyright (C) 2009 Jorge Pereira <jorge@jorgepereira.com.br>
 * 
 * vinagre-vnc-plugin.h is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * vinagre-vnc-plugin.h is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __VINAGRE_VNC_PLUGIN_H__
#define __VINAGRE_VNC_PLUGIN_H__

#include <glib.h>
#include <glib-object.h>
#include <vinagre/vinagre-plugin.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define VINAGRE_TYPE_VNC_PLUGIN                 (vinagre_vnc_plugin_get_type ())
#define VINAGRE_VNC_PLUGIN(o)                   (G_TYPE_CHECK_INSTANCE_CAST ((o), VINAGRE_TYPE_VNC_PLUGIN, VinagreVncPlugin))
#define VINAGRE_VNC_PLUGIN_CLASS(k)             (G_TYPE_CHECK_CLASS_CAST((k), VINAGRE_TYPE_VNC_PLUGIN, VinagreVncPluginClass))
#define VINAGRE_IS_VNC_PLUGIN(o)                (G_TYPE_CHECK_INSTANCE_TYPE ((o), VINAGRE_TYPE_VNC_PLUGIN))
#define VINAGRE_IS_VNC_PLUGIN_CLASS(k)          (G_TYPE_CHECK_CLASS_TYPE ((k), VINAGRE_TYPE_VNC_PLUGIN))
#define VINAGRE_VNC_PLUGIN_GET_CLASS(o)         (G_TYPE_INSTANCE_GET_CLASS ((o), VINAGRE_TYPE_VNC_PLUGIN, VinagreVncPluginClass))

/* Private structure type */
typedef struct _VinagreVncPluginPrivate	VinagreVncPluginPrivate;

/*
 * Main object structure
 */
typedef struct _VinagreVncPlugin		VinagreVncPlugin;

struct _VinagreVncPlugin
{
  VinagrePlugin parent_instance;
};

/*
 * Class definition
 */
typedef struct _VinagreVncPluginClass	VinagreVncPluginClass;

struct _VinagreVncPluginClass
{
  VinagrePluginClass parent_class;
};

/*
 * Public methods
 */
GType	vinagre_vnc_plugin_get_type		(void) G_GNUC_CONST;

/* All the plugins must implement this function */
G_MODULE_EXPORT GType register_vinagre_plugin (GTypeModule *module);

G_END_DECLS

#endif /* __VINAGRE_VNC_PLUGIN_H__ */
/* vim: set ts=8: */
