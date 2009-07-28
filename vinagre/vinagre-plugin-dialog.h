/*
 * vinagre-plugins-dialog.h
 * This file is part of vinagre
 *
 * Copyright (C) 2009 Jorge Pereira <jorge@jorgepereira.com.br>
 * 
 * vinagre-plugins-dialog.h is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * vinagre-plugins-dialog.h is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __VINAGRE_PLUGIN_DIALOG_H__
#define __VINAGRE_PLUGIN_DIALOG_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define VINAGRE_TYPE_PLUGIN_DIALOG              (vinagre_plugin_dialog_get_type())
#define VINAGRE_PLUGIN_DIALOG(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), VINAGRE_TYPE_PLUGIN_DIALOG, VinagrePluginDialog))
#define VINAGRE_PLUGIN_DIALOG_CONST(obj)        (G_TYPE_CHECK_INSTANCE_CAST((obj), VINAGRE_TYPE_PLUGIN_DIALOG, VinagrePluginDialog const))
#define VINAGRE_PLUGIN_DIALOG_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass),  VINAGRE_TYPE_PLUGIN_DIALOG, VinagrePluginDialogClass))
#define VINAGRE_IS_PLUGIN_DIALOG(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), VINAGRE_TYPE_PLUGIN_DIALOG))
#define VINAGRE_IS_PLUGIN_DIALOG_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), VINAGRE_TYPE_PLUGIN_DIALOG))
#define VINAGRE_PLUGIN_DIALOG_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj),  VINAGRE_TYPE_PLUGIN_DIALOG, VinagrePluginDialogClass))

typedef struct _VinagrePluginDialogPrivate VinagrePluginDialogPrivate;
typedef struct _VinagrePluginDialog VinagrePluginDialog;
typedef struct _VinagrePluginDialogClass VinagrePluginDialogClass;

struct _VinagrePluginDialog 
{
  GtkDialog dialog;
  VinagrePluginDialogPrivate *priv;
};

struct _VinagrePluginDialogClass 
{
  GtkDialogClass parent_class;
};

GType	vinagre_plugin_dialog_get_type	(void) G_GNUC_CONST;

void	vinagre_plugin_dialog_show	(GtkWindow *parent);

G_END_DECLS

#endif /* __VINAGRE_PLUGIN_DIALOG_H__ */
/* vim: set ts=8: */
