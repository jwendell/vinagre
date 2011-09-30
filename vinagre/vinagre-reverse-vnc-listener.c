/*
 * vinagre-reverse-vnc-listener.c
 * This file is part of vinagre
 *
 * Copyright (C) 2009-2010 - Jonh Wendell <wendell@bani.com.br>
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

#include <config.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include "vinagre-commands.h"
#include "vinagre-reverse-vnc-listener.h"
#include "plugins/vnc/vinagre-vnc-connection.h"
#include "vinagre-vala.h"

struct _VinagreReverseVncListenerPrivate
{
  GSocketService *service;
  gboolean        listening;
  gint            port;
  VinagreWindow  *window;
};

enum
{
  PROP_0,
  PROP_LISTENING,
  PROP_PORT
};

static VinagreReverseVncListener *listener_singleton = NULL;

G_DEFINE_TYPE (VinagreReverseVncListener, vinagre_reverse_vnc_listener, G_TYPE_OBJECT);

static void
vinagre_reverse_vnc_listener_init (VinagreReverseVncListener *listener)
{
  listener->priv = G_TYPE_INSTANCE_GET_PRIVATE (listener, VINAGRE_TYPE_REVERSE_VNC_LISTENER, VinagreReverseVncListenerPrivate);

  listener->priv->listening = FALSE;
  listener->priv->port = 0;
}

static void
vinagre_reverse_vnc_listener_dispose (GObject *object)
{
  VinagreReverseVncListener *listener = VINAGRE_REVERSE_VNC_LISTENER (object);

  vinagre_reverse_vnc_listener_stop (listener);

  G_OBJECT_CLASS (vinagre_reverse_vnc_listener_parent_class)->dispose (object);
}

static void
vinagre_reverse_vnc_listener_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
  VinagreReverseVncListener *listener;

  g_return_if_fail (VINAGRE_IS_REVERSE_VNC_LISTENER (object));

  listener = VINAGRE_REVERSE_VNC_LISTENER (object);

  switch (prop_id)
    {
      case PROP_LISTENING:
	g_value_set_boolean (value, listener->priv->listening);
	break;

      case PROP_PORT:
	g_value_set_int (value, vinagre_reverse_vnc_listener_get_port (listener));
	break;

      default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	break;
    }
}

static void
vinagre_reverse_vnc_listener_class_init (VinagreReverseVncListenerClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (VinagreReverseVncListenerPrivate));

  object_class->dispose = vinagre_reverse_vnc_listener_dispose;
  object_class->get_property = vinagre_reverse_vnc_listener_get_property;

  g_object_class_install_property (object_class,
                                   PROP_LISTENING,
                                   g_param_spec_boolean ("listening",
                                                        "Listening",
	                                                "If we are listening for incoming (reverse) VNC connections",
                                                        FALSE,
	                                                G_PARAM_READABLE |
                                                        G_PARAM_STATIC_NICK |
                                                        G_PARAM_STATIC_NAME |
                                                        G_PARAM_STATIC_BLURB));
  g_object_class_install_property (object_class,
                                   PROP_PORT,
                                   g_param_spec_int ("port",
                                                     "Port",
	                                             "TCP port in which we are listening for reverse connections",
                                                     5500,
                                                     5600,
                                                     5500,
	                                             G_PARAM_READABLE |
                                                     G_PARAM_STATIC_NICK |
                                                     G_PARAM_STATIC_NAME |
                                                     G_PARAM_STATIC_BLURB));

}

VinagreReverseVncListener *
vinagre_reverse_vnc_listener_get_default (void)
{
  if (G_UNLIKELY (!listener_singleton))
    {
      listener_singleton = VINAGRE_REVERSE_VNC_LISTENER (g_object_new (VINAGRE_TYPE_REVERSE_VNC_LISTENER, NULL));
      g_object_add_weak_pointer (G_OBJECT (listener_singleton), (gpointer *)&listener_singleton);
      return listener_singleton;
    }

  return g_object_ref (listener_singleton);
}

static gboolean
incoming (GSocketService *service,
	  GSocketConnection *connection,
	  GObject *source,
	  VinagreReverseVncListener *listener)
{
  VinagreConnection *conn;
  GSocketAddress *address;

  g_return_val_if_fail (listener->priv->window != NULL, FALSE);

  conn = vinagre_vnc_connection_new ();
  vinagre_vnc_connection_set_socket (VINAGRE_VNC_CONNECTION (conn),
				     g_socket_connection_get_socket (connection));

  address = g_socket_connection_get_remote_address (connection, NULL);
  if (address)
    {
      gchar *host = g_inet_address_to_string (g_inet_socket_address_get_address (G_INET_SOCKET_ADDRESS (address)));

      vinagre_connection_set_host (conn, host);
      vinagre_connection_set_port (conn, g_inet_socket_address_get_port (G_INET_SOCKET_ADDRESS (address)));

      g_object_unref (address);
      g_free (host);
    }

  vinagre_cmd_direct_connect (conn, listener->priv->window);

  return TRUE;
}

void
vinagre_reverse_vnc_listener_start (VinagreReverseVncListener *listener)
{
  VinagreReverseVncListenerPrivate *priv = listener->priv;
  GError *error;
  int port;

  g_return_if_fail (VINAGRE_IS_REVERSE_VNC_LISTENER (listener));

  if (priv->listening)
    return;

  priv->service = g_socket_service_new ();

  for (port=5500; port<=5600; port++)
    {
      error = NULL;
      if (g_socket_listener_add_inet_port (G_SOCKET_LISTENER (priv->service), port, NULL, &error))
	break;
      else
	{
	  g_message ("%s", error->message);
	  g_clear_error (&error);
	}
    }
  if (port > 5600)
    {
      vinagre_utils_show_error_dialog (_("Error activating reverse connections"),
				_("The program could not find any available TCP ports starting at 5500. Is there any other running program consuming all your TCP ports?"),
				GTK_WINDOW (listener->priv->window));
      g_object_unref (priv->service);
      priv->service = NULL;
      return;
    }

  g_signal_connect (priv->service, "incoming", G_CALLBACK (incoming), listener);
  g_socket_service_start (priv->service);

  priv->port = port;
  priv->listening = TRUE;
  g_object_notify (G_OBJECT (listener), "listening");
}

void
vinagre_reverse_vnc_listener_stop (VinagreReverseVncListener *listener)
{
  VinagreReverseVncListenerPrivate *priv = listener->priv;

  g_return_if_fail (VINAGRE_IS_REVERSE_VNC_LISTENER (listener));

  if (!priv->listening)
    return;

  g_socket_service_stop (priv->service);
  g_object_unref (priv->service);
  priv->service = NULL;

  priv->listening = FALSE;
  g_object_notify (G_OBJECT (listener), "listening");
}

gboolean
vinagre_reverse_vnc_listener_is_listening (VinagreReverseVncListener *listener)
{
  g_return_val_if_fail (VINAGRE_IS_REVERSE_VNC_LISTENER (listener), FALSE);

  return listener->priv->listening;
}

gint
vinagre_reverse_vnc_listener_get_port (VinagreReverseVncListener *listener)
{
  g_return_val_if_fail (VINAGRE_IS_REVERSE_VNC_LISTENER (listener), 0);

  return listener->priv->listening ? listener->priv->port : 0;
}

void
vinagre_reverse_vnc_listener_set_window (VinagreReverseVncListener *listener,
                                         VinagreWindow *window)
{
  g_return_if_fail (VINAGRE_IS_REVERSE_VNC_LISTENER (listener));

  if (listener->priv->window)
    g_object_unref (listener->priv->window);

  listener->priv->window = window ? g_object_ref (window) : NULL;
}
