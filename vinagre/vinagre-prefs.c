/*
 * vinagre-prefs.c
 * This file is part of vinagre
 *
 * Copyright (C) Jonh Wendell 2008,2009 <wendell@bani.com.br>
 * 
 * vinagre-prefs.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * vinagre-prefs.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>
#include <glib/gi18n.h>
#include "vinagre-prefs.h"

#define VINAGRE_SCHEMA_NAME		"org.gnome.Vinagre"
#define VM_ALWAYS_SHOW_TABS		"always-show-tabs"
#define VM_SHOW_ACCELS			"show-accels"
#define VM_HISTORY_SIZE			"history-size"
#define VM_ALWAYS_ENABLE_LISTENING	"always-enable-listening"
#define VM_SHARED_FLAG			"shared-flag"

struct _VinagrePrefsPrivate
{
  GSettings *gsettings;
};

/* Properties */
enum
{
  PROP_0,
  PROP_SHARED_FLAG,
  PROP_HISTORY_SIZE,
  PROP_LAST_PROTOCOL,
  PROP_ALWAYS_ENABLE_LISTENING
};

G_DEFINE_TYPE (VinagrePrefs, vinagre_prefs, G_TYPE_OBJECT);

static VinagrePrefs *prefs_singleton = NULL;

/**
 * vinagre_prefs_get_default:
 *
 * Return value: (transfer none):
 */
VinagrePrefs *
vinagre_prefs_get_default (void)
{
  if (G_UNLIKELY (!prefs_singleton))
    prefs_singleton = VINAGRE_PREFS (g_object_new (VINAGRE_TYPE_PREFS,
						   NULL));
  return prefs_singleton;
}

/**
 * vinagre_prefs_get_default_gsettings:
 *
 * Return value: (transfer none):
 */
GSettings *
vinagre_prefs_get_default_gsettings (void)
{
  VinagrePrefs *pref = vinagre_prefs_get_default ();

  return pref->priv->gsettings;
}

static void
vinagre_prefs_init (VinagrePrefs *prefs)
{
  prefs->priv = G_TYPE_INSTANCE_GET_PRIVATE (prefs, VINAGRE_TYPE_PREFS, VinagrePrefsPrivate);

  prefs->priv->gsettings = g_settings_new (VINAGRE_SCHEMA_NAME);
  if (prefs->priv->gsettings == NULL)
    g_critical (_("Cannot initialize preferences manager."));

}

static void
vinagre_prefs_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
  VinagrePrefs *prefs = VINAGRE_PREFS (object);

  switch (prop_id)
    {
      case PROP_SHARED_FLAG:
	g_settings_set_boolean (prefs->priv->gsettings, VM_SHARED_FLAG, g_value_get_boolean (value));
	break;
      case PROP_HISTORY_SIZE:
	g_settings_set_int (prefs->priv->gsettings, VM_HISTORY_SIZE, g_value_get_int (value));
	break;
      case PROP_ALWAYS_ENABLE_LISTENING:
	g_settings_set_boolean (prefs->priv->gsettings, VM_ALWAYS_ENABLE_LISTENING, g_value_get_boolean (value));
	break;
      default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	break;
    }
}

static void
vinagre_prefs_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
  VinagrePrefs *prefs = VINAGRE_PREFS (object);

  switch (prop_id)
    {
      case PROP_SHARED_FLAG:
	g_value_set_boolean (value, g_settings_get_boolean (prefs->priv->gsettings, VM_SHARED_FLAG));
	break;
      case PROP_HISTORY_SIZE:
	g_value_set_int (value, g_settings_get_int (prefs->priv->gsettings, VM_HISTORY_SIZE));
	break;
      case PROP_ALWAYS_ENABLE_LISTENING:
	g_value_set_boolean (value, g_settings_get_boolean (prefs->priv->gsettings, VM_ALWAYS_ENABLE_LISTENING));
	break;
      default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	break;
    }
}

static void
vinagre_prefs_dispose (GObject *object)
{
  VinagrePrefs *prefs = VINAGRE_PREFS (object);

  if (prefs->priv->gsettings)
    {
      g_object_unref (prefs->priv->gsettings);
      prefs->priv->gsettings = NULL;
    }

  G_OBJECT_CLASS (vinagre_prefs_parent_class)->dispose (object);
}


static void
vinagre_prefs_class_init (VinagrePrefsClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (VinagrePrefsPrivate));

  object_class->dispose = vinagre_prefs_dispose;
  object_class->set_property = vinagre_prefs_set_property;
  object_class->get_property = vinagre_prefs_get_property;

  g_object_class_install_property (object_class,
				   PROP_SHARED_FLAG,
				   g_param_spec_boolean ("shared-flag",
							 "Shared Flag",
							 "Whether we should share the remote connection",
							 TRUE,
							 G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class,
				   PROP_HISTORY_SIZE,
				   g_param_spec_int ("history-size",
						     "History size",
						     "Max number of items in history dropdown entry",
						     0, G_MAXINT, 15,
						     G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class,
				   PROP_ALWAYS_ENABLE_LISTENING,
				   g_param_spec_boolean ("always-enable-listening",
							 "Always enable listening",
							 "Whether we always should listen for reverse connections",
							 FALSE,
							 G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
}

/* Preferences dialog */

typedef struct {
  GtkBuilder  *xml;
  GtkWidget   *dialog;
  GtkWidget   *show_tabs;
  GtkWidget   *show_accels;
  GtkWindow   *parent;
} VinagrePrefsDialog;

static void
vinagre_prefs_dialog_response (GtkDialog *d, gint response_id, VinagrePrefsDialog *dialog)
{
  if (response_id > 0)
    {
      vinagre_utils_show_help (dialog->parent, "preferences");
      return;
    }

  gtk_widget_destroy (dialog->dialog);
  g_object_unref (dialog->xml);
  g_slice_free (VinagrePrefsDialog, dialog);
  dialog = NULL;
}

void
vinagre_prefs_dialog_show (VinagreWindow *window)
{
  VinagrePrefsDialog *dialog;
  VinagrePrefs *pref;

  dialog = g_slice_new (VinagrePrefsDialog);

  dialog->xml = vinagre_utils_get_builder ();
  dialog->dialog = GTK_WIDGET (gtk_builder_get_object (dialog->xml, "preferences_dialog"));
  dialog->parent = GTK_WINDOW (window);
  gtk_window_set_transient_for (GTK_WINDOW (dialog->dialog), dialog->parent);

  dialog->show_tabs = GTK_WIDGET (gtk_builder_get_object (dialog->xml, "always_show_tabs_check"));
  dialog->show_accels = GTK_WIDGET (gtk_builder_get_object (dialog->xml, "show_accels_check"));

  pref = vinagre_prefs_get_default ();
  g_settings_bind (pref->priv->gsettings, VM_ALWAYS_SHOW_TABS, dialog->show_tabs, "active", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind (pref->priv->gsettings, VM_SHOW_ACCELS, dialog->show_accels, "active", G_SETTINGS_BIND_DEFAULT);

  g_signal_connect (dialog->dialog,
		    "response",
		    G_CALLBACK (vinagre_prefs_dialog_response),
		    dialog);

  gtk_widget_show_all (dialog->dialog);
}
/* vim: set ts=8: */
