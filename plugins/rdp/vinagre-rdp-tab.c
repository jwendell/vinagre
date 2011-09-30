/*
 * vinagre-rdp-tab.c
 * RDP Implementation for VinagreRdpTab widget
 * This file is part of vinagre
 *
 * Copyright (C) 2010 - Jonh Wendell <wendell@bani.com.br>
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

#include <config.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <glib/gi18n.h>
#include <gtk/gtkx.h>

#include <vinagre/vinagre-prefs.h>

#include "vinagre-rdp-tab.h"
#include "vinagre-rdp-connection.h"
#include "vinagre-vala.h"

#define VINAGRE_RDP_TAB_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), VINAGRE_TYPE_RDP_TAB, VinagreRdpTabPrivate))

struct _VinagreRdpTabPrivate
{
  GtkWidget *box;
  GPid pid;
  guint child;
};

G_DEFINE_TYPE (VinagreRdpTab, vinagre_rdp_tab, VINAGRE_TYPE_TAB)

static gchar *
rdp_tab_get_tooltip (VinagreTab *tab)
{
  VinagreConnection *conn = vinagre_tab_get_conn (tab);

  return  g_markup_printf_escaped (
				  "<b>%s</b> %s\n"
				  "<b>%s</b> %d",
				  _("Host:"), vinagre_connection_get_host (conn),
				  _("Port:"), vinagre_connection_get_port (conn));
}

static void
emit_connected_signal (GtkSocket *socket, VinagreRdpTab *rdp_tab)
{
  g_signal_emit_by_name (rdp_tab, "tab-initialized");
  gtk_widget_grab_focus (rdp_tab->priv->box);
}

static void
child_exited (GPid pid, gint status, VinagreRdpTab *rdp_tab)
{
  if (rdp_tab->priv->pid > 0)
    {
      g_spawn_close_pid (rdp_tab->priv->pid);
      rdp_tab->priv->pid = 0;
      if (WIFEXITED (status))
        vinagre_tab_remove_from_notebook (VINAGRE_TAB (rdp_tab));
      else
        g_signal_emit_by_name (rdp_tab, "tab-disconnected");
    }
}

static gboolean
delay_connect (GObject *object)
{
  gchar **arg;
  const gchar *username;
  gint i;
  GError *error = NULL;
  VinagreRdpTab *rdp_tab = VINAGRE_RDP_TAB (object);
  VinagreTab    *tab = VINAGRE_TAB (object);
  VinagreConnection *conn = vinagre_tab_get_conn (tab);

  username = vinagre_connection_get_username (conn);
  i = 0;

  arg = g_new (gchar *, 9);
  arg[i++] = g_strdup ("rdesktop");

  arg[i++] = g_strdup ("-K");

  if (vinagre_connection_get_fullscreen (conn))
    arg[i++] = g_strdup ("-f");

  arg[i++] = g_strdup ("-X");
  arg[i++] = g_strdup_printf ("%d", (int)gtk_socket_get_id (GTK_SOCKET (rdp_tab->priv->box)));

  if (username && *username)
    {
      arg[i++] = g_strdup ("-u");
      arg[i++] = g_strdup (username);
    }

  arg[i++] = g_strdup_printf ("%s:%d",
			      vinagre_connection_get_host (conn),
			      vinagre_connection_get_port (conn));
  arg[i++] = NULL;

  if (!g_spawn_async (NULL,
		      arg,
		      NULL,
		      G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD,
		      NULL,
		      NULL,
		      &rdp_tab->priv->pid,
		      &error))
    {
      vinagre_utils_show_error_dialog (_("Error while executing rdesktop"),
				error ? error->message : _("Unknown error"),
				GTK_WINDOW (vinagre_tab_get_window (tab)));
      vinagre_tab_remove_from_notebook (tab);
      goto _end;
    }

  rdp_tab->priv->child = g_child_watch_add (rdp_tab->priv->pid,
					    (GChildWatchFunc)child_exited, rdp_tab);
  gtk_widget_show_all (GTK_WIDGET (rdp_tab));

  vinagre_tab_add_recent_used (tab);
  vinagre_tab_set_state (tab, VINAGRE_TAB_STATE_CONNECTED);

_end:
  g_strfreev (arg);

  return FALSE;
}

static void
vinagre_rdp_tab_constructed (GObject *object)
{
  if (G_OBJECT_CLASS (vinagre_rdp_tab_parent_class)->constructed)
    G_OBJECT_CLASS (vinagre_rdp_tab_parent_class)->constructed (object);

  g_idle_add ((GSourceFunc)delay_connect, object);
}

static void
vinagre_rdp_tab_dispose (GObject *object)
{
  VinagreRdpTab *rdp_tab = VINAGRE_RDP_TAB (object);

  if (rdp_tab->priv->pid > 0)
    {
      g_spawn_close_pid (rdp_tab->priv->pid);
      kill (rdp_tab->priv->pid, SIGTERM);
      rdp_tab->priv->pid = 0;
    }

  if (rdp_tab->priv->child > 0)
    {
      g_source_remove (rdp_tab->priv->child);
      rdp_tab->priv->child = 0;
    }

  G_OBJECT_CLASS (vinagre_rdp_tab_parent_class)->dispose (object);
}

static void 
vinagre_rdp_tab_class_init (VinagreRdpTabClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  VinagreTabClass* tab_class = VINAGRE_TAB_CLASS (klass);

  object_class->constructed = vinagre_rdp_tab_constructed;
  object_class->dispose = vinagre_rdp_tab_dispose;

  tab_class->impl_get_tooltip = rdp_tab_get_tooltip;

  g_type_class_add_private (object_class, sizeof (VinagreRdpTabPrivate));
}

static void
vinagre_rdp_tab_init (VinagreRdpTab *rdp_tab)
{
  rdp_tab->priv = VINAGRE_RDP_TAB_GET_PRIVATE (rdp_tab);

  rdp_tab->priv->pid = 0;
  rdp_tab->priv->child = 0;

  /* Create the rdp widget */
  rdp_tab->priv->box = gtk_socket_new ();
  gtk_widget_set_can_focus (rdp_tab->priv->box, TRUE);
  vinagre_tab_add_view (VINAGRE_TAB (rdp_tab), rdp_tab->priv->box);
  gtk_widget_show (rdp_tab->priv->box);

  g_signal_connect (rdp_tab->priv->box,
		    "plug-added",
		    G_CALLBACK (emit_connected_signal),
		    rdp_tab);

}

GtkWidget *
vinagre_rdp_tab_new (VinagreConnection *conn,
		     VinagreWindow     *window)
{
  return GTK_WIDGET (g_object_new (VINAGRE_TYPE_RDP_TAB,
				   "conn", conn,
				   "window", window,
				   NULL));
}

/* vim: set ts=8: */
