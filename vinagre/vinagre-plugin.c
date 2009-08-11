/*
 * vinagre-plugin.c
 * This file is part of vinagre

 * Based on gedit plugin system
 * Copyright (C) 2002-2005 Paolo Maggi
 * Copyright (C) 2009 Jorge Pereira <jorge@jorgepereira.com.br>
 * Copyright (C) 2009 Jonh Wendell <wendell@bani.com.br>
 * 
 * vinagre-plugin.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * vinagre-plugin.c is distributed in the hope that it will be useful, but
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

#include "vinagre-plugin.h"
#include "vinagre-plugin-info.h"
#include "vinagre-dirs.h"

/* properties */
enum {
	PROP_0,
	PROP_INSTALL_DIR,
	PROP_DATA_DIR_NAME,
	PROP_DATA_DIR
};

typedef struct _VinagrePluginPrivate VinagrePluginPrivate;

struct _VinagrePluginPrivate
{
  gchar *install_dir;
  gchar *data_dir_name;
};

#define VINAGRE_PLUGIN_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), VINAGRE_TYPE_PLUGIN, VinagrePluginPrivate))

G_DEFINE_TYPE(VinagrePlugin, vinagre_plugin, G_TYPE_OBJECT)

static void
dummy (VinagrePlugin *plugin, VinagreWindow *window)
{
  /* Empty */
}

static GtkWidget *
create_configure_dialog	(VinagrePlugin *plugin)
{
  return NULL;
}

static GSList *
default_context_groups (VinagrePlugin *plugin)
{
  return NULL;
}

static const gchar *
default_get_protocol (VinagrePlugin *plugin)
{
  return NULL;
}

static gchar **
default_get_public_description (VinagrePlugin *plugin)
{
  return NULL;
}

static VinagreConnection *
default_new_connection (VinagrePlugin *plugin)
{
  return NULL;
}

static GtkWidget *
default_new_tab (VinagrePlugin *plugin,
		 VinagreConnection *conn,
		 VinagreWindow     *window)
{
  return NULL;
}

static VinagreConnection *
default_new_connection_from_file (VinagrePlugin *plugin,
				  const gchar   *data,
				  gboolean       use_bookmarks,
				  gchar        **error_msg)
{
  return NULL;
}

static gint
default_get_default_port (VinagrePlugin *plugin)
{
  return -1;
}

static gboolean
is_configurable (VinagrePlugin *plugin)
{
	return (VINAGRE_PLUGIN_GET_CLASS (plugin)->create_configure_dialog !=
		create_configure_dialog);
}

static GtkWidget *
default_get_connect_widget (VinagrePlugin     *plugin,
			    VinagreConnection *initial_settings)
{
  return NULL;
}

static GtkFileFilter *
default_get_file_filter (VinagrePlugin *plugin)
{
  return NULL;
}

static void
vinagre_plugin_get_property (GObject    *object,
			   guint       prop_id,
			   GValue     *value,
			   GParamSpec *pspec)
{
	switch (prop_id)
	{
		case PROP_INSTALL_DIR:
			g_value_take_string (value, vinagre_plugin_get_install_dir (VINAGRE_PLUGIN (object)));
			break;
		case PROP_DATA_DIR:
			g_value_take_string (value, vinagre_plugin_get_data_dir (VINAGRE_PLUGIN (object)));
			break;
		default:
			g_return_if_reached ();
	}
}

static void
vinagre_plugin_set_property (GObject      *object,
			   guint         prop_id,
			   const GValue *value,
			   GParamSpec   *pspec)
{
	VinagrePluginPrivate *priv = VINAGRE_PLUGIN_GET_PRIVATE (object);

	switch (prop_id)
	{
		case PROP_INSTALL_DIR:
			priv->install_dir = g_value_dup_string (value);
			break;
		case PROP_DATA_DIR_NAME:
			priv->data_dir_name = g_value_dup_string (value);
			break;
		default:
			g_return_if_reached ();
	}
}

static void
vinagre_plugin_finalize (GObject *object)
{
	VinagrePluginPrivate *priv = VINAGRE_PLUGIN_GET_PRIVATE (object);

	g_free (priv->install_dir);
	g_free (priv->data_dir_name);

	G_OBJECT_CLASS (vinagre_plugin_parent_class)->finalize (object);
}

static void 
vinagre_plugin_class_init (VinagrePluginClass *klass)
{
    	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	klass->activate = dummy;
	klass->deactivate = dummy;
	klass->update_ui = dummy;
	klass->get_context_groups = default_context_groups;
	klass->get_protocol = default_get_protocol;
	klass->get_public_description = default_get_public_description;
	klass->get_default_port = default_get_default_port;
	klass->new_connection = default_new_connection;
	klass->new_connection_from_file = default_new_connection_from_file;
	klass->get_mdns_service = default_get_protocol;
	klass->new_tab = default_new_tab;
	klass->get_connect_widget = default_get_connect_widget;
	klass->get_file_filter = default_get_file_filter;
	
	klass->create_configure_dialog = create_configure_dialog;
	klass->is_configurable = is_configurable;

	object_class->get_property = vinagre_plugin_get_property;
	object_class->set_property = vinagre_plugin_set_property;
	object_class->finalize = vinagre_plugin_finalize;

	g_object_class_install_property (object_class,
					 PROP_INSTALL_DIR,
					 g_param_spec_string ("install-dir",
							      "Install Directory",
							      "The directory where the plugin is installed",
							      NULL,
							      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

	/* the basename of the data dir is set at construction time by the plugin loader
	 * while the full path is constructed on the fly to take into account relocability
	 * that's why we have a writeonly prop and a readonly prop */
	g_object_class_install_property (object_class,
					 PROP_DATA_DIR_NAME,
					 g_param_spec_string ("data-dir-name",
							      "Basename of the data directory",
							      "The basename of the directory where the plugin should look for its data files",
							      NULL,
							      G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property (object_class,
					 PROP_DATA_DIR,
					 g_param_spec_string ("data-dir",
							      "Data Directory",
							      "The full path of the directory where the plugin should look for its data files",
							      NULL,
							      G_PARAM_READABLE));

	g_type_class_add_private (klass, sizeof (VinagrePluginPrivate));
}

static void
vinagre_plugin_init (VinagrePlugin *plugin)
{
	/* Empty */
}

/**
 * vinagre_plugin_get_install_dir:
 * @plugin: a #VinagrePlugin
 *
 * Get the path of the directory where the plugin is installed.
 *
 * Return value: a newly allocated string with the path of the
 * directory where the plugin is installed
 */
gchar *
vinagre_plugin_get_install_dir (VinagrePlugin *plugin)
{
	g_return_val_if_fail (VINAGRE_IS_PLUGIN (plugin), NULL);

	return g_strdup (VINAGRE_PLUGIN_GET_PRIVATE (plugin)->install_dir);
}

/**
 * vinagre_plugin_get_data_dir:
 * @plugin: a #VinagrePlugin
 *
 * Get the path of the directory where the plugin should look for
 * its data files.
 *
 * Return value: a newly allocated string with the path of the
 * directory where the plugin should look for its data files
 */
gchar *
vinagre_plugin_get_data_dir (VinagrePlugin *plugin)
{
	VinagrePluginPrivate *priv;
	gchar *vinagre_lib_dir;
	gchar *data_dir;

	g_return_val_if_fail (VINAGRE_IS_PLUGIN (plugin), NULL);

	priv = VINAGRE_PLUGIN_GET_PRIVATE (plugin);

	/* If it's a "user" plugin the data dir is
	 * install_dir/data_dir_name if instead it's a
	 * "system" plugin the data dir is under vinagre_data_dir,
	 * so it's under $prefix/share/vinagre-2/plugins/data_dir_name
	 * where data_dir_name usually it's the name of the plugin
	 */
//	vinagre_lib_dir = vinagre_dirs_get_vinagre_lib_dir ();

	/* CHECK: is checking the prefix enough or should we be more
	 * careful about normalizing paths etc? */
	if (g_str_has_prefix (priv->install_dir, vinagre_lib_dir))
	{
		gchar *vinagre_data_dir;

//		vinagre_data_dir = vinagre_dirs_get_vinagre_data_dir ();

		data_dir = g_build_filename (vinagre_data_dir,
					     "plugins",
					     priv->data_dir_name,
					     NULL);

		g_free (vinagre_data_dir);
	}
	else
	{
		data_dir = g_build_filename (priv->install_dir,
					     priv->data_dir_name,
					     NULL);
	}

	g_free (vinagre_lib_dir);

	return data_dir;
}

/**
 * vinagre_plugin_activate:
 * @plugin: a #VinagrePlugin
 * @window: a #VinagreWindow
 * 
 * Activates the plugin.
 */
void
vinagre_plugin_activate (VinagrePlugin *plugin,
			 VinagreWindow *window)
{
  g_return_if_fail (VINAGRE_IS_PLUGIN (plugin));
	
  VINAGRE_PLUGIN_GET_CLASS (plugin)->activate (plugin, window);
}

/**
 * vinagre_plugin_deactivate:
 * @plugin: a #VinagrePlugin
 * @window: a #VinagreWindow
 * 
 * Deactivates the plugin.
 */
void
vinagre_plugin_deactivate (VinagrePlugin *plugin,
			   VinagreWindow *window)
{
  g_return_if_fail (VINAGRE_IS_PLUGIN (plugin));

  VINAGRE_PLUGIN_GET_CLASS (plugin)->deactivate (plugin, window);
}

/**
 * vinagre_plugin_update_ui:
 * @plugin: a #VinagrePlugin
 * @window: a #VinagreWindow
 *
 * Triggers an update of the user interface to take into account state changes
 * caused by the plugin.
 */		 
void
vinagre_plugin_update_ui	(VinagrePlugin *plugin,
			 VinagreWindow *window)
{
	g_return_if_fail (VINAGRE_IS_PLUGIN (plugin));
	g_return_if_fail (VINAGRE_IS_WINDOW (window));

	VINAGRE_PLUGIN_GET_CLASS (plugin)->update_ui (plugin, window);
}

/**
 * vinagre_plugin_is_configurable:
 * @plugin: a #VinagrePlugin
 *
 * Whether the plugin is configurable.
 *
 * Returns: TRUE if the plugin is configurable:
 */
gboolean
vinagre_plugin_is_configurable (VinagrePlugin *plugin)
{
	g_return_val_if_fail (VINAGRE_IS_PLUGIN (plugin), FALSE);

	return VINAGRE_PLUGIN_GET_CLASS (plugin)->is_configurable (plugin);
}

/**
 * vinagre_plugin_create_configure_dialog:
 * @plugin: a #VinagrePlugin
 *
 * Creates the configure dialog widget for the plugin.
 *
 * Returns: the configure dialog widget for the plugin.
 */
GtkWidget *
vinagre_plugin_create_configure_dialog (VinagrePlugin *plugin)
{
  g_return_val_if_fail (VINAGRE_IS_PLUGIN (plugin), NULL);
	
  return VINAGRE_PLUGIN_GET_CLASS (plugin)->create_configure_dialog (plugin);
}

/**
 * vinagre_plugin_get_context_groups
 * @plugin: a #VinagrePlugin
 *
 *
 * Returns: a list of context groups to be used on command line, if available
 */
GSList *
vinagre_plugin_get_context_groups (VinagrePlugin *plugin)
{
  g_return_val_if_fail (VINAGRE_IS_PLUGIN (plugin), NULL);
	
  return VINAGRE_PLUGIN_GET_CLASS (plugin)->get_context_groups (plugin);
}

/**
 * vinagre_plugin_get_protocol
 * @plugin: a #VinagrePlugin
 *
 *
 * Returns: a protocol, like "vnc" or "rdp"
 */
const gchar *
vinagre_plugin_get_protocol (VinagrePlugin *plugin)
{
  g_return_val_if_fail (VINAGRE_IS_PLUGIN (plugin), NULL);
	
  return VINAGRE_PLUGIN_GET_CLASS (plugin)->get_protocol (plugin);
}

/**
 * vinagre_plugin_get_public_description
 * @plugin: a #VinagrePlugin
 *
 *
 * Returns: an array of strings:
 * [0] -> the protocol name, like VNC, or SSH
 * [1] -> the protocol description, like "A secure shell access"
 */
gchar **
vinagre_plugin_get_public_description (VinagrePlugin *plugin)
{
  g_return_val_if_fail (VINAGRE_IS_PLUGIN (plugin), NULL);
	
  return VINAGRE_PLUGIN_GET_CLASS (plugin)->get_public_description (plugin);
}

/**
 * vinagre_plugin_new_connection
 * @plugin: a #VinagrePlugin
 *
 *
 * Returns: a subclass of the Connection class
 */
VinagreConnection *
vinagre_plugin_new_connection (VinagrePlugin *plugin)
{
  g_return_val_if_fail (VINAGRE_IS_PLUGIN (plugin), NULL);
	
  return VINAGRE_PLUGIN_GET_CLASS (plugin)->new_connection (plugin);
}

/**
 * vinagre_plugin_new_connection_from_file
 * @plugin: a #VinagrePlugin
 *
 *
 * Returns: a subclass of the Connection class
 */
VinagreConnection *
vinagre_plugin_new_connection_from_file (VinagrePlugin *plugin,
					 const gchar   *data,
					 gboolean       use_bookmarks,
					 gchar        **error_msg)
{
  g_return_val_if_fail (VINAGRE_IS_PLUGIN (plugin), NULL);
	
  return VINAGRE_PLUGIN_GET_CLASS (plugin)->new_connection_from_file (plugin, data, use_bookmarks, error_msg);
}

/**
 * vinagre_plugin_get_default_port
 * @plugin: a #VinagrePlugin
 *
 *
 * Returns: the default port (ex: 5900 for vnc)
 */
gint
vinagre_plugin_get_default_port (VinagrePlugin *plugin)
{
  g_return_val_if_fail (VINAGRE_IS_PLUGIN (plugin), -1);
	
  return VINAGRE_PLUGIN_GET_CLASS (plugin)->get_default_port (plugin);
}

/**
 * vinagre_plugin_get_mdns_service
 * @plugin: a #VinagrePlugin
 *
 *
 * Returns: a mDNS service for this plugin, like rfb.tcp
 */
const gchar *
vinagre_plugin_get_mdns_service (VinagrePlugin *plugin)
{
  g_return_val_if_fail (VINAGRE_IS_PLUGIN (plugin), NULL);
	
  return VINAGRE_PLUGIN_GET_CLASS (plugin)->get_mdns_service (plugin);
}

/**
 * vinagre_plugin_new_tab
 * @plugin: a #VinagreTab
 *
 *
 * Returns: a subclass of the Tab class
 */
GtkWidget *
vinagre_plugin_new_tab (VinagrePlugin     *plugin,
			VinagreConnection *conn,
			VinagreWindow     *window)
{
  g_return_val_if_fail (VINAGRE_IS_PLUGIN (plugin), NULL);

  return VINAGRE_PLUGIN_GET_CLASS (plugin)->new_tab (plugin, conn, window);
}

/**
 * vinagre_plugin_get_connect_widget
 * @plugin: a #VinagrePlugin
 * @initial_settings: a #VinagreConnection object, or NULL
 *
 *
 * Returns: a widget to be put inside connect dialog
 */
GtkWidget *
vinagre_plugin_get_connect_widget (VinagrePlugin     *plugin,
				   VinagreConnection *initial_settings)
{
  g_return_val_if_fail (VINAGRE_IS_PLUGIN (plugin), NULL);

  return VINAGRE_PLUGIN_GET_CLASS (plugin)->get_connect_widget (plugin, initial_settings);
}

/**
 * vinagre_plugin_get_file_filter
 * @plugin: a #VinagrePlugin
 *
 *
 * Returns: a filter to be used at Open File dialog
 */
GtkFileFilter *
vinagre_plugin_get_file_filter (VinagrePlugin *plugin)
{
  g_return_val_if_fail (VINAGRE_IS_PLUGIN (plugin), NULL);

  return VINAGRE_PLUGIN_GET_CLASS (plugin)->get_file_filter (plugin);
}

GdkPixbuf *
vinagre_plugin_get_icon (VinagrePlugin *plugin, gint size)
{
  VinagrePluginInfo *info;

  g_return_val_if_fail (VINAGRE_IS_PLUGIN (plugin), NULL);

  info = g_object_get_data (G_OBJECT (plugin), "info");
  g_return_val_if_fail (info != NULL, NULL);

  return gtk_icon_theme_load_icon (gtk_icon_theme_get_default (),
				   vinagre_plugin_info_get_icon_name (info),
				   size,
				   0,
				   NULL);
}

const gchar *
vinagre_plugin_get_icon_name (VinagrePlugin *plugin)
{
  VinagrePluginInfo *info;

  g_return_val_if_fail (VINAGRE_IS_PLUGIN (plugin), NULL);

  info = g_object_get_data (G_OBJECT (plugin), "info");
  g_return_val_if_fail (info != NULL, NULL);

  return vinagre_plugin_info_get_icon_name (info);
}

/* vim: set ts=8: */
