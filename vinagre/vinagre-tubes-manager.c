/*
 * vinagre-tubes-manager.c
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

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include <telepathy-glib/enums.h>
#include <telepathy-glib/dbus.h>
#include <telepathy-glib/interfaces.h>

#include "vinagre-tubes-manager.h"
#include "vinagre-tube-handler.h"
#include "vinagre-debug.h"

#define BUS_NAME "org.gnome.Empathy.StreamTubeHandler.rfb"
#define OBJECT_PATH "/org/gnome/Empathy/StreamTubeHandler/rfb"

static gboolean
vinagre_tubes_manager_handle_tube (VinagreTubesManager *object,
    const gchar *bus_name,
    const gchar *connection,
    const gchar *channel,
    guint handle_type,
    guint handle,
    GError **error);

#include "dbus-interface-glue.h"

G_DEFINE_TYPE (VinagreTubesManager, vinagre_tubes_manager, G_TYPE_OBJECT);

#define VINAGRE_TUBES_MANAGER_GET_PRIVATE(obj)\
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), VINAGRE_TYPE_TUBES_MANAGER,\
    VinagreTubesManagerPrivate))

typedef struct _VinagreTubesManagerPrivate VinagreTubesManagerPrivate;

struct _VinagreTubesManagerPrivate
{
  VinagreWindow *window;
  GSList *tubes_handler;
};

enum
{
  PROP_0,
  PROP_VINAGRE_WINDOW
};

typedef struct
{
  VinagreTubesManager *tmanager;
  gchar *bus_name;
  gchar *connection;
  gchar *channel;
  guint handle_type;
  guint handle;
} IdleData;

static void
vinagre_tubes_manager_dispose (GObject *object)
{
  VinagreTubesManagerPrivate *priv = VINAGRE_TUBES_MANAGER_GET_PRIVATE
      (object);
  GSList *l;

  for (l = priv->tubes_handler; l; l = l->next)
    g_object_unref (l->data);

  g_slist_free (priv->tubes_handler);
  priv->tubes_handler = NULL;

  G_OBJECT_CLASS (vinagre_tubes_manager_parent_class)->dispose (object);
}

static void
vinagre_tubes_manager_set_vinagre_window (VinagreTubesManager *self,
    gpointer *vinagre_window)
{
  g_return_if_fail (VINAGRE_IS_TUBES_MANAGER (self));

  VinagreTubesManagerPrivate *priv = VINAGRE_TUBES_MANAGER_GET_PRIVATE (self);
  priv->window = VINAGRE_WINDOW (vinagre_window);
}

static void
vinagre_tubes_manager_set_property (GObject *object, guint prop_id,
    const GValue *value, GParamSpec   *pspec)
{
  VinagreTubesManager *self = VINAGRE_TUBES_MANAGER (object);

  switch (prop_id)
    {
    case PROP_VINAGRE_WINDOW:
      vinagre_tubes_manager_set_vinagre_window (self,
          g_value_get_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
vinagre_tubes_manager_get_property (GObject *object, guint prop_id,
    GValue *value, GParamSpec *pspec)
{
  VinagreTubesManagerPrivate *priv = VINAGRE_TUBES_MANAGER_GET_PRIVATE
      (object);

  switch (prop_id)
    {
    case PROP_VINAGRE_WINDOW:
      g_value_set_object (value, priv->window);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
vinagre_tubes_manager_disconnected_cb (VinagreTubeHandler *htube,
    gpointer self)
{
  VinagreTubesManagerPrivate *priv = VINAGRE_TUBES_MANAGER_GET_PRIVATE (self);
  priv->tubes_handler = g_slist_remove (priv->tubes_handler, htube);
  g_object_unref (htube);
}

static void
vinagre_tubes_manager_tube_ready_destroy_notify (gpointer data)
{
  IdleData *idle_data = data;

  g_free (idle_data->bus_name);
  g_free (idle_data->connection);
  g_free (idle_data->channel);
  g_slice_free (IdleData, idle_data);
}

static void
vinagre_tubes_manager_channel_ready_cb (TpChannel *channel,
    const GError *error,
    gpointer data)
{
  IdleData *idle_data = data;
  VinagreTubeHandler *htube;
  VinagreTubesManagerPrivate *priv = VINAGRE_TUBES_MANAGER_GET_PRIVATE
      (idle_data->tmanager);

  if (error != NULL)
    {
      vinagre_debug_message (DEBUG_TUBE,"channel has been invalidated: %s", error->message);
      vinagre_tubes_manager_tube_ready_destroy_notify (data);
      g_object_unref (channel);
      return;
    }

  htube = vinagre_tube_handler_new (priv->window, channel);

  priv->tubes_handler = g_slist_prepend (priv->tubes_handler, htube);

  g_signal_connect (G_OBJECT (htube), "disconnected", G_CALLBACK
      (vinagre_tubes_manager_disconnected_cb), idle_data->tmanager);

  vinagre_tubes_manager_tube_ready_destroy_notify (data);
}

static void
vinagre_tubes_manager_connection_ready_cb (TpConnection *connection,
    const GError *error,
    gpointer data)
{
  TpChannel *channel;
  IdleData *idle_data = data;

  if (error != NULL)
    {
      vinagre_debug_message (DEBUG_TUBE,"connection has been invalidated: %s", error->message);
      vinagre_tubes_manager_tube_ready_destroy_notify (data);
      g_object_unref (connection);
      return;
    }

  channel = tp_channel_new (connection, idle_data->channel,
      TP_IFACE_CHANNEL_TYPE_TUBES, idle_data->handle_type,
      idle_data->handle, NULL);
  tp_channel_call_when_ready (channel,
      vinagre_tubes_manager_channel_ready_cb, idle_data);
}

static gboolean
vinagre_tubes_manager_handle_tube_idle_cb (gpointer data)
{
  IdleData *idle_data = data;
  TpConnection *connection;
  static TpDBusDaemon *daemon = NULL;

  vinagre_debug_message (DEBUG_TUBE,"New tube to be handled");

  if (!daemon)
    daemon = tp_dbus_daemon_new (tp_get_bus ());

  connection = tp_connection_new (daemon, idle_data->bus_name,
      idle_data->connection, NULL);
  tp_connection_call_when_ready (connection,
      vinagre_tubes_manager_connection_ready_cb, idle_data);

  g_object_unref (connection);
  return FALSE;
}

static gboolean
vinagre_tubes_manager_handle_tube (VinagreTubesManager *object,
    const gchar *bus_name,
    const gchar *connection,
    const gchar *channel,
    guint handle_type,
    guint handle,
    GError **error)
{
  VinagreTubesManager * self = VINAGRE_TUBES_MANAGER (object);
  IdleData *data;

  data = g_slice_new (IdleData);
  data->tmanager = self;
  data->bus_name = g_strdup (bus_name);
  data->connection = g_strdup (connection);
  data->channel = g_strdup (channel);
  data->handle_type = handle_type;
  data->handle = handle;

  g_idle_add_full (G_PRIORITY_HIGH, vinagre_tubes_manager_handle_tube_idle_cb,
      data, NULL);
  return TRUE;
}

static void
vinagre_tubes_manager_register_tube_handler (GObject *object)
{
  DBusGProxy *proxy;
  guint result;
  GError *error = NULL;

  dbus_g_object_type_install_info (VINAGRE_TYPE_TUBES_MANAGER,
      &dbus_glib_vinagre_tubes_manager_object_info);

  proxy = dbus_g_proxy_new_for_name (tp_get_bus (), DBUS_SERVICE_DBUS,
      DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS);

  if (!dbus_g_proxy_call (proxy, "RequestName", &error,
      G_TYPE_STRING, BUS_NAME, G_TYPE_UINT, DBUS_NAME_FLAG_DO_NOT_QUEUE,
      G_TYPE_INVALID, G_TYPE_UINT, &result, G_TYPE_INVALID))
    {
      vinagre_debug_message (DEBUG_TUBE,"Failed to request name: %s",
          error ? error->message : "No error given");
      g_clear_error (&error);
      goto OUT;
    }

  vinagre_debug_message (DEBUG_TUBE,"Creating tube handler %s object_path:%s\n", BUS_NAME,
      OBJECT_PATH);
  dbus_g_connection_register_g_object (tp_get_bus (), OBJECT_PATH,
      G_OBJECT (object));

OUT:
  g_object_unref (proxy);
}

static void
vinagre_tubes_manager_constructed (GObject *object)
{
  VinagreTubesManagerPrivate *priv = VINAGRE_TUBES_MANAGER_GET_PRIVATE
      (object);

  vinagre_tubes_manager_register_tube_handler (object);
}

static void
vinagre_tubes_manager_class_init (VinagreTubesManagerClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->constructed = vinagre_tubes_manager_constructed;
  gobject_class->set_property = vinagre_tubes_manager_set_property;
  gobject_class->get_property = vinagre_tubes_manager_get_property;
  gobject_class->dispose = vinagre_tubes_manager_dispose;

  g_object_class_install_property (gobject_class,
      PROP_VINAGRE_WINDOW,
      g_param_spec_object ("vinagre-window",
      "Vinagre window",
      "The Vinagre window",
      VINAGRE_TYPE_WINDOW,
      G_PARAM_READWRITE      |
      G_PARAM_CONSTRUCT_ONLY |
      G_PARAM_STATIC_STRINGS));

  g_type_class_add_private (klass, sizeof (VinagreTubesManagerPrivate));
}

static void
vinagre_tubes_manager_init (VinagreTubesManager *object)
{
}

VinagreTubesManager *
vinagre_tubes_manager_new (VinagreWindow *vinagre_window)
{
  return g_object_new (VINAGRE_TYPE_TUBES_MANAGER,
      "vinagre-window", vinagre_window,
      NULL);
}
