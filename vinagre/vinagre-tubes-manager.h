/*
 * vinagre-tubes-manager.h
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

#ifndef __VINAGRE_TUBES_MANAGER_H__
#define __VINAGRE_TUBES_MANAGER_H__

#include <glib-object.h>

#include "vinagre-window.h"

G_BEGIN_DECLS

#define VINAGRE_TYPE_TUBES_MANAGER (vinagre_tubes_manager_get_type())
#define VINAGRE_TUBES_MANAGER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj),\
    VINAGRE_TYPE_TUBES_MANAGER, VinagreTubesManager))
#define VINAGRE_IS_TUBES_MANAGER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj),\
    VINAGRE_TYPE_TUBES_MANAGER))
#define VINAGRE_TUBES_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
    VINAGRE_TYPE_TUBES_MANAGER, VinagreTubesManagerClass))
#define VINAGRE_IS_TUBES_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE \
    ((klass), VINAGRE_TYPE_TUBES_MANAGER))
#define VINAGRE_TUBES_MANAGER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS \
    ((obj), VINAGRE_TYPE_TUBES_MANAGER, VinagreTubesManagerClass))

typedef struct _VinagreTubesManager VinagreTubesManager;
typedef struct _VinagreTubesManagerClass VinagreTubesManagerClass;


struct _VinagreTubesManager
{
  GObject parent_instance;
};

struct _VinagreTubesManagerClass
{
  GObjectClass parent_class;
};

GType vinagre_tubes_manager_get_type (void) G_GNUC_CONST;
VinagreTubesManager* vinagre_tubes_manager_new (VinagreWindow
    *vinagre_window);

G_END_DECLS

#endif