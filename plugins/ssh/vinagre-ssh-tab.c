/*
 * vinagre-ssh-tab.c
 * SSH Implementation for VinagreSshTab widget
 * This file is part of vinagre
 *
 * Copyright (C) 2009 - Jonh Wendell <wendell@bani.com.br>
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

#include <glib/gi18n.h>
#include <vte/vte.h>
#include <gdk/gdkkeysyms.h>

#include <vinagre/vinagre-utils.h>
#include <vinagre/vinagre-prefs.h>

#include "vinagre-ssh-tab.h"
#include "vinagre-ssh-connection.h"

#define VINAGRE_SSH_TAB_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), VINAGRE_TYPE_SSH_TAB, VinagreSshTabPrivate))

struct _VinagreSshTabPrivate
{
  GtkWidget  *vte;
};

G_DEFINE_TYPE (VinagreSshTab, vinagre_ssh_tab, VINAGRE_TYPE_TAB)

static gchar *
ssh_tab_get_tooltip (VinagreTab *tab)
{
  VinagreConnection *conn = vinagre_tab_get_conn (tab);

  return  g_markup_printf_escaped (
				  "<b>%s</b> %s\n"
				  "<b>%s</b> %d",
				  _("Host:"), vinagre_connection_get_host (conn),
				  _("Port:"), vinagre_connection_get_port (conn));
}

static GdkPixbuf *
ssh_tab_get_screenshot (VinagreTab *tab)
{
  return NULL;
}

static gboolean
emit_delayed_signal (GObject *object)
{
  VinagreSshTab *ssh_tab = VINAGRE_SSH_TAB (object);

  g_signal_emit_by_name (object, "tab-initialized");
  gtk_widget_grab_focus (ssh_tab->priv->vte);
  return FALSE;
}

static void
vinagre_ssh_tab_constructed (GObject *object)
{
  gchar **arg;
  VinagreSshTab *ssh_tab = VINAGRE_SSH_TAB (object);
  VinagreTab    *tab = VINAGRE_TAB (object);

  if (G_OBJECT_CLASS (vinagre_ssh_tab_parent_class)->constructed)
    G_OBJECT_CLASS (vinagre_ssh_tab_parent_class)->constructed (object);

  arg = g_new (gchar *, 5);
  arg[0] = g_strdup ("ssh");
  arg[1] = g_strdup (vinagre_connection_get_host (vinagre_tab_get_conn (tab)));
  arg[2] = g_strdup ("-p");
  arg[3] = g_strdup_printf ("%d", vinagre_connection_get_port (vinagre_tab_get_conn (tab)));
  arg[4] = NULL;

  vte_terminal_fork_command (VTE_TERMINAL (ssh_tab->priv->vte),
			     "ssh",
			     arg,
			     NULL,
			     NULL,
			     FALSE,
			     FALSE,
			     FALSE);
  g_strfreev (arg);
  gtk_widget_show_all (GTK_WIDGET (ssh_tab));

  vinagre_tab_add_recent_used (tab);
  vinagre_tab_set_state (tab, VINAGRE_TAB_STATE_CONNECTED);
  g_idle_add ((GSourceFunc)emit_delayed_signal, object);
}

static void 
vinagre_ssh_tab_class_init (VinagreSshTabClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  VinagreTabClass* tab_class = VINAGRE_TAB_CLASS (klass);

  object_class->constructed = vinagre_ssh_tab_constructed;

  tab_class->impl_get_tooltip = ssh_tab_get_tooltip;
  tab_class->impl_get_screenshot = ssh_tab_get_screenshot;

  g_type_class_add_private (object_class, sizeof (VinagreSshTabPrivate));
}

static void
ssh_disconnected_cb (VteTerminal *ssh, VinagreSshTab *tab)
{
  g_signal_emit_by_name (G_OBJECT (tab), "tab-disconnected");
}

static void
vinagre_ssh_tab_init (VinagreSshTab *ssh_tab)
{
  ssh_tab->priv = VINAGRE_SSH_TAB_GET_PRIVATE (ssh_tab);

  /* Create the ssh widget */
  ssh_tab->priv->vte = vte_terminal_new ();
  vinagre_tab_add_view (VINAGRE_TAB (ssh_tab), ssh_tab->priv->vte);

  g_signal_connect (ssh_tab->priv->vte,
		    "child-exited",
		    G_CALLBACK (ssh_disconnected_cb),
		    ssh_tab);
}

GtkWidget *
vinagre_ssh_tab_new (VinagreConnection *conn,
		     VinagreWindow     *window)
{
  return GTK_WIDGET (g_object_new (VINAGRE_TYPE_SSH_TAB,
				   "conn", conn,
				   "window", window,
				   NULL));
}

/* vim: set ts=8: */
