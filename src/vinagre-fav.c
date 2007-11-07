/*
 * vinagre-fav.c
 * This file is part of vinagre
 *
 * Copyright (C) 2007 - Jonh Wendell <wendell@bani.com.br>
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
#include "vinagre-fav.h"
#include "vinagre-utils.h"
#include "vinagre-bookmarks.h"
#include "vinagre-window-private.h"

#define VINAGRE_FAV_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), VINAGRE_TYPE_FAV, VinagreFavPrivate))

struct _VinagreFavPrivate
{
  VinagreWindow *window;
  GtkWidget     *tree;
  GtkTreeModel  *model;
};

G_DEFINE_TYPE(VinagreFav, vinagre_fav, GTK_TYPE_VBOX)

/* Signals */
enum
{
  FAV_ACTIVATED,
  FAV_SELECTED,
  LAST_SIGNAL
};

/* TreeModel */
enum {
  NAME_COL = 0,
  CONN_COL,
  NUM_COLS
};

/* Properties */
enum
{
  PROP_0,
  PROP_WINDOW,
};


static guint signals[LAST_SIGNAL] = { 0 };

static void
vinagre_fav_finalize (GObject *object)
{
//  VinagreFav *fav = VINAGRE_FAV (object);

  G_OBJECT_CLASS (vinagre_fav_parent_class)->finalize (object);
}

static void
vinagre_fav_set_property (GObject      *object,
			  guint         prop_id,
			  const GValue *value,
			  GParamSpec   *pspec)
{
  VinagreFav *fav = VINAGRE_FAV (object);

  switch (prop_id)
    {
      case PROP_WINDOW:
	fav->priv->window = VINAGRE_WINDOW (g_value_get_object (value));
	break;

      default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	break;
    }
}

static void
vinagre_fav_get_property (GObject    *object,
			  guint       prop_id,
			  GValue     *value,
			  GParamSpec *pspec)
{
  VinagreFav *fav = VINAGRE_FAV (object);

  switch (prop_id)
    {
      case PROP_WINDOW:
	g_value_set_object (value, fav->priv->window);
	break;

      default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	break;			
    }
}

static void 
vinagre_fav_class_init (VinagreFavClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = vinagre_fav_finalize;
  object_class->get_property = vinagre_fav_get_property;
  object_class->set_property = vinagre_fav_set_property;

  g_object_class_install_property (object_class,
				   PROP_WINDOW,
				   g_param_spec_object ("window",
							"Window",
							"The VinagreWindow this panel is associated with",
							 VINAGRE_TYPE_WINDOW,
							 G_PARAM_READWRITE |
							 G_PARAM_CONSTRUCT_ONLY));	

  signals[FAV_ACTIVATED] =
		g_signal_new ("fav-activated",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (VinagreFavClass, fav_activated),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__POINTER,
			      G_TYPE_NONE,
			      1,
			      G_TYPE_POINTER);

  signals[FAV_SELECTED] =
		g_signal_new ("fav-selected",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (VinagreFavClass, fav_selected),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__POINTER,
			      G_TYPE_NONE,
			      1,
			      G_TYPE_POINTER);

  g_type_class_add_private (object_class, sizeof (VinagreFavPrivate));
}

static void
vinagre_fav_row_activated_cb (GtkTreeView       *treeview,
			      GtkTreePath       *path,
			      GtkTreeViewColumn *column,
			      VinagreFav         *fav)
{
  GtkTreeIter iter;
  VinagreConnection *conn = NULL;

  gtk_tree_model_get_iter (fav->priv->model, &iter, path);
  gtk_tree_model_get (fav->priv->model, 
		      &iter,
		      CONN_COL, &conn, 
		      -1);

  /* Emits the signal saying that user has activated a bookmark */
  g_signal_emit (G_OBJECT (fav), 
		 signals[FAV_ACTIVATED],
		 0, 
		 conn);
}

static void
vinagre_fav_selection_changed_cb (GtkTreeSelection *selection,
				  VinagreFav       *fav)
{
  GtkTreeIter iter;
  VinagreConnection *conn = NULL;

  if (gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
      gtk_tree_model_get (fav->priv->model, 
			  &iter,
			  CONN_COL, &conn, 
			  -1);
    }

  /* Emits the signal saying that user has selected a bookmark */
  g_signal_emit (G_OBJECT (fav), 
		 signals[FAV_SELECTED],
		 0, 
		 conn);
}

static GtkTreePath *
get_current_path (VinagreFav *fav)
{
  GtkTreePath *path = NULL;
  GtkTreeIter iter;
  GtkTreeSelection *selection;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (fav->priv->tree));

  if (gtk_tree_selection_get_selected (selection, NULL, &iter))
    path = gtk_tree_model_get_path (fav->priv->model, &iter);

  return path;
}

static void
menu_position (GtkMenu    *menu,
	       gint       *x,
	       gint       *y,
	       gboolean   *push_in,
	       VinagreFav *fav)
{
  GtkTreePath *path;
  GdkRectangle rect;
  gint wx, wy;
  GtkRequisition requisition;
  GtkWidget *w;

  w = fav->priv->tree;
  path = get_current_path (fav);

  gtk_tree_view_get_cell_area (GTK_TREE_VIEW (w),
			       path,
			       NULL,
			       &rect);

  wx = rect.x;
  wy = rect.y;

  gdk_window_get_origin (w->window, x, y);
	
  gtk_widget_size_request (GTK_WIDGET (menu), &requisition);

  if (gtk_widget_get_direction (w) == GTK_TEXT_DIR_RTL)
    *x += w->allocation.x + w->allocation.width - requisition.width - 10;
  else
    *x += w->allocation.x + 10 ;

  wy = MAX (*y + 5, *y + wy + 5);
  wy = MIN (wy, *y + w->allocation.height - requisition.height - 5);
	
  *y = wy;

  *push_in = TRUE;

  if (path)
    gtk_tree_path_free (path);
}

static gboolean
show_popup_menu (VinagreFav     *fav,
		 GdkEventButton *event)
{
  GtkWidget *menu = NULL;

  menu = gtk_ui_manager_get_widget (vinagre_window_get_ui_manager (fav->priv->window),
				    "/FavPopup");
  g_return_val_if_fail (menu != NULL, FALSE);

  if (event)
    {
      gtk_menu_popup (GTK_MENU (menu), 
		      NULL, 
		      NULL,
		      NULL, 
		      NULL,
		      event->button, 
		      event->time);
    }
  else
    {
      gtk_menu_popup (GTK_MENU (menu), 
		      NULL, 
		      NULL,
		      (GtkMenuPositionFunc) menu_position, 
		      fav,
		      0, 
		      gtk_get_current_event_time ());

      gtk_menu_shell_select_first (GTK_MENU_SHELL (menu), FALSE);

    }

  return TRUE;
}

static gboolean
fav_button_press_event (GtkTreeView    *treeview,
			GdkEventButton *event,
			VinagreFav     *fav)
{
	if ((GDK_BUTTON_PRESS == event->type) && (3 == event->button))
	{
		GtkTreePath* path = NULL;
		
		if (event->window == gtk_tree_view_get_bin_window (treeview))
		{
			/* Change the cursor position */
			if (gtk_tree_view_get_path_at_pos (treeview,
							   event->x,
							   event->y,
							   &path,
							   NULL,
							   NULL,
							   NULL))
			{				
			
				gtk_tree_view_set_cursor (treeview,
							  path,
							  NULL,
							  FALSE);
					
				gtk_tree_path_free (path);
							   
				/* A row exists at mouse position */
				return show_popup_menu (fav, event);
			}
		}
	}
	
	return FALSE;
}


static gboolean
fav_popup_menu (GtkWidget  *treeview,
		VinagreFav *fav)
{
  /* Only respond if the treeview is the actual focus */
  if (gtk_window_get_focus (GTK_WINDOW (fav->priv->window)) == treeview)
    return show_popup_menu (fav, NULL);

  return FALSE;
}


static void
vinagre_fav_create_tree (VinagreFav *fav)
{
  GtkCellRenderer   *cell;
  GtkWidget         *scroll;
  GtkTreeSelection  *selection;

  /* Create the scrolled window */
  scroll = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);

  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scroll),
				       GTK_SHADOW_ETCHED_OUT);

  gtk_box_pack_start (GTK_BOX(fav), scroll, TRUE, TRUE, 0);

  /* Create the model */
  fav->priv->model = GTK_TREE_MODEL (gtk_list_store_new (NUM_COLS,
							 G_TYPE_STRING,
							 G_TYPE_POINTER));

  fav->priv->tree = gtk_tree_view_new_with_model (fav->priv->model);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (fav->priv->tree), FALSE);
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (fav->priv->tree));
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
  g_signal_connect (selection, "changed", G_CALLBACK (vinagre_fav_selection_changed_cb), fav);

  cell = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes ( GTK_TREE_VIEW (fav->priv->tree),
						-1,
						_("Name"),
						cell,
						"text", NAME_COL,
						NULL);

  g_signal_connect (fav->priv->tree,
		    "row-activated",
		    G_CALLBACK (vinagre_fav_row_activated_cb),
		    fav);

  g_signal_connect (fav->priv->tree,
		    "button-press-event",
		    G_CALLBACK (fav_button_press_event),
		    fav);

  g_signal_connect (fav->priv->tree,
		    "popup-menu",
		    G_CALLBACK (fav_popup_menu),
		    fav);


  vinagre_fav_update_list (fav);

  gtk_container_add (GTK_CONTAINER(scroll), fav->priv->tree);
  gtk_widget_show (fav->priv->tree);
  g_object_unref (G_OBJECT (fav->priv->model));
}


static void
vinagre_fav_hide (GtkButton *button, VinagreFav *fav)
{
  GtkAction *action;

  action = gtk_action_group_get_action (fav->priv->window->priv->always_sensitive_action_group,
					"ViewBookmarks");
  gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), FALSE);
}

static void
vinagre_fav_init (VinagreFav *fav)
{
  GtkWidget *label_box, *label, *close_button;

  fav->priv = VINAGRE_FAV_GET_PRIVATE (fav);

  /* setup label */
  label_box = gtk_hbox_new (FALSE, 0);
  label = gtk_label_new (_("Bookmarks"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label), 6, 6);
  gtk_box_pack_start (GTK_BOX(label_box), label, TRUE, TRUE, 0);

  /* setup small close button */
  close_button = vinagre_utils_create_small_close_button ();
  gtk_box_pack_start (GTK_BOX(label_box), close_button, FALSE, FALSE, 0);
  gtk_widget_set_tooltip_text (close_button, _("Hide bookmarks"));
  gtk_box_pack_start (GTK_BOX(fav), label_box, FALSE, FALSE, 0);
  g_signal_connect (close_button,
		    "clicked",
		    G_CALLBACK (vinagre_fav_hide),
		    fav);

  /* setup the tree */
  vinagre_fav_create_tree (fav);
}

GtkWidget *
vinagre_fav_new (VinagreWindow *window)
{
  g_return_val_if_fail (VINAGRE_IS_WINDOW (window), NULL);

  return GTK_WIDGET (g_object_new (VINAGRE_TYPE_FAV,
				   "window", window,
				   NULL));
}

void
vinagre_fav_update_list (VinagreFav *fav)
{
  GtkTreeIter        iter;
  GtkListStore      *store;
  GList             *list;
  VinagreConnection *conn;
  gchar             *name;

  g_return_if_fail (VINAGRE_IS_FAV (fav));

  store = GTK_LIST_STORE (fav->priv->model);
  gtk_list_store_clear (store);

  list = vinagre_bookmarks_get_all ();
  while (list)
    {
      conn = (VinagreConnection *) list->data;
      name = vinagre_connection_best_name (conn);

      gtk_list_store_append (store, &iter);
      gtk_list_store_set (store, &iter,
                          NAME_COL, name,
                          CONN_COL, conn,
                          -1);
      list = list->next;
      g_free (name);
    }

  g_list_free (list);
}
/* vim: ts=8 */
