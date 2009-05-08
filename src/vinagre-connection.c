/*
 * vinagre-connection.c
 * This file is part of vinagre
 *
 * Copyright (C) 2007,2008,2009 - Jonh Wendell <wendell@bani.com.br>
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

#include <stdlib.h>
#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

#include "vinagre-connection.h"
#include "vinagre-enums.h"
#include "vinagre-bookmarks.h"
#include "vinagre-vnc-connection.h"

struct _VinagreConnectionPrivate
{
  VinagreConnectionProtocol protocol;
  gchar *host;
  gint   port;
  gchar *username;
  gchar *password;
  gchar *name;
  gboolean fullscreen;
};

enum
{
  PROP_0,
  PROP_PROTOCOL,
  PROP_HOST,
  PROP_PORT,
  PROP_USERNAME,
  PROP_PASSWORD,
  PROP_NAME,
  PROP_BEST_NAME,
  PROP_ICON,
  PROP_FULLSCREEN
};

gint   vinagre_connection_default_port [VINAGRE_CONNECTION_PROTOCOL_INVALID-1] = {5900, 3389};
gchar* vinagre_connection_protos [VINAGRE_CONNECTION_PROTOCOL_INVALID-1] = {"vnc", "rdp"};

#define VINAGRE_CONNECTION_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), VINAGRE_TYPE_CONNECTION, VinagreConnectionPrivate))
G_DEFINE_ABSTRACT_TYPE (VinagreConnection, vinagre_connection, G_TYPE_OBJECT);

static void
vinagre_connection_init (VinagreConnection *conn)
{
  conn->priv = G_TYPE_INSTANCE_GET_PRIVATE (conn, VINAGRE_TYPE_CONNECTION, VinagreConnectionPrivate);

  conn->priv->protocol = VINAGRE_CONNECTION_PROTOCOL_INVALID;
  conn->priv->host = NULL;
  conn->priv->port = 0;
  conn->priv->password = NULL;
  conn->priv->username = NULL;
  conn->priv->name = NULL;
  conn->priv->fullscreen = FALSE;
}

static void
vinagre_connection_finalize (GObject *object)
{
  VinagreConnection *conn = VINAGRE_CONNECTION (object);

  g_free (conn->priv->host);
  g_free (conn->priv->username);
  g_free (conn->priv->password);
  g_free (conn->priv->name);

  G_OBJECT_CLASS (vinagre_connection_parent_class)->finalize (object);
}

static void
vinagre_connection_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
  VinagreConnection *conn;

  g_return_if_fail (VINAGRE_IS_CONNECTION (object));

  conn = VINAGRE_CONNECTION (object);

  switch (prop_id)
    {
      case PROP_PROTOCOL:
	vinagre_connection_set_protocol (conn, g_value_get_enum (value));
	break;

      case PROP_HOST:
	vinagre_connection_set_host (conn, g_value_get_string (value));
	break;

      case PROP_PORT:
	vinagre_connection_set_port (conn, g_value_get_int (value));
	break;

      case PROP_USERNAME:
	vinagre_connection_set_username (conn, g_value_get_string (value));
	break;

      case PROP_PASSWORD:
	vinagre_connection_set_password (conn, g_value_get_string (value));
	break;

      case PROP_FULLSCREEN:
	vinagre_connection_set_fullscreen (conn, g_value_get_boolean (value));
	break;

      case PROP_NAME:
	vinagre_connection_set_name (conn, g_value_get_string (value));
	break;

      default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	break;
    }
}

static void
vinagre_connection_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
  VinagreConnection *conn;

  g_return_if_fail (VINAGRE_IS_CONNECTION (object));

  conn = VINAGRE_CONNECTION (object);


  switch (prop_id)
    {
      case PROP_PROTOCOL:
	g_value_set_enum (value, conn->priv->protocol);
	break;

      case PROP_HOST:
	g_value_set_string (value, conn->priv->host);
	break;

      case PROP_PORT:
	g_value_set_int (value, conn->priv->port);
	break;

      case PROP_USERNAME:
	g_value_set_string (value, conn->priv->username);
	break;

      case PROP_PASSWORD:
	g_value_set_string (value, conn->priv->password);
	break;

      case PROP_ICON:
	g_value_set_object (value, vinagre_connection_get_icon (conn));
	break;

      case PROP_FULLSCREEN:
	g_value_set_boolean (value, conn->priv->fullscreen);
	break;

      case PROP_NAME:
	g_value_set_string (value, conn->priv->name);
	break;

      case PROP_BEST_NAME:
	g_value_set_string (value, vinagre_connection_get_best_name (conn));
	break;

      default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	break;
    }
}

static void
default_fill_writer (VinagreConnection *conn, xmlTextWriter *writer)
{
  xmlTextWriterWriteElement (writer, "name", conn->priv->name);
  xmlTextWriterWriteElement (writer, "host", conn->priv->host);
  xmlTextWriterWriteFormatElement (writer, "port", "%d", conn->priv->port);
  xmlTextWriterWriteFormatElement (writer, "fullscreen", "%d", conn->priv->fullscreen);
}

static void
default_parse_item (VinagreConnection *conn, xmlNode *root)
{
  xmlNode *curr;
  xmlChar *s_value;

  for (curr = root->children; curr; curr = curr->next)
    {
      s_value = xmlNodeGetContent (curr);

      if (!xmlStrcmp(curr->name, (const xmlChar *)"host"))
	vinagre_connection_set_host (conn, s_value);
      else if (!xmlStrcmp(curr->name, (const xmlChar *)"name"))
	vinagre_connection_set_name (conn, s_value);
      else if (!xmlStrcmp(curr->name, (const xmlChar *)"port"))
	vinagre_connection_set_port (conn, atoi (s_value));
      else if (!xmlStrcmp(curr->name, (const xmlChar *)"fullscreen"))
	vinagre_connection_set_fullscreen (conn, vinagre_utils_parse_boolean (s_value));

      xmlFree (s_value);
    }

  if (conn->priv->port <= 0)
    vinagre_connection_set_port (conn, vinagre_connection_default_port [conn->priv->protocol-1]);

}

static gchar *
default_get_best_name (VinagreConnection *conn)
{
  if (conn->priv->name)
    return g_strdup (conn->priv->name);

  if (conn->priv->host)
    return vinagre_connection_get_string_rep (conn, FALSE);

  return NULL;
}

static void
vinagre_connection_class_init (VinagreConnectionClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (VinagreConnectionPrivate));

  object_class->finalize = vinagre_connection_finalize;
  object_class->set_property = vinagre_connection_set_property;
  object_class->get_property = vinagre_connection_get_property;

  klass->impl_fill_writer = default_fill_writer;
  klass->impl_parse_item = default_parse_item;
  klass->impl_get_best_name = default_get_best_name;
  klass->impl_fill_conn_from_file = NULL;

  g_object_class_install_property (object_class,
                                   PROP_PROTOCOL,
                                   g_param_spec_enum ("protocol",
                                                      "protocol",
	                                              "connection protocol",
                                                      VINAGRE_TYPE_CONNECTION_PROTOCOL,
	                                              VINAGRE_CONNECTION_PROTOCOL_VNC,
	                                              G_PARAM_READWRITE |
                                                      G_PARAM_CONSTRUCT |
                                                      G_PARAM_STATIC_NICK |
                                                      G_PARAM_STATIC_NAME |
                                                      G_PARAM_STATIC_BLURB));

  g_object_class_install_property (object_class,
                                   PROP_HOST,
                                   g_param_spec_string ("host",
                                                        "hostname",
	                                                "hostname or ip address of this connection",
                                                        NULL,
	                                                G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT |
                                                        G_PARAM_STATIC_NICK |
                                                        G_PARAM_STATIC_NAME |
                                                        G_PARAM_STATIC_BLURB));

  g_object_class_install_property (object_class,
                                   PROP_PORT,
                                   g_param_spec_int ("port",
                                                     "port",
	                                              "tcp/ip port of this connection",
                                                      0,
                                                      G_MAXINT,
                                                      0,
	                                              G_PARAM_READWRITE |
                                                      G_PARAM_CONSTRUCT |
                                                      G_PARAM_STATIC_NICK |
                                                      G_PARAM_STATIC_NAME |
                                                      G_PARAM_STATIC_BLURB));

  g_object_class_install_property (object_class,
                                   PROP_NAME,
                                   g_param_spec_string ("username",
                                                        "username",
	                                                "username (if any) necessary for complete this connection",
                                                        NULL,
	                                                G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT |
                                                        G_PARAM_STATIC_NICK |
                                                        G_PARAM_STATIC_NAME |
                                                        G_PARAM_STATIC_BLURB));

  g_object_class_install_property (object_class,
                                   PROP_PASSWORD,
                                   g_param_spec_string ("password",
                                                        "password",
	                                                "password (if any) necessary for complete this connection",
                                                        NULL,
	                                                G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT |
                                                        G_PARAM_STATIC_NICK |
                                                        G_PARAM_STATIC_NAME |
                                                        G_PARAM_STATIC_BLURB));

  g_object_class_install_property (object_class,
                                   PROP_NAME,
                                   g_param_spec_string ("name",
                                                        "connection name",
	                                                "friendly name for this connection",
                                                        NULL,
	                                                G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT |
                                                        G_PARAM_STATIC_NICK |
                                                        G_PARAM_STATIC_NAME |
                                                        G_PARAM_STATIC_BLURB));

  g_object_class_install_property (object_class,
                                   PROP_BEST_NAME,
                                   g_param_spec_string ("best-name",
                                                        "best-name",
	                                                "preferred name for this connection",
                                                        NULL,
	                                                G_PARAM_READABLE |
                                                        G_PARAM_STATIC_NICK |
                                                        G_PARAM_STATIC_NAME |
                                                        G_PARAM_STATIC_BLURB));

  g_object_class_install_property (object_class,
                                   PROP_ICON,
                                   g_param_spec_object ("icon",
                                                        "icon",
	                                                "icon of this connection",
                                                        GDK_TYPE_PIXBUF,
	                                                G_PARAM_READABLE |
                                                        G_PARAM_STATIC_NICK |
                                                        G_PARAM_STATIC_NAME |
                                                        G_PARAM_STATIC_BLURB));
  g_object_class_install_property (object_class,
                                   PROP_FULLSCREEN,
                                   g_param_spec_boolean ("fullscreen",
                                                        "Full screen connection",
	                                                "Whether this connection is a view-only one",
                                                        FALSE,
	                                                G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT |
                                                        G_PARAM_STATIC_NICK |
                                                        G_PARAM_STATIC_NAME |
                                                        G_PARAM_STATIC_BLURB));

}

void
vinagre_connection_set_protocol (VinagreConnection *conn,
			         VinagreConnectionProtocol protocol)
{
  g_return_if_fail (VINAGRE_IS_CONNECTION (conn));

  conn->priv->protocol = protocol;
}
VinagreConnectionProtocol
vinagre_connection_get_protocol (VinagreConnection *conn)
{
  g_return_val_if_fail (VINAGRE_IS_CONNECTION (conn),
                        VINAGRE_CONNECTION_PROTOCOL_INVALID);

  return conn->priv->protocol;
}

const gchar *
vinagre_connection_get_protocol_as_string (VinagreConnection *conn)
{
  g_return_val_if_fail (VINAGRE_IS_CONNECTION (conn), NULL);

  return vinagre_connection_protos [conn->priv->protocol-1];
}

VinagreConnectionProtocol
vinagre_connection_protocol_by_name (const gchar *protocol)
{
  int i;

  for (i = VINAGRE_CONNECTION_PROTOCOL_VNC; i <= VINAGRE_CONNECTION_PROTOCOL_INVALID; i++)
    if (g_strcmp0 (vinagre_connection_protos [i-1], protocol) == 0)
      return i;

  return VINAGRE_CONNECTION_PROTOCOL_INVALID;
}

void
vinagre_connection_set_host (VinagreConnection *conn,
			     const gchar *host)
{
  g_return_if_fail (VINAGRE_IS_CONNECTION (conn));

  g_free (conn->priv->host);
  conn->priv->host = g_strdup (host);
}
const gchar *
vinagre_connection_get_host (VinagreConnection *conn)
{
  g_return_val_if_fail (VINAGRE_IS_CONNECTION (conn), NULL);

  return conn->priv->host;
}

void
vinagre_connection_set_port (VinagreConnection *conn,
			     gint port)
{
  g_return_if_fail (VINAGRE_IS_CONNECTION (conn));

  conn->priv->port = port;
}
gint
vinagre_connection_get_port (VinagreConnection *conn)
{
  g_return_val_if_fail (VINAGRE_IS_CONNECTION (conn), 0);

  return conn->priv->port;
}

void
vinagre_connection_set_username (VinagreConnection *conn,
			     const gchar *username)
{
  g_return_if_fail (VINAGRE_IS_CONNECTION (conn));

  g_free (conn->priv->username);
  conn->priv->username = g_strdup (username);
}
const gchar *
vinagre_connection_get_username (VinagreConnection *conn)
{
  g_return_val_if_fail (VINAGRE_IS_CONNECTION (conn), NULL);

  return conn->priv->username;
}

void
vinagre_connection_set_password (VinagreConnection *conn,
			         const gchar *password)
{
  g_return_if_fail (VINAGRE_IS_CONNECTION (conn));

  g_free (conn->priv->password);
  conn->priv->password = g_strdup (password);
}
const gchar *
vinagre_connection_get_password (VinagreConnection *conn)
{
  g_return_val_if_fail (VINAGRE_IS_CONNECTION (conn), NULL);

  return conn->priv->password;
}

void
vinagre_connection_set_fullscreen (VinagreConnection *conn,
				  gboolean value)
{
  g_return_if_fail (VINAGRE_IS_CONNECTION (conn));

  conn->priv->fullscreen = value;
}
gboolean
vinagre_connection_get_fullscreen (VinagreConnection *conn)
{
  g_return_val_if_fail (VINAGRE_IS_CONNECTION (conn), FALSE);

  return conn->priv->fullscreen;
}

void
vinagre_connection_set_name (VinagreConnection *conn,
			     const gchar *name)
{
  g_return_if_fail (VINAGRE_IS_CONNECTION (conn));

  g_free (conn->priv->name);
  conn->priv->name = g_strdup (name);
}
const gchar *
vinagre_connection_get_name (VinagreConnection *conn)
{
  g_return_val_if_fail (VINAGRE_IS_CONNECTION (conn), NULL);

  return conn->priv->name;
}

GdkPixbuf *
vinagre_connection_get_icon (VinagreConnection *conn)
{
  GdkPixbuf         *pixbuf;
  GtkIconTheme      *icon_theme;
  gchar             *icon_name;

  g_return_val_if_fail (VINAGRE_IS_CONNECTION (conn), NULL);
  g_return_val_if_fail (conn->priv->protocol != VINAGRE_CONNECTION_PROTOCOL_INVALID, NULL);

  icon_name = g_strdup_printf ("application-x-%s",
			       vinagre_connection_protos [conn->priv->protocol-1]);
  icon_theme = gtk_icon_theme_get_default ();
  pixbuf = gtk_icon_theme_load_icon (icon_theme,
				     icon_name,
				     16,
				     0,
				     NULL);

  g_free (icon_name);
  return pixbuf;
}

VinagreConnectionProtocol
protocol_by_name (const gchar *protocol)
{
  gint i;

  for (i=0; i<VINAGRE_CONNECTION_PROTOCOL_INVALID; i++)
    if (g_strcmp0 (vinagre_connection_protos[i], protocol) == 0)
      return i;

  return VINAGRE_CONNECTION_PROTOCOL_INVALID;
}

gboolean
vinagre_connection_split_string (const gchar *uri,
				 VinagreConnectionProtocol *protocol,
				 gchar **host,
				 gint *port,
				 gchar **error_msg)
{
  gchar **server, **url;
  gint    lport;
  gchar  *lhost;
  gchar   ipv6_host[255] = {0,};

  *error_msg = NULL;
  *host = NULL;
  *port = 0;

  url = g_strsplit (uri, "://", 2);
  if (g_strv_length (url) == 2)
    {
      *protocol = protocol_by_name (url[0]);
      if (*protocol == VINAGRE_CONNECTION_PROTOCOL_INVALID)
	{
	  *error_msg = g_strdup_printf (_("The protocol %s is not supported."),
					url[0]);
	  g_strfreev (url);
	  return FALSE;
	}
      lhost = url[1];
    }
  else
    {
      *protocol = VINAGRE_CONNECTION_PROTOCOL_VNC;
      lhost = (gchar *) uri;
    }

  if (lhost[0] == '[')
    {
      int i;
      for (i = 1; lhost[i] && lhost[i] != ']'; i++)
	{
	  ipv6_host[i-1] = lhost[i];
	  lhost[i-1] = '_';
	}
      ipv6_host[i-1] = '\0';
      lhost[i] = '_';
    }

  if (g_strrstr (lhost, "::") != NULL)
    {
      server = g_strsplit (lhost, "::", 2);
      lport = server[1] ? atoi (server[1]) : vinagre_connection_default_port [*protocol];
    }
  else
    {
      server = g_strsplit (lhost, ":", 2);
      lport = server[1] ? atoi (server[1]) : vinagre_connection_default_port [*protocol];

      if ((*protocol == VINAGRE_CONNECTION_PROTOCOL_VNC) && (lport < 1024))
        lport += 5900;
    }

  lhost = ipv6_host[0] ? ipv6_host : server[0];

  *host = g_strdup (lhost);
  *port = lport;

  g_strfreev (server);
  g_strfreev (url);

  return TRUE;
}

VinagreConnection *
vinagre_connection_new_from_string (const gchar *uri, gchar **error_msg, gboolean use_bookmarks)
{
  VinagreConnection *conn = NULL;
  VinagreConnectionProtocol protocol;
  gint    port;
  gchar  *host;

  if (!vinagre_connection_split_string (uri, &protocol, &host, &port, error_msg))
    return NULL;

  if (use_bookmarks)
    conn = vinagre_bookmarks_exists (vinagre_bookmarks_get_default (),
				     protocol,
				     host,
				     port);
  if (!conn)
    {
      conn = vinagre_connection_new (protocol);
      vinagre_connection_set_host (conn, host);
      vinagre_connection_set_port (conn, port);
    }

  g_free (host);
  return conn;
}

VinagreConnection *
vinagre_connection_new_from_file (const gchar *uri, gchar **error_msg, gboolean use_bookmarks)
{
  GKeyFile          *file;
  GError            *error;
  gboolean           loaded;
  VinagreConnection *conn;
  gchar             *host, *actual_host, *data;
  gint               port;
  int                file_size;
  GFile             *file_a;
  VinagreConnectionProtocol protocol;

  *error_msg = NULL;
  host = NULL;
  data = NULL;
  conn = NULL;
  error = NULL;
  file = NULL;
  conn = NULL;

  file_a = g_file_new_for_commandline_arg (uri);
  loaded = g_file_load_contents (file_a,
				 NULL,
				 &data,
				 &file_size,
				 NULL,
				 &error);
  if (!loaded)
    {
      if (error)
	{
	  *error_msg = g_strdup (error->message);
	  g_error_free (error);
	}
      else
	*error_msg = g_strdup (_("Could not open the file."));

      goto the_end;
    }

  file = g_key_file_new ();
  loaded = g_key_file_load_from_data (file,
				      data,
				      file_size,
				      0,
				      &error);
  if (!loaded)
    {
      if (error)
	{
	  *error_msg = g_strdup (error->message);
	  g_error_free (error);
	}
      else
	*error_msg = g_strdup (_("Could not parse the file."));

      goto the_end;
    }

  host = g_key_file_get_string (file, "connection", "host", NULL);
  port = g_key_file_get_integer (file, "connection", "port", NULL);
  if (host)
    {
      if (!port)
	{
	  if (!vinagre_connection_split_string (host, &protocol, &actual_host, &port, error_msg))
	    goto the_end;

	  g_free (host);
	  host = actual_host;
	}

      if (use_bookmarks)
        conn = vinagre_bookmarks_exists (vinagre_bookmarks_get_default (), protocol, host, port);
      if (!conn)
	{
	  gchar *username, *password;
	  gint shared;
	  GError *e = NULL;

	  conn = vinagre_connection_new (protocol);
	  vinagre_connection_set_host (conn, host);
	  vinagre_connection_set_port (conn, port);

	  username = g_key_file_get_string  (file, "connection", "username", NULL);
	  vinagre_connection_set_username (conn, username);
	  g_free (username);

	  password = g_key_file_get_string  (file, "connection", "password", NULL);
	  vinagre_connection_set_password (conn, password);
	  g_free (password);

	  if (VINAGRE_CONNECTION_GET_CLASS (conn)->impl_fill_conn_from_file)
	    VINAGRE_CONNECTION_GET_CLASS (conn)->impl_fill_conn_from_file (conn, file);
	}

      g_free (host);
    }
  else
    *error_msg = g_strdup (_("Could not find the host address in the file."));

the_end:
  g_free (data);
  g_object_unref (file_a);
  if (file)
    g_key_file_free (file);

  return conn;
}

gchar*
vinagre_connection_get_string_rep (VinagreConnection *conn,
				   gboolean has_protocol)
{
  GString *uri;
  gchar *result;
  gboolean is_ipv6;

  g_return_val_if_fail (VINAGRE_IS_CONNECTION (conn), NULL);

  is_ipv6 = g_strstr_len (conn->priv->host, -1, ":") != NULL;

  if (has_protocol)
    {
      uri = g_string_new (vinagre_connection_protos [conn->priv->protocol-1]);
      g_string_append (uri, "://");
    }
  else
    uri = g_string_new (NULL);

  if (is_ipv6)
    g_string_append_c (uri, '[');
  g_string_append (uri, conn->priv->host);
  if (is_ipv6)
    g_string_append_c (uri, ']');

  if (vinagre_connection_default_port [conn->priv->protocol-1] != conn->priv->port)
    g_string_append_printf (uri, "::%d", conn->priv->port);

  result = uri->str;
  g_string_free (uri, FALSE);

  return result;
}

VinagreConnection *
vinagre_connection_new (VinagreConnectionProtocol protocol)
{
  switch (protocol)
    {
      case VINAGRE_CONNECTION_PROTOCOL_VNC: return vinagre_vnc_connection_new ();
      default: g_assert_not_reached ();
    }
}

void
vinagre_connection_fill_writer (VinagreConnection *conn,
				xmlTextWriter *writer)
{
  VINAGRE_CONNECTION_GET_CLASS (conn)->impl_fill_writer (conn, writer);
}

void
vinagre_connection_parse_item (VinagreConnection *conn,
			       xmlNode *root)
{
  VINAGRE_CONNECTION_GET_CLASS (conn)->impl_parse_item (conn, root);
}

gchar*
vinagre_connection_get_best_name (VinagreConnection *conn)
{
  return VINAGRE_CONNECTION_GET_CLASS (conn)->impl_get_best_name (conn);
}
/* vim: set ts=8: */
