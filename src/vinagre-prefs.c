/*
 * vinagre-prefs.c
 * This file is part of vinagre
 *
 * Copyright (C) Jonh Wendell 2008 <wendell@bani.com.br>
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

#include <gconf/gconf-client.h>
#include <glib/gi18n.h>
#include "vinagre-prefs.h"

#define VINAGRE_BASE_KEY		"/apps/vinagre"
#define VM_ALWAYS_SHOW_TABS		VINAGRE_BASE_KEY "/always_show_tabs"
#define VM_SHARED_FLAG			VINAGRE_BASE_KEY "/shared_flag"
#define VM_TOOLBAR_VISIBLE	 	VINAGRE_BASE_KEY "/toolbar_visible"
#define VM_STATUSBAR_VISIBLE		VINAGRE_BASE_KEY "/statusbar_visible"
#define VM_SIDE_PANEL_VISIBLE		VINAGRE_BASE_KEY "/side_pane_visible"

#define VM_WINDOW_STATE			VINAGRE_BASE_KEY "/window_state"
#define VM_WINDOW_WIDTH			VINAGRE_BASE_KEY "/window_width"
#define VM_WINDOW_HEIGHT		VINAGRE_BASE_KEY "/window_height"
#define VM_SIDE_PANEL_SIZE		VINAGRE_BASE_KEY "/side_panel_size"

struct _VinagrePrefsPrivate
{
  GConfClient *gconf_client;
};

/* Properties */
enum
{
  PROP_0,
  PROP_SHARED_FLAG,
  PROP_ALWAYS_SHOW_TABS,
  PROP_TOOLBAR_VISIBLE,
  PROP_STATUSBAR_VISIBLE,
  PROP_SIDE_PANEL_VISIBLE,
  PROP_WINDOW_STATE,
  PROP_WINDOW_WIDTH,
  PROP_WINDOW_HEIGHT,
  PROP_SIDE_PANEL_SIZE
};

G_DEFINE_TYPE (VinagrePrefs, vinagre_prefs, G_TYPE_OBJECT);

static VinagrePrefs *prefs_singleton = NULL;

VinagrePrefs *
vinagre_prefs_get_default (void)
{
  if (G_UNLIKELY (!prefs_singleton))
    prefs_singleton = VINAGRE_PREFS (g_object_new (VINAGRE_TYPE_PREFS,
                                                   NULL));
  return prefs_singleton;
}

static gboolean
vinagre_prefs_get_bool (VinagrePrefs *prefs, const gchar* key, gboolean def)
{
  GError* error = NULL;
  GConfValue* val;

  val = gconf_client_get (prefs->priv->gconf_client, key, &error);

  if (val != NULL)
    {
      gboolean retval = def;

      g_return_val_if_fail (error == NULL, retval);
      
      if (val->type == GCONF_VALUE_BOOL)
        retval = gconf_value_get_bool (val);

      gconf_value_free (val);

      return retval;
    }
  else
      return def;
}

static gboolean
vinagre_prefs_get_int (VinagrePrefs *prefs, const gchar* key, gint def)
{
  GError* error = NULL;
  GConfValue* val;

  val = gconf_client_get (prefs->priv->gconf_client, key, &error);

  if (val != NULL)
    {
      gint retval = def;

      g_return_val_if_fail (error == NULL, retval);
      
      if (val->type == GCONF_VALUE_INT)
        retval = gconf_value_get_int (val);

      gconf_value_free (val);

      return retval;
    }
  else
      return def;
}

static void		 
vinagre_prefs_set_bool (VinagrePrefs *prefs, const gchar* key, gboolean value)
{
  g_return_if_fail (gconf_client_key_is_writable (
		    prefs->priv->gconf_client, key, NULL));
			
  gconf_client_set_bool (prefs->priv->gconf_client, key, value, NULL);
}

static void		 
vinagre_prefs_set_int (VinagrePrefs *prefs, const gchar* key, gint value)
{
  g_return_if_fail (gconf_client_key_is_writable (
		    prefs->priv->gconf_client, key, NULL));
			
  gconf_client_set_int (prefs->priv->gconf_client, key, value, NULL);
}

static void
vinagre_prefs_always_show_tabs_notify (GConfClient           *client,
				       guint                  cnx_id,
				       GConfEntry            *entry,
				       VinagrePrefs          *prefs)
{
  g_object_notify (G_OBJECT (prefs), "always-show-tabs");
}

static void
vinagre_prefs_init (VinagrePrefs *prefs)
{
  prefs->priv = G_TYPE_INSTANCE_GET_PRIVATE (prefs, VINAGRE_TYPE_PREFS, VinagrePrefsPrivate);

  prefs->priv->gconf_client = gconf_client_get_default ();
  if (prefs->priv->gconf_client == NULL)
    g_critical (_("Cannot initialize preferences manager."));

  gconf_client_add_dir (prefs->priv->gconf_client,
			VINAGRE_BASE_KEY,
			GCONF_CLIENT_PRELOAD_ONELEVEL,
			NULL);

  gconf_client_notify_add (prefs->priv->gconf_client,
			   VM_ALWAYS_SHOW_TABS,
                           (GConfClientNotifyFunc) vinagre_prefs_always_show_tabs_notify,
                           prefs, NULL, NULL);

}

static void
vinagre_prefs_finalize (GObject *object)
{
  VinagrePrefs *prefs = VINAGRE_PREFS (object);

  gconf_client_remove_dir (prefs->priv->gconf_client,
			   VINAGRE_BASE_KEY,
			   NULL);
  g_object_unref (prefs->priv->gconf_client);
  prefs->priv->gconf_client = NULL;

  G_OBJECT_CLASS (vinagre_prefs_parent_class)->finalize (object);
}

static void
vinagre_prefs_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
  VinagrePrefs *prefs = VINAGRE_PREFS (object);

  switch (prop_id)
    {
      case PROP_SHARED_FLAG:
	vinagre_prefs_set_bool (prefs, VM_SHARED_FLAG, g_value_get_boolean (value));
	break;
      case PROP_ALWAYS_SHOW_TABS:
	vinagre_prefs_set_bool (prefs, VM_ALWAYS_SHOW_TABS, g_value_get_boolean (value));
	break;
      case PROP_TOOLBAR_VISIBLE:
	vinagre_prefs_set_bool (prefs, VM_TOOLBAR_VISIBLE, g_value_get_boolean (value));
	break;
      case PROP_STATUSBAR_VISIBLE:
	vinagre_prefs_set_bool (prefs, VM_STATUSBAR_VISIBLE, g_value_get_boolean (value));
	break;
      case PROP_SIDE_PANEL_VISIBLE:
	vinagre_prefs_set_bool (prefs, VM_SIDE_PANEL_VISIBLE, g_value_get_boolean (value));
	break;
      case PROP_WINDOW_STATE:
	vinagre_prefs_set_int (prefs, VM_WINDOW_STATE, g_value_get_int (value));
	break;
      case PROP_WINDOW_WIDTH:
	vinagre_prefs_set_int (prefs, VM_WINDOW_WIDTH, g_value_get_int (value));
	break;
      case PROP_WINDOW_HEIGHT:
	vinagre_prefs_set_int (prefs, VM_WINDOW_HEIGHT, g_value_get_int (value));
	break;
      case PROP_SIDE_PANEL_SIZE:
	vinagre_prefs_set_int (prefs, VM_SIDE_PANEL_SIZE, g_value_get_int (value));
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
	g_value_set_boolean (value, vinagre_prefs_get_bool (prefs, VM_SHARED_FLAG, TRUE));
	break;
      case PROP_ALWAYS_SHOW_TABS:
	g_value_set_boolean (value, vinagre_prefs_get_bool (prefs, VM_ALWAYS_SHOW_TABS, TRUE));
	break;
      case PROP_TOOLBAR_VISIBLE:
	g_value_set_boolean (value, vinagre_prefs_get_bool (prefs, VM_TOOLBAR_VISIBLE, TRUE));
	break;
      case PROP_STATUSBAR_VISIBLE:
	g_value_set_boolean (value, vinagre_prefs_get_bool (prefs, VM_STATUSBAR_VISIBLE, TRUE));
	break;
      case PROP_SIDE_PANEL_VISIBLE:
	g_value_set_boolean (value, vinagre_prefs_get_bool (prefs, VM_SIDE_PANEL_VISIBLE, TRUE));
	break;
      case PROP_WINDOW_STATE:
	g_value_set_int (value, vinagre_prefs_get_int (prefs, VM_WINDOW_STATE, 0));
	break;
      case PROP_WINDOW_WIDTH:
	g_value_set_int (value, vinagre_prefs_get_int (prefs, VM_WINDOW_WIDTH, 650));
	break;
      case PROP_WINDOW_HEIGHT:
	g_value_set_int (value, vinagre_prefs_get_int (prefs, VM_WINDOW_HEIGHT, 500));
	break;
      case PROP_SIDE_PANEL_SIZE:
	g_value_set_int (value, vinagre_prefs_get_int (prefs, VM_SIDE_PANEL_SIZE, 200));
	break;
      default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	break;
    }
}

static void
vinagre_prefs_class_init (VinagrePrefsClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);
  GObjectClass* parent_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (VinagrePrefsPrivate));

  object_class->finalize = vinagre_prefs_finalize;
  object_class->set_property = vinagre_prefs_set_property;
  object_class->get_property = vinagre_prefs_get_property;

  g_object_class_install_property (object_class,
				   PROP_SHARED_FLAG,
				   g_param_spec_boolean ("shared-flag",
							 "Shared Flag",
							 "Whether we should share the remote connection",
							 TRUE,
							 G_PARAM_READWRITE));
  g_object_class_install_property (object_class,
				   PROP_ALWAYS_SHOW_TABS,
				   g_param_spec_boolean ("always-show-tabs",
							 "Always show tabs",
							 "Whether we should show the tabs even when there is ony one active connection",
							 TRUE,
							 G_PARAM_READWRITE));
  g_object_class_install_property (object_class,
				   PROP_TOOLBAR_VISIBLE,
				   g_param_spec_boolean ("toolbar-visible",
							 "Toolbar Visibility",
							 "Whether the toolbar is visible",
							 TRUE,
							 G_PARAM_READWRITE));
  g_object_class_install_property (object_class,
				   PROP_STATUSBAR_VISIBLE,
				   g_param_spec_boolean ("statusbar-visible",
							 "Statusbar Visibility",
							 "Whether the statusbar is visible",
							 TRUE,
							 G_PARAM_READWRITE));
  g_object_class_install_property (object_class,
				   PROP_SIDE_PANEL_VISIBLE,
				   g_param_spec_boolean ("side-panel-visible",
							 "Side Panel Visibility",
							 "Whether the side panel is visible",
							 TRUE,
							 G_PARAM_READWRITE));
  g_object_class_install_property (object_class,
				   PROP_WINDOW_STATE,
				   g_param_spec_int ("window-state",
						     "Window State",
						     "Whether the window is maximised",
						     G_MININT, G_MAXINT, 0,
						     G_PARAM_READWRITE));
  g_object_class_install_property (object_class,
				   PROP_WINDOW_WIDTH,
				   g_param_spec_int ("window-width",
						     "Window Width",
						     "The width of window",
						     100, G_MAXINT, 650,
						     G_PARAM_READWRITE));
  g_object_class_install_property (object_class,
				   PROP_WINDOW_HEIGHT,
				   g_param_spec_int ("window-height",
						     "Window Height",
						     "The height of window",
						     100, G_MAXINT, 500,
						     G_PARAM_READWRITE));
  g_object_class_install_property (object_class,
				   PROP_SIDE_PANEL_SIZE,
				   g_param_spec_int ("side-panel-size",
						     "Side Panel Width",
						     "The width of side panel",
						     100, G_MAXINT, 200,
						     G_PARAM_READWRITE));

}
/* vim: ts=8 */
