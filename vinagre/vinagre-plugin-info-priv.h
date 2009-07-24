/*
 * vinagre-plugin-info-priv.h
 * This file is part of vinagre
 *
 * Based on gedit plugin system
 * Copyright (C) 2002-2005 - Paolo Maggi 
 * Copyright (C) 2007 - Paolo Maggi, Steve Frécinaux
 * 
 * vinagre-plugin-info-priv.h is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * vinagre-plugin-info-priv.h is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __VINAGRE_PLUGIN_INFO_PRIV_H__
#define __VINAGRE_PLUGIN_INFO_PRIV_H__

#include "vinagre-plugin-info.h"
#include "vinagre-plugin.h"

struct _VinagrePluginInfo
{
  gint               refcount;

  VinagrePlugin     *plugin;
  gchar             *file;

  gchar             *module_name;
  gchar	            *loader;
  gchar            **dependencies;

  gchar             *name;
  gchar             *desc;
  gchar             *icon_name;
  gchar            **authors;
  gchar             *copyright;
  gchar             *website;
  gchar             *version;

  /* A plugin is unavailable if it is not possible to activate it
     due to an error loading the plugin module (e.g. for Python plugins
     when the interpreter has not been correctly initializated) */
  gint               available : 1;
  gint               engine : 1;
};

VinagrePluginInfo	*_vinagre_plugin_info_new	(const gchar *file);
void			 _vinagre_plugin_info_ref	(VinagrePluginInfo *info);
void			 _vinagre_plugin_info_unref	(VinagrePluginInfo *info);

#endif /* __VINAGRE_PLUGIN_INFO_PRIV_H__ */
/* vim: set ts=8: */
