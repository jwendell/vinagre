/*
 * vinagre-prefs-manager.c
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

#include <string.h>

#include <glib/gi18n.h>
#include <gconf/gconf-client.h>

#include "vinagre-prefs-manager.h"

#define DEFINE_BOOL_PREF(name, key, def) gboolean 			\
vinagre_prefs_manager_get_ ## name (void)					\
{									\
	return vinagre_prefs_manager_get_bool (key,			\
					     (def));			\
}									\
									\
void 									\
vinagre_prefs_manager_set_ ## name (gboolean v)				\
{									\
	vinagre_prefs_manager_set_bool (key,				\
				      v);				\
}									\
				      					\

#define DEFINE_INT_PREF(name, key, def) gint	 			\
vinagre_prefs_manager_get_ ## name (void)			 		\
{									\
	return vinagre_prefs_manager_get_int (key,			\
					    (def));			\
}									\
									\
void 									\
vinagre_prefs_manager_set_ ## name (gint v)				\
{									\
	vinagre_prefs_manager_set_int (key,				\
				     v);				\
}									\
				      					\


#define DEFINE_STRING_PREF(name, key, def) gchar*	 		\
vinagre_prefs_manager_get_ ## name (void)			 		\
{									\
	return vinagre_prefs_manager_get_string (key,			\
					       def);			\
}									\
									\
void 									\
vinagre_prefs_manager_set_ ## name (const gchar* v)			\
{									\
	vinagre_prefs_manager_set_string (key,				\
				        v);				\
}									\
				      					\

struct _VinagrePrefsManager {
  GConfClient *gconf_client;
};

VinagrePrefsManager *vinagre_prefs_manager = NULL;

static gint window_state = -1;
static gint window_height = -1;
static gint window_width = -1;
static gint side_panel_size = -1;

static gboolean 	 gconf_client_get_bool_with_default 	(GConfClient* client, 
								 const gchar* key, 
								 gboolean def, 
								 GError** err);

static gchar		*gconf_client_get_string_with_default 	(GConfClient* client, 
								 const gchar* key,
								 const gchar* def, 
								 GError** err);

static gint		 gconf_client_get_int_with_default 	(GConfClient* client, 
								 const gchar* key,
								 gint def, 
								 GError** err);

static gboolean		 vinagre_prefs_manager_get_bool		(const gchar* key, 
								 gboolean def);

static gint		 vinagre_prefs_manager_get_int		(const gchar* key, 
								 gint def);

static gchar		*vinagre_prefs_manager_get_string	(const gchar* key, 
								 const gchar* def);


gboolean
vinagre_prefs_manager_init (void)
{
  if (vinagre_prefs_manager == NULL)
    {
      GConfClient *gconf_client;

      gconf_client = gconf_client_get_default ();
      if (gconf_client == NULL)
        {
          g_warning (_("Cannot initialize preferences manager."));
          return FALSE;
        }

      gconf_client_add_dir (gconf_client, VINAGRE_BASE_KEY, GCONF_CLIENT_PRELOAD_ONELEVEL, NULL);
      vinagre_prefs_manager = g_new0 (VinagrePrefsManager, 1);
      vinagre_prefs_manager->gconf_client = gconf_client;
    }

  if (vinagre_prefs_manager->gconf_client == NULL)
    {
      g_free (vinagre_prefs_manager);
      vinagre_prefs_manager = NULL;
    }

  return vinagre_prefs_manager != NULL;
}

void
vinagre_prefs_manager_shutdown ()
{
  g_return_if_fail (vinagre_prefs_manager != NULL);

  gconf_client_remove_dir (vinagre_prefs_manager->gconf_client,
			   VINAGRE_BASE_KEY,
			   NULL);
  g_object_unref (vinagre_prefs_manager->gconf_client);
  vinagre_prefs_manager->gconf_client = NULL;
}

static gboolean		 
vinagre_prefs_manager_get_bool (const gchar* key, gboolean def)
{
  g_return_val_if_fail (vinagre_prefs_manager != NULL, def);
  g_return_val_if_fail (vinagre_prefs_manager->gconf_client != NULL, def);

  return gconf_client_get_bool_with_default (vinagre_prefs_manager->gconf_client,
					     key,
					     def,
					     NULL);
}

static gint 
vinagre_prefs_manager_get_int (const gchar* key, gint def)
{
  g_return_val_if_fail (vinagre_prefs_manager != NULL, def);
  g_return_val_if_fail (vinagre_prefs_manager->gconf_client != NULL, def);

  return gconf_client_get_int_with_default (vinagre_prefs_manager->gconf_client,
					    key,
					    def,
					    NULL);
}

static gchar *
vinagre_prefs_manager_get_string (const gchar* key, const gchar* def)
{
  g_return_val_if_fail (vinagre_prefs_manager != NULL, 
			def ? g_strdup (def) : NULL);
  g_return_val_if_fail (vinagre_prefs_manager->gconf_client != NULL, 
			def ? g_strdup (def) : NULL);

  return gconf_client_get_string_with_default (vinagre_prefs_manager->gconf_client,
					       key,
					       def,
					       NULL);
}

static void		 
vinagre_prefs_manager_set_bool (const gchar* key, gboolean value)
{
  g_return_if_fail (vinagre_prefs_manager != NULL);
  g_return_if_fail (vinagre_prefs_manager->gconf_client != NULL);
  g_return_if_fail (gconf_client_key_is_writable (
		    vinagre_prefs_manager->gconf_client, key, NULL));
			
  gconf_client_set_bool (vinagre_prefs_manager->gconf_client, key, value, NULL);
}

static void		 
vinagre_prefs_manager_set_int (const gchar* key, gint value)
{
  g_return_if_fail (vinagre_prefs_manager != NULL);
  g_return_if_fail (vinagre_prefs_manager->gconf_client != NULL);
  g_return_if_fail (gconf_client_key_is_writable (
		    vinagre_prefs_manager->gconf_client, key, NULL));
			
  gconf_client_set_int (vinagre_prefs_manager->gconf_client, key, value, NULL);
}

static void		 
vinagre_prefs_manager_set_string (const gchar* key, const gchar* value)
{
  g_return_if_fail (value != NULL);
	
  g_return_if_fail (vinagre_prefs_manager != NULL);
  g_return_if_fail (vinagre_prefs_manager->gconf_client != NULL);
  g_return_if_fail (gconf_client_key_is_writable (
		    vinagre_prefs_manager->gconf_client, key, NULL));
			
  gconf_client_set_string (vinagre_prefs_manager->gconf_client, key, value, NULL);
}

static gboolean 
vinagre_prefs_manager_key_is_writable (const gchar* key)
{
  g_return_val_if_fail (vinagre_prefs_manager != NULL, FALSE);
  g_return_val_if_fail (vinagre_prefs_manager->gconf_client != NULL, FALSE);

  return gconf_client_key_is_writable (vinagre_prefs_manager->gconf_client, key, NULL);
}

/* Toolbar visibility */
DEFINE_BOOL_PREF (toolbar_visible,
		  VPM_TOOLBAR_VISIBLE,
		  VPM_DEFAULT_TOOLBAR_VISIBLE)

/* Statusbar visibility */
DEFINE_BOOL_PREF (statusbar_visible,
		  VPM_STATUSBAR_VISIBLE,
		  VPM_DEFAULT_STATUSBAR_VISIBLE)
		  
/* Side Pane visibility */
DEFINE_BOOL_PREF (side_pane_visible,
		  VPM_SIDE_PANE_VISIBLE,
		  VPM_DEFAULT_SIDE_PANE_VISIBLE)

/* Window state */
gint
vinagre_prefs_manager_get_window_state (void)
{
  if (window_state == -1)
    window_state = vinagre_prefs_manager_get_int (VPM_WINDOW_STATE,
						  VPM_DEFAULT_WINDOW_STATE);

  return window_state;
}
			
void
vinagre_prefs_manager_set_window_state (gint ws)
{
  g_return_if_fail (ws > -1);
	
  window_state = ws;
  vinagre_prefs_manager_set_int (VPM_WINDOW_STATE, ws);
}

/* Window size */
void
vinagre_prefs_manager_get_window_size (gint *width, gint *height)
{
  g_return_if_fail (width != NULL && height != NULL);

  if (window_width == -1)
    window_width = vinagre_prefs_manager_get_int (VPM_WINDOW_WIDTH,
						  VPM_DEFAULT_WINDOW_WIDTH);

  if (window_height == -1)
    window_height = vinagre_prefs_manager_get_int (VPM_WINDOW_HEIGHT,
						   VPM_DEFAULT_WINDOW_HEIGHT);

  *width  = window_width;
  *height = window_height;
}

void
vinagre_prefs_manager_set_window_size (gint width, gint height)
{
  g_return_if_fail (width > -1 && height > -1);

  window_width = width;
  window_height = height;

  vinagre_prefs_manager_set_int (VPM_WINDOW_WIDTH, width);
  vinagre_prefs_manager_set_int (VPM_WINDOW_HEIGHT, height);
}

/* Side panel */
gint
vinagre_prefs_manager_get_side_panel_size (void)
{
  if (side_panel_size == -1)
    side_panel_size = vinagre_prefs_manager_get_int (VPM_SIDE_PANEL_SIZE,
						     VPM_DEFAULT_SIDE_PANEL_SIZE);

  return side_panel_size;
}

void 
vinagre_prefs_manager_set_side_panel_size (gint ps)
{
  g_return_if_fail (ps > -1);
	
  if (side_panel_size == ps)
    return;
		
  side_panel_size = ps;
  vinagre_prefs_manager_set_int (VPM_SIDE_PANEL_SIZE, ps);
}

/* The following functions are taken from gconf-client.c 
 * and partially modified. 
 * The licensing terms on these is: 
 *
 * 
 * GConf
 * Copyright (C) 1999, 2000, 2000 Red Hat Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


static const gchar* 
gconf_value_type_to_string(GConfValueType type)
{
  switch (type)
    {
    case GCONF_VALUE_INT:
      return "int";
      break;
    case GCONF_VALUE_STRING:
      return "string";
      break;
    case GCONF_VALUE_FLOAT:
      return "float";
      break;
    case GCONF_VALUE_BOOL:
      return "bool";
      break;
    case GCONF_VALUE_SCHEMA:
      return "schema";
      break;
    case GCONF_VALUE_LIST:
      return "list";
      break;
    case GCONF_VALUE_PAIR:
      return "pair";
      break;
    case GCONF_VALUE_INVALID:
      return "*invalid*";
      break;
    default:
      g_return_val_if_reached (NULL);
      break;
    }
}

/* Emit the proper signals for the error, and fill in err */
static gboolean
handle_error (GConfClient* client, GError* error, GError** err)
{
  if (error != NULL)
    {
      gconf_client_error(client, error);
      
      if (err == NULL)
        {
          gconf_client_unreturned_error(client, error);

          g_error_free(error);
        }
      else
        *err = error;

      return TRUE;
    }
  else
    return FALSE;
}

static gboolean
check_type (const gchar* key, GConfValue* val, GConfValueType t, GError** err)
{
  if (val->type != t)
    {
      /*
      gconf_set_error(err, GCONF_ERROR_TYPE_MISMATCH,
                      _("Expected `%s' got `%s' for key %s"),
                      gconf_value_type_to_string(t),
                      gconf_value_type_to_string(val->type),
                      key);
      */
      g_set_error (err, GCONF_ERROR, GCONF_ERROR_TYPE_MISMATCH,
	  	   _("Expected `%s' got `%s' for key %s"),
                   gconf_value_type_to_string(t),
                   gconf_value_type_to_string(val->type),
                   key);
	      
      return FALSE;
    }
  else
    return TRUE;
}

static gboolean
gconf_client_get_bool_with_default (GConfClient* client, const gchar* key,
                        	    gboolean def, GError** err)
{
  GError* error = NULL;
  GConfValue* val;

  g_return_val_if_fail (err == NULL || *err == NULL, def);

  val = gconf_client_get (client, key, &error);

  if (val != NULL)
    {
      gboolean retval = def;

      g_return_val_if_fail (error == NULL, retval);
      
      if (check_type (key, val, GCONF_VALUE_BOOL, &error))
        retval = gconf_value_get_bool (val);
      else
        handle_error (client, error, err);

      gconf_value_free (val);

      return retval;
    }
  else
    {
      if (error != NULL)
        handle_error (client, error, err);
      return def;
    }
}

static gchar*
gconf_client_get_string_with_default (GConfClient* client, const gchar* key,
                        	      const gchar* def, GError** err)
{
  GError* error = NULL;
  gchar* val;

  g_return_val_if_fail (err == NULL || *err == NULL, def ? g_strdup (def) : NULL);

  val = gconf_client_get_string (client, key, &error);

  if (val != NULL)
    {
      g_return_val_if_fail (error == NULL, def ? g_strdup (def) : NULL);
      
      return val;
    }
  else
    {
      if (error != NULL)
        handle_error (client, error, err);
      return def ? g_strdup (def) : NULL;
    }
}

static gint
gconf_client_get_int_with_default (GConfClient* client, const gchar* key,
                        	   gint def, GError** err)
{
  GError* error = NULL;
  GConfValue* val;

  g_return_val_if_fail (err == NULL || *err == NULL, def);

  val = gconf_client_get (client, key, &error);

  if (val != NULL)
    {
      gint retval = def;

      g_return_val_if_fail (error == NULL, def);
      
      if (check_type (key, val, GCONF_VALUE_INT, &error))
        retval = gconf_value_get_int(val);
      else
        handle_error (client, error, err);

      gconf_value_free (val);

      return retval;
    }
  else
    {
      if (error != NULL)
        handle_error (client, error, err);
      return def;
    }
}
