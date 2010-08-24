/*
 * vinagre-reverse-vnc-plugin.c
 * This file is part of vinagre
 *
 * Copyright (C) 2009-2010 Jonh Wendell <wendell@bani.com.br>
 * 
 * vinagre-reverse-vnc-plugin.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * vinagre-reverse-vnc-plugin.c is distributed in the hope that it will be useful, but
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

#include <glib/gi18n-lib.h>
#include <gmodule.h>

#include <vinagre/vinagre-prefs.h>

#include "vinagre-reverse-vnc-plugin.h"
#include "vinagre-reverse-vnc-listener-dialog.h"
#include "vinagre-reverse-vnc-listener.h"

#define WINDOW_DATA_KEY "VinagreVNCPluginWindowData"

static void vnc_activatable_iface_init (PeasActivatableInterface *iface);
G_DEFINE_DYNAMIC_TYPE_EXTENDED (VinagreReverseVncPlugin,
				vinagre_reverse_vnc_plugin,
				PEAS_TYPE_EXTENSION_BASE,
				0,
				G_IMPLEMENT_INTERFACE_DYNAMIC (PEAS_TYPE_ACTIVATABLE,
							       vnc_activatable_iface_init))

struct _VinagreReverseVncPluginPrivate
{
  VinagreWindow *window;
  GtkActionGroup *ui_action_group;
  guint ui_id;
  VinagreReverseVncListener *listener;
};

enum
{
  PROP_0,
  PROP_OBJECT
};

static void
listening_cb (GtkAction *action, VinagreReverseVncPlugin *plugin)
{
  vinagre_reverse_vnc_listener_dialog_show (GTK_WINDOW (plugin->priv->window));
}

static GtkActionEntry action_entries[] =
{
  { "VNCListener",
    NULL,
    /* Translators: "Reverse" here is an adjective, not a verb. */
    N_("_Reverse Connections..."),
    NULL,
    N_("Configure incoming VNC connections"),
    G_CALLBACK (listening_cb)
  }
};

static void
impl_activate (PeasActivatable *activatable)
{
  VinagreReverseVncPluginPrivate *priv;
  GtkUIManager *manager;
  gboolean always;

  priv = VINAGRE_REVERSE_VNC_PLUGIN (activatable)->priv;

  manager = vinagre_window_get_ui_manager (priv->window);

  priv->ui_action_group = gtk_action_group_new ("VinagreReverseVNCPluginActions");
  gtk_action_group_set_translation_domain (priv->ui_action_group, GETTEXT_PACKAGE);
  gtk_action_group_add_actions (priv->ui_action_group,
				action_entries,
				G_N_ELEMENTS (action_entries),
				activatable);
  gtk_ui_manager_insert_action_group (manager,
				      priv->ui_action_group,
				      -1);

  priv->ui_id = gtk_ui_manager_new_merge_id (manager);
  gtk_ui_manager_add_ui (manager,
			 priv->ui_id,
			 "/MenuBar/MachineMenu/MachineOps_1",
			 "VNCListener",
			 "VNCListener",
			 GTK_UI_MANAGER_AUTO,
			 TRUE);

  g_object_get (vinagre_prefs_get_default (),
		"always-enable-listening", &always,
		NULL);
  if (always)
    vinagre_reverse_vnc_listener_start (priv->listener);
}

static void
impl_deactivate (PeasActivatable *activatable)
{
  VinagreReverseVncPluginPrivate *priv;
  GtkUIManager *manager;

  priv = VINAGRE_REVERSE_VNC_PLUGIN (activatable)->priv;

  manager = vinagre_window_get_ui_manager (priv->window);

  gtk_ui_manager_remove_ui (manager, priv->ui_id);
  gtk_ui_manager_remove_action_group (manager, priv->ui_action_group);
}

static void
vinagre_reverse_vnc_plugin_set_property (GObject      *object,
					 guint         prop_id,
					 const GValue *value,
					 GParamSpec   *pspec)
{
  VinagreReverseVncPlugin *plugin = VINAGRE_REVERSE_VNC_PLUGIN (object);

  switch (prop_id)
    {
      case PROP_OBJECT:
	plugin->priv->window = VINAGRE_WINDOW (g_value_dup_object (value));
	break;

      default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	break;
    }
}

static void
vinagre_reverse_vnc_plugin_get_property (GObject    *object,
					 guint       prop_id,
					 GValue     *value,
					 GParamSpec *pspec)
{
  VinagreReverseVncPlugin *plugin = VINAGRE_REVERSE_VNC_PLUGIN (object);

  switch (prop_id)
    {
      case PROP_OBJECT:
	g_value_set_object (value, plugin->priv->window);
	break;

      default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	break;
    }
}

static void
vinagre_reverse_vnc_plugin_dispose (GObject *object)
{
  VinagreReverseVncPluginPrivate *priv = VINAGRE_REVERSE_VNC_PLUGIN (object)->priv;

  if (priv->ui_action_group)
    {
      g_object_unref (priv->ui_action_group);
      priv->ui_action_group = NULL;
    }

  if (priv->window != NULL)
    {
      g_object_unref (priv->window);
      priv->window = NULL;
    }

  if (priv->listener)
    {
      g_object_unref (priv->listener);
      priv->listener = NULL;
    }

  G_OBJECT_CLASS (vinagre_reverse_vnc_plugin_parent_class)->dispose (object);
}

static void
vinagre_reverse_vnc_plugin_class_init (VinagreReverseVncPluginClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = vinagre_reverse_vnc_plugin_dispose;
  object_class->set_property = vinagre_reverse_vnc_plugin_set_property;
  object_class->get_property = vinagre_reverse_vnc_plugin_get_property;

  g_object_class_override_property (object_class, PROP_OBJECT, "object");

  g_type_class_add_private (klass, sizeof (VinagreReverseVncPluginPrivate));

}

static void
vinagre_reverse_vnc_plugin_class_finalize (VinagreReverseVncPluginClass *klass)
{
}

static void
vinagre_reverse_vnc_plugin_init (VinagreReverseVncPlugin *plugin)
{
  plugin->priv = G_TYPE_INSTANCE_GET_PRIVATE (plugin,
					      VINAGRE_TYPE_REVERSE_VNC_PLUGIN,
					      VinagreReverseVncPluginPrivate);

  plugin->priv->listener = vinagre_reverse_vnc_listener_get_default ();
}

static void
vnc_activatable_iface_init (PeasActivatableInterface *iface)
{
  iface->activate   = impl_activate;
  iface->deactivate = impl_deactivate;
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
  vinagre_reverse_vnc_plugin_register_type (G_TYPE_MODULE (module));
  peas_object_module_register_extension_type (module,
					      PEAS_TYPE_ACTIVATABLE,
					      VINAGRE_TYPE_REVERSE_VNC_PLUGIN);
}

/* vim: set ts=8: */
