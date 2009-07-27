/*
 * vinagre-dummy-plugin.c
 * This file is part of vinagre
 *
 * Copyright (C) 2009 Jorge Pereira <jorge@jorgepereira.com.br>
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

#include <string.h>
#include <glib/gi18n-lib.h>
#include <gmodule.h>

#include <vinagre/vinagre-debug.h>
#include <vinagre/vinagre-utils.h>

#include "vinagre-dummy-plugin.h"

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

/**
 * If your plugin have OptionGroup, should be done here!
 */
static GOptionGroup *
impl_get_context_group (VinagrePlugin *plugin)
{
  vinagre_debug_message (DEBUG_PLUGINS, "VinagreDummyPlugin Get Context Group");

  return NULL;
}

/**
 * If plugin handler some protocol, put the name here!
 */
static const gchar *
impl_get_protocol (VinagrePlugin *plugin)
{
  return "dummy";
}

/**
 * This a description of your plugin
 */
static gchar **
impl_get_public_description (VinagrePlugin *plugin)
{
  gchar **result = g_new (gchar *, 3);

  result[0] = g_strdup ("DUMMY");
  result[1] = g_strdup ("The dummy example of plugin.");
  result[2] = NULL;

  return result;
}

/**
 * This a name for search at mdns (avahi)
 */
static const gchar *
impl_get_mdns_service (VinagrePlugin *plugin)
{
  return "_dummy._tcp";
}

static VinagreConnection *
impl_new_connection (VinagrePlugin *plugin)
{
  return NULL;
}

/**
 * You maybe can receive the new connection from file, eg: note-toing.dummy
 */
static VinagreConnection *
impl_new_connection_from_file (VinagrePlugin *plugin,
			       const gchar   *data,
			       gboolean       use_bookmarks,
			       gchar        **error_msg)
{
  /* All rule for connection from file config */
  return NULL;
}

/**
 * Called always that create a newtab with your plugin.
 */
static GtkWidget *
impl_new_tab (VinagrePlugin *plugin,
	      VinagreConnection *conn,
	      VinagreWindow     *window)
{
  /* All rule create the tab */
  return NULL;
}

/**
 * This is the way to get all information for your plugin!
 */
static GtkWidget *
impl_get_connect_widget (VinagrePlugin *plugin, 
                         VinagreConnection *conn)
{
  GtkWidget *box, *check, *label;
  GtkTable  *table;

  box = gtk_vbox_new (TRUE, 0);

  label = gtk_label_new (_("<b>DUMMY Options</b>"));
  gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 0);

  table = GTK_TABLE (gtk_table_new (2, 2, FALSE));
  label = gtk_label_new ("  ");
  gtk_table_attach (table, label, 0, 1, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);

  check = gtk_check_button_new_with_mnemonic ("_Mastering the world");
  g_object_set_data (G_OBJECT (box), "view_only", check);
  gtk_table_attach_defaults (table, check, 1, 2, 0, 1);

  gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (table), TRUE, TRUE, 0);
  return box;
}

/**
 * The function is called when
 */
static gint
impl_get_default_port (VinagrePlugin *plugin)
{
  return -1;
}

/**
 * The function is called when
 */
static GtkFileFilter *
impl_get_file_filter (VinagrePlugin *plugin)
{
  GtkFileFilter *filter;

  filter = gtk_file_filter_new ();
  /* Translators: this is a pattern to open *.dummy files in a open dialog. */
  gtk_file_filter_set_name (filter, _("DUMMY Files"));
  gtk_file_filter_add_pattern (filter, "*.dummy");

  return filter;
}

/**
 * The function is called when
 */
static void
vinagre_dummy_plugin_init (VinagreDummyPlugin *plugin)
{
  vinagre_debug_message (DEBUG_PLUGINS, "VinagreDummyPlugin initializing");
}

/**
 * The function is called when
 */
static void
vinagre_dummy_plugin_finalize (GObject *object)
{
  vinagre_debug_message (DEBUG_PLUGINS, "VinagreDummyPlugin finalizing");

  G_OBJECT_CLASS (vinagre_dummy_plugin_parent_class)->finalize (object);
}

/**
 * The function is called when
 */
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
  plugin_class->get_protocol  = impl_get_protocol;
  plugin_class->get_public_description  = impl_get_public_description;
  plugin_class->new_connection = impl_new_connection;
  plugin_class->new_connection_from_file = impl_new_connection_from_file;
  plugin_class->get_mdns_service  = impl_get_mdns_service;
  plugin_class->new_tab = impl_new_tab;
  plugin_class->get_connect_widget = impl_get_connect_widget;
  plugin_class->get_default_port = impl_get_default_port;
  plugin_class->get_file_filter = impl_get_file_filter;
}
/* vim: set ts=8: */
