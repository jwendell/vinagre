/*
 * vinagre-dirs.c
 * This file is part of vinagre
 *
 * Copyright (C) 2009 Jorge Pereira <jorge@jorgepereira.com.br>
 * 
 * vinagre-dirs.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * vinagre-dirs.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "vinagre-dirs.h"

gchar *
vinagre_dirs_get_user_config_dir ()
{
	gchar *config_dir = NULL;

#ifndef G_OS_WIN32
	const gchar *home;
	
	home = g_get_home_dir ();

	if (home != NULL)
	{
		config_dir = g_build_filename (home,
					       ".gnome2",
					       "vinagre",
					       NULL);
	}
#else
	config_dir = g_build_filename (g_get_user_config_dir (),
				       "vinagre",
				       NULL);
#endif

	return config_dir;
}

gchar *
vinagre_dirs_get_user_cache_dir ()
{
	const gchar *cache_dir;

	cache_dir = g_get_user_cache_dir ();

	return g_build_filename (cache_dir,
				 "vinagre",
				 NULL);
}

gchar *
vinagre_dirs_get_user_plugins_dir (void)
{
	gchar *config_dir;
	gchar *plugin_dir;

	config_dir = vinagre_dirs_get_user_config_dir ();

	plugin_dir = g_build_filename (config_dir,
				       "plugins",
				       NULL);
	g_free (config_dir);
	
	return plugin_dir;
}

gchar *
vinagre_dirs_get_user_accels_file ()
{
	gchar *accels = NULL;

#ifndef G_OS_WIN32
	const gchar *home;
	
	home = g_get_home_dir ();

	if (home != NULL)
	{
		/* on linux accels are stored in .gnome2/accels
		 * for historic reasons (backward compat with the
		 * old libgnome that took care of saving them */
		accels = g_build_filename (home,
					   ".gnome2",
					   "accels",
					   "vinagre",
					   NULL);
	}
#else
	{
		gchar *config_dir = NULL;

		config_dir = vinagre_dirs_get_user_config_dir ();
		accels = g_build_filename (config_dir,
					   "accels",
					   "vinagre",
					   NULL);

		g_free (config_dir);
	}
#endif

	return accels;
}

gchar *
vinagre_dirs_get_vinagre_data_dir (void)
{
	gchar *data_dir;

#ifndef G_OS_WIN32
	data_dir = g_build_filename (DATADIR,
				     "vinagre",
				     NULL);
#else
	gchar *win32_dir;
	
	win32_dir = g_win32_get_package_installation_directory_of_module (NULL);

	data_dir = g_build_filename (win32_dir,
				     "share",
				     "vinagre",
				     NULL);
	
	g_free (win32_dir);
#endif

	return data_dir;
}

gchar *
vinagre_dirs_get_vinagre_locale_dir (void)
{
	gchar *locale_dir;

#ifndef G_OS_WIN32
	locale_dir = g_build_filename (DATADIR,
				       "locale",
				       NULL);
#else
	gchar *win32_dir;
	
	win32_dir = g_win32_get_package_installation_directory_of_module (NULL);

	locale_dir = g_build_filename (win32_dir,
				       "share",
				       "locale",
				       NULL);
	
	g_free (win32_dir);
#endif

	return locale_dir;
}

gchar *
vinagre_dirs_get_vinagre_lib_dir (void)
{
	gchar *lib_dir;

#ifndef G_OS_WIN32
	lib_dir = g_build_filename (LIBDIR,
				    "vinagre-1",
				    NULL);
#else
	gchar *win32_dir;
	
	win32_dir = g_win32_get_package_installation_directory_of_module (NULL);

	lib_dir = g_build_filename (win32_dir,
				    "lib",
				    "vinagre-1",
				    NULL);
	
	g_free (win32_dir);
#endif

	return lib_dir;
}

gchar *
vinagre_dirs_get_vinagre_plugins_dir (void)
{
	gchar *lib_dir;
	gchar *plugin_dir;
	
	lib_dir = vinagre_dirs_get_vinagre_lib_dir ();
	
	plugin_dir = g_build_filename (lib_dir,
				       "plugins",
				       NULL);
	g_free (lib_dir);
	
	return plugin_dir;
}

gchar *
vinagre_dirs_get_vinagre_plugin_loaders_dir (void)
{
	gchar *lib_dir;
	gchar *loader_dir;
	
	lib_dir = vinagre_dirs_get_vinagre_lib_dir ();
	
	loader_dir = g_build_filename (lib_dir,
				       "plugin-loaders",
				       NULL);
	g_free (lib_dir);
	
	return loader_dir;
}

gchar *
vinagre_dirs_get_ui_file (const gchar *file)
{
	gchar *datadir;
	gchar *ui_file;

	g_return_val_if_fail (file != NULL, NULL);
	
	datadir = vinagre_dirs_get_vinagre_data_dir ();
	ui_file = g_build_filename (datadir,
				    "ui",
				    file,
				    NULL);
	g_free (datadir);
	
	return ui_file;
}
