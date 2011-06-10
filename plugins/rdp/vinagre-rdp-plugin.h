/*
 * vinagre-rdp-plugin.h
 * This file is part of vinagre
 *
 * Copyright (C) 2010 Jonh Wendell <wendell@bani.com.br>
 * 
 * vinagre-rdp-plugin.h is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * vinagre-rdp-plugin.h is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __VINAGRE_RDP_PLUGIN_H__
#define __VINAGRE_RDP_PLUGIN_H__

#include <glib.h>
#include <glib-object.h>

#include "vinagre/vinagre-static-extension.h"

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define VINAGRE_TYPE_RDP_PLUGIN                 (vinagre_rdp_plugin_get_type ())
#define VINAGRE_RDP_PLUGIN(o)                   (G_TYPE_CHECK_INSTANCE_CAST ((o), VINAGRE_TYPE_RDP_PLUGIN, VinagreRdpPlugin))
#define VINAGRE_RDP_PLUGIN_CLASS(k)             (G_TYPE_CHECK_CLASS_CAST((k), VINAGRE_TYPE_RDP_PLUGIN, VinagreRdpPluginClass))
#define VINAGRE_IS_RDP_PLUGIN(o)                (G_TYPE_CHECK_INSTANCE_TYPE ((o), VINAGRE_TYPE_RDP_PLUGIN))
#define VINAGRE_IS_RDP_PLUGIN_CLASS(k)          (G_TYPE_CHECK_CLASS_TYPE ((k), VINAGRE_TYPE_RDP_PLUGIN))
#define VINAGRE_RDP_PLUGIN_GET_CLASS(o)         (G_TYPE_INSTANCE_GET_CLASS ((o), VINAGRE_TYPE_RDP_PLUGIN, VinagreRdpPluginClass))

typedef struct _VinagreRdpPlugin	VinagreRdpPlugin;
typedef struct _VinagreRdpPluginClass	VinagreRdpPluginClass;

struct _VinagreRdpPlugin
{
  VinagreStaticExtension parent_instance;
};

struct _VinagreRdpPluginClass
{
  VinagreStaticExtensionClass parent_class;
};

GType vinagre_rdp_plugin_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __VINAGRE_RDP_PLUGIN_H__ */

/* vim: set ts=8: */
