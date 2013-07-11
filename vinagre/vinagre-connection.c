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

#include <config.h>
#include <stdlib.h>
#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

#include "vinagre-connection.h"
#include "vinagre-bookmarks.h"
#include "vinagre-plugins-engine.h"
#include "vinagre-vala.h"

#define DEFAULT_WIDTH   800
#define DEFAULT_HEIGHT  600
#define MIN_SIZE          1
#define MAX_SIZE       8192

struct _VinagreConnectionPrivate
{
  gchar *protocol;
  gchar *host;
  gint   port;
  gchar *username;
  gchar *password;
  gchar *name;
  gboolean fullscreen;
  guint  width;
  guint  height;
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
  PROP_FULLSCREEN,
  PROP_WIDTH,
  PROP_HEIGHT
};

#define VINAGRE_CONNECTION_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), VINAGRE_TYPE_CONNECTION, VinagreConnectionPrivate))
G_DEFINE_ABSTRACT_TYPE (VinagreConnection, vinagre_connection, G_TYPE_OBJECT);

static void
vinagre_connection_init (VinagreConnection *conn)
{
  conn->priv = G_TYPE_INSTANCE_GET_PRIVATE (conn, VINAGRE_TYPE_CONNECTION, VinagreConnectionPrivate);

  conn->priv->protocol = NULL;
  conn->priv->host = NULL;
  conn->priv->port = 0;
  conn->priv->password = NULL;
  conn->priv->username = NULL;
  conn->priv->name = NULL;
  conn->priv->fullscreen = FALSE;
  conn->priv->width = DEFAULT_WIDTH;
  conn->priv->height = DEFAULT_HEIGHT;
}

static void
vinagre_connection_finalize (GObject *object)
{
  VinagreConnection *conn = VINAGRE_CONNECTION (object);

  g_free (conn->priv->protocol);
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
	vinagre_connection_set_protocol (conn, g_value_get_string (value));
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

      case PROP_WIDTH:
	vinagre_connection_set_width (conn, g_value_get_uint (value));
	break;

      case PROP_HEIGHT:
	vinagre_connection_set_height (conn, g_value_get_uint (value));
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
	g_value_set_string (value, conn->priv->protocol);
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

      case PROP_FULLSCREEN:
	g_value_set_boolean (value, conn->priv->fullscreen);
	break;

      case PROP_NAME:
	g_value_set_string (value, conn->priv->name);
	break;

      case PROP_BEST_NAME:
	g_value_set_string (value, vinagre_connection_get_best_name (conn));
	break;

      case PROP_WIDTH:
	g_value_set_uint (value, conn->priv->width);
	break;

      case PROP_HEIGHT:
	g_value_set_uint (value, conn->priv->height);
	break;

      default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	break;
    }
}

static void
default_fill_writer (VinagreConnection *conn, xmlTextWriter *writer)
{
  if (conn->priv->protocol)
    xmlTextWriterWriteElement (writer, BAD_CAST "protocol", BAD_CAST conn->priv->protocol);
  xmlTextWriterWriteElement (writer, BAD_CAST "name", BAD_CAST conn->priv->name);
  xmlTextWriterWriteElement (writer, BAD_CAST "host", BAD_CAST conn->priv->host);
  xmlTextWriterWriteElement (writer, BAD_CAST "username", BAD_CAST (conn->priv->username ? conn->priv->username : ""));
  xmlTextWriterWriteFormatElement (writer, BAD_CAST "port", "%d", conn->priv->port);
  xmlTextWriterWriteFormatElement (writer, BAD_CAST "fullscreen", "%d", conn->priv->fullscreen);
  xmlTextWriterWriteFormatElement (writer, BAD_CAST "width", "%d", conn->priv->width);
  xmlTextWriterWriteFormatElement (writer, BAD_CAST "height", "%d", conn->priv->height);
}

static void
default_parse_item (VinagreConnection *conn, xmlNode *root)
{
  xmlNode *curr;
  xmlChar *s_value;

  for (curr = root->children; curr; curr = curr->next)
    {
      s_value = xmlNodeGetContent (curr);

      if (!xmlStrcmp(curr->name, BAD_CAST "host"))
	vinagre_connection_set_host (conn, (const gchar *)s_value);
      else if (!xmlStrcmp(curr->name, BAD_CAST "name"))
	vinagre_connection_set_name (conn, (const gchar *)s_value);
      else if (!xmlStrcmp(curr->name, BAD_CAST "username"))
	vinagre_connection_set_username (conn, (const gchar *)s_value);
      else if (!xmlStrcmp(curr->name, BAD_CAST "port"))
	vinagre_connection_set_port (conn, atoi ((const char *)s_value));
      else if (!xmlStrcmp(curr->name, BAD_CAST "fullscreen"))
	vinagre_connection_set_fullscreen (conn, vinagre_utils_parse_boolean ((const gchar *)s_value));
      else if (!xmlStrcmp(curr->name, BAD_CAST "width"))
	vinagre_connection_set_width (conn, atoi ((const char *)s_value));
      else if (!xmlStrcmp(curr->name, BAD_CAST "height"))
	vinagre_connection_set_height (conn, atoi ((const char *)s_value));

      xmlFree (s_value);
    }
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
default_parse_options_widget (VinagreConnection *conn, GtkWidget *widget)
{
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
  klass->impl_parse_options_widget = default_parse_options_widget;

  g_object_class_install_property (object_class,
                                   PROP_PROTOCOL,
                                   g_param_spec_string ("protocol",
                                                        "protocol",
	                                                "connection protocol",
                                                        NULL,
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
                                   PROP_USERNAME,
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

  g_object_class_install_property (object_class,
                                   PROP_WIDTH,
                                   g_param_spec_uint ("width",
                                                      "width",
                                                      "width of screen",
                                                       MIN_SIZE,
                                                       MAX_SIZE,
                                                       DEFAULT_WIDTH,
                                                       G_PARAM_READWRITE |
                                                       G_PARAM_CONSTRUCT |
                                                       G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class,
                                   PROP_HEIGHT,
                                   g_param_spec_uint ("height",
                                                      "height",
                                                      "height of screen",
                                                       MIN_SIZE,
                                                       MAX_SIZE,
                                                       DEFAULT_HEIGHT,
                                                       G_PARAM_READWRITE |
                                                       G_PARAM_CONSTRUCT |
                                                       G_PARAM_STATIC_STRINGS));

}

void
vinagre_connection_set_protocol (VinagreConnection *conn,
			         const gchar       *protocol)
{
  g_return_if_fail (VINAGRE_IS_CONNECTION (conn));

  g_free (conn->priv->protocol);
  conn->priv->protocol = g_strdup (protocol);
}
const gchar *
vinagre_connection_get_protocol (VinagreConnection *conn)
{
  g_return_val_if_fail (VINAGRE_IS_CONNECTION (conn), NULL);

  return conn->priv->protocol;
}

void
vinagre_connection_set_host (VinagreConnection *conn,
			     const gchar *host)
{
  g_return_if_fail (VINAGRE_IS_CONNECTION (conn));

  g_free (conn->priv->host);
  if (host)
    conn->priv->host = g_strdup (g_strstrip ((gchar *)host));
  else
    conn->priv->host = NULL;
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

void
vinagre_connection_set_width (VinagreConnection *conn,
			      guint width)
{
  g_return_if_fail (VINAGRE_IS_CONNECTION (conn));

  conn->priv->width = width;
}
guint
vinagre_connection_get_width (VinagreConnection *conn)
{
  g_return_val_if_fail (VINAGRE_IS_CONNECTION (conn), 0);

  return conn->priv->width;
}

void
vinagre_connection_set_height (VinagreConnection *conn,
			       guint height)
{
  g_return_if_fail (VINAGRE_IS_CONNECTION (conn));

  conn->priv->height = height;
}
guint
vinagre_connection_get_height (VinagreConnection *conn)
{
  g_return_val_if_fail (VINAGRE_IS_CONNECTION (conn), 0);

  return conn->priv->height;
}

/**
 * vinagre_connection_split_string:
 * @uri: The URI to be splitted.
 * @known_protocol: The protocol, if it's known. NULL otherwise.
 * @protocol: Will hold the protocol of this URI
 * @host: Will hold the host of this URI
 * @port: Will hold the port of this URI
 * @error_msg: Will hold an error message in case of fail
 *
 * Splits a URI into its several parts.
 *
 * Returns: %TRUE if the URI is splitted successfuly. FALSE otherwise.
 */
gboolean
vinagre_connection_split_string (const gchar *uri,
				 const gchar *known_protocol,
				 gchar       **protocol,
				 gchar       **host,
				 gint         *port,
				 gchar       **error_msg)
{
  gchar **server, **url;
  gint    lport;
  gchar  *lhost;
  gchar   ipv6_host[255] = {0,};
  VinagreProtocol *ext;

  *error_msg = NULL;
  *host = NULL;
  *port = 0;

  url = g_strsplit (uri, "://", 2);
  if (g_strv_length (url) == 2)
    {
      if (known_protocol)
	*protocol = g_strdup (known_protocol);
      else
	*protocol = g_strdup (url[0]);
      lhost = url[1];
    }
  else
    {
      if (known_protocol)
	*protocol = g_strdup (known_protocol);
      else
	*protocol = g_strdup ("vnc");
      lhost = (gchar *) uri;
    }

  ext = vinagre_plugins_engine_get_plugin_by_protocol (vinagre_plugins_engine_get_default (),
						       *protocol);
  if (!ext)
    {
      *error_msg = g_strdup_printf (_("The protocol %s is not supported."), *protocol);
      g_free (*protocol);
      *protocol = NULL;
      g_strfreev (url);
      return FALSE;
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
      lport = server[1] ? atoi (server[1]) : vinagre_protocol_get_default_port (ext);
    }
  else
    {
      server = g_strsplit (lhost, ":", 2);
      lport = server[1] ? atoi (server[1]) : vinagre_protocol_get_default_port (ext);

      if ((g_str_equal (*protocol, "vnc")) && (lport < 1024))
        lport += 5900;
    }

  lhost = ipv6_host[0] ? ipv6_host : (server[0] && server[0][0] ? server[0] : "localhost");

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
  gint    port;
  gchar  *host, *protocol;
  VinagreProtocol *ext;

  if (!vinagre_connection_split_string (uri, NULL, &protocol, &host, &port, error_msg))
    return NULL;

  if (use_bookmarks)
    conn = vinagre_bookmarks_exists (vinagre_bookmarks_get_default (),
				     protocol,
				     host,
				     port);
  if (!conn)
    {
      ext = vinagre_plugins_engine_get_plugin_by_protocol (vinagre_plugins_engine_get_default (),
							   protocol);
      if (!ext)
	goto finalize;

      conn = vinagre_protocol_new_connection (ext);
      vinagre_connection_set_host (conn, host);
      vinagre_connection_set_port (conn, port);
    }

finalize:
  g_free (host);
  g_free (protocol);
  return conn;
}

VinagreConnection *
vinagre_connection_new_from_file (const gchar *uri, gchar **error_msg, gboolean use_bookmarks)
{
  VinagreConnection *conn;
  gchar             *data;
  GFile             *file_a;
  GError            *error;
  GHashTable        *extensions;
  GHashTableIter     iter;
  gpointer           ext;

  *error_msg = NULL;
  data = NULL;
  conn = NULL;
  error = NULL;

  file_a = g_file_new_for_commandline_arg (uri);
  if (!g_file_load_contents (file_a,
			     NULL,
			     &data,
			     NULL,
			     NULL,
			     &error))
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

  extensions = vinagre_plugins_engine_get_plugins_by_protocol (vinagre_plugins_engine_get_default ());
  g_hash_table_iter_init (&iter, extensions);
  while (g_hash_table_iter_next (&iter, NULL, &ext))
    {
      VinagreProtocol *protocol = VINAGRE_PROTOCOL (ext);

      if (vinagre_protocol_recognize_file (protocol, file_a))
        {
          conn = vinagre_protocol_new_connection_from_file (protocol,
							    data,
							    use_bookmarks,
							    error_msg);
          break;
        }
    }

the_end:
  g_free (data);
  g_object_unref (file_a);

  if (!conn && !*error_msg)
    *error_msg = g_strdup (_("The file was not recognized by any of the plugins."));

  return conn;
}

gchar*
vinagre_connection_get_string_rep (VinagreConnection *conn,
				   gboolean has_protocol)
{
  GString *uri;
  gchar *result;
  gboolean is_ipv6;
  VinagreProtocol *ext;

  g_return_val_if_fail (VINAGRE_IS_CONNECTION (conn), NULL);

  is_ipv6 = g_strstr_len (conn->priv->host, -1, ":") != NULL;

  if (has_protocol)
    {
      uri = g_string_new (conn->priv->protocol);
      g_string_append (uri, "://");
    }
  else
    uri = g_string_new (NULL);

  if (is_ipv6)
    g_string_append_c (uri, '[');
  g_string_append (uri, conn->priv->host);
  if (is_ipv6)
    g_string_append_c (uri, ']');

  ext = vinagre_plugins_engine_get_plugin_by_protocol (vinagre_plugins_engine_get_default (), conn->priv->protocol);
  if (ext)
    if (vinagre_protocol_get_default_port (ext) != conn->priv->port)
      g_string_append_printf (uri, "::%d", conn->priv->port);

  result = uri->str;
  g_string_free (uri, FALSE);

  return result;
}

void
vinagre_connection_fill_writer (VinagreConnection *conn,
				xmlTextWriterPtr   writer)
{
  g_return_if_fail (VINAGRE_IS_CONNECTION (conn));

  VINAGRE_CONNECTION_GET_CLASS (conn)->impl_fill_writer (conn, writer);
}

void
vinagre_connection_parse_item (VinagreConnection *conn,
			       xmlNode           *root)
{
  g_return_if_fail (VINAGRE_IS_CONNECTION (conn));

  VINAGRE_CONNECTION_GET_CLASS (conn)->impl_parse_item (conn, root);
}

gchar*
vinagre_connection_get_best_name (VinagreConnection *conn)
{
  g_return_val_if_fail (VINAGRE_IS_CONNECTION (conn), NULL);

  return VINAGRE_CONNECTION_GET_CLASS (conn)->impl_get_best_name (conn);
}

void
vinagre_connection_parse_options_widget (VinagreConnection *conn,
					 GtkWidget         *widget)
{
  g_return_if_fail (VINAGRE_IS_CONNECTION (conn));
  g_return_if_fail (GTK_IS_WIDGET (widget));

  VINAGRE_CONNECTION_GET_CLASS (conn)->impl_parse_options_widget (conn, widget);
}
/* vim: set ts=8: */
