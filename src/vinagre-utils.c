/*
 * vinagre-utils.c
 * This file is part of vinagre
 *
 * Copyright (C) 2007,2008 - Jonh Wendell <wendell@bani.com.br>
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

#include <string.h>
#include "vinagre-utils.h"

#define VINAGRE_GLADE_FILE  "vinagre.glade"
#define VINAGRE_UI_XML_FILE "vinagre-ui.xml"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

GtkWidget *
vinagre_utils_create_small_close_button ()
{
  GtkRcStyle *rcstyle;
  GtkWidget *image;
  GtkWidget *close_button;

  close_button = gtk_button_new ();
  gtk_button_set_relief (GTK_BUTTON (close_button),
			 GTK_RELIEF_NONE);
  /* don't allow focus on the close button */
  gtk_button_set_focus_on_click (GTK_BUTTON (close_button), FALSE);

  /* make it as small as possible */
  rcstyle = gtk_rc_style_new ();
  rcstyle->xthickness = rcstyle->ythickness = 0;
  gtk_widget_modify_style (close_button, rcstyle);
  gtk_rc_style_unref (rcstyle),

  image = gtk_image_new_from_stock (GTK_STOCK_CLOSE,
				    GTK_ICON_SIZE_MENU);
  gtk_container_add (GTK_CONTAINER (close_button), image);

  return close_button;
}

void
vinagre_utils_show_error (const gchar *message, GtkWindow *parent)
{
  GtkWidget *d;

  d = gtk_message_dialog_new (parent,
			      GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			      GTK_MESSAGE_ERROR,
			      GTK_BUTTONS_CLOSE,
			      message);

  g_signal_connect_swapped (d,
			    "response", 
			    G_CALLBACK (gtk_widget_destroy),
			    d);
  gtk_widget_show_all (GTK_WIDGET(d));
}

void
vinagre_utils_toggle_widget_visible (GtkWidget *widget)
{
  if (GTK_WIDGET_VISIBLE (widget))
    gtk_widget_hide (widget);
  else
    gtk_widget_show_all (widget);
}

const gchar *
vinagre_utils_get_glade_filename (void)
{
  if (g_file_test (VINAGRE_GLADE_FILE, G_FILE_TEST_EXISTS))
    return VINAGRE_GLADE_FILE;
  else
    return VINAGRE_DATADIR "/" VINAGRE_GLADE_FILE;
}

const gchar *
vinagre_utils_get_ui_xml_filename (void)
{
  if (g_file_test (VINAGRE_UI_XML_FILE, G_FILE_TEST_EXISTS))
    return VINAGRE_UI_XML_FILE;
  else
    return VINAGRE_DATADIR "/" VINAGRE_UI_XML_FILE;
}

/*
 * Doubles underscore to avoid spurious menu accels.
 */
gchar * 
vinagre_utils_escape_underscores (const gchar* text,
				  gssize       length)
{
	GString *str;
	const gchar *p;
	const gchar *end;

	g_return_val_if_fail (text != NULL, NULL);

	if (length < 0)
		length = strlen (text);

	str = g_string_sized_new (length);

	p = text;
	end = text + length;

	while (p != end)
	{
		const gchar *next;
		next = g_utf8_next_char (p);

		switch (*p)
		{
			case '_':
				g_string_append (str, "__");
				break;
			default:
				g_string_append_len (str, p, next - p);
				break;
		}

		p = next;
	}

	return g_string_free (str, FALSE);
}

void
vinagre_utils_show_many_errors (const gchar *message, GSList *items, GtkWindow *parent)
{
  GString *msg;
  GSList  *l;

  msg = g_string_new (message);
  g_string_append_c (msg, '\n');

  for (l = items; l; l = l->next)
    g_string_append_printf (msg, "\n%s", (gchar *)l->data);

  vinagre_utils_show_error (msg->str, parent);
  g_string_free (msg, TRUE);
}
/* vim: ts=8 */
