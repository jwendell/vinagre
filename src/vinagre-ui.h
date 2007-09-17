/*
 * vinagre-ui.h
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

#ifndef __VINAGRE_UI_H__
#define __VINAGRE_UI_H__

#include <gtk/gtk.h>

#include "vinagre-commands.h"

G_BEGIN_DECLS

static const GtkActionEntry vinagre_always_sensitive_menu_entries[] =
{
  /* Toplevel */
  { "Machine", NULL, N_("_Machine") },
  { "Edit", NULL, N_("_Edit") },
  { "View", NULL, N_("_View") },
  { "Favorites", NULL, N_("_Favorites") },
  { "Help", NULL, N_("_Help") },

  /* Machine menu */
  { "MachineConnect", GTK_STOCK_CONNECT, NULL, "<control>N",
    N_("Connect into a remote machine"), G_CALLBACK (vinagre_cmd_machine_connect) },
  { "MachineQuit", GTK_STOCK_QUIT, NULL, "<control>Q",
    N_("Quit the program"), G_CALLBACK (gtk_main_quit) },
  	
  /* Edit menu */
  { "EditPreferences", GTK_STOCK_PREFERENCES, N_("_Preferences"), NULL,
    N_("Configure the application"), NULL },

  /* Favorites menu */
  { "FavoritesOpen", GTK_STOCK_CONNECT, N_("_Open favorite"), NULL,
    N_("Connect to this favorite"), G_CALLBACK (vinagre_cmd_favorites_open) },
  { "FavoritesEdit", GTK_STOCK_EDIT, N_("_Edit favorite"), NULL,
    N_("Edit the details of selected favorite"), G_CALLBACK (vinagre_cmd_favorites_edit) },
  { "FavoritesDel", GTK_STOCK_DELETE, N_("_Remove from favorites"), NULL,
    N_("Remove current selected connection from favorites"), G_CALLBACK (vinagre_cmd_favorites_del) },

  /* Help menu */
  {"HelpContents", GTK_STOCK_HELP, N_("_Contents"), "F1",
    N_("Open the vinagre manual"), NULL },
  { "HelpAbout", GTK_STOCK_ABOUT, NULL, NULL,
    N_("About this application"), G_CALLBACK (vinagre_cmd_help_about) }
};

static const GtkActionEntry vinagre_menu_entries[] =
{
  /* Machine menu */
  { "MachineClose", GTK_STOCK_CLOSE, NULL, "<control>W",
    N_("Close the current connection"), G_CALLBACK (vinagre_cmd_machine_close) },
  { "MachineTakeScreenshot", "applets-screenshooter", N_("Take screenshot"), NULL,
    N_("Take a screenshot of active connection"), G_CALLBACK (vinagre_cmd_machine_take_screenshot) },
  { "MachineCloseAll", GTK_STOCK_CLOSE, N_("C_lose All"), "<control><shift>W",
    N_("Close all active connections"), G_CALLBACK (vinagre_cmd_machine_close_all) },

  /* Edit menu */
  { "EditCopy", GTK_STOCK_COPY, NULL, "<control>C",
    N_("Copy the selection"), NULL },
  { "EditPaste", GTK_STOCK_PASTE, NULL, "<control>V",
    N_("Paste the clipboard"), NULL },

  /* View menu */
  { "ViewFullScreen", GTK_STOCK_FULLSCREEN, NULL, "F11",
    N_("View the current machine in full screen"), G_CALLBACK (vinagre_cmd_view_fullscreen) },

  /* Favorites menu */
  { "FavoritesAdd", GTK_STOCK_SAVE, N_("_Add to favorites"), "<control>D",
    N_("Add current connection to favorites"), G_CALLBACK (vinagre_cmd_favorites_add) }

};

static const GtkToggleActionEntry vinagre_always_sensitive_toggle_menu_entries[] =
{
  { "ViewToolbar", NULL, N_("_Toolbar"), NULL,
    N_("Show or hide the toolbar"),
    G_CALLBACK (vinagre_cmd_view_show_toolbar), FALSE },

  { "ViewStatusbar", NULL, N_("_Statusbar"), NULL,
    N_("Show or hide the statusbar"),
    G_CALLBACK (vinagre_cmd_view_show_statusbar), FALSE },

  { "ViewFavorites", NULL, N_("_Favorites"), "F9",
    N_("Show or hide the favorites panel"),
    G_CALLBACK (vinagre_cmd_view_show_fav_panel), FALSE }
};

G_END_DECLS

#endif  /* __VINAGRE_UI_H__  */
