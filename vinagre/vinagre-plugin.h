/*
 * vinagre-plugin.h
 * This file is part of vinagre
 *
 * Copyright (C) 2009 Jorge Pereira <jorge@jorgepereira.com.br>
 * 
 * vinagre-plugin.h is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * vinagre-plugin.h is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __VINAGRE_PLUGIN_H__
#define __VINAGRE_PLUGIN_H__

#include <glib-object.h>

#include <vinagre/vinagre-window.h>
#include <vinagre/vinagre-debug.h>

/* TODO: add a .h file that includes all the .h files normally needed to
 * develop a plugin */ 

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define VINAGRE_TYPE_PLUGIN              (vinagre_plugin_get_type())
#define VINAGRE_PLUGIN(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), VINAGRE_TYPE_PLUGIN, VinagrePlugin))
#define VINAGRE_PLUGIN_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), VINAGRE_TYPE_PLUGIN, VinagrePluginClass))
#define VINAGRE_IS_PLUGIN(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), VINAGRE_TYPE_PLUGIN))
#define VINAGRE_IS_PLUGIN_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), VINAGRE_TYPE_PLUGIN))
#define VINAGRE_PLUGIN_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), VINAGRE_TYPE_PLUGIN, VinagrePluginClass))

/*
 * Main object structure
 */
typedef struct _VinagrePlugin VinagrePlugin;

struct _VinagrePlugin 
{
	GObject parent;
};

/*
 * Class definition
 */
typedef struct _VinagrePluginClass VinagrePluginClass;

struct _VinagrePluginClass 
{
	GObjectClass parent_class;

	/* Virtual public methods */
	
	void 		(*activate)		(VinagrePlugin *plugin,
						 VinagreWindow *window);
	void 		(*deactivate)		(VinagrePlugin *plugin,
						 VinagreWindow *window);

	void 		(*update_ui)		(VinagrePlugin *plugin,
						 VinagreWindow *window);

	GtkWidget 	*(*create_configure_dialog)
						(VinagrePlugin *plugin);

	/* Plugins should not override this, it's handled automatically by
	   the VinagrePluginClass */
	gboolean 	(*is_configurable)
						(VinagrePlugin *plugin);

	/* Padding for future expansion */
	void		(*_vinagre_reserved1)	(void);
	void		(*_vinagre_reserved2)	(void);
	void		(*_vinagre_reserved3)	(void);
	void		(*_vinagre_reserved4)	(void);
};

/*
 * Public methods
 */
GType 		 vinagre_plugin_get_type 		(void) G_GNUC_CONST;

gchar 		*vinagre_plugin_get_install_dir	(VinagrePlugin *plugin);
gchar 		*vinagre_plugin_get_data_dir	(VinagrePlugin *plugin);

void 		 vinagre_plugin_activate		(VinagrePlugin *plugin,
						 VinagreWindow *window);
void 		 vinagre_plugin_deactivate	(VinagrePlugin *plugin,
						 VinagreWindow *window);
				 
void 		 vinagre_plugin_update_ui		(VinagrePlugin *plugin,
						 VinagreWindow *window);

gboolean	 vinagre_plugin_is_configurable	(VinagrePlugin *plugin);
GtkWidget	*vinagre_plugin_create_configure_dialog		
						(VinagrePlugin *plugin);

/**
 * VINAGRE_PLUGIN_REGISTER_TYPE_WITH_CODE(PluginName, plugin_name, CODE):
 *
 * Utility macro used to register plugins with additional code.
 */
#define VINAGRE_PLUGIN_REGISTER_TYPE_WITH_CODE(PluginName, plugin_name, CODE) 	\
	G_DEFINE_DYNAMIC_TYPE_EXTENDED (PluginName,				\
					plugin_name,				\
					VINAGRE_TYPE_PLUGIN,			\
					0,					\
					GTypeModule *module G_GNUC_UNUSED = type_module; /* back compat */	\
					CODE)					\
										\
/* This is not very nice, but G_DEFINE_DYNAMIC wants it and our old macro	\
 * did not support it */							\
static void									\
plugin_name##_class_finalize (PluginName##Class *klass)				\
{										\
}										\
										\
										\
G_MODULE_EXPORT GType								\
register_vinagre_plugin (GTypeModule *type_module)				\
{										\
	plugin_name##_register_type (type_module);				\
										\
	return plugin_name##_get_type();					\
}

/**
 * VINAGRE_PLUGIN_REGISTER_TYPE(PluginName, plugin_name):
 * 
 * Utility macro used to register plugins.
 */
#define VINAGRE_PLUGIN_REGISTER_TYPE(PluginName, plugin_name)			\
	VINAGRE_PLUGIN_REGISTER_TYPE_WITH_CODE(PluginName, plugin_name, ;)

/**
 * VINAGRE_PLUGIN_DEFINE_TYPE_WITH_CODE(ObjectName, object_name, PARENT_TYPE, CODE):
 *
 * Utility macro used to register gobject types in plugins with additional code.
 *
 * Deprecated: use G_DEFINE_DYNAMIC_TYPE_EXTENDED instead
 */
#define VINAGRE_PLUGIN_DEFINE_TYPE_WITH_CODE(ObjectName, object_name, PARENT_TYPE, CODE) \
										\
static GType g_define_type_id = 0;						\
										\
GType										\
object_name##_get_type (void)							\
{										\
	return g_define_type_id;						\
}										\
										\
static void     object_name##_init              (ObjectName        *self);	\
static void     object_name##_class_init        (ObjectName##Class *klass);	\
static gpointer object_name##_parent_class = NULL;				\
static void     object_name##_class_intern_init (gpointer klass)		\
{										\
	object_name##_parent_class = g_type_class_peek_parent (klass);		\
	object_name##_class_init ((ObjectName##Class *) klass);			\
}										\
										\
GType										\
object_name##_register_type (GTypeModule *type_module)				\
{										\
	GTypeModule *module G_GNUC_UNUSED = type_module; /* back compat */			\
	static const GTypeInfo our_info =					\
	{									\
		sizeof (ObjectName##Class),					\
		NULL, /* base_init */						\
		NULL, /* base_finalize */					\
		(GClassInitFunc) object_name##_class_intern_init,		\
		NULL,								\
		NULL, /* class_data */						\
		sizeof (ObjectName),						\
		0, /* n_preallocs */						\
		(GInstanceInitFunc) object_name##_init				\
	};									\
										\
	g_define_type_id = g_type_module_register_type (type_module,		\
					   	        PARENT_TYPE,		\
					                #ObjectName,		\
					                &our_info,		\
					                0);			\
										\
	CODE									\
										\
	return g_define_type_id;						\
}


/**
 * VINAGRE_PLUGIN_DEFINE_TYPE(ObjectName, object_name, PARENT_TYPE):
 *
 * Utility macro used to register gobject types in plugins.
 *
 * Deprecated: use G_DEFINE_DYNAMIC instead
 */
#define VINAGRE_PLUGIN_DEFINE_TYPE(ObjectName, object_name, PARENT_TYPE)		\
	VINAGRE_PLUGIN_DEFINE_TYPE_WITH_CODE(ObjectName, object_name, PARENT_TYPE, ;)

/**
 * VINAGRE_PLUGIN_IMPLEMENT_INTERFACE(TYPE_IFACE, iface_init):
 *
 * Utility macro used to register interfaces for gobject types in plugins.
 */
#define VINAGRE_PLUGIN_IMPLEMENT_INTERFACE(object_name, TYPE_IFACE, iface_init)	\
	const GInterfaceInfo object_name##_interface_info = 			\
	{ 									\
		(GInterfaceInitFunc) iface_init,				\
		NULL, 								\
		NULL								\
	};									\
										\
	g_type_module_add_interface (type_module, 					\
				     g_define_type_id, 				\
				     TYPE_IFACE, 				\
				     &object_name##_interface_info);

G_END_DECLS

#endif  /* __VINAGRE_PLUGIN_H__ */
/* vim: set ts=8: */
