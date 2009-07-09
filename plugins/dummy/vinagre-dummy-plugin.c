/*
 * vinagre-dummy-plugin.c
 * This file is part of vinagre
 *
 * Copyright (C) 2009 Jonh Wendell <wendell@bani.com.br>
 * 
 * vinagre-dummy-plugin.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * vinagre-dummy-plugin.c is distributed in the hope that it will be useful, but
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

#include "vinagre-dummy-plugin.h"

#include <string.h>
#include <glib/gi18n-lib.h>
#include <gmodule.h>

#include <vinagre/vinagre-debug.h>
#include <vinagre/vinagre-utils.h>

#define VINAGRE_DUMMY_PLUGIN_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), VINAGRE_TYPE_DUMMY_PLUGIN, VinagreDummyPluginPrivate))

VINAGRE_PLUGIN_REGISTER_TYPE(VinagreDummyPlugin, vinagre_dummy_plugin)

static void
impl_activate (VinagrePlugin *plugin,
               VinagreWindow *window)
{
  vinagre_debug_message (DEBUG_PLUGINS, "VinagreDummyPlugin Activate");
}

static void
impl_deactivate  (VinagrePlugin *plugin,
                  VinagreWindow *window)
{
  vinagre_debug_message (DEBUG_PLUGINS, "VinagreDummyPlugin Deactivate");
}

static void
impl_update_ui (VinagrePlugin *plugin,
                VinagreWindow *window)
{
  vinagre_debug_message (DEBUG_PLUGINS, "VinagreDummyPlugin Update UI");
}

static GOptionGroup *
impl_get_context_group (VinagrePlugin *plugin)
{
  vinagre_debug_message (DEBUG_PLUGINS, "VinagreDummyPlugin Get Context Group");

  return NULL; //dummy_display_get_option_group ();
}

static void
vinagre_dummy_plugin_init (VinagreDummyPlugin *plugin)
{
  vinagre_debug_message (DEBUG_PLUGINS, "VinagreDummyPlugin initializing");
}

static void
vinagre_dummy_plugin_finalize (GObject *object)
{
  vinagre_debug_message (DEBUG_PLUGINS, "VinagreDummyPlugin finalizing");

  G_OBJECT_CLASS (vinagre_dummy_plugin_parent_class)->finalize (object);
}

static void
vinagre_dummy_plugin_class_init (VinagreDummyPluginClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  VinagrePluginClass *plugin_class = VINAGRE_PLUGIN_CLASS (klass);

  object_class->finalize   = vinagre_dummy_plugin_finalize;

  plugin_class->activate   = impl_activate;
  plugin_class->deactivate = impl_deactivate;
  plugin_class->update_ui  = impl_update_ui;
  plugin_class->get_context_group = impl_get_context_group;
}
/* vim: set ts=8: */
