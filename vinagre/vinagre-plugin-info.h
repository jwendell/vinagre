/*
 * vinagre-plugin-info.h
 * This file is part of vinagre
 * Based on gedit plugin system
 * Copyright (C) 2002-2005 - Paolo Maggi 
 * Copyright (C) 2007 - Paolo Maggi, Steve Frécinaux
 * 
 * vinagre-plugin-info.h is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * vinagre-plugin-info.h is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __VINAGRE_PLUGIN_INFO_H__
#define __VINAGRE_PLUGIN_INFO_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define VINAGRE_TYPE_PLUGIN_INFO			(vinagre_plugin_info_get_type ())
#define VINAGRE_PLUGIN_INFO(obj)			((VinagrePluginInfo *) (obj))

typedef struct _VinagrePluginInfo			VinagrePluginInfo;

GType		 vinagre_plugin_info_get_type		(void) G_GNUC_CONST;

gboolean 	 vinagre_plugin_info_is_active		(VinagrePluginInfo *info);
gboolean 	 vinagre_plugin_info_is_available	(VinagrePluginInfo *info);
gboolean	 vinagre_plugin_info_is_configurable	(VinagrePluginInfo *info);

const gchar	*vinagre_plugin_info_get_module_name	(VinagrePluginInfo *info);

const gchar	*vinagre_plugin_info_get_name		(VinagrePluginInfo *info);
const gchar	*vinagre_plugin_info_get_description	(VinagrePluginInfo *info);
const gchar	*vinagre_plugin_info_get_icon_name	(VinagrePluginInfo *info);
const gchar    **vinagre_plugin_info_get_authors	(VinagrePluginInfo *info);
const gchar	*vinagre_plugin_info_get_website	(VinagrePluginInfo *info);
const gchar	*vinagre_plugin_info_get_copyright	(VinagrePluginInfo *info);
const gchar	*vinagre_plugin_info_get_version	(VinagrePluginInfo *info);
gboolean	 vinagre_plugin_info_is_engine		(VinagrePluginInfo *info);

G_END_DECLS

#endif /* __VINAGRE_PLUGIN_INFO_H__ */
/* vim: set ts=8: */
