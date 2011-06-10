/*
 * vinagre-static-extension.h
 * This file is part of vinagre
 *
 * vinagre-static-extension.h is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * vinagre-static-extension.h is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef VINAGRE_STATIC_EXTENSION_H_
#define VINAGRE_STATIC_EXTENSION_H_

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define VINAGRE_TYPE_STATIC_EXTENSION (vinagre_static_extension_get_type ())
#define VINAGRE_STATIC_EXTENSION(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), VINAGRE_TYPE_STATIC_EXTENSION, VinagreStaticExtension))
#define VINAGRE_STATIC_EXTENSION_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), VINAGRE_TYPE_STATIC_EXTENSION, VinagreStaticExtensionClass))
#define VINAGRE_IS_STATIC_EXTENSION(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), VINAGRE_TYPE_STATIC_EXTENSION))
#define VINAGRE_IS_STATIC_EXTENSION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), VINAGRE_TYPE_STATIC_EXTENSION))
#define VINAGRE_STATIC_EXTENSION_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), VINAGRE_TYPE_STATIC_EXTENSION, VinagreStaticExtensionClass))

typedef struct _VinagreStaticExtension VinagreStaticExtension;
typedef struct _VinagreStaticExtensionClass VinagreStaticExtensionClass;
typedef struct _VinagreStaticExtensionPrivate VinagreStaticExtensionPrivate;

struct _VinagreStaticExtension {
    GObject parent_instance;
    VinagreStaticExtensionPrivate * priv;
};

struct _VinagreStaticExtensionClass {
    GObjectClass parent_class;
};

GType vinagre_static_extension_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* VINAGRE_STATIC_EXTENSION_H_ */
