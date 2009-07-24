/*
 * vinagre-plugins-manager.h
 * This file is part of vinagre
 *
 * Based on gedit plugin system
 * Copyright (C) 2002-2005 Paolo Maggi
 * 
 * vinagre-plugins-manager.h is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * vinagre-plugins-manager.h is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __VINAGRE_PLUGIN_MANAGER_H__
#define __VINAGRE_PLUGIN_MANAGER_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define VINAGRE_TYPE_PLUGIN_MANAGER              (vinagre_plugin_manager_get_type())
#define VINAGRE_PLUGIN_MANAGER(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), VINAGRE_TYPE_PLUGIN_MANAGER, VinagrePluginManager))
#define VINAGRE_PLUGIN_MANAGER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), VINAGRE_TYPE_PLUGIN_MANAGER, VinagrePluginManagerClass))
#define VINAGRE_IS_PLUGIN_MANAGER(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), VINAGRE_TYPE_PLUGIN_MANAGER))
#define VINAGRE_IS_PLUGIN_MANAGER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), VINAGRE_TYPE_PLUGIN_MANAGER))
#define VINAGRE_PLUGIN_MANAGER_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), VINAGRE_TYPE_PLUGIN_MANAGER, VinagrePluginManagerClass))

/* Private structure type */
typedef struct _VinagrePluginManagerPrivate VinagrePluginManagerPrivate;

/*
 * Main object structure
 */
typedef struct _VinagrePluginManager VinagrePluginManager;

struct _VinagrePluginManager 
{
	GtkVBox vbox;

	/*< private > */
	VinagrePluginManagerPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _VinagrePluginManagerClass VinagrePluginManagerClass;

struct _VinagrePluginManagerClass 
{
	GtkVBoxClass parent_class;
};

/*
 * Public methods
 */
GType		 vinagre_plugin_manager_get_type		(void) G_GNUC_CONST;

GtkWidget	*vinagre_plugin_manager_new		(void);
   
G_END_DECLS

#endif  /* __VINAGRE_PLUGIN_MANAGER_H__  */

