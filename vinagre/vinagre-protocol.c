/*
 * vinagre-protocol.c
 * This file is part of vinagre
 *
 * Copyright (C) 2010 Jonh Wendell <wendell@bani.com.br>
 * 
 * vinagre-protocol.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * vinagre-protocol.h is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "vinagre-protocol.h"

G_DEFINE_INTERFACE (VinagreProtocol, vinagre_protocol, G_TYPE_OBJECT)

static const gchar *
dummy (VinagreProtocol *protocol)
{
  return NULL;
}

static gchar **
default_get_public_description (VinagreProtocol *protocol)
{
  return NULL;
}

static gint
default_get_default_port (VinagreProtocol *protocol)
{
  return 0;
}

static GSList *
default_get_context_groups (VinagreProtocol *protocol)
{
  return NULL;
}

static GtkFileFilter *
default_get_file_filter (VinagreProtocol *protocol)
{
  return NULL;
}

static GtkWidget *
default_new_tab (VinagreProtocol   *protocol,
		 VinagreConnection *conn,
		 VinagreWindow     *window)
{
  return NULL;
}

static VinagreConnection *
default_new_connection (VinagreProtocol *protocol)
{
  return NULL;
}

static VinagreConnection *
default_new_connection_from_file (VinagreProtocol *protocol,
				  const gchar     *data,
				  gboolean         use_bookmarks,
				  gchar           **error_msg)
{
  return NULL;
}

static gboolean
default_recognize_file (VinagreProtocol *protocol,
                        GFile           *file)
{
  return FALSE;
}

static GtkWidget *
default_get_connect_widget (VinagreProtocol   *protocol,
			    VinagreConnection *initial_settings)
{
  return NULL;
}

static void
default_parse_mdns_dialog (VinagreProtocol *protocol,
			   GtkWidget       *connect_widget,
			   GtkWidget       *dialog)
{
}

static GdkPixbuf *
default_get_icon (VinagreProtocol *protocol,
		  gint             size)
{
  return NULL;
}

void
vinagre_protocol_default_init (VinagreProtocolInterface *iface)
{
  iface->get_protocol = dummy;
  iface->get_public_description = default_get_public_description;
  iface->get_default_port = default_get_default_port;
  iface->get_mdns_service = dummy;
  iface->get_context_groups = default_get_context_groups;
  iface->get_file_filter = default_get_file_filter;
  iface->new_tab = default_new_tab;
  iface->new_connection = default_new_connection;
  iface->new_connection_from_file = default_new_connection_from_file;
  iface->recognize_file = default_recognize_file;
  iface->get_connect_widget = default_get_connect_widget;
  iface->parse_mdns_dialog = default_parse_mdns_dialog;
  iface->get_icon_name = dummy;
  iface->get_icon = default_get_icon;
}

const gchar *
vinagre_protocol_get_protocol (VinagreProtocol *protocol)
{
  VinagreProtocolInterface *iface;

  g_return_val_if_fail (VINAGRE_IS_PROTOCOL (protocol), NULL);

  iface = VINAGRE_PROTOCOL_GET_IFACE (protocol);

  if (iface->get_protocol != NULL)
    {
      return iface->get_protocol (protocol);
    }

  return NULL;
}

/**
 * vinagre_protocol_get_public_description:
 * @protocol: A protocol
 *
 * Return value: (array zero-terminated=1) (element-type utf8) (transfer full):
 */
gchar **
vinagre_protocol_get_public_description (VinagreProtocol *protocol)
{
  VinagreProtocolInterface *iface;

  g_return_val_if_fail (VINAGRE_IS_PROTOCOL (protocol), NULL);

  iface = VINAGRE_PROTOCOL_GET_IFACE (protocol);

  if (iface->get_public_description != NULL)
    {
      return iface->get_public_description (protocol);
    }

  return NULL;
}

gint
vinagre_protocol_get_default_port (VinagreProtocol *protocol)
{
  VinagreProtocolInterface *iface;

  g_return_val_if_fail (VINAGRE_IS_PROTOCOL (protocol), 0);

  iface = VINAGRE_PROTOCOL_GET_IFACE (protocol);

  if (iface->get_default_port != NULL)
    {
      return iface->get_default_port (protocol);
    }

  return 0;
}

const gchar *
vinagre_protocol_get_mdns_service (VinagreProtocol *protocol)
{
  VinagreProtocolInterface *iface;

  g_return_val_if_fail (VINAGRE_IS_PROTOCOL (protocol), NULL);

  iface = VINAGRE_PROTOCOL_GET_IFACE (protocol);

  if (iface->get_mdns_service != NULL)
    {
      return iface->get_mdns_service (protocol);
    }

  return NULL;
}

/**
 * vinagre_protocol_get_context_groups:
 * @protocol: A protocol
 *
 * Return value: (allow-none) (element-type any) (transfer container):
 */
GSList *
vinagre_protocol_get_context_groups (VinagreProtocol *protocol)
{
  VinagreProtocolInterface *iface;

  g_return_val_if_fail (VINAGRE_IS_PROTOCOL (protocol), NULL);

  iface = VINAGRE_PROTOCOL_GET_IFACE (protocol);

  if (iface->get_context_groups != NULL)
    {
      return iface->get_context_groups (protocol);
    }

  return NULL;
}

/**
 * vinagre_protocol_get_file_filter:
 * @protocol: A protocol
 *
 * Return value: (allow-none) (transfer full):
 */
GtkFileFilter *
vinagre_protocol_get_file_filter (VinagreProtocol *protocol)
{
  VinagreProtocolInterface *iface;

  g_return_val_if_fail (VINAGRE_IS_PROTOCOL (protocol), NULL);

  iface = VINAGRE_PROTOCOL_GET_IFACE (protocol);

  if (iface->get_file_filter != NULL)
    {
      return iface->get_file_filter (protocol);
    }

  return NULL;
}

/**
 * vinagre_protocol_new_tab:
 * @protocol: A protocol
 *
 * Return value: (allow-none) (transfer full):
 */
GtkWidget *
vinagre_protocol_new_tab (VinagreProtocol   *protocol,
			  VinagreConnection *conn,
			  VinagreWindow     *window)
{
  VinagreProtocolInterface *iface;

  g_return_val_if_fail (VINAGRE_IS_PROTOCOL (protocol), NULL);

  iface = VINAGRE_PROTOCOL_GET_IFACE (protocol);

  if (iface->new_tab != NULL)
    {
      return iface->new_tab (protocol, conn, window);
    }

  return NULL;
}

/**
 * vinagre_protocol_new_connection:
 * @protocol: A protocol
 *
 * Return value: (allow-none) (transfer full):
 */
VinagreConnection *
vinagre_protocol_new_connection (VinagreProtocol *protocol)
{
  VinagreProtocolInterface *iface;

  g_return_val_if_fail (VINAGRE_IS_PROTOCOL (protocol), NULL);

  iface = VINAGRE_PROTOCOL_GET_IFACE (protocol);

  if (iface->new_connection != NULL)
    {
      return iface->new_connection (protocol);
    }

  return NULL;
}

/**
 * vinagre_protocol_new_connection_from_file:
 * @protocol: A protocol
 *
 * Return value: (allow-none) (transfer full):
 */
VinagreConnection *
vinagre_protocol_new_connection_from_file (VinagreProtocol *protocol,
					   const gchar     *data,
					   gboolean         use_bookmarks,
					   gchar           **error_msg)
{
  VinagreProtocolInterface *iface;

  g_return_val_if_fail (VINAGRE_IS_PROTOCOL (protocol), NULL);

  iface = VINAGRE_PROTOCOL_GET_IFACE (protocol);

  if (iface->new_connection_from_file != NULL)
    {
      return iface->new_connection_from_file (protocol, data, use_bookmarks, error_msg);
    }

  return NULL;
}

gboolean
vinagre_protocol_recognize_file (VinagreProtocol *protocol,
                                 GFile           *file)
{
  VinagreProtocolInterface *iface;

  g_return_val_if_fail (VINAGRE_IS_PROTOCOL (protocol), FALSE);

  iface = VINAGRE_PROTOCOL_GET_IFACE (protocol);

  if (iface->recognize_file != NULL)
    {
      return iface->recognize_file (protocol, file);
    }

  return FALSE;
}

/**
 * vinagre_protocol_get_connect_widget:
 *
 * @protocol: a protocol
 * @initial_settings: (allow-none): bla bla
 * Return value: (allow-none) (transfer full): a widget
 */
GtkWidget *
vinagre_protocol_get_connect_widget (VinagreProtocol   *protocol,
				     VinagreConnection *initial_settings)
{
  VinagreProtocolInterface *iface;

  g_return_val_if_fail (VINAGRE_IS_PROTOCOL (protocol), NULL);

  iface = VINAGRE_PROTOCOL_GET_IFACE (protocol);

  if (iface->get_connect_widget != NULL)
    {
      return iface->get_connect_widget (protocol, initial_settings);
    }

  return NULL;
}

void
vinagre_protocol_parse_mdns_dialog (VinagreProtocol *protocol,
				    GtkWidget       *connect_widget,
				    GtkWidget       *dialog)
{
  VinagreProtocolInterface *iface;

  g_return_if_fail (VINAGRE_IS_PROTOCOL (protocol));

  iface = VINAGRE_PROTOCOL_GET_IFACE (protocol);

  if (iface->parse_mdns_dialog != NULL)
    {
      iface->parse_mdns_dialog (protocol, connect_widget, dialog);
    }
}

const gchar *
vinagre_protocol_get_icon_name (VinagreProtocol *protocol)
{
  VinagreProtocolInterface *iface;

  g_return_val_if_fail (VINAGRE_IS_PROTOCOL (protocol), NULL);

  iface = VINAGRE_PROTOCOL_GET_IFACE (protocol);

  if (iface->get_icon_name != NULL)
    {
      return iface->get_icon_name (protocol);
    }

  return NULL;
}

/**
 * vinagre_protocol_get_icon:
 * @protocol: A protocol
 *
 * Return value: (allow-none) (transfer full):
 */
GdkPixbuf *
vinagre_protocol_get_icon (VinagreProtocol *protocol,
			   gint             size)
{
  VinagreProtocolInterface *iface;

  g_return_val_if_fail (VINAGRE_IS_PROTOCOL (protocol), NULL);

  iface = VINAGRE_PROTOCOL_GET_IFACE (protocol);

  if (iface->get_icon != NULL)
    {
      return iface->get_icon (protocol, size);
    }

  return NULL;
}

/* vim: set ts=8: */
