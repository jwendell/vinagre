/*
 * vinagre-dummy-plugin.h
 * This file is part of vinagre
 *
 * Copyright (C) 2009 Jorge Pereira <jorge@jorgepereira.com.br>
 * 
 * vinagre-dummy-plugin.h is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * vinagre-dummy-plugin.h is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __VINAGRE_SORT_PLUGIN_H__
#define __VINAGRE_SORT_PLUGIN_H__

#include <glib.h>
#include <glib-object.h>
#include <vinagre/vinagre-plugin.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define VINAGRE_TYPE_SORT_PLUGIN		(vinagre_dummy_plugin_get_type ())
#define VINAGRE_SORT_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), VINAGRE_TYPE_SORT_PLUGIN, VinagreDummyPlugin))
#define VINAGRE_SORT_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), VINAGRE_TYPE_SORT_PLUGIN, VinagreDummyPluginClass))
#define VINAGRE_IS_SORT_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), VINAGRE_TYPE_SORT_PLUGIN))
#define VINAGRE_IS_SORT_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), VINAGRE_TYPE_SORT_PLUGIN))
#define VINAGRE_SORT_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), VINAGRE_TYPE_SORT_PLUGIN, VinagreDummyPluginClass))

/* Private structure type */
typedef struct _VinagreDummyPluginPrivate	VinagreDummyPluginPrivate;

/*
 * Main object structure
 */
typedef struct _VinagreDummyPlugin		VinagreDummyPlugin;

struct _VinagreDummyPlugin
{
	VinagrePlugin parent_instance;
};

/*
 * Class definition
 */
typedef struct _VinagreDummyPluginClass	VinagreDummyPluginClass;

struct _VinagreDummyPluginClass
{
	VinagrePluginClass parent_class;
};

/*
 * Public methods
 */
GType	vinagre_dummy_plugin_get_type		(void) G_GNUC_CONST;

/* All the plugins must implement this function */
G_MODULE_EXPORT GType register_vinagre_plugin (GTypeModule *module);

G_END_DECLS

#endif /* __VINAGRE_SORT_PLUGIN_H__ */
