/*
 * vinagre-plugin-loader.h
 * This file is part of vinagre
 *
 * Based on gedit plugin system
 * Copyright (C) 2008 - Jesse van den Kieboom
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA. 
 */

#ifndef __VINAGRE_PLUGIN_LOADER_H__
#define __VINAGRE_PLUGIN_LOADER_H__

#include <glib-object.h>
#include "vinagre-plugin.h"
#include "vinagre-plugin-info.h"

G_BEGIN_DECLS

#define VINAGRE_TYPE_PLUGIN_LOADER                (vinagre_plugin_loader_get_type ())
#define VINAGRE_PLUGIN_LOADER(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), VINAGRE_TYPE_PLUGIN_LOADER, VinagrePluginLoader))
#define VINAGRE_IS_PLUGIN_LOADER(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), VINAGRE_TYPE_PLUGIN_LOADER))
#define VINAGRE_PLUGIN_LOADER_GET_INTERFACE(inst) (G_TYPE_INSTANCE_GET_INTERFACE ((inst), VINAGRE_TYPE_PLUGIN_LOADER, VinagrePluginLoaderInterface))

typedef struct _VinagrePluginLoader VinagrePluginLoader; /* dummy object */
typedef struct _VinagrePluginLoaderInterface VinagrePluginLoaderInterface;

struct _VinagrePluginLoaderInterface {
	GTypeInterface parent;

	const gchar *(*get_id)		(void);

	VinagrePlugin *(*load) 		(VinagrePluginLoader 	*loader,
			     		 VinagrePluginInfo	*info,
			      		 const gchar       	*path);

	void 	     (*unload)		(VinagrePluginLoader 	*loader,
					 VinagrePluginInfo       	*info);

	void         (*garbage_collect) 	(VinagrePluginLoader	*loader);
};

GType vinagre_plugin_loader_get_type (void);

const gchar *vinagre_plugin_loader_type_get_id	(GType 			 type);
VinagrePlugin *vinagre_plugin_loader_load		(VinagrePluginLoader 	*loader,
						 VinagrePluginInfo 	*info,
						 const gchar		*path);
void vinagre_plugin_loader_unload			(VinagrePluginLoader 	*loader,
						 VinagrePluginInfo	*info);
void vinagre_plugin_loader_garbage_collect	(VinagrePluginLoader 	*loader);

/**
 * VINAGRE_PLUGIN_LOADER_IMPLEMENT_INTERFACE(TYPE_IFACE, iface_init):
 *
 * Utility macro used to register interfaces for gobject types in plugin loaders.
 */
#define VINAGRE_PLUGIN_LOADER_IMPLEMENT_INTERFACE(TYPE_IFACE, iface_init)		\
	const GInterfaceInfo g_implement_interface_info = 			\
	{ 									\
		(GInterfaceInitFunc) iface_init,				\
		NULL, 								\
		NULL								\
	};									\
										\
	g_type_module_add_interface (type_module,				\
				     g_define_type_id, 				\
				     TYPE_IFACE, 				\
				     &g_implement_interface_info);

/**
 * VINAGRE_PLUGIN_LOADER_REGISTER_TYPE(PluginLoaderName, plugin_loader_name, PARENT_TYPE, loader_interface_init):
 *
 * Utility macro used to register plugin loaders.
 */
#define VINAGRE_PLUGIN_LOADER_REGISTER_TYPE(PluginLoaderName, plugin_loader_name, PARENT_TYPE, loader_iface_init) 	\
	G_DEFINE_DYNAMIC_TYPE_EXTENDED (PluginLoaderName,			\
					plugin_loader_name,			\
					PARENT_TYPE,			\
					0,					\
					VINAGRE_PLUGIN_LOADER_IMPLEMENT_INTERFACE(VINAGRE_TYPE_PLUGIN_LOADER, loader_iface_init));	\
										\
										\
G_MODULE_EXPORT GType								\
register_vinagre_plugin_loader (GTypeModule *type_module)				\
{										\
	plugin_loader_name##_register_type (type_module);			\
										\
	return plugin_loader_name##_get_type();					\
}

G_END_DECLS

#endif /* __VINAGRE_PLUGIN_LOADER_H__ */
