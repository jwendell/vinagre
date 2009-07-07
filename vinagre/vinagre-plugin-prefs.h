/*
 * vinagre-plugin-prefs.h
 * This file is part of vinagre
 *
 * Copyright (C) 2009 Jorge Pereira <jorge@jorgepereira.com.br>
 * 
 * vinagre-plugin-prefs.h is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * vinagre-plugin-prefs.h is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _VINAGRE_PLUGIN_PREFS_H_
#define _VINAGRE_PLUGIN_PREFS_H_

#include <glib-object.h>
#include "vinagre-window.h"

G_BEGIN_DECLS

#define VINAGRE_TYPE_PLUGIN_PREFS             (vinagre_plugin_prefs_get_type ())
#define VINAGRE_PLUGIN_PREFS(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), VINAGRE_TYPE_PLUGIN_PREFS, VinagrePluginPrefs))
#define VINAGRE_PLUGIN_PREFS_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), VINAGRE_TYPE_PLUGIN_PREFS, VinagrePluginPrefsClass))
#define VINAGRE_IS_PLUGIN_PREFS(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), VINAGRE_TYPE_PLUGIN_PREFS))
#define VINAGRE_IS_PLUGIN_PREFS_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), VINAGRE_TYPE_PLUGIN_PREFS))
#define VINAGRE_PLUGIN_PREFS_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), VINAGRE_TYPE_PLUGIN_PREFS, VinagrePluginPrefsClass))

typedef struct _VinagrePluginPrefsClass VinagrePluginPrefsClass;
typedef struct _VinagrePluginPrefs VinagrePluginPrefs;
typedef struct _VinagrePluginPrefsPrivate VinagrePluginPrefsPrivate;

struct _VinagrePluginPrefsClass
{
  GObjectClass parent_class;
};

struct _VinagrePluginPrefs
{
  GObject parent_instance;
  VinagrePluginPrefsPrivate *priv;
};

GType vinagre_plugin_prefs_get_type (void) G_GNUC_CONST;

VinagrePluginPrefs	*vinagre_plugin_prefs_get_default (void);

void		vinagre_plugin_prefs_dialog_show (VinagreWindow *window);
G_END_DECLS

#endif /* _VINAGRE_PLUGIN_PREFS_H_ */
/* vim: set ts=8: */
