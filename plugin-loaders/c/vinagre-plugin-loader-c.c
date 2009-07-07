/*
 * vinagre-plugin-loader-c.c
 * This file is part of vinagre
 *
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

#include "vinagre-plugin-loader-c.h"
#include <vinagre/vinagre-object-module.h>

#define VINAGRE_PLUGIN_LOADER_C_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), VINAGRE_TYPE_PLUGIN_LOADER_C, VinagrePluginLoaderCPrivate))

struct _VinagrePluginLoaderCPrivate
{
	GHashTable *loaded_plugins;
};

static void vinagre_plugin_loader_iface_init (gpointer g_iface, gpointer iface_data);

VINAGRE_PLUGIN_LOADER_REGISTER_TYPE (VinagrePluginLoaderC, vinagre_plugin_loader_c, G_TYPE_OBJECT, vinagre_plugin_loader_iface_init);


static const gchar *
vinagre_plugin_loader_iface_get_id (void)
{
	return "C";
}

static VinagrePlugin *
vinagre_plugin_loader_iface_load (VinagrePluginLoader *loader,
				VinagrePluginInfo   *info,
				const gchar       *path)
{
	VinagrePluginLoaderC *cloader = VINAGRE_PLUGIN_LOADER_C (loader);
	VinagreObjectModule *module;
	const gchar *module_name;
	VinagrePlugin *result;

	module = (VinagreObjectModule *)g_hash_table_lookup (cloader->priv->loaded_plugins, info);
	module_name = vinagre_plugin_info_get_module_name (info);

	if (module == NULL)
	{
		/* For now we force all modules to be resident */
		module = vinagre_object_module_new (module_name,
						  path,
						  "register_vinagre_plugin",
						  TRUE);

		/* Infos are available for all the lifetime of the loader.
		 * If this changes, we should use weak refs or something */

		g_hash_table_insert (cloader->priv->loaded_plugins, info, module);
	}

	if (!g_type_module_use (G_TYPE_MODULE (module)))
	{
		g_warning ("Could not load plugin module: %s", vinagre_plugin_info_get_name (info));

		return NULL;
	}

	/* TODO: for now we force data-dir-name = module-name... if needed we can
	 * add a datadir field to the plugin descriptor file.
	 */
	result = (VinagrePlugin *)vinagre_object_module_new_object (module,
								"install-dir", path,
								"data-dir-name", module_name,
								NULL);

	if (!result)
	{
		g_warning ("Could not create plugin object: %s", vinagre_plugin_info_get_name (info));
		g_type_module_unuse (G_TYPE_MODULE (module));
		
		return NULL;
	}

	g_type_module_unuse (G_TYPE_MODULE (module));
	
	return result;
}

static void
vinagre_plugin_loader_iface_unload (VinagrePluginLoader *loader,
				  VinagrePluginInfo   *info)
{
	//VinagrePluginLoaderC *cloader = VINAGRE_PLUGIN_LOADER_C (loader);
	
	/* this is a no-op, since the type module will be properly unused as
	   the last reference to the plugin dies. When the plugin is activated
	   again, the library will be reloaded */
}

static void
vinagre_plugin_loader_iface_init (gpointer g_iface, 
				gpointer iface_data)
{
	VinagrePluginLoaderInterface *iface = (VinagrePluginLoaderInterface *)g_iface;
	
	iface->get_id = vinagre_plugin_loader_iface_get_id;
	iface->load = vinagre_plugin_loader_iface_load;
	iface->unload = vinagre_plugin_loader_iface_unload;
}

static void
vinagre_plugin_loader_c_finalize (GObject *object)
{
	VinagrePluginLoaderC *cloader = VINAGRE_PLUGIN_LOADER_C (object);
	GList *infos;
	GList *item;

	/* FIXME: this sanity check it's not efficient. Let's remove it
	 * once we are confident with the code */

	infos = g_hash_table_get_keys (cloader->priv->loaded_plugins);
	
	for (item = infos; item; item = item->next)
	{
		VinagrePluginInfo *info = (VinagrePluginInfo *)item->data;

		if (vinagre_plugin_info_is_active (info))
		{
			g_warning ("There are still C plugins loaded during destruction");
			break;
		}
	}

	g_list_free (infos);	

	g_hash_table_destroy (cloader->priv->loaded_plugins);
	
	G_OBJECT_CLASS (vinagre_plugin_loader_c_parent_class)->finalize (object);
}

static void
vinagre_plugin_loader_c_class_init (VinagrePluginLoaderCClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	
	object_class->finalize = vinagre_plugin_loader_c_finalize;

	g_type_class_add_private (object_class, sizeof (VinagrePluginLoaderCPrivate));
}

static void
vinagre_plugin_loader_c_class_finalize (VinagrePluginLoaderCClass *klass)
{
}

static void
vinagre_plugin_loader_c_init (VinagrePluginLoaderC *self)
{
	self->priv = VINAGRE_PLUGIN_LOADER_C_GET_PRIVATE (self);
	
	/* loaded_plugins maps VinagrePluginInfo to a VinagreObjectModule */
	self->priv->loaded_plugins = g_hash_table_new (g_direct_hash,
						       g_direct_equal);
}

VinagrePluginLoaderC *
vinagre_plugin_loader_c_new ()
{
	GObject *loader = g_object_new (VINAGRE_TYPE_PLUGIN_LOADER_C, NULL);

	return VINAGRE_PLUGIN_LOADER_C (loader);
}
