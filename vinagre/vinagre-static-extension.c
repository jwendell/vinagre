/*
 * vinagre-static-extension.c
 * This file is part of vinagre
 *
 * vinagre-static-extension.c is free software: you can redistribute it and/or modify it
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

#include "vinagre-static-extension.h"

G_DEFINE_TYPE (VinagreStaticExtension, vinagre_static_extension, G_TYPE_OBJECT);

static void
vinagre_static_extension_class_init (VinagreStaticExtensionClass * klass)
{
    vinagre_static_extension_parent_class = g_type_class_peek_parent (klass);
}

static void
vinagre_static_extension_init (VinagreStaticExtension * self)
{
}
