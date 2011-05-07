/*
 * vinagre-spice-connection.c
 * Child class of abstract VinagreConnection, specific to SPICE protocol
 * This file is part of vinagre
 *
 * Copyright (C) 2011 Red Hat, Inc.
 * Author: Marc-Andre Lureau <marcandre.lureau@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib/gi18n.h>
#include <stdlib.h>
#include "vinagre/vinagre-util.h"
#include <vinagre/vinagre-cache-prefs.h>

#include "vinagre-spice-connection.h"

struct _VinagreSpiceConnectionPrivate
{
  gchar	   *desktop_name;
  gboolean view_only;
  gboolean scaling;
  gboolean resize_guest;
  gboolean auto_clipboard;
  gint	   fd;
  gchar	   *ssh_tunnel_host;
  GSocket  *socket;
};

enum
{
  PROP_0,
  PROP_DESKTOP_NAME,
  PROP_VIEW_ONLY,
  PROP_SCALING,
  PROP_RESIZE_GUEST,
  PROP_AUTO_CLIPBOARD,
  PROP_FD,
  PROP_SSH_TUNNEL_HOST,
  PROP_SOCKET
};

#define VINAGRE_SPICE_CONNECTION_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), VINAGRE_TYPE_SPICE_CONNECTION, VinagreSpiceConnectionPrivate))
G_DEFINE_TYPE (VinagreSpiceConnection, vinagre_spice_connection, VINAGRE_TYPE_CONNECTION);

static void
vinagre_spice_connection_init (VinagreSpiceConnection *conn)
{
  conn->priv = G_TYPE_INSTANCE_GET_PRIVATE (conn, VINAGRE_TYPE_SPICE_CONNECTION, VinagreSpiceConnectionPrivate);

  conn->priv->desktop_name = NULL;
  conn->priv->view_only = FALSE;
  conn->priv->scaling = FALSE;
  conn->priv->resize_guest = TRUE;
  conn->priv->auto_clipboard = TRUE;
  conn->priv->fd = 0;
  conn->priv->ssh_tunnel_host = NULL;
  conn->priv->socket = NULL;
}

static void
vinagre_spice_connection_constructed (GObject *object)
{
  vinagre_connection_set_protocol (VINAGRE_CONNECTION (object), "spice");
}

static void
vinagre_spice_connection_finalize (GObject *object)
{
  VinagreSpiceConnection *conn = VINAGRE_SPICE_CONNECTION (object);

  g_free (conn->priv->desktop_name);
  g_free (conn->priv->ssh_tunnel_host);

  G_OBJECT_CLASS (vinagre_spice_connection_parent_class)->finalize (object);
}

static void
vinagre_spice_connection_dispose (GObject *object)
{
  VinagreSpiceConnection *conn = VINAGRE_SPICE_CONNECTION (object);

  if (conn->priv->socket)
    {
      g_object_unref (conn->priv->socket);
      conn->priv->socket = NULL;
    }

  G_OBJECT_CLASS (vinagre_spice_connection_parent_class)->dispose (object);
}

static void
vinagre_spice_connection_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
  VinagreSpiceConnection *conn;

  g_return_if_fail (VINAGRE_IS_SPICE_CONNECTION (object));

  conn = VINAGRE_SPICE_CONNECTION (object);

  switch (prop_id)
    {
    case PROP_DESKTOP_NAME:
      vinagre_spice_connection_set_desktop_name (conn, g_value_get_string (value));
      break;

    case PROP_VIEW_ONLY:
      vinagre_spice_connection_set_view_only (conn, g_value_get_boolean (value));
      break;

    case PROP_SCALING:
      vinagre_spice_connection_set_scaling (conn, g_value_get_boolean (value));
      break;

    case PROP_RESIZE_GUEST:
      vinagre_spice_connection_set_resize_guest (conn, g_value_get_boolean (value));
      break;

    case PROP_AUTO_CLIPBOARD:
      vinagre_spice_connection_set_auto_clipboard (conn, g_value_get_boolean (value));
      break;

    case PROP_SSH_TUNNEL_HOST:
      vinagre_spice_connection_set_ssh_tunnel_host (conn, g_value_get_string (value));
      break;

    case PROP_FD:
      vinagre_spice_connection_set_fd (conn, g_value_get_int (value));
      break;

    case PROP_SOCKET:
      vinagre_spice_connection_set_socket (conn, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
vinagre_spice_connection_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
  VinagreSpiceConnection *conn;

  g_return_if_fail (VINAGRE_IS_SPICE_CONNECTION (object));

  conn = VINAGRE_SPICE_CONNECTION (object);

  switch (prop_id)
    {
    case PROP_DESKTOP_NAME:
      g_value_set_string (value, conn->priv->desktop_name);
      break;

    case PROP_VIEW_ONLY:
      g_value_set_boolean (value, conn->priv->view_only);
      break;

    case PROP_SCALING:
      g_value_set_boolean (value, conn->priv->scaling);
      break;

    case PROP_RESIZE_GUEST:
      g_value_set_boolean (value, conn->priv->resize_guest);
      break;

    case PROP_AUTO_CLIPBOARD:
      g_value_set_boolean (value, conn->priv->auto_clipboard);
      break;

    case PROP_SSH_TUNNEL_HOST:
      g_value_set_string (value, conn->priv->ssh_tunnel_host);
      break;

    case PROP_FD:
      g_value_set_int (value, vinagre_spice_connection_get_fd (conn));
      break;

    case PROP_SOCKET:
      g_value_set_object (value, conn->priv->socket);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
spice_fill_writer (VinagreConnection *conn, xmlTextWriter *writer)
{
  VinagreSpiceConnection *spice_conn = VINAGRE_SPICE_CONNECTION (conn);
  VINAGRE_CONNECTION_CLASS (vinagre_spice_connection_parent_class)->impl_fill_writer (conn, writer);

  xmlTextWriterWriteFormatElement (writer, (const xmlChar *)"view_only", "%d", spice_conn->priv->view_only);
  xmlTextWriterWriteFormatElement (writer, (const xmlChar *)"scaling", "%d", spice_conn->priv->scaling);
  xmlTextWriterWriteFormatElement (writer, (const xmlChar *)"resize_guest", "%d", spice_conn->priv->resize_guest);
  xmlTextWriterWriteFormatElement (writer, (const xmlChar *)"auto_clipboard", "%d", spice_conn->priv->auto_clipboard);

  if (spice_conn->priv->ssh_tunnel_host && *spice_conn->priv->ssh_tunnel_host)
    xmlTextWriterWriteFormatElement (writer, (const xmlChar *)"ssh_tunnel_host", "%s", spice_conn->priv->ssh_tunnel_host);
}

static void
spice_parse_item (VinagreConnection *conn, xmlNode *root)
{
  xmlNode *curr;
  xmlChar *s_value;
  VinagreSpiceConnection *spice_conn = VINAGRE_SPICE_CONNECTION (conn);

  VINAGRE_CONNECTION_CLASS (vinagre_spice_connection_parent_class)->impl_parse_item (conn, root);

  for (curr = root->children; curr; curr = curr->next)
    {
      s_value = xmlNodeGetContent (curr);

      if (!xmlStrcmp(curr->name, (const xmlChar *)"view_only"))
	{
	  vinagre_spice_connection_set_view_only (spice_conn, vinagre_utils_parse_boolean ((const gchar *)s_value));
	}
      else if (!xmlStrcmp(curr->name, (const xmlChar *)"scaling"))
	{
	  vinagre_spice_connection_set_scaling (spice_conn, vinagre_utils_parse_boolean ((const gchar *)s_value));
	}
      else if (!xmlStrcmp(curr->name, (const xmlChar *)"resize_guest"))
	{
	  vinagre_spice_connection_set_resize_guest (spice_conn, vinagre_utils_parse_boolean ((const gchar *)s_value));
	}
      else if (!xmlStrcmp(curr->name, (const xmlChar *)"auto_clipboard"))
	{
	  vinagre_spice_connection_set_auto_clipboard (spice_conn, vinagre_utils_parse_boolean ((const gchar *)s_value));
	}
      else if (!xmlStrcmp(curr->name, (const xmlChar *)"ssh_tunnel_host"))
	{
	  vinagre_spice_connection_set_ssh_tunnel_host (spice_conn, (const gchar *)s_value);
	}

      xmlFree (s_value);
    }
}

static gchar *
spice_get_best_name (VinagreConnection *conn)
{
  VinagreSpiceConnection *spice_conn = VINAGRE_SPICE_CONNECTION (conn);

  if (vinagre_connection_get_name (conn))
    return g_strdup (vinagre_connection_get_name (conn));

  if (spice_conn->priv->desktop_name)
    return g_strdup (spice_conn->priv->desktop_name);

  if (vinagre_connection_get_host (conn))
    return vinagre_connection_get_string_rep (conn, FALSE);

  return NULL;
}

static void
spice_parse_options_widget (VinagreConnection *conn, GtkWidget *widget)
{
  GtkWidget *view_only, *scaling, *ssh_host, *resize_guest, *auto_clipboard;

  view_only = g_object_get_data (G_OBJECT (widget), "view_only");
  scaling = g_object_get_data (G_OBJECT (widget), "scaling");
  resize_guest = g_object_get_data (G_OBJECT (widget), "resize_guest");
  auto_clipboard = g_object_get_data (G_OBJECT (widget), "auto_clipboard");
  ssh_host = g_object_get_data (G_OBJECT (widget), "ssh_host");
  if (!view_only || !scaling || !ssh_host || !resize_guest || !auto_clipboard)
    {
      g_warning ("Wrong widget passed to spice_parse_options_widget()");
      return;
    }

  vinagre_cache_prefs_set_boolean ("spice-connection", "view-only", gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (view_only)));
  vinagre_cache_prefs_set_boolean ("spice-connection", "scaling", gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (scaling)));
  vinagre_cache_prefs_set_boolean ("spice-connection", "resize-guest", gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (resize_guest)));
  vinagre_cache_prefs_set_boolean ("spice-connection", "auto-clipboard", gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (auto_clipboard)));
  vinagre_cache_prefs_set_string  ("spice-connection", "ssh-tunnel-host", gtk_entry_get_text (GTK_ENTRY (ssh_host)));

  g_object_set (conn,
		"resize-guest", gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (resize_guest)),
		"auto-clipboard", gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (auto_clipboard)),
		"view-only", gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (view_only)),
		"scaling", gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (scaling)),
		"ssh-tunnel-host", gtk_entry_get_text (GTK_ENTRY (ssh_host)),
		NULL);
}

static void
vinagre_spice_connection_class_init (VinagreSpiceConnectionClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);
  VinagreConnectionClass* parent_class = VINAGRE_CONNECTION_CLASS (klass);

  g_type_class_add_private (klass, sizeof (VinagreSpiceConnectionPrivate));

  object_class->finalize = vinagre_spice_connection_finalize;
  object_class->dispose = vinagre_spice_connection_dispose;
  object_class->set_property = vinagre_spice_connection_set_property;
  object_class->get_property = vinagre_spice_connection_get_property;
  object_class->constructed  = vinagre_spice_connection_constructed;

  parent_class->impl_fill_writer = spice_fill_writer;
  parent_class->impl_parse_item	 = spice_parse_item;
  parent_class->impl_get_best_name = spice_get_best_name;
  parent_class->impl_parse_options_widget = spice_parse_options_widget;

  g_object_class_install_property (object_class,
				   PROP_DESKTOP_NAME,
				   g_param_spec_string ("desktop-name",
							"desktop-name",
							"name of this connection as reported by the server",
							NULL,
							G_PARAM_READWRITE |
							G_PARAM_CONSTRUCT |
							G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class,
				   PROP_VIEW_ONLY,
				   g_param_spec_boolean ("view-only",
							 "View-only connection",
							 "Whether this connection is a view-only one",
							 FALSE,
							 G_PARAM_READWRITE |
							 G_PARAM_CONSTRUCT |
							 G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class,
				   PROP_SCALING,
				   g_param_spec_boolean ("scaling",
							 "Use scaling",
							 "Whether to use scaling on this connection",
							 FALSE,
							 G_PARAM_READWRITE |
							 G_PARAM_CONSTRUCT |
							 G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class,
				   PROP_RESIZE_GUEST,
				   g_param_spec_boolean ("resize-guest",
							 "Resize guest",
							 "Whether to use guest resize on this connection",
							 TRUE,
							 G_PARAM_READWRITE |
							 G_PARAM_CONSTRUCT |
							 G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class,
				   PROP_RESIZE_GUEST,
				   g_param_spec_boolean ("auto-clipboard",
							 "Auto clipboard",
							 "Whether clipboard sharing is automatic",
							 TRUE,
							 G_PARAM_READWRITE |
							 G_PARAM_CONSTRUCT |
							 G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class,
				   PROP_FD,
				   g_param_spec_int ("fd",
						     "file descriptor",
						     "the file descriptor for this connection",
						     0,
						     G_MAXINT,
						     0,
						     G_PARAM_READWRITE |
						     G_PARAM_CONSTRUCT |
						     G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class,
				   PROP_SSH_TUNNEL_HOST,
				   g_param_spec_string ("ssh-tunnel-host",
							"SSH Tunnel Host",
							"hostname used to create the SSH tunnel",
							NULL,
							G_PARAM_READWRITE |
							G_PARAM_CONSTRUCT |
							G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class,
				   PROP_SOCKET,
				   g_param_spec_object ("socket",
							"Socket",
							"A GSocket for this connection",
							G_TYPE_SOCKET,
							G_PARAM_READWRITE |
							G_PARAM_CONSTRUCT |
							G_PARAM_STATIC_STRINGS));

}

VinagreConnection *
vinagre_spice_connection_new (void)
{
  return VINAGRE_CONNECTION (g_object_new (VINAGRE_TYPE_SPICE_CONNECTION, NULL));
}

void
vinagre_spice_connection_set_desktop_name (VinagreSpiceConnection *conn,
					   const gchar *desktop_name)
{
  g_return_if_fail (VINAGRE_IS_SPICE_CONNECTION (conn));

  g_free (conn->priv->desktop_name);
  conn->priv->desktop_name = g_strdup (desktop_name);
}

const gchar *
vinagre_spice_connection_get_desktop_name (VinagreSpiceConnection *conn)
{
  g_return_val_if_fail (VINAGRE_IS_SPICE_CONNECTION (conn), NULL);

  return conn->priv->desktop_name;
}

void
vinagre_spice_connection_set_view_only (VinagreSpiceConnection *conn,
					gboolean value)
{
  g_return_if_fail (VINAGRE_IS_SPICE_CONNECTION (conn));

  conn->priv->view_only = value;
}
gboolean
vinagre_spice_connection_get_view_only (VinagreSpiceConnection *conn)
{
  g_return_val_if_fail (VINAGRE_IS_SPICE_CONNECTION (conn), FALSE);

  return conn->priv->view_only;
}

void
vinagre_spice_connection_set_scaling (VinagreSpiceConnection *conn,
				      gboolean value)
{
  g_return_if_fail (VINAGRE_IS_SPICE_CONNECTION (conn));

  conn->priv->scaling = value;
}
gboolean
vinagre_spice_connection_get_scaling (VinagreSpiceConnection *conn)
{
  g_return_val_if_fail (VINAGRE_IS_SPICE_CONNECTION (conn), FALSE);

  return conn->priv->scaling;
}

void
vinagre_spice_connection_set_resize_guest (VinagreSpiceConnection *conn,
					   gboolean value)
{
  g_return_if_fail (VINAGRE_IS_SPICE_CONNECTION (conn));

  conn->priv->resize_guest = value;
}
gboolean
vinagre_spice_connection_get_resize_guest (VinagreSpiceConnection *conn)
{
  g_return_val_if_fail (VINAGRE_IS_SPICE_CONNECTION (conn), FALSE);

  return conn->priv->resize_guest;
}

void
vinagre_spice_connection_set_auto_clipboard (VinagreSpiceConnection *conn,
					     gboolean value)
{
  g_return_if_fail (VINAGRE_IS_SPICE_CONNECTION (conn));

  conn->priv->auto_clipboard = value;
}

gboolean
vinagre_spice_connection_get_auto_clipboard (VinagreSpiceConnection *conn)
{
  g_return_val_if_fail (VINAGRE_IS_SPICE_CONNECTION (conn), FALSE);

  return conn->priv->auto_clipboard;
}

void
vinagre_spice_connection_set_fd (VinagreSpiceConnection *conn,
				 gint		         value)
{
  g_return_if_fail (VINAGRE_IS_SPICE_CONNECTION (conn));
  g_return_if_fail (value >= 0);

  conn->priv->fd = value;
}

gint
vinagre_spice_connection_get_fd (VinagreSpiceConnection *conn)
{
  g_return_val_if_fail (VINAGRE_IS_SPICE_CONNECTION (conn), 0);

  if (conn->priv->socket)
    return g_socket_get_fd (conn->priv->socket);
  else
    return conn->priv->fd;
}

void
vinagre_spice_connection_set_socket (VinagreSpiceConnection *conn,
				     GSocket		    *socket)
{
  g_return_if_fail (VINAGRE_IS_SPICE_CONNECTION (conn));

  if (socket)
    conn->priv->socket = g_object_ref (socket);
}

GSocket *
vinagre_spice_connection_get_socket (VinagreSpiceConnection *conn)
{
  g_return_val_if_fail (VINAGRE_IS_SPICE_CONNECTION (conn), 0);

  return conn->priv->socket;
}

void
vinagre_spice_connection_set_ssh_tunnel_host (VinagreSpiceConnection *conn,
					      const gchar *host)
{
  g_return_if_fail (VINAGRE_IS_SPICE_CONNECTION (conn));

  g_free (conn->priv->ssh_tunnel_host);
  conn->priv->ssh_tunnel_host = g_strdup (host);
}

const gchar *
vinagre_spice_connection_get_ssh_tunnel_host (VinagreSpiceConnection *conn)
{
  g_return_val_if_fail (VINAGRE_IS_SPICE_CONNECTION (conn), NULL);

  return conn->priv->ssh_tunnel_host;
}

/* vim: set ts=8: */
