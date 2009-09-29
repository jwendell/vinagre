/*
 * vinagre-vnc-listener.h
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

#ifndef __VINAGRE_VNC_LISTENER_H__
#define __VINAGRE_VNC_LISTENER_H__

#include <glib.h>

G_BEGIN_DECLS

#define VINAGRE_TYPE_VNC_LISTENER             (vinagre_vnc_listener_get_type ())
#define VINAGRE_VNC_LISTENER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), VINAGRE_TYPE_VNC_LISTENER, VinagreVncListener))
#define VINAGRE_VNC_LISTENER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), VINAGRE_TYPE_VNC_LISTENER, VinagreVncListenerClass))
#define VINAGRE_IS_VNC_LISTENER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), VINAGRE_TYPE_VNC_LISTENER))
#define VINAGRE_IS_VNC_LISTENER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), VINAGRE_TYPE_VNC_LISTENER))
#define VINAGRE_VNC_LISTENER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), VINAGRE_TYPE_VNC_LISTENER, VinagreVncListenerClass))

typedef struct _VinagreVncListenerClass   VinagreVncListenerClass;
typedef struct _VinagreVncListener        VinagreVncListener;
typedef struct _VinagreVncListenerPrivate VinagreVncListenerPrivate;

struct _VinagreVncListenerClass
{
  GObjectClass parent_class;
};

struct _VinagreVncListener
{
  GObject parent_instance;
  VinagreVncListenerPrivate *priv;
};


GType vinagre_vnc_listener_get_type (void) G_GNUC_CONST;

VinagreVncListener*	vinagre_vnc_listener_get_default (void);
void			vinagre_vnc_listener_start (VinagreVncListener *listener);
void			vinagre_vnc_listener_stop  (VinagreVncListener *listener);
gboolean		vinagre_vnc_listener_is_listening (VinagreVncListener *listener);
gint			vinagre_vnc_listener_get_port (VinagreVncListener *listener);

G_END_DECLS

#endif /* __VINAGRE_VNC_LISTENER_H__  */
/* vim: set ts=8: */
