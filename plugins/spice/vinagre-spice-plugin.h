/*
 * vinagre-spice-plugin.h
 * This file is part of vinagre
 *
 * Copyright (C) 2011 Red Hat, Inc.
 * Author: Marc-Andre Lureau <marcandre.lureau@redhat.com>
 *
 * vinagre-spice-plugin.h is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * vinagre-spice-plugin.h is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __VINAGRE_SPICE_PLUGIN_H__
#define __VINAGRE_SPICE_PLUGIN_H__

#include <glib.h>
#include <glib-object.h>

#include "vinagre/vinagre-static-extension.h"

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define VINAGRE_TYPE_SPICE_PLUGIN                 (vinagre_spice_plugin_get_type ())
#define VINAGRE_SPICE_PLUGIN(o)                   (G_TYPE_CHECK_INSTANCE_CAST ((o), VINAGRE_TYPE_SPICE_PLUGIN, VinagreSpicePlugin))
#define VINAGRE_SPICE_PLUGIN_CLASS(k)             (G_TYPE_CHECK_CLASS_CAST((k), VINAGRE_TYPE_SPICE_PLUGIN, VinagreSpicePluginClass))
#define VINAGRE_IS_SPICE_PLUGIN(o)                (G_TYPE_CHECK_INSTANCE_TYPE ((o), VINAGRE_TYPE_SPICE_PLUGIN))
#define VINAGRE_IS_SPICE_PLUGIN_CLASS(k)          (G_TYPE_CHECK_CLASS_TYPE ((k), VINAGRE_TYPE_SPICE_PLUGIN))
#define VINAGRE_SPICE_PLUGIN_GET_CLASS(o)         (G_TYPE_INSTANCE_GET_CLASS ((o), VINAGRE_TYPE_SPICE_PLUGIN, VinagreSpicePluginClass))

typedef struct _VinagreSpicePlugin	VinagreSpicePlugin;
typedef struct _VinagreSpicePluginClass	VinagreSpicePluginClass;

struct _VinagreSpicePlugin
{
  VinagreStaticExtension parent_instance;
};

struct _VinagreSpicePluginClass
{
  VinagreStaticExtensionClass parent_class;
};

GType vinagre_spice_plugin_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __VINAGRE_SPICE_PLUGIN_H__ */

/* vim: set ts=8: */
