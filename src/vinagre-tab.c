/*
 * vinagre-tab.c
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA.
 */
 
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n.h>
#include <glade/glade.h>
#include <vncdisplay.h>

#include "vinagre-notebook.h"
#include "vinagre-tab.h"
#include "vinagre-main.h"
#include "vinagre-utils.h"

#define VINAGRE_TAB_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), VINAGRE_TYPE_TAB, VinagreTabPrivate))

struct _VinagreTabPrivate
{
  GtkWidget *vnc;
  VinagreConnection *conn;
};

G_DEFINE_TYPE(VinagreTab, vinagre_tab, GTK_TYPE_VBOX)

/* Signals */
enum
{
  TAB_CONNECTED,
  TAB_DISCONNECTED,
  TAB_INITIALIZED,
  LAST_SIGNAL
};

/* Properties */
enum
{
  PROP_0,
  PROP_CONN
};

static guint signals[LAST_SIGNAL] = { 0 };

static void
vinagre_tab_get_property (GObject    *object,
			  guint       prop_id,
			  GValue     *value,
			  GParamSpec *pspec)
{
  VinagreTab *tab = VINAGRE_TAB (object);

  switch (prop_id)
    {
      case PROP_CONN:
        g_value_set_pointer (value, tab->priv->conn);
	break;
      default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	break;			
    }
}

static void
vinagre_tab_set_property (GObject      *object,
			  guint         prop_id,
			  const GValue *value,
			  GParamSpec   *pspec)
{
  VinagreTab *tab = VINAGRE_TAB (object);

  switch (prop_id)
    {
      case PROP_CONN:
        tab->priv->conn = g_value_get_pointer (value);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;			
    }
}

static void
vinagre_tab_finalize (GObject *object)
{
  VinagreTab *tab = VINAGRE_TAB (object);

  vinagre_connection_free (tab->priv->conn);
	
  G_OBJECT_CLASS (vinagre_tab_parent_class)->finalize (object);
}

static void 
vinagre_tab_class_init (VinagreTabClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = vinagre_tab_finalize;
  object_class->get_property = vinagre_tab_get_property;
  object_class->set_property = vinagre_tab_set_property;
	
  g_object_class_install_property (object_class,
				   PROP_CONN,
				   g_param_spec_pointer ("conn",
							 "Connection",
							 "The connection",
							 G_PARAM_READWRITE |
							 G_PARAM_CONSTRUCT_ONLY |
							 G_PARAM_STATIC_NAME |
							 G_PARAM_STATIC_NICK |
							 G_PARAM_STATIC_BLURB));

  signals[TAB_CONNECTED] =
		g_signal_new ("tab-connected",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (VinagreTabClass, tab_connected),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);

  signals[TAB_DISCONNECTED] =
		g_signal_new ("tab-disconnected",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (VinagreTabClass, tab_disconnected),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);

  signals[TAB_INITIALIZED] =
		g_signal_new ("tab-initialized",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (VinagreTabClass, tab_initialized),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);

  g_type_class_add_private (object_class, sizeof (VinagreTabPrivate));
}

static gboolean
open_vnc (VinagreTab *tab)
{
  gchar *port;
  
  port = g_strdup_printf ("%d", tab->priv->conn->port);
  
  if (!vnc_display_open_host (VNC_DISPLAY(tab->priv->vnc), tab->priv->conn->host, port))
    vinagre_utils_show_error (_("Error connecting to host."), NULL);

  vnc_display_set_keyboard_grab (VNC_DISPLAY(tab->priv->vnc), TRUE);
  vnc_display_set_pointer_grab  (VNC_DISPLAY(tab->priv->vnc), TRUE);
  vnc_display_set_pointer_local (VNC_DISPLAY(tab->priv->vnc), TRUE);

  g_free (port);
  gtk_widget_grab_focus (tab->priv->vnc);
  return FALSE;
}

static void
vnc_connected_cb (VncDisplay *vnc, VinagreTab *tab)
{
  /* Emits the signal saying that we have connected to the machine */
  g_signal_emit (G_OBJECT (tab),
		 signals[TAB_CONNECTED],
		 0);
}

static void
vnc_disconnected_cb (VncDisplay *vnc, VinagreTab *tab)
{
  /* Emits the signal saying that we have disconnected from the machine */
  g_signal_emit (G_OBJECT (tab),
		 signals[TAB_DISCONNECTED],
		 0);
}

static void
vnc_initialized_cb (VncDisplay *vnc, VinagreTab *tab)
{
  /* Emits the signal saying that we have connected to the machine */
  g_signal_emit (G_OBJECT (tab),
		 signals[TAB_INITIALIZED],
		 0);
}

static gchar *
ask_password()
{
  GladeXML   *xml;
  const char *glade_file;
  GtkWidget *password_dialog, *password_entry;
  gchar *password;
  int result;

  glade_file = vinagre_utils_get_glade_filename ();
  xml = glade_xml_new (glade_file, NULL, NULL);

  password_dialog = glade_xml_get_widget (xml, "password_required_dialog");
  gtk_window_set_transient_for (GTK_WINDOW(password_dialog), GTK_WINDOW(main_window));

  result = gtk_dialog_run (GTK_DIALOG (password_dialog));
  if (result != -5)
    {
      gtk_widget_destroy (GTK_WIDGET (password_dialog));
      return NULL;
    }

  password_entry = glade_xml_get_widget (xml, "password_entry");
  password = g_strdup (gtk_entry_get_text (GTK_ENTRY (password_entry)));

  gtk_widget_destroy (GTK_WIDGET (password_dialog));

  g_object_unref (xml);
  return password;
}

static void
vnc_authentication_cb (VncDisplay *vnc, GValueArray *credList, VinagreTab *tab)
{
  gchar *password;
  
  password = ask_password ();
  vinagre_connection_set_password (tab->priv->conn, password);
  vnc_display_set_credential (vnc, VNC_DISPLAY_CREDENTIAL_PASSWORD, password);
}

static void
vinagre_tab_init (VinagreTab *tab)
{
  GtkWidget *scroll;
  GtkWidget *align;

  tab->priv = VINAGRE_TAB_GET_PRIVATE (tab);

  /* Create the alignment */
  align = gtk_alignment_new (0.5, 0.5, 0, 0);

  /* Create the scrolled window */
  scroll = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scroll),
				       GTK_SHADOW_IN);

  gtk_box_pack_end (GTK_BOX(tab), scroll, TRUE, TRUE, 0);

  /* Create the vnc widget */
  tab->priv->vnc = vnc_display_new ();
  gtk_container_add (GTK_CONTAINER (align), tab->priv->vnc);

  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scroll),
					 align);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scroll),
				       GTK_SHADOW_IN);

  g_signal_connect (tab->priv->vnc,
		    "vnc-connected",
		    G_CALLBACK (vnc_connected_cb),
		    tab);

  g_signal_connect (tab->priv->vnc,
		    "vnc-initialized",
		    G_CALLBACK (vnc_initialized_cb),
		    tab);

  g_signal_connect (tab->priv->vnc,
		    "vnc-disconnected",
		    G_CALLBACK (vnc_disconnected_cb),
		    tab);

  g_signal_connect (tab->priv->vnc,
		    "vnc-auth-credential",
		    G_CALLBACK (vnc_authentication_cb),
		    tab);

 /* connect VNC */
 /* FIXME: i had to add a timeout because private conn is not available at this time*/
  g_timeout_add (1000,
		 (GSourceFunc) open_vnc,
		 tab);

  gtk_widget_show_all (GTK_WIDGET (tab));
}

GtkWidget *
vinagre_tab_new (VinagreConnection *conn)
{
  return GTK_WIDGET (g_object_new (VINAGRE_TYPE_TAB, 
				   "conn", conn,
				   NULL));
}


gchar *
vinagre_tab_get_tooltips (VinagreTab *tab)
{
  gchar *tip;

  g_return_val_if_fail (VINAGRE_IS_TAB (tab), NULL);

  tip =  g_markup_printf_escaped (
				  "<b>%s</b> %s\n\n"
				  "<b>%s</b> %s\n"
				  "<b>%s</b> %d\n"
				  "<b>%s</b> %dx%d",
				  _("Desktop Name:"), vnc_display_get_name (VNC_DISPLAY (tab->priv->vnc)),
				  _("Host:"), tab->priv->conn->host,
				  _("Port:"), tab->priv->conn->port,
				  _("Dimensions:"), vnc_display_get_width (VNC_DISPLAY (tab->priv->vnc)), vnc_display_get_height (VNC_DISPLAY (tab->priv->vnc)));

  return tip;
}

VinagreConnection *
vinagre_tab_get_conn (VinagreTab *tab)
{
  g_return_val_if_fail (VINAGRE_IS_TAB (tab), NULL);

  return tab->priv->conn;
}

GtkWidget *
vinagre_tab_get_vnc (VinagreTab *tab)
{
  g_return_val_if_fail (VINAGRE_IS_TAB (tab), NULL);

  return tab->priv->vnc;
}

void
vinagre_tab_set_title (VinagreTab *tab,
		       const char *title)
{
  GtkLabel *label;

  g_return_if_fail (VINAGRE_IS_TAB (tab));

  label = GTK_LABEL (g_object_get_data (G_OBJECT (tab),  "label"));
  gtk_label_set_label (label, title);
}
