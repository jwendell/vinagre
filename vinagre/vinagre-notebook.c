/*
 * vinagre-notebook.c
 * This file is part of vinagre
 *
 * Copyright (C) 2007,2008,2009 - Jonh Wendell <wendell@bani.com.br>
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
 
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n.h>

#include "vinagre-notebook.h"
#include "vinagre-utils.h"
#include "vinagre-dnd.h"
#include "vinagre-prefs.h"
#include "vinagre-spinner.h"

#define VINAGRE_NOTEBOOK_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), VINAGRE_TYPE_NOTEBOOK, VinagreNotebookPrivate))

struct _VinagreNotebookPrivate
{
  VinagreWindow *window;
  GtkUIManager  *manager;
  guint         ui_merge_id;
  VinagreTab    *active_tab;
  GSList        *tabs;
};

/* Properties */
enum
{
  PROP_0,
  PROP_WINDOW
};

G_DEFINE_TYPE(VinagreNotebook, vinagre_notebook, GTK_TYPE_NOTEBOOK)

static void
vinagre_notebook_get_property (GObject    *object,
			       guint       prop_id,
			       GValue     *value,
			       GParamSpec *pspec)
{
  VinagreNotebook *nb = VINAGRE_NOTEBOOK (object);

  switch (prop_id)
    {
      case PROP_WINDOW:
        g_value_set_object (value, nb->priv->window);
	break;
      default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	break;			
    }
}

static void
vinagre_notebook_set_window (VinagreNotebook *nb, VinagreWindow *window)
{
  nb->priv->window = window;
  nb->priv->manager = vinagre_window_get_ui_manager (window);
  nb->priv->ui_merge_id = gtk_ui_manager_new_merge_id (nb->priv->manager);
}

static void
vinagre_notebook_set_property (GObject      *object,
			       guint         prop_id,
			       const GValue *value,
			       GParamSpec   *pspec)
{
  VinagreNotebook *nb = VINAGRE_NOTEBOOK (object);

  switch (prop_id)
    {
      case PROP_WINDOW:
	vinagre_notebook_set_window (nb, VINAGRE_WINDOW (g_value_get_object (value)));
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;			
    }
}

static void
vinagre_notebook_finalize (GObject *object)
{
  VinagreNotebook *nb = VINAGRE_NOTEBOOK (object);

  if (nb->priv->tabs)
    {
      g_slist_free (nb->priv->tabs);
      nb->priv->tabs = NULL;
    }

  G_OBJECT_CLASS (vinagre_notebook_parent_class)->finalize (object);
}

static void
vinagre_notebook_class_init (VinagreNotebookClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = vinagre_notebook_get_property;
  object_class->set_property = vinagre_notebook_set_property;
  object_class->finalize = vinagre_notebook_finalize;

  g_object_class_install_property (object_class,
				   PROP_WINDOW,
				   g_param_spec_object ("window",
							"Window",
							"The VinagreWindow",
							VINAGRE_TYPE_WINDOW,
							G_PARAM_READWRITE |
							G_PARAM_CONSTRUCT_ONLY |
							G_PARAM_STATIC_NAME |
							G_PARAM_STATIC_NICK |
							G_PARAM_STATIC_BLURB));

  g_type_class_add_private (object_class, sizeof(VinagreNotebookPrivate));
}

VinagreNotebook *
vinagre_notebook_new (VinagreWindow *window)
{
  return VINAGRE_NOTEBOOK (g_object_new (VINAGRE_TYPE_NOTEBOOK, "window", window, NULL));
}

void
vinagre_notebook_show_hide_tabs (VinagreNotebook *nb)
{
  gboolean always, fs;
  gint     n;

  fs = vinagre_window_is_fullscreen (nb->priv->window);
  n = gtk_notebook_get_n_pages (GTK_NOTEBOOK (nb));

  g_object_get (vinagre_prefs_get_default (),
		"always-show-tabs", &always,
		NULL);

  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (nb),
			      ((n > 1) || (always)) && !fs);
}

static void
drag_data_get_handl (GtkWidget *widget,
		     GdkDragContext *context,
		     GtkSelectionData *selection_data,
		     guint target_type,
		     guint time,
		     VinagreTab *tab)
{
  gchar *uri, *name, *data;
  VinagreConnection *conn;

  g_assert (selection_data != NULL);

  switch (target_type)
    {
      case TARGET_VINAGRE:
	conn = vinagre_tab_get_conn (tab);
	uri = vinagre_connection_get_string_rep (conn, TRUE);
	name = vinagre_connection_get_best_name (conn);
	data = g_strdup_printf ("%s||%s", name, uri);

	/*FIXME: Set other properties of the connection*/
	gtk_selection_data_set (selection_data,
				selection_data->target,
				8,
				(guchar*) data,
				strlen (data));

	g_free (data);
	g_free (name);
	g_free (uri);
	break;

      case TARGET_STRING:
	/*FIXME: Create a .VNC file*/
	break;

      default:
	g_assert_not_reached ();
    }
}

static void
vinagre_notebook_update_ui_sentitivity (VinagreNotebook *nb)
{
  gboolean       active;
  GtkActionGroup *action_group;

  active = gtk_notebook_get_n_pages (GTK_NOTEBOOK (nb)) > 0;
  action_group = vinagre_window_get_connected_action (nb->priv->window);
  gtk_action_group_set_sensitive (action_group, active);

  action_group = vinagre_window_get_initialized_action (nb->priv->window);
  active = (nb->priv->active_tab) &&
	   (vinagre_tab_get_state (VINAGRE_TAB (nb->priv->active_tab)) == VINAGRE_TAB_STATE_CONNECTED);
  gtk_action_group_set_sensitive (action_group, active);

  if (nb->priv->active_tab)
    {
      GtkWidget *spinner, *icon;

      spinner = g_object_get_data (G_OBJECT (nb->priv->active_tab), "spinner");
      icon = g_object_get_data (G_OBJECT (nb->priv->active_tab), "icon");

      if (vinagre_tab_get_state (VINAGRE_TAB (nb->priv->active_tab)) == VINAGRE_TAB_STATE_CONNECTED)
	{
	  gtk_widget_hide (spinner);
	  vinagre_spinner_stop (VINAGRE_SPINNER (spinner));
	  gtk_widget_show (icon);
	}
      else
	{
	  gtk_widget_hide (icon);
	  gtk_widget_show (spinner);
	  vinagre_spinner_start (VINAGRE_SPINNER (spinner));
	}
    }
}

static void
vinagre_notebook_update_window_title (VinagreNotebook *nb)
{
  gchar *title, *name, *extra;

  if (nb->priv->active_tab == NULL)
    {
      gtk_window_set_title (GTK_WINDOW (nb->priv->window), g_get_application_name ());
      return;
    }

  extra = vinagre_tab_get_extra_title (nb->priv->active_tab);
  name = vinagre_connection_get_best_name (vinagre_tab_get_conn (nb->priv->active_tab));
  if (extra)
    title = g_strdup_printf ("%s %s - %s",
			     name,
			     extra,
			     g_get_application_name ());
  else
    title = g_strdup_printf ("%s - %s",
			     name,
			     g_get_application_name ());

  gtk_window_set_title (GTK_WINDOW (nb->priv->window), title);
  g_free (title);
  g_free (extra);
  g_free (name);
}

static void
insert_actions_ui (VinagreNotebook *nb, GtkActionGroup *action_group, const GSList *actions)
{
  VinagreTabUiAction *action;
  const gchar        *name;
  gint               i;

  while (actions)
    {
      action = (VinagreTabUiAction *) actions->data;
      name = gtk_action_get_name (action->action);

      gtk_action_group_add_action (action_group, action->action);

      for (i = 0; i < g_strv_length (action->paths); i++)
	gtk_ui_manager_add_ui (nb->priv->manager,
			       nb->priv->ui_merge_id,
			       action->paths[i],
			       name,
			       name,
			       GTK_UI_MANAGER_AUTO,
			       FALSE);

      actions = actions->next;
    }
}

static void
merge_tab_ui (VinagreNotebook *nb)
{
  const GSList   *actions;
  GtkActionGroup *action_group;

  if (!nb->priv->active_tab)
    return;

  /* Always sensitive actions */
  action_group = vinagre_window_get_always_sensitive_action (nb->priv->window);
  actions = vinagre_tab_get_always_sensitive_actions (nb->priv->active_tab);
  insert_actions_ui (nb, action_group, actions);

  /* Connected actions */
  action_group = vinagre_window_get_connected_action (nb->priv->window);
  actions = vinagre_tab_get_connected_actions (nb->priv->active_tab);
  insert_actions_ui (nb, action_group, actions);

  /* Initialized actions */
  action_group = vinagre_window_get_initialized_action (nb->priv->window);
  actions = vinagre_tab_get_initialized_actions (nb->priv->active_tab);
  insert_actions_ui (nb, action_group, actions);
}

static void
remove_actions_ui (VinagreNotebook *nb, GtkActionGroup *action_group, const GSList *actions)
{
  VinagreTabUiAction *action;

  while (actions)
    {
      action = (VinagreTabUiAction *) actions->data;

      gtk_action_group_remove_action (action_group, action->action);
      actions = actions->next;
    }
}

static void
unmerge_tab_ui (VinagreNotebook *nb)
{
  const GSList   *actions;
  GtkActionGroup *action_group;

  if (!nb->priv->active_tab)
    return;

  gtk_ui_manager_remove_ui (nb->priv->manager,
			    nb->priv->ui_merge_id);

  /* Always sensitive actions */
  action_group = vinagre_window_get_always_sensitive_action (nb->priv->window);
  actions = vinagre_tab_get_always_sensitive_actions (nb->priv->active_tab);
  remove_actions_ui (nb, action_group, actions);

  /* Connected actions */
  action_group = vinagre_window_get_connected_action (nb->priv->window);
  actions = vinagre_tab_get_connected_actions (nb->priv->active_tab);
  remove_actions_ui (nb, action_group, actions);

  /* Initialized actions */
  action_group = vinagre_window_get_initialized_action (nb->priv->window);
  actions = vinagre_tab_get_initialized_actions (nb->priv->active_tab);
  remove_actions_ui (nb, action_group, actions);
}

static void 
vinagre_notebook_page_switched (GtkNotebook     *notebook,
				GtkNotebookPage *pg,
				gint            page_num, 
				gpointer        data)
{
  VinagreNotebook *nb = VINAGRE_NOTEBOOK (notebook);
  VinagreTab *tab;

  tab = VINAGRE_TAB (gtk_notebook_get_nth_page (notebook, page_num));
  if (tab == nb->priv->active_tab)
    return;

  unmerge_tab_ui (nb);
  nb->priv->active_tab = tab;
  merge_tab_ui (nb);

  vinagre_notebook_update_window_title (nb);
  vinagre_notebook_update_ui_sentitivity (nb);
}

static void
vinagre_notebook_page_removed_cb (VinagreNotebook *nb)
{
  gint n;

  if ((gtk_notebook_get_n_pages (GTK_NOTEBOOK (nb)) == 0) &&
      (vinagre_window_is_fullscreen (nb->priv->window)))
    vinagre_window_toggle_fullscreen (nb->priv->window);

vinagre_notebook_show_hide_tabs (nb);
}

static void
vinagre_notebook_init (VinagreNotebook *nb)
{
  nb->priv = VINAGRE_NOTEBOOK_GET_PRIVATE (nb);
  nb->priv->active_tab = NULL;
  nb->priv->tabs = NULL;

  gtk_notebook_set_scrollable (GTK_NOTEBOOK (nb), TRUE);

  g_signal_connect (nb,
		    "page-added",
		    G_CALLBACK (vinagre_notebook_show_hide_tabs),
		    NULL);
  g_signal_connect (nb,
		    "page-removed",
		    G_CALLBACK (vinagre_notebook_page_removed_cb),
		    NULL);
  g_signal_connect (nb,
		    "switch-page",
		    G_CALLBACK (vinagre_notebook_page_switched),
		    NULL);
  g_signal_connect_swapped (vinagre_prefs_get_default (),
			    "notify::always-show-tabs",
			     G_CALLBACK (vinagre_notebook_show_hide_tabs),
			     nb);
}

static void
close_button_clicked_cb (GtkWidget *widget, 
			 GtkWidget *tab)
{
  VinagreNotebook *notebook;

  notebook = VINAGRE_NOTEBOOK (gtk_widget_get_parent (tab));
  vinagre_notebook_close_tab (notebook, VINAGRE_TAB (tab));
}

static void
vinagre_notebook_update_tab_tooltip (VinagreNotebook *nb, VinagreTab *tab)
{
  char       *str;
  GtkWidget  *label;

  label = GTK_WIDGET (g_object_get_data (G_OBJECT (tab), "label-ebox"));
  g_return_if_fail (label != NULL);

  str = vinagre_tab_get_tooltip (tab);
  gtk_widget_set_tooltip_markup (label, str);

  g_free (str);
}

static void
tab_tooltip_changed_cb (GObject *object, GParamSpec *pspec, VinagreNotebook *nb)
{
  vinagre_notebook_update_tab_tooltip (nb, VINAGRE_TAB (object));
}

static void
tab_disconnected_cb (VinagreTab *tab, VinagreNotebook *nb)
{
  gchar *message, *name, *emphasis;

  name = vinagre_connection_get_best_name (vinagre_tab_get_conn (tab));
  emphasis = g_strdup_printf ("<i>%s</i>", name);
  /* Translators: %s is a host name or IP address. */
  message = g_strdup_printf (_("Connection to host %s was closed."),
			     emphasis);
  vinagre_utils_show_error (_("Connection closed"), message, GTK_WINDOW (nb->priv->window));
  g_free (message);
  g_free (name);
  g_free (emphasis);

  vinagre_notebook_close_tab (nb, tab);
}

static void
tab_auth_failed_cb (VinagreTab *tab, const gchar *msg, VinagreNotebook *nb)
{
  GString *message;
  gchar   *name, *emphasis;

  message = g_string_new (NULL);
  name = vinagre_connection_get_best_name (vinagre_tab_get_conn (tab));

  emphasis = g_strdup_printf ("<i>%s</i>", name);
  /* Translators: %s is a host name or IP address. */
  g_string_printf (message, _("Authentication to host %s has failed"),
		   emphasis);
  if (msg)
  	g_string_append_printf (message, " (%s)", msg);
  g_string_append_c (message, '.');

  vinagre_utils_show_error (_("Authentication failed"), message->str, GTK_WINDOW (nb->priv->window));
  g_string_free (message, TRUE);
  g_free (name);
  g_free (emphasis);

  vinagre_notebook_close_tab (nb, tab);
}

static void
tab_initialized_cb (VinagreTab *tab, VinagreNotebook *nb)
{
  VinagreConnection *conn = vinagre_tab_get_conn (tab);

  vinagre_notebook_update_ui_sentitivity (nb);
  vinagre_notebook_update_tab_tooltip (nb, tab);

  if (vinagre_connection_get_fullscreen (conn) && (!vinagre_window_is_fullscreen (nb->priv->window)))
    vinagre_window_toggle_fullscreen (nb->priv->window);
}

static GtkWidget *
build_tab_label (VinagreNotebook *nb, 
		 VinagreTab      *tab)
{
  GtkWidget *hbox, *label_hbox, *label_ebox;
  GtkWidget *label, *dummy_label;
  GtkWidget *close_button, *spinner;
  GtkRcStyle *rcstyle;
  GtkWidget *image;
  GtkWidget *icon;
  gchar     *name;

  hbox = gtk_hbox_new (FALSE, 4);

  label_ebox = gtk_event_box_new ();
  gtk_event_box_set_visible_window (GTK_EVENT_BOX (label_ebox), FALSE);
  gtk_box_pack_start (GTK_BOX (hbox), label_ebox, TRUE, TRUE, 0);
  gtk_widget_set_tooltip_text (label_ebox, _("Connecting..."));

  label_hbox = gtk_hbox_new (FALSE, 4);
  gtk_container_add (GTK_CONTAINER (label_ebox), label_hbox);

  /* setup close button */
  close_button = gtk_button_new ();
  gtk_button_set_relief (GTK_BUTTON (close_button),
			 GTK_RELIEF_NONE);

  /* don't allow focus on the close button */
  gtk_button_set_focus_on_click (GTK_BUTTON (close_button), FALSE);

  /* make it as small as possible */
  rcstyle = gtk_rc_style_new ();
  rcstyle->xthickness = rcstyle->ythickness = 0;
  gtk_widget_modify_style (close_button, rcstyle);
  g_object_unref (rcstyle);
  gint w, h;
  gtk_icon_size_lookup (GTK_ICON_SIZE_MENU, &w, &h);
  gtk_widget_set_size_request (GTK_WIDGET (close_button), w+2, h+2);

  image = gtk_image_new_from_stock (GTK_STOCK_CLOSE,
					  GTK_ICON_SIZE_MENU);
  gtk_container_add (GTK_CONTAINER (close_button), image);
  gtk_box_pack_start (GTK_BOX (hbox), close_button, FALSE, FALSE, 0);

  gtk_widget_set_tooltip_text (close_button, _("Close connection"));

  g_signal_connect (close_button,
		    "clicked",
		    G_CALLBACK (close_button_clicked_cb),
		    tab);

  /* setup spinner */
  spinner = vinagre_spinner_new ();
  vinagre_spinner_set_size (VINAGRE_SPINNER (spinner), GTK_ICON_SIZE_MENU);
  gtk_box_pack_start (GTK_BOX (label_hbox), spinner, FALSE, FALSE, 0);

  /* setup site icon */
  icon = gtk_image_new_from_icon_name (vinagre_tab_get_icon_name (tab),
				       GTK_ICON_SIZE_MENU);
  gtk_box_pack_start (GTK_BOX (label_hbox), icon, FALSE, FALSE, 0);
	
  /* setup label */
  name = vinagre_connection_get_best_name (vinagre_tab_get_conn (tab));
  label = gtk_label_new (name);
  g_free (name);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label), 0, 0);
  gtk_box_pack_start (GTK_BOX (label_hbox), label, FALSE, FALSE, 0);

  dummy_label = gtk_label_new ("");
  gtk_box_pack_start (GTK_BOX (label_hbox), dummy_label, TRUE, TRUE, 0);
	
  gtk_widget_show (hbox);
  gtk_widget_show (label_ebox);
  gtk_widget_show (label_hbox);
  gtk_widget_show (label);
  gtk_widget_show (dummy_label);	
  gtk_widget_show (image);
  gtk_widget_show (close_button);
  
  g_object_set_data (G_OBJECT (hbox), "label", label);
  g_object_set_data (G_OBJECT (tab),  "label", label);
  g_object_set_data (G_OBJECT (hbox), "label-ebox", label_ebox);
  g_object_set_data (G_OBJECT (tab),  "label-ebox", label_ebox);
  g_object_set_data (G_OBJECT (tab), "spinner", spinner);
  g_object_set_data (G_OBJECT (tab), "icon", icon);
  g_object_set_data (G_OBJECT (hbox), "close-button", close_button);
  g_object_set_data (G_OBJECT (tab),  "close-button", close_button);

  gtk_drag_source_set ( GTK_WIDGET (hbox),
			GDK_BUTTON1_MASK,
			vinagre_target_list,
			G_N_ELEMENTS (vinagre_target_list),
			GDK_ACTION_COPY );
  g_signal_connect (hbox,
		    "drag-data-get",
		    G_CALLBACK (drag_data_get_handl),
		    tab);

  return hbox;
}

void
vinagre_notebook_add_tab (VinagreNotebook *nb,
			  VinagreTab      *tab,
			  gint           position)
{
  GtkWidget      *label;
  GtkActionGroup *action_group;
  int            pos;

  g_return_if_fail (VINAGRE_IS_NOTEBOOK (nb));
  g_return_if_fail (VINAGRE_IS_TAB (tab));

  /* Unmerge the UI for the current tab */
  unmerge_tab_ui (nb);

  nb->priv->active_tab = tab;
  nb->priv->tabs = g_slist_append (nb->priv->tabs, tab);
  
  /* Merge the UI for the new tab */
  merge_tab_ui (nb);

  /* Actually add the new tab */
  label = build_tab_label (nb, tab);
  pos = gtk_notebook_insert_page (GTK_NOTEBOOK (nb), 
				  GTK_WIDGET (tab),
				  label, 
				  position);

  gtk_notebook_set_current_page (GTK_NOTEBOOK (nb), pos);
  vinagre_tab_set_notebook (tab, nb);

  g_signal_connect (tab,
		    "notify::tooltip",
		    G_CALLBACK (tab_tooltip_changed_cb),
		    nb);

  g_signal_connect (tab,
		    "tab-disconnected",
		    G_CALLBACK (tab_disconnected_cb),
		    nb);

  g_signal_connect (tab,
		    "tab-auth-failed",
		    G_CALLBACK (tab_auth_failed_cb),
		    nb);

  g_signal_connect (tab,
		    "tab-initialized",
		    G_CALLBACK (tab_initialized_cb),
		    nb);

  vinagre_notebook_update_window_title (nb);
  vinagre_notebook_update_ui_sentitivity (nb);
}

void
vinagre_notebook_close_tab (VinagreNotebook *nb,
			    VinagreTab      *tab)
{
  gint           position;
  GtkActionGroup *action_group;
  GtkNotebook    *notebook;
  VinagreTab     *previous_active_tab;

  g_return_if_fail (VINAGRE_IS_NOTEBOOK (nb));
  g_return_if_fail (VINAGRE_IS_TAB (tab));

  notebook = GTK_NOTEBOOK (nb);
  previous_active_tab = nb->priv->active_tab;

  g_signal_handlers_disconnect_by_func (tab,
					G_CALLBACK (tab_disconnected_cb),
					nb);
  g_signal_handlers_disconnect_by_func (tab,
					G_CALLBACK (tab_auth_failed_cb),
					nb);

  /* If it's closing the current tab, unmerge the UI */
  if (nb->priv->active_tab == tab)
    unmerge_tab_ui (nb);

  position = gtk_notebook_page_num (notebook, GTK_WIDGET (tab));
  g_signal_handlers_block_by_func (notebook, vinagre_notebook_page_switched, NULL);
  gtk_notebook_remove_page (notebook, position);
  g_signal_handlers_unblock_by_func (notebook, vinagre_notebook_page_switched, NULL);
  
  position = gtk_notebook_get_current_page (notebook);
  nb->priv->active_tab = VINAGRE_TAB (gtk_notebook_get_nth_page (notebook,
								 position));
  nb->priv->tabs = g_slist_remove (nb->priv->tabs, tab);

  /* Merge the UI for the new tab (if one exists) */
  if (nb->priv->active_tab != previous_active_tab)
    {
      merge_tab_ui (nb);
      vinagre_notebook_update_window_title (nb);
      vinagre_notebook_update_ui_sentitivity (nb);
    }
}

static void
close_tab (VinagreTab *tab,
	    VinagreNotebook *nb)
{
  vinagre_notebook_close_tab (nb, tab);
}

void
vinagre_notebook_close_all_tabs (VinagreNotebook *nb)
{	
  g_return_if_fail (VINAGRE_IS_NOTEBOOK (nb));
	
  gtk_container_foreach (GTK_CONTAINER (nb),
			(GtkCallback) close_tab,
			 nb);
}

void
vinagre_notebook_close_active_tab (VinagreNotebook *nb)
{
  g_return_if_fail (VINAGRE_IS_NOTEBOOK (nb));
  g_return_if_fail (nb->priv->active_tab != NULL);

  vinagre_notebook_close_tab (nb, nb->priv->active_tab);
}

VinagreTab *
vinagre_notebook_get_active_tab (VinagreNotebook *nb)
{
  g_return_val_if_fail (VINAGRE_IS_NOTEBOOK (nb), NULL);

  return nb->priv->active_tab;
}

GSList *
vinagre_notebook_get_tabs (VinagreNotebook *nb)
{
  g_return_val_if_fail (VINAGRE_IS_NOTEBOOK (nb), NULL);

  return nb->priv->tabs;
}

/* vim: set ts=8: */
