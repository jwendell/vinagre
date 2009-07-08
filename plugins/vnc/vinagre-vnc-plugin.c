/*
 * vinagre-vnc-plugin.c
 * This file is part of vinagre
 *
 * Copyright (C) 2009 Jonh Wendell <wendell@bani.com.br>
 * 
 * vinagre-vnc-plugin.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * vinagre-vnc-plugin.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "vinagre-vnc-plugin.h"

#include <string.h>
#include <glib/gi18n-lib.h>
#include <gmodule.h>
#include <vncdisplay.h>

#include <vinagre/vinagre-debug.h>
#include <vinagre/vinagre-utils.h>

#define VINAGRE_SORT_PLUGIN_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), VINAGRE_TYPE_SORT_PLUGIN, VinagreVncPluginPrivate))

VINAGRE_PLUGIN_REGISTER_TYPE(VinagreVncPlugin, vinagre_vnc_plugin)

static void
impl_activate (VinagrePlugin *plugin,
               VinagreWindow *window)
{
  vinagre_debug_message (DEBUG_PLUGINS, "VinagreVncPlugin Activate");
}

static void
impl_deactivate  (VinagrePlugin *plugin,
                  VinagreWindow *window)
{
  vinagre_debug_message (DEBUG_PLUGINS, "VinagreVncPlugin Deactivate");
}

static void
impl_update_ui (VinagrePlugin *plugin,
                VinagreWindow *window)
{
  vinagre_debug_message (DEBUG_PLUGINS, "VinagreVncPlugin Update UI");
}

static GOptionGroup *
impl_get_context_group (VinagrePlugin *plugin)
{
  vinagre_debug_message (DEBUG_PLUGINS, "VinagreVncPlugin Get Context Group");

  return vnc_display_get_option_group ();
}

static void
vinagre_vnc_plugin_init (VinagreVncPlugin *plugin)
{
  vinagre_debug_message (DEBUG_PLUGINS, "VinagreVncPlugin initializing");
}

static void
vinagre_vnc_plugin_finalize (GObject *object)
{
  vinagre_debug_message (DEBUG_PLUGINS, "VinagreVncPlugin finalizing");

  G_OBJECT_CLASS (vinagre_vnc_plugin_parent_class)->finalize (object);
}

static void
vinagre_vnc_plugin_class_init (VinagreVncPluginClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  VinagrePluginClass *plugin_class = VINAGRE_PLUGIN_CLASS (klass);

  object_class->finalize   = vinagre_vnc_plugin_finalize;

  plugin_class->activate   = impl_activate;
  plugin_class->deactivate = impl_deactivate;
  plugin_class->update_ui  = impl_update_ui;
  plugin_class->get_context_group = impl_get_context_group;
}
/* vim: set ts=8: */
