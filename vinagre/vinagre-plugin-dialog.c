/*
 * vinagre-plugins-dialog.c
 * This file is part of vinagre
 *
 * Copyright (C) 2009 Jorge Pereira <jorge@jorgepereira.com.br>
 * 
 * vinagre-plugins-dialog.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * vinagre-plugins-dialog.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include <glib/gi18n.h>
#include <gconf/gconf-client.h>

#include "vinagre-plugin-dialog.h"
#include "vinagre-debug.h"

static GtkWidget *plugin_dialog = NULL;

#define VINAGRE_PLUGIN_DIALOG_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), \
						     VINAGRE_TYPE_PLUGIN_DIALOG, \
						     VinagrePluginDialogPrivate))

struct _VinagrePluginDialogPrivate
{
	GtkWidget	*plugin_box;
};

G_DEFINE_TYPE(VinagrePluginDialog, vinagre_plugin_dialog, GTK_TYPE_DIALOG)

static void 
vinagre_plugin_dialog_class_init (VinagrePluginDialogClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (VinagrePluginDialogPrivate));
}

static void
dialog_response_handler (GtkDialog *dlg, 
	                  		 gint       res_id)
{
	vinagre_debug (DEBUG_PREFS);

	switch (res_id)
	{
    case GTK_RESPONSE_CLOSE:
		default:
			gtk_widget_destroy (GTK_WIDGET(dlg));
	}
}

static void
setup_plugins_page (VinagrePluginDialog *dlg)
{
	GtkWidget *page_content;

  vinagre_debug (DEBUG_PREFS);

	page_content = GTK_WIDGET (vinagre_plugin_manager_new ());
	g_return_if_fail (page_content != NULL);

  gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dlg)->vbox), page_content);

	gtk_widget_show_all (page_content);
}

static void
vinagre_plugin_dialog_init (VinagrePluginDialog *dlg)
{
	GtkWidget *error_widget;
	
	vinagre_debug (DEBUG_PREFS);

	dlg->priv = VINAGRE_PLUGIN_DIALOG_GET_PRIVATE (dlg);
	gtk_dialog_add_buttons (GTK_DIALOG (dlg),
				GTK_STOCK_CLOSE,
				GTK_RESPONSE_CLOSE,
				NULL);

	gtk_window_set_title (GTK_WINDOW (dlg), _("Plugin Manager"));
	gtk_window_set_resizable (GTK_WINDOW (dlg), FALSE);
	gtk_dialog_set_has_separator (GTK_DIALOG (dlg), TRUE);
	gtk_window_set_destroy_with_parent (GTK_WINDOW (dlg), TRUE);

	g_signal_connect (dlg,
			  "response",
			  G_CALLBACK (dialog_response_handler),
			  NULL);

	setup_plugins_page (dlg);
}

void
vinagre_plugin_dialog_show (VinagreWindow *parent)
{
	vinagre_debug (DEBUG_PREFS);

	g_return_if_fail (VINAGRE_IS_WINDOW (parent));

	if (plugin_dialog == NULL)
	{
		plugin_dialog = GTK_WIDGET (g_object_new (VINAGRE_TYPE_PLUGIN_DIALOG, NULL));
		g_signal_connect (plugin_dialog,
				  "destroy",
				  G_CALLBACK (gtk_widget_destroyed),
				  &plugin_dialog);
	}

	if (GTK_WINDOW (parent) != gtk_window_get_transient_for (GTK_WINDOW (plugin_dialog)))
	{
		gtk_window_set_transient_for (GTK_WINDOW (plugin_dialog),
					      GTK_WINDOW (parent));
	}

	gtk_window_present (GTK_WINDOW (plugin_dialog));
}

