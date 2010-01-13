/*
 * handler.h - convenience class to implement a Telepathy Handler
 *
 * Copyright (C) 2009 Collabora Ltd. <http://www.collabora.co.uk/>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __VINAGRE_HANDLER_H__
#define __VINAGRE_HANDLER_H__

#include <glib-object.h>

#include <telepathy-glib/account.h>
#include <telepathy-glib/connection.h>
#include <telepathy-glib/channel.h>
#include <telepathy-glib/channel-request.h>
#include <telepathy-glib/dbus-properties-mixin.h>

G_BEGIN_DECLS

#define TP_TYPE_HANDLER	(vinagre_handler_get_type ())
#define VINAGRE_HANDLER(obj)	(G_TYPE_CHECK_INSTANCE_CAST ((obj), TP_TYPE_HANDLER, VinagreHandler))
#define VINAGRE_HANDLER_CLASS(obj)	(G_TYPE_CHECK_CLASS_CAST ((obj), TP_TYPE_HANDLER, VinagreHandlerClass))
#define TP_IS_HANDLER(obj)	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), TP_TYPE_HANDLER))
#define TP_IS_HANDLER_CLASS(obj)	(G_TYPE_CHECK_CLASS_TYPE ((obj), TP_TYPE_HANDLER))
#define VINAGRE_HANDLER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), TP_TYPE_HANDLER, VinagreHandlerClass))

typedef struct _VinagreHandler VinagreHandler;
struct _VinagreHandler
{
  GObject parent;
};

typedef struct _VinagreHandlerClass VinagreHandlerClass;
struct _VinagreHandlerClass
{
  GObjectClass parent_class;
  TpDBusPropertiesMixinClass dbus_props_class;

  void (* add_request) (VinagreHandler        *self,
                        TpChannelRequest *request,
                        GHashTable       *properties);
  void (* remove_request) (VinagreHandler        *self,
                           TpChannelRequest *request,
                           const char       *error,
                           const char       *message);

  gpointer _padding[8];
};

/**
 * VinagreHandlerHandleChannelsCb:
 * @self: the handler
 * @account: the Account with which the channels are associated
 * @connection: the Connection with which the channels are associated
 * @channels: a NULL-terminated list of channels to handle
 * @requests_satisfied: the ChannelRequests that handling this request satisfies
 * @user_action_time: the time at which user action occurred, used for
 * focus-stealing prevention
 * @handler_info: an a{sv} map of additional handler information
 *
 * This callback will be called whenever channels are dispatched to your
 * Handler for handling.
 *
 * <example><programlisting>
 * static void
 * handle_channels (VinagreHandler         *self,
 *                  TpAccount         *account,
 *                  TpConnection      *connection,
 *                  TpChannel        **channels,
 *                  TpChannelRequest **requests,
 *                  guint64            user_action_time,
 *                  GHashTable        *handler_info)
 * {
 *   g_print ("handle channels\n");
 *   g_print ("  account = %p: %s\n", account,
 *       tp_proxy_get_object_path (TP_PROXY (account)));
 *   g_print ("  connection = %p: %s\n", connection,
 *       tp_proxy_get_object_path (TP_PROXY (connection)));
 *
 *     {
 *       TpChannel **ptr;
 *
 *       for (ptr = channels; *ptr; ptr++)
 *         {
 *           TpChannel *channel = *ptr;
 *
 *           g_print ("  channel = %p: %s\n", channel,
 *               tp_proxy_get_object_path (TP_PROXY (channel)));
 *
 *         }
 *     }
 * }
 * </programlisting></example>
 *
 * Returns: TRUE if the Handler success, FALSE to return a NotAvailable to the
 * Channel Dispatcher
 */
typedef gboolean (* VinagreHandlerHandleChannelsCb) (VinagreHandler         *self,
                                                TpAccount         *account,
                                                TpConnection      *connection,
                                                TpChannel        **channels,
                                                TpChannelRequest **request_satisfied,
                                                guint64            user_action_time,
                                                GHashTable        *handler_info);

GType vinagre_handler_get_type (void);
VinagreHandler *vinagre_handler_new (GPtrArray                 *channel_filter,
                           gboolean                   bypass_approval,
                           VinagreHandlerHandleChannelsCb  handle_channels_cb);
GList *vinagre_handler_get_pending_requests (VinagreHandler *self);

G_END_DECLS

#endif
