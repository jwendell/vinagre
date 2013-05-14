/*
 * vinagre-rdp-connection.c
 * Child class of abstract VinagreConnection, specific to RDP protocol
 * This file is part of vinagre
 *
 * Copyright (C) 2010 - Jonh Wendell <wendell@bani.com.br>
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib/gi18n.h>
#include <vinagre/vinagre-cache-prefs.h>
#include "vinagre-rdp-connection.h"

struct _VinagreRdpConnectionPrivate
{
  gint dummy;
};

#define VINAGRE_RDP_CONNECTION_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), VINAGRE_TYPE_RDP_CONNECTION, VinagreRdpConnectionPrivate))
G_DEFINE_TYPE (VinagreRdpConnection, vinagre_rdp_connection, VINAGRE_TYPE_CONNECTION);

static void
vinagre_rdp_connection_init (VinagreRdpConnection *conn)
{
  conn->priv = G_TYPE_INSTANCE_GET_PRIVATE (conn, VINAGRE_TYPE_RDP_CONNECTION, VinagreRdpConnectionPrivate);
}

static void
vinagre_rdp_connection_constructed (GObject *object)
{
  vinagre_connection_set_protocol (VINAGRE_CONNECTION (object), "rdp");
}

static void
rdp_fill_writer (VinagreConnection *conn, xmlTextWriter *writer)
{
  VINAGRE_CONNECTION_CLASS (vinagre_rdp_connection_parent_class)->impl_fill_writer (conn, writer);
}

static void
rdp_parse_item (VinagreConnection *conn, xmlNode *root)
{
  VINAGRE_CONNECTION_CLASS (vinagre_rdp_connection_parent_class)->impl_parse_item (conn, root);
}

static void
rdp_parse_options_widget (VinagreConnection *conn, GtkWidget *widget)
{
  GtkWidget *u_entry, *spin_button;
  guint      width, height;

  u_entry = g_object_get_data (G_OBJECT (widget), "username_entry");
  if (!u_entry)
    {
      g_warning ("Wrong widget passed to rdp_parse_options_widget()");
      return;
    }

  vinagre_cache_prefs_set_string  ("rdp-connection", "username", gtk_entry_get_text (GTK_ENTRY (u_entry)));

  g_object_set (conn,
		"username", gtk_entry_get_text (GTK_ENTRY (u_entry)),
		NULL);


  spin_button = g_object_get_data (G_OBJECT (widget), "width_spin_button");
  if (!spin_button)
    {
      g_warning ("Wrong widget passed to rdp_parse_options_widget()");
      return;
    }

  width = (guint) gtk_spin_button_get_value (GTK_SPIN_BUTTON (spin_button));

  vinagre_cache_prefs_set_integer  ("rdp-connection", "width", width);

  vinagre_connection_set_width (conn, width);


  spin_button = g_object_get_data (G_OBJECT (widget), "height_spin_button");
  if (!spin_button)
    {
      g_warning ("Wrong widget passed to rdp_parse_options_widget()");
      return;
    }

  height = (guint) gtk_spin_button_get_value (GTK_SPIN_BUTTON (spin_button));

  vinagre_cache_prefs_set_integer  ("rdp-connection", "height", height);

  vinagre_connection_set_height (conn, height);
}

static void
vinagre_rdp_connection_class_init (VinagreRdpConnectionClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);
  VinagreConnectionClass* parent_class = VINAGRE_CONNECTION_CLASS (klass);

  g_type_class_add_private (klass, sizeof (VinagreRdpConnectionPrivate));

  object_class->constructed  = vinagre_rdp_connection_constructed;

  parent_class->impl_fill_writer = rdp_fill_writer;
  parent_class->impl_parse_item  = rdp_parse_item;
  parent_class->impl_parse_options_widget = rdp_parse_options_widget;
}

VinagreConnection *
vinagre_rdp_connection_new (void)
{
  return VINAGRE_CONNECTION (g_object_new (VINAGRE_TYPE_RDP_CONNECTION, NULL));
}

/* vim: set ts=8: */
