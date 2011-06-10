/*
 * vinagre-reverse-vnc-listener.h
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

#ifndef VINAGRE_REVERSE_VNC_LISTENER_H_
#define VINAGRE_REVERSE_VNC_LISTENER_H_

#include <glib.h>

G_BEGIN_DECLS

#define VINAGRE_TYPE_REVERSE_VNC_LISTENER             (vinagre_reverse_vnc_listener_get_type ())
#define VINAGRE_REVERSE_VNC_LISTENER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), VINAGRE_TYPE_REVERSE_VNC_LISTENER, VinagreReverseVncListener))
#define VINAGRE_REVERSE_VNC_LISTENER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), VINAGRE_TYPE_REVERSE_VNC_LISTENER, VinagreReverseVncListenerClass))
#define VINAGRE_IS_REVERSE_VNC_LISTENER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), VINAGRE_TYPE_REVERSE_VNC_LISTENER))
#define VINAGRE_IS_REVERSE_VNC_LISTENER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), VINAGRE_TYPE_REVERSE_VNC_LISTENER))
#define VINAGRE_REVERSE_VNC_LISTENER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), VINAGRE_TYPE_REVERSE_VNC_LISTENER, VinagreReverseVncListenerClass))

typedef struct _VinagreReverseVncListenerClass   VinagreReverseVncListenerClass;
typedef struct _VinagreReverseVncListener        VinagreReverseVncListener;
typedef struct _VinagreReverseVncListenerPrivate VinagreReverseVncListenerPrivate;

struct _VinagreReverseVncListenerClass
{
  GObjectClass parent_class;
};

struct _VinagreReverseVncListener
{
  GObject parent_instance;
  VinagreReverseVncListenerPrivate *priv;
};


GType vinagre_reverse_vnc_listener_get_type (void) G_GNUC_CONST;

VinagreReverseVncListener*	vinagre_reverse_vnc_listener_get_default (void);
void			vinagre_reverse_vnc_listener_start (VinagreReverseVncListener *listener);
void			vinagre_reverse_vnc_listener_stop  (VinagreReverseVncListener *listener);
gboolean		vinagre_reverse_vnc_listener_is_listening (VinagreReverseVncListener *listener);
gint			vinagre_reverse_vnc_listener_get_port (VinagreReverseVncListener *listener);
void			vinagre_reverse_vnc_listener_set_window (VinagreReverseVncListener *listener,
								 VinagreWindow *window);

G_END_DECLS

#endif /* VINAGRE_REVERSE_VNC_LISTENER_H_  */
