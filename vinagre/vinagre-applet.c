/*
 * vinagre-applet.c
 * This file is part of vinagre
 *
 * Copyright (C) 2008,2009 - Jonh Wendell <wendell@bani.com.br>
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
#include <glib.h>
#include <gtk/gtk.h>
#include <locale.h>
#include <glib/gi18n.h>
#include <panel-applet.h>

#include "vinagre-bookmarks.h"
#include "vinagre-bookmarks-entry.h"
#include "vinagre-utils.h"
#include "vinagre-connection.h"
#include "vinagre-commands.h"
#include "vinagre-plugins-engine.h"

#include <config.h>

#ifdef VINAGRE_HAVE_AVAHI
#include "vinagre-mdns.h"
#endif

#define VINAGRE_TYPE_APPLET		(vinagre_applet_get_type ())
#define VINAGRE_APPLET(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), VINAGRE_TYPE_APPLET, VinagreApplet))
#define VINAGRE_APPLET_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), VINAGRE_TYPE_APPLET, VinagreAppletClass))
#define VINAGRE_IS_APPLET(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), VINAGRE_TYPE_APPLET))
#define VINAGRE_IS_APPLET_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), VINAGRE_TYPE_APPLET))
#define VINAGRE_APPLET_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), VINAGRE_TYPE_APPLET, VinagreAppletClass))

static const gchar VINAGRE_APPLET_FACTORY_ID[] = "VinagreAppletFactory";
static const gchar VINAGRE_APPLET_ID[] = "VinagreApplet";
#define PANEL_APPLET_VERTICAL(p)	 (((p) == PANEL_APPLET_ORIENT_LEFT) || ((p) == PANEL_APPLET_ORIENT_RIGHT))

typedef struct{
    PanelApplet parent;
    GtkWidget *icon;
    gint icon_width, icon_height, size;
} VinagreApplet;

typedef struct{
    PanelAppletClass parent_class;
} VinagreAppletClass;

GType vinagre_applet_get_type (void);
static void vinagre_applet_class_init (VinagreAppletClass *klass);
static void vinagre_applet_init (VinagreApplet *applet);

static void vinagre_applet_get_icon (VinagreApplet *applet);
static void vinagre_applet_destroy_cb (GtkWidget *widget);

G_DEFINE_TYPE (VinagreApplet, vinagre_applet, PANEL_TYPE_APPLET)

static void
vinagre_applet_get_icon (VinagreApplet *applet)
{
    GdkPixbuf *pixbuf;

    if(applet->icon != NULL)
    {
        g_object_unref(applet->icon);
        applet->icon = NULL;
    }

    if(applet->size <= 2)
        return;

    pixbuf = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(),
        PACKAGE_TARNAME, applet->size - 2, 0, NULL);

    applet->icon_height = gdk_pixbuf_get_height(pixbuf);
    applet->icon_width = gdk_pixbuf_get_width(pixbuf);
    applet->icon = gtk_image_new_from_pixbuf(pixbuf);
}

static void
vinagre_applet_destroy_cb (GtkWidget *widget)
{
  VinagreApplet *applet = VINAGRE_APPLET (widget);

  if(applet->icon != NULL)
      g_object_unref (applet->icon);
}

static void
vinagre_applet_class_init (VinagreAppletClass *klass)
{
  /* nothing to do here */
}

static void
menu_position (GtkMenu    *menu,
	       gint       *x,
	       gint       *y,
	       gboolean   *push_in,
	       GtkWidget  *applet)
{
  int applet_height, applet_width;
  GtkRequisition requisition;
  GdkWindow *window = gtk_widget_get_window (applet);

  gdk_window_get_origin (window, x, y);
  applet_width = gdk_window_get_width(window);
  applet_height = gdk_window_get_height(window);
  gtk_widget_size_request (GTK_WIDGET (menu), &requisition);

  switch (panel_applet_get_orient (PANEL_APPLET (applet)))
    {
      case PANEL_APPLET_ORIENT_DOWN:
	*y += applet_height + 1;
	break;

      case PANEL_APPLET_ORIENT_UP:
	*y = MAX (*y - requisition.height - 2, 2*applet_height);
	break;

      case PANEL_APPLET_ORIENT_LEFT:
	*x = MAX (*x - requisition.width - 2, 2*applet_width);
	break;

      case PANEL_APPLET_ORIENT_RIGHT:
	*x += applet_height + 1;
	break;

      default:
	g_assert_not_reached ();
    }

  *push_in = TRUE;
}

static void
open_connection_cb (GtkMenuItem *item,
    gpointer    user_data)
{
    GAppInfo *info;
    GdkAppLaunchContext *context;
    GError *error = NULL;
    VinagreConnection *conn;
    gchar *commandline;
    static const gchar application_name[] = PACKAGE_TARNAME;
  
    conn = VINAGRE_CONNECTION (g_object_get_data (G_OBJECT (item), "conn"));
  
    commandline = vinagre_connection_get_string_rep (conn, TRUE);

    info = g_app_info_create_from_commandline(commandline, application_name,
        G_APP_INFO_CREATE_SUPPORTS_STARTUP_NOTIFICATION, &error);
    context = gdk_display_get_app_launch_context(gdk_display_get_default());
    g_app_info_launch(info, NULL, G_APP_LAUNCH_CONTEXT(context), &error);
  
    if(error)
    {
        vinagre_utils_show_error (_("Could not run vinagre:"), error->message,
            NULL);
        g_error_free(error);
    }
 
    g_free(commandline);
}

static void
fill_recursive_menu (GSList *entries, GtkWidget *menu)
{
  GSList    *l;
  GtkWidget *item, *image, *child;

  for (l = entries; l; l = l->next)
    {
      VinagreBookmarksEntry *entry = VINAGRE_BOOKMARKS_ENTRY (l->data);
      VinagreConnection     *conn;
      VinagreProtocol *ext;

      switch (vinagre_bookmarks_entry_get_node (entry))
	{
	  case VINAGRE_BOOKMARKS_ENTRY_NODE_FOLDER:
	    image = gtk_image_new_from_icon_name ("folder", GTK_ICON_SIZE_MENU);
	    item = gtk_image_menu_item_new_with_label (vinagre_bookmarks_entry_get_name (entry));
	    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item),
					   image);
	    gtk_menu_shell_append (GTK_MENU_SHELL (menu),
				   item);

	    child = gtk_menu_new ();
	    gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), child);

	    fill_recursive_menu (vinagre_bookmarks_entry_get_children (entry),
				 child);
	    break;

	  case VINAGRE_BOOKMARKS_ENTRY_NODE_CONN:
	    conn = vinagre_bookmarks_entry_get_conn (entry);
	    ext = vinagre_plugins_engine_get_plugin_by_protocol (vinagre_plugins_engine_get_default (),
								 vinagre_connection_get_protocol (conn));

	    image = gtk_image_new_from_icon_name (vinagre_protocol_get_icon_name (ext),
						  GTK_ICON_SIZE_MENU);
	    item = gtk_image_menu_item_new_with_label (vinagre_connection_get_name (conn));
	    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item),
					   image);

	    gtk_menu_shell_append (GTK_MENU_SHELL (menu),
				   item);
	    g_object_set_data (G_OBJECT (item), "conn", conn);
	    g_signal_connect (item, "activate", G_CALLBACK (open_connection_cb), NULL);
	    break;

	  default:
	    g_assert_not_reached ();
	}
    }

}

static void
fill_menu (GSList *all, GtkWidget *menu)
{
  GtkWidget *item;

  if (g_slist_length (all) == 0)
    return;

  /* Separator */
  item = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

  fill_recursive_menu (all, menu);
}

static void
open_vinagre_cb (GtkMenuItem *item,
		 gpointer    *user_data)
{
  GError *err = NULL;

  if (!g_spawn_command_line_async ("vinagre", &err))
    {
      vinagre_utils_show_error (_("Could not run vinagre:"), err->message, NULL);
      g_error_free (err);
    }
}

static gboolean
vinagre_applet_click_cb (GtkWidget      *applet,
			 GdkEventButton *event,
			 gpointer        user_data)
{
  GtkWidget *menu, *item, *image;
  GSList *all;

  if ((event->type != GDK_BUTTON_PRESS) || (event->button != 1))
    return FALSE;

  menu = gtk_menu_new ();

  /* Open, first item */
  image = gtk_image_new_from_icon_name ("vinagre", GTK_ICON_SIZE_MENU);
  item = gtk_image_menu_item_new_with_label (_("Open Remote Desktop Viewer"));
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item),
				 image);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
  g_signal_connect (item, "activate", G_CALLBACK (open_vinagre_cb), NULL);

  all = vinagre_bookmarks_get_all (vinagre_bookmarks_get_default ());
  fill_menu (all, menu);

#ifdef VINAGRE_HAVE_AVAHI
  all = vinagre_mdns_get_all (vinagre_mdns_get_default ());
  fill_menu (all, menu);
#endif

  gtk_widget_show_all (menu);
  gtk_menu_popup (GTK_MENU (menu), NULL, NULL, (GtkMenuPositionFunc) menu_position, applet, 
		  event->button, event->time);
  return TRUE;
}

static void
vinagre_applet_help_cb (GtkAction *action,
    gpointer data)
{
    vinagre_utils_help_contents (NULL, NULL);
}

static void
vinagre_applet_about_cb (GtkAction *action,
    gpointer data)
{
    vinagre_utils_help_about (NULL);
}


static void
vinagre_applet_init (VinagreApplet *applet)
{
    GtkActionGroup *action_group;
    gchar *ui_path;
    gchar *tooltip;
#ifdef VINAGRE_HAVE_AVAHI
    VinagreMdns *mdns;
#endif

    static const GtkActionEntry menu_actions[] = {
        { "VinagreHelp", GTK_STOCK_HELP, N_("_Help"), NULL, NULL, G_CALLBACK(vinagre_applet_help_cb) },
        { "VinagreAbout", GTK_STOCK_ABOUT, N_("_About"), NULL, NULL, G_CALLBACK(vinagre_applet_about_cb) }
    };

    gtk_window_set_default_icon_name (PACKAGE_TARNAME);
    panel_applet_set_flags (PANEL_APPLET(applet), PANEL_APPLET_EXPAND_MINOR);
    g_set_application_name (_("Remote Desktop Viewer"));

    action_group = gtk_action_group_new("Vinagre Applet Actions");
    gtk_action_group_set_translation_domain(action_group, GETTEXT_PACKAGE);
    gtk_action_group_add_actions(action_group, menu_actions,
        G_N_ELEMENTS(menu_actions), applet);
    ui_path = g_build_filename(VINAGRE_DATADIR, "VinagreApplet.xml", NULL);
    panel_applet_setup_menu_from_file(PANEL_APPLET(applet), ui_path,
        action_group);
    g_free(ui_path);
    g_object_unref(action_group);

    tooltip = g_strdup_printf("%s\n%s", _("Remote Desktop Viewer"),
        _("Access your bookmarks"));
    gtk_widget_set_tooltip_text(GTK_WIDGET(applet), tooltip);
    g_free(tooltip);

    panel_applet_set_background_widget(PANEL_APPLET(applet),
        GTK_WIDGET(applet));

    /* TODO: Replace with gtk_widget_get_allocation(). */
    applet->size = panel_applet_get_size(PANEL_APPLET(applet));
    applet->icon = NULL;
    vinagre_applet_get_icon(applet);

    /* TODO: Deal with theme changes. */
    gtk_container_add(GTK_CONTAINER(applet), applet->icon);

    g_signal_connect (G_OBJECT (applet), "button-press-event",
        G_CALLBACK (vinagre_applet_click_cb), NULL);

    g_signal_connect (G_OBJECT (applet), "destroy",
        G_CALLBACK (vinagre_applet_destroy_cb), NULL);

#ifdef VINAGRE_HAVE_AVAHI
    mdns = vinagre_mdns_get_default();
#endif

    gtk_widget_show_all(GTK_WIDGET(applet));
}

static gboolean
vinagre_applet_start(VinagreApplet *applet)
{
    gtk_widget_show(GTK_WIDGET(applet));

    return TRUE;
}

static gboolean
vinagre_applet_factory (VinagreApplet *applet,
    const gchar *iid,
    gpointer data)
{
    gboolean retval = FALSE;

    if(!g_strcmp0 (iid, VINAGRE_APPLET_ID))
        retval = vinagre_applet_start(applet);

    return retval;
}

PANEL_APPLET_OUT_PROCESS_FACTORY (VINAGRE_APPLET_FACTORY_ID,
    VINAGRE_TYPE_APPLET,
    (PanelAppletFactoryCallback) vinagre_applet_factory,
    NULL);
