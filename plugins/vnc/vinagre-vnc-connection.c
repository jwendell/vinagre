/*
 * vinagre-vnc-connection.c
 * Child class of abstract VinagreConnection, specific to VNC protocol
 * This file is part of vinagre
 *
 * Copyright (C) 2009 - Jonh Wendell <wendell@bani.com.br>
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
#include <vinagre/vinagre-utils.h>
#include "vinagre-vnc-connection.h"

struct _VinagreVncConnectionPrivate
{
  gchar    *desktop_name;
  gboolean view_only;
  gboolean scaling;
  gint     shared;
};

enum
{
  PROP_0,
  PROP_DESKTOP_NAME,
  PROP_VIEW_ONLY,
  PROP_SCALING,
  PROP_SHARED,
};

#define VINAGRE_VNC_CONNECTION_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), VINAGRE_TYPE_VNC_CONNECTION, VinagreVncConnectionPrivate))
G_DEFINE_TYPE (VinagreVncConnection, vinagre_vnc_connection, VINAGRE_TYPE_CONNECTION);

static void
vinagre_vnc_connection_init (VinagreVncConnection *conn)
{
  conn->priv = G_TYPE_INSTANCE_GET_PRIVATE (conn, VINAGRE_TYPE_VNC_CONNECTION, VinagreVncConnectionPrivate);

  conn->priv->desktop_name = NULL;
  conn->priv->view_only = FALSE;
  conn->priv->scaling = FALSE;
  conn->priv->shared = -1;
}

static void
vinagre_vnc_connection_constructed (GObject *object)
{
  vinagre_connection_set_protocol (VINAGRE_CONNECTION (object), "vnc");
}

static void
vinagre_vnc_connection_finalize (GObject *object)
{
  VinagreVncConnection *conn = VINAGRE_VNC_CONNECTION (object);

  g_free (conn->priv->desktop_name);

  G_OBJECT_CLASS (vinagre_vnc_connection_parent_class)->finalize (object);
}

static void
vinagre_vnc_connection_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
  VinagreVncConnection *conn;

  g_return_if_fail (VINAGRE_IS_VNC_CONNECTION (object));

  conn = VINAGRE_VNC_CONNECTION (object);

  switch (prop_id)
    {
      case PROP_DESKTOP_NAME:
	vinagre_vnc_connection_set_desktop_name (conn, g_value_get_string (value));
	break;

      case PROP_VIEW_ONLY:
	vinagre_vnc_connection_set_view_only (conn, g_value_get_boolean (value));
	break;

      case PROP_SCALING:
	vinagre_vnc_connection_set_scaling (conn, g_value_get_boolean (value));
	break;

      case PROP_SHARED:
	vinagre_vnc_connection_set_shared (conn, g_value_get_int (value));
	break;

      default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	break;
    }
}

static void
vinagre_vnc_connection_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
  VinagreVncConnection *conn;

  g_return_if_fail (VINAGRE_IS_VNC_CONNECTION (object));

  conn = VINAGRE_VNC_CONNECTION (object);

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

      case PROP_SHARED:
	g_value_set_int (value, conn->priv->shared);
	break;

      default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	break;
    }
}

static void
vnc_fill_writer (VinagreConnection *conn, xmlTextWriter *writer)
{
  VinagreVncConnection *vnc_conn = VINAGRE_VNC_CONNECTION (conn);
  VINAGRE_CONNECTION_CLASS (vinagre_vnc_connection_parent_class)->impl_fill_writer (conn, writer);

  xmlTextWriterWriteFormatElement (writer, (const xmlChar *)"view_only", "%d", vnc_conn->priv->view_only);
  xmlTextWriterWriteFormatElement (writer, (const xmlChar *)"scaling", "%d", vnc_conn->priv->scaling);
}

static void
vnc_parse_item (VinagreConnection *conn, xmlNode *root)
{
  xmlNode *curr;
  xmlChar *s_value;
  VinagreVncConnection *vnc_conn = VINAGRE_VNC_CONNECTION (conn);

  VINAGRE_CONNECTION_CLASS (vinagre_vnc_connection_parent_class)->impl_parse_item (conn, root);

  for (curr = root->children; curr; curr = curr->next)
    {
      s_value = xmlNodeGetContent (curr);

      if (!xmlStrcmp(curr->name, (const xmlChar *)"view_only"))
	vinagre_vnc_connection_set_view_only (vnc_conn, vinagre_utils_parse_boolean ((const gchar *)s_value));
      else if (!xmlStrcmp(curr->name, (const xmlChar *)"scaling"))
	{
	  if (!scaling_command_line)
	    vinagre_vnc_connection_set_scaling (vnc_conn, vinagre_utils_parse_boolean ((const gchar *)s_value));
	}

      xmlFree (s_value);
    }
}

static gchar *
vnc_get_best_name (VinagreConnection *conn)
{
  VinagreVncConnection *vnc_conn = VINAGRE_VNC_CONNECTION (conn);

  if (vinagre_connection_get_name (conn))
    return g_strdup (vinagre_connection_get_name (conn));

  if (vnc_conn->priv->desktop_name)
    return g_strdup (vnc_conn->priv->desktop_name);

  if (vinagre_connection_get_host (conn))
    return vinagre_connection_get_string_rep (conn, FALSE);

  return NULL;
}

static void
vnc_fill_conn_from_file (VinagreConnection *conn, GKeyFile *file)
{
  gint shared;
  GError *e = NULL;

  shared = g_key_file_get_integer (file, "options", "shared", &e);
  if (e)
    {
      g_error_free (e);
      return;
    }
  else
    if (shared == 0 || shared == 1)
      vinagre_vnc_connection_set_shared (VINAGRE_VNC_CONNECTION (conn), shared);
    else
      g_message (_("Bad value for 'shared' flag: %d. It is supposed to be 0 or 1. Ignoring it."), shared);
}

static void
vnc_parse_options_widget (VinagreConnection *conn, GtkWidget *widget)
{
  GtkWidget *view_only, *scaling;

  view_only = g_object_get_data (G_OBJECT (widget), "view_only");
  scaling = g_object_get_data (G_OBJECT (widget), "scaling");
  if (!view_only || !scaling)
    {
      g_warning ("Wrong widget passed to vnc_parse_options_widget()");
      return;
    }

  g_object_set (conn,
		"view-only", gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (view_only)),
		"scaling", gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (scaling)),
		NULL);
}

static void
vinagre_vnc_connection_class_init (VinagreVncConnectionClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);
  VinagreConnectionClass* parent_class = VINAGRE_CONNECTION_CLASS (klass);

  g_type_class_add_private (klass, sizeof (VinagreVncConnectionPrivate));

  object_class->finalize = vinagre_vnc_connection_finalize;
  object_class->set_property = vinagre_vnc_connection_set_property;
  object_class->get_property = vinagre_vnc_connection_get_property;
  object_class->constructed  = vinagre_vnc_connection_constructed;

  parent_class->impl_fill_writer = vnc_fill_writer;
  parent_class->impl_parse_item  = vnc_parse_item;
  parent_class->impl_get_best_name = vnc_get_best_name;
  parent_class->impl_fill_conn_from_file = vnc_fill_conn_from_file;
  parent_class->impl_parse_options_widget = vnc_parse_options_widget;

  g_object_class_install_property (object_class,
                                   PROP_DESKTOP_NAME,
                                   g_param_spec_string ("desktop-name",
                                                        "desktop-name",
	                                                "name of this connection as reported by the server",
                                                        NULL,
	                                                G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT |
                                                        G_PARAM_STATIC_NICK |
                                                        G_PARAM_STATIC_NAME |
                                                        G_PARAM_STATIC_BLURB));

  g_object_class_install_property (object_class,
                                   PROP_VIEW_ONLY,
                                   g_param_spec_boolean ("view-only",
                                                        "View-only connection",
	                                                "Whether this connection is a view-only one",
                                                        FALSE,
	                                                G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT |
                                                        G_PARAM_STATIC_NICK |
                                                        G_PARAM_STATIC_NAME |
                                                        G_PARAM_STATIC_BLURB));
  g_object_class_install_property (object_class,
                                   PROP_SCALING,
                                   g_param_spec_boolean ("scaling",
                                                        "Use scaling",
	                                                "Whether to use scaling on this connection",
                                                        FALSE,
	                                                G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT |
                                                        G_PARAM_STATIC_NICK |
                                                        G_PARAM_STATIC_NAME |
                                                        G_PARAM_STATIC_BLURB));

  g_object_class_install_property (object_class,
                                   PROP_SHARED,
                                   g_param_spec_int ("shared",
                                                     "shared flag",
	                                              "if the server should allow more than one client connected",
                                                      -1,
                                                      1,
                                                      -1,
	                                              G_PARAM_READWRITE |
                                                      G_PARAM_CONSTRUCT |
                                                      G_PARAM_STATIC_NICK |
                                                      G_PARAM_STATIC_NAME |
                                                      G_PARAM_STATIC_BLURB));

}

VinagreConnection *
vinagre_vnc_connection_new (void)
{
  return VINAGRE_CONNECTION (g_object_new (VINAGRE_TYPE_VNC_CONNECTION, NULL));
}

void
vinagre_vnc_connection_set_desktop_name (VinagreVncConnection *conn,
					 const gchar *desktop_name)
{
  g_return_if_fail (VINAGRE_IS_VNC_CONNECTION (conn));

  g_free (conn->priv->desktop_name);
  conn->priv->desktop_name = g_strdup (desktop_name);
}
const gchar *
vinagre_vnc_connection_get_desktop_name (VinagreVncConnection *conn)
{
  g_return_val_if_fail (VINAGRE_IS_VNC_CONNECTION (conn), NULL);

  return conn->priv->desktop_name;
}

void
vinagre_vnc_connection_set_shared (VinagreVncConnection *conn,
				   gint value)
{
  g_return_if_fail (VINAGRE_IS_VNC_CONNECTION (conn));
  g_return_if_fail (value >=-1 && value <=1);

  conn->priv->shared = value;
}
gint
vinagre_vnc_connection_get_shared (VinagreVncConnection *conn)
{
  g_return_val_if_fail (VINAGRE_IS_VNC_CONNECTION (conn), -1);

  return conn->priv->shared;
}

void
vinagre_vnc_connection_set_view_only (VinagreVncConnection *conn,
				      gboolean value)
{
  g_return_if_fail (VINAGRE_IS_VNC_CONNECTION (conn));

  conn->priv->view_only = value;
}
gboolean
vinagre_vnc_connection_get_view_only (VinagreVncConnection *conn)
{
  g_return_val_if_fail (VINAGRE_IS_VNC_CONNECTION (conn), FALSE);

  return conn->priv->view_only;
}

void
vinagre_vnc_connection_set_scaling (VinagreVncConnection *conn,
				    gboolean value)
{
  g_return_if_fail (VINAGRE_IS_VNC_CONNECTION (conn));

  conn->priv->scaling = value;
}
gboolean
vinagre_vnc_connection_get_scaling (VinagreVncConnection *conn)
{
  g_return_val_if_fail (VINAGRE_IS_VNC_CONNECTION (conn), FALSE);

  return conn->priv->scaling;
}

/* vim: set ts=8: */
