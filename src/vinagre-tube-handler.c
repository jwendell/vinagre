/*
 * vinagre-tube-handler.c
 * This file is part of vinagre
 *
 * © 2009, Collabora Ltd
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
 *
 * Authors:
 *      Arnaud Maillet <arnaud.maillet@collabora.co.uk>
 */

#include <glib-object.h>
#include <glib/gi18n.h>

#include <telepathy-glib/contact.h>
#include <telepathy-glib/util.h>

#include "vinagre-tube-handler.h"
#include "vinagre-commands.h"
#include "vinagre-tab.h"
#include "vinagre-notebook.h"

G_DEFINE_TYPE (VinagreTubeHandler, vinagre_tube_handler, G_TYPE_OBJECT);

#define VINAGRE_TUBE_HANDLER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE \
    ((obj), VINAGRE_TYPE_TUBE_HANDLER, VinagreTubeHandlerPrivate))

typedef struct _VinagreTubeHandlerPrivate VinagreTubeHandlerPrivate;

struct _VinagreTubeHandlerPrivate
{
  VinagreWindow *window;
  TpChannel *channel;
  VinagreTab *tab;
  VinagreNotebook *nb;
  gulong signal_disconnect_id;
  gulong signal_invalidated_id;
};

enum
{
  DISCONNECTED,
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_VINAGRE_WINDOW,
  PROP_CHANNEL
};

static guint signals[LAST_SIGNAL] = { 0 };

static void
vinagre_tube_handler_dispose (GObject *object)
{
  VinagreTubeHandlerPrivate *priv = VINAGRE_TUBE_HANDLER_GET_PRIVATE(object);

  g_debug ("-- Destruction of a Tube handler --\n");

  if (priv->channel != NULL)
    {
      tp_cli_channel_call_close (priv->channel, -1,
          NULL, NULL, NULL, NULL);
      g_object_unref (priv->channel);
      priv->channel = NULL;
    }

  G_OBJECT_CLASS (vinagre_tube_handler_parent_class)->dispose (object);
}

static void
vinagre_tube_handler_set_vinagre_window (VinagreTubeHandler *self,
    gpointer *vinagre_window)
{
  g_return_if_fail (VINAGRE_IS_TUBE_HANDLER (self));

  VinagreTubeHandlerPrivate *priv = VINAGRE_TUBE_HANDLER_GET_PRIVATE (self);
  priv->window = VINAGRE_WINDOW (vinagre_window);
}

static void
vinagre_tube_handler_set_channel (VinagreTubeHandler *self,
    gpointer *channel)
{
  g_return_if_fail (VINAGRE_IS_TUBE_HANDLER (self));

  VinagreTubeHandlerPrivate *priv = VINAGRE_TUBE_HANDLER_GET_PRIVATE (self);
  priv->channel = g_object_ref (channel);
}

static void
vinagre_tube_handler_set_property (GObject *object,
    guint         prop_id,
    const GValue *value,
    GParamSpec   *pspec)
{
  VinagreTubeHandler *self = VINAGRE_TUBE_HANDLER (object);

  switch (prop_id)
    {
    case PROP_VINAGRE_WINDOW:
      vinagre_tube_handler_set_vinagre_window (self,
          g_value_get_object (value));
      break;
    case PROP_CHANNEL:
      vinagre_tube_handler_set_channel (self,
          g_value_get_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
vinagre_tube_handler_get_property (GObject *object,
    guint prop_id,
    GValue *value,
    GParamSpec *pspec)
{
  VinagreTubeHandlerPrivate *priv = VINAGRE_TUBE_HANDLER_GET_PRIVATE (object);

  switch (prop_id)
    {
    case PROP_VINAGRE_WINDOW:
      g_value_set_object (value, priv->window);
      break;
    case PROP_CHANNEL:
      g_value_set_object (value, priv->channel);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
vinagre_tube_handler_fire_disconnected (VinagreTubeHandler *self)
{
  VinagreTubeHandlerPrivate *priv = VINAGRE_TUBE_HANDLER_GET_PRIVATE (self);

  g_signal_handler_disconnect (G_OBJECT (priv->nb),
      priv->signal_disconnect_id);

  if (priv->channel != NULL)
    g_signal_handler_disconnect (G_OBJECT (priv->channel),
        priv->signal_invalidated_id);

  g_signal_emit (G_OBJECT (self), signals[DISCONNECTED], 0);
}

static void
vinagre_tube_handler_tab_disconnected_cb (GtkNotebook *notebook,
    GtkWidget *child,
    guint page_num,
    gpointer self)
{
  VinagreTubeHandlerPrivate *priv = VINAGRE_TUBE_HANDLER_GET_PRIVATE (self);
  VinagreTab *tab = VINAGRE_TAB (child);

  if (priv->tab == tab)
    {
      g_debug ("Tab has been destroyed. Closing the tube handler.");
      vinagre_tube_handler_fire_disconnected (self);
    }
}

static void
vinagre_tube_handler_tube_invalidated (TpProxy *channel,
    guint domain,
    gint code,
    gchar *message,
    gpointer self)
{
  VinagreTubeHandlerPrivate *priv = VINAGRE_TUBE_HANDLER_GET_PRIVATE (self);

  g_debug ("Tube is invalidated : %s\n", message);

  g_object_unref (priv->channel);
  priv->channel = NULL;

  vinagre_tube_handler_fire_disconnected (self);
}

static void
vinagre_tube_handler_accept_stream_tube_cb (TpChannel *channel,
    const GValue *address,
    const GError *error,
    gpointer self,
    GObject *weak_object)
{
  gchar *hostname;
  guint port;
  gchar *port_s;
  gchar *host;
  gchar *error_msg = NULL;
  VinagreConnection *conn = NULL;

  VinagreTubeHandlerPrivate *priv = VINAGRE_TUBE_HANDLER_GET_PRIVATE (self);

  if (error != NULL)
    {
      g_printerr ("Impossible to accept the stream tube: %s\n",
          error->message);
      g_signal_emit (G_OBJECT (self), signals[DISCONNECTED], 0);
      return ;
    }

  priv->signal_invalidated_id = g_signal_connect (G_OBJECT (channel),
      "invalidated", G_CALLBACK (vinagre_tube_handler_tube_invalidated),
      self);

  dbus_g_type_struct_get (address, 0, &hostname, 1, &port,
      G_MAXUINT);

  port_s = g_strdup_printf ("%u", port);

  host = g_strconcat (hostname, ":", port_s, NULL);

  conn = vinagre_connection_new_from_string (host, &error_msg, TRUE);

  g_free (port_s);
  g_free (hostname);
  g_free (host);

  if (conn == NULL)
    {
      g_printerr ("Impossible to create the connection: %s\n",
          error_msg);
      g_signal_emit (G_OBJECT (self), signals[DISCONNECTED], 0);
      g_free (error_msg);
      return ;
    }

  vinagre_cmd_direct_connect (conn, priv->window);

  priv->tab = vinagre_window_conn_exists (priv->window, conn);

  priv->nb = vinagre_tab_get_notebook (priv->tab);

  g_assert (priv->tab != NULL);

  g_assert (priv->nb != NULL);

  priv->signal_disconnect_id = g_signal_connect (G_OBJECT (priv->nb),
      "page-removed", G_CALLBACK
      (vinagre_tube_handler_tab_disconnected_cb), self);
}

static void
vinagre_tube_handler_dialog_response_cb (GtkDialog *dialog,
    gint response,
    gpointer self)
{
  VinagreTubeHandlerPrivate *priv = VINAGRE_TUBE_HANDLER_GET_PRIVATE (self);
  GValue access_control_param = {0,};

  if (response == GTK_RESPONSE_OK)
    {
      g_value_init (&access_control_param, G_TYPE_STRING);
      /* With the Localhost access control method, the access
      control param is unused; set a dummy value */
      g_value_set_static_string (&access_control_param, "");
      tp_cli_channel_type_stream_tube_call_accept (priv->channel, -1,
          TP_SOCKET_ADDRESS_TYPE_IPV4, TP_SOCKET_ACCESS_CONTROL_LOCALHOST,
          &access_control_param, vinagre_tube_handler_accept_stream_tube_cb,
          self, NULL, NULL);
      g_value_unset (&access_control_param);
    }
  else
    {
      g_signal_emit (G_OBJECT (self), signals[DISCONNECTED], 0);
    }

  gtk_widget_destroy (GTK_WIDGET (dialog));
}

static void
vinagre_tube_handler_factory_handle_cb (TpConnection *connection,
    guint n_contacts,
    TpContact * const *contacts,
    guint n_failed,
    const TpHandle *failed,
    const GError *error,
    gpointer self,
    GObject *weak_object)
{
  GtkWidget *dialog;
  VinagreTubeHandlerPrivate *priv = VINAGRE_TUBE_HANDLER_GET_PRIVATE (self);
  TpContact * contact;
  const gchar * alias;

  if (error != NULL)
    {
      g_printerr ("Impossible to get the contact name: %s\n", error->message);
      g_signal_emit (G_OBJECT (self), signals[DISCONNECTED], 0);
      return;
    }

  contact = contacts[0];
  alias = tp_contact_get_alias (contact);

  dialog = gtk_message_dialog_new (GTK_WINDOW (priv->window),
      GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION,
      GTK_BUTTONS_OK_CANCEL, _("%s wants to share his desktop with you."),
      alias);

  gtk_window_set_title (GTK_WINDOW (dialog), _("Desktop sharing invitation"));

  g_signal_connect (G_OBJECT (dialog), "response", G_CALLBACK
      (vinagre_tube_handler_dialog_response_cb), self);

  gtk_widget_show_all (GTK_WIDGET (dialog));
}

static void
vinagre_tube_handler_constructed (GObject *object)
{
  VinagreTubeHandlerPrivate *priv = VINAGRE_TUBE_HANDLER_GET_PRIVATE (object);
  TpConnection *connection;
  TpHandle handle;
  TpContactFeature features[] = { TP_CONTACT_FEATURE_ALIAS };

  g_debug (" -- New Tube handler --\n");

  g_object_get (priv->channel, "connection", &connection, NULL);

  handle = tp_channel_get_handle (priv->channel, NULL);

  tp_connection_get_contacts_by_handle (connection, 1, &handle, 1,
      features, vinagre_tube_handler_factory_handle_cb,
      object, NULL, NULL);
}

static void
vinagre_tube_handler_class_init (VinagreTubeHandlerClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->constructed = vinagre_tube_handler_constructed;
  gobject_class->set_property = vinagre_tube_handler_set_property;
  gobject_class->get_property = vinagre_tube_handler_get_property;
  gobject_class->dispose = vinagre_tube_handler_dispose;

  signals[DISCONNECTED] =
      g_signal_new ("disconnected",
      G_OBJECT_CLASS_TYPE (gobject_class),
      G_SIGNAL_RUN_FIRST,
      G_STRUCT_OFFSET (VinagreTubeHandlerClass, disconnected),
      NULL, NULL,
      g_cclosure_marshal_VOID__VOID,
      G_TYPE_NONE,
      0);

  g_object_class_install_property (gobject_class,
      PROP_VINAGRE_WINDOW,
      g_param_spec_object ("vinagre-window",
      "Vinagre window",
      "The Vinagre window",
      VINAGRE_TYPE_WINDOW,
      G_PARAM_READWRITE      |
      G_PARAM_CONSTRUCT_ONLY |
      G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class,
      PROP_CHANNEL,
      g_param_spec_object ("channel",
      "Vinagre tp channel",
      "The Vinagre tp channel",
      TP_TYPE_CHANNEL,
      G_PARAM_READWRITE      |
      G_PARAM_CONSTRUCT_ONLY |
      G_PARAM_STATIC_STRINGS));

  g_type_class_add_private (klass, sizeof (VinagreTubeHandlerPrivate));
}

static void
vinagre_tube_handler_init (VinagreTubeHandler *object)
{
}

VinagreTubeHandler *
vinagre_tube_handler_new (VinagreWindow *vinagre_window, TpChannel *channel)
{
  return g_object_new (VINAGRE_TYPE_TUBE_HANDLER,
      "vinagre-window", vinagre_window,
      "channel", channel,
      NULL);
}
