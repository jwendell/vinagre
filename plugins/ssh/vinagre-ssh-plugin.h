/*
 * vinagre-ssh-plugin.h
 * This file is part of vinagre
 *
 * Copyright (C) 2009 Jonh Wendell <wendell@bani.com.br>
 * 
 * vinagre-ssh-plugin.h is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * vinagre-ssh-plugin.h is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __VINAGRE_SSH_PLUGIN_H__
#define __VINAGRE_SSH_PLUGIN_H__

#include <glib.h>
#include <glib-object.h>
#include <vinagre/vinagre-plugin.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define VINAGRE_TYPE_SSH_PLUGIN                 (vinagre_ssh_plugin_get_type ())
#define VINAGRE_SSH_PLUGIN(o)                   (G_TYPE_CHECK_INSTANCE_CAST ((o), VINAGRE_TYPE_SSH_PLUGIN, VinagreSshPlugin))
#define VINAGRE_SSH_PLUGIN_CLASS(k)             (G_TYPE_CHECK_CLASS_CAST((k), VINAGRE_TYPE_SSH_PLUGIN, VinagreSshPluginClass))
#define VINAGRE_IS_SSH_PLUGIN(o)                (G_TYPE_CHECK_INSTANCE_TYPE ((o), VINAGRE_TYPE_SSH_PLUGIN))
#define VINAGRE_IS_SSH_PLUGIN_CLASS(k)          (G_TYPE_CHECK_CLASS_TYPE ((k), VINAGRE_TYPE_SSH_PLUGIN))
#define VINAGRE_SSH_PLUGIN_GET_CLASS(o)         (G_TYPE_INSTANCE_GET_CLASS ((o), VINAGRE_TYPE_SSH_PLUGIN, VinagreSshPluginClass))

/* Private structure type */
typedef struct _VinagreSshPluginPrivate	VinagreSshPluginPrivate;

/*
 * Main object structure
 */
typedef struct _VinagreSshPlugin		VinagreSshPlugin;

struct _VinagreSshPlugin
{
  VinagrePlugin parent_instance;
};

/*
 * Class definition
 */
typedef struct _VinagreSshPluginClass	VinagreSshPluginClass;

struct _VinagreSshPluginClass
{
  VinagrePluginClass parent_class;
};

/*
 * Public methods
 */
GType	vinagre_ssh_plugin_get_type		(void) G_GNUC_CONST;

/* All the plugins must implement this function */
G_MODULE_EXPORT GType register_vinagre_plugin (GTypeModule *module);

G_END_DECLS

#endif /* __VINAGRE_SSH_PLUGIN_H__ */
/* vim: set ts=8: */
