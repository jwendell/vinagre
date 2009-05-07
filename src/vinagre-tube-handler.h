/*
 * vinagre-tube-handler.h
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

#ifndef __VINAGRE_TUBE_HANDLER_H__
#define __VINAGRE_TUBE_HANDLER_H__

#include <glib-object.h>

#include <telepathy-glib/channel.h>

#include "vinagre-window.h"

G_BEGIN_DECLS

#define VINAGRE_TYPE_TUBE_HANDLER (vinagre_tube_handler_get_type())
#define VINAGRE_TUBE_HANDLER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj),\
    VINAGRE_TYPE_TUBE_HANDLER, VinagreTubeHandler))
#define VINAGRE_IS_TUBE_HANDLER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj),\
    VINAGRE_TYPE_TUBE_HANDLER))
#define VINAGRE_TUBE_HANDLER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
    VINAGRE_TYPE_TUBE_HANDLER, VinagreTubeHandlerClass))
#define VINAGRE_IS_TUBE_HANDLER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE \
    ((klass), VINAGRE_TYPE_TUBE_HANDLER))
#define VINAGRE_TUBE_HANDLER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS \
    ((obj), VINAGRE_TYPE_TUBE_HANDLER, VinagreTubeHandlerClass))

typedef struct _VinagreTubeHandler VinagreTubeHandler;
typedef struct _VinagreTubeHandlerClass VinagreTubeHandlerClass;


struct _VinagreTubeHandler
{
  GObject parent_instance;
};

struct _VinagreTubeHandlerClass
{
  GObjectClass parent_class;

  void (* disconnected) (VinagreTubeHandler *htube);
};

GType vinagre_tube_handler_get_type (void) G_GNUC_CONST;
VinagreTubeHandler* vinagre_tube_handler_new (VinagreWindow *vinagre_window,
    TpChannel *channel);

G_END_DECLS

#endif