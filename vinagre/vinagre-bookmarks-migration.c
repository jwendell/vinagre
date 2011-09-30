/*
 * vinagre-bookmarks-migration.c
 * This file is part of vinagre
 *
 * Copyright (C) 2008  Jonh Wendell <wendell@bani.com.br>
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

/*
 * This is a temporary hack to migrate the bookmarks from an .ini format to
 * a XML one. The new format is used in Vinagre 2.25.
 */

#include <config.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <glib/gi18n.h>
#include <libxml/parser.h>
#include <libxml/xmlwriter.h>

#include "vinagre-bookmarks-entry.h"
#include "vinagre-connection.h"
#include "vinagre-bookmarks-migration.h"
#include "vinagre-bookmarks.h"
#include "vinagre-plugins-engine.h"
#include "vinagre-vala.h"

static void
fill_xml (GSList *list, xmlTextWriter *writer)
{
  GSList                *l;
  VinagreBookmarksEntry *entry;
  VinagreConnection     *conn;

  for (l = list; l; l = l->next)
    {
      entry = (VinagreBookmarksEntry *) l->data;
      switch (vinagre_bookmarks_entry_get_node (entry))
	{
	  case VINAGRE_BOOKMARKS_ENTRY_NODE_FOLDER:
	    xmlTextWriterStartElement (writer, BAD_CAST "folder");
	    xmlTextWriterWriteAttribute (writer, BAD_CAST "name", BAD_CAST vinagre_bookmarks_entry_get_name (entry));

	    fill_xml (vinagre_bookmarks_entry_get_children (entry), writer);
	    xmlTextWriterEndElement (writer);
	    break;

	  case VINAGRE_BOOKMARKS_ENTRY_NODE_CONN:
	    conn = vinagre_bookmarks_entry_get_conn (entry);

	    xmlTextWriterStartElement (writer, BAD_CAST "item");
	    xmlTextWriterWriteElement (writer, BAD_CAST "name", BAD_CAST vinagre_connection_get_name (conn));
	    xmlTextWriterWriteElement (writer, BAD_CAST "host", BAD_CAST vinagre_connection_get_host (conn));
	    xmlTextWriterWriteFormatElement (writer, BAD_CAST "port", "%d", vinagre_connection_get_port (conn));
	    xmlTextWriterWriteFormatElement (writer, BAD_CAST "fullscreen", "%d", vinagre_connection_get_fullscreen (conn));

	    xmlTextWriterEndElement (writer);
	    break;

	  default:
	    g_assert_not_reached ();
	}
    }
}

static gboolean
save_to_file (GSList *entries, const gchar *filename)
{
  xmlTextWriter *writer;
  xmlBuffer     *buf;
  int            rc;
  GError        *error;
  gboolean       result;

  writer = NULL;
  buf    = NULL;
  result = FALSE;

  buf = xmlBufferCreate ();
  if (!buf)
    {
      g_warning (_("Error while migrating bookmarks: Failed to create the XML structure"));
      return FALSE;
    }

  writer = xmlNewTextWriterMemory(buf, 0);
  if (!writer)
    {
      g_warning (_("Error while migrating bookmarks: Failed to create the XML structure"));
      goto finalize;
    }

  rc = xmlTextWriterStartDocument (writer, NULL, "utf-8", NULL);
  if (rc < 0)
    {
      g_warning (_("Error while migrating bookmarks: Failed to initialize the XML structure"));
      goto finalize;
    }

  rc = xmlTextWriterStartElement (writer, BAD_CAST "vinagre-bookmarks");
  if (rc < 0)
    {
      g_warning (_("Error while migrating bookmarks: Failed to initialize the XML structure"));
      goto finalize;
    }

  fill_xml (entries, writer);

  rc = xmlTextWriterEndDocument (writer);
  if (rc < 0)
    {
      g_warning (_("Error while migrating bookmarks: Failed to finalize the XML structure"));
      goto finalize;
    }

  error  = NULL;
  if (!g_file_set_contents (filename,
			    (const char *) buf->content,
			    -1,
			    &error))
    {
      g_warning (_("Error while migrating bookmarks: %s"), error?error->message:_("Unknown error"));
      if (error)
	g_error_free (error);
      goto finalize;
    }

  result = TRUE;

finalize:
  if (writer)
    xmlFreeTextWriter (writer);
  if (buf)
    xmlBufferFree (buf);

  return result;
}

static GSList *
create_list (GKeyFile *kf)
{
  GSList *entries;
  gchar **conns;
  gsize   length, i;
  VinagreProtocol *ext;

  ext = vinagre_plugins_engine_get_plugin_by_protocol (vinagre_plugins_engine_get_default (), "vnc");

  if (!ext)
    {
      g_warning (_("Error while migrating bookmarks: VNC plugin is not activated"));
      return NULL;
    }

  entries = NULL;
  conns = g_key_file_get_groups (kf, &length);
  for (i=0; i<length; i++)
    {
      VinagreConnection *conn;
      gchar             *s_value;
      gint               i_value;

      s_value = g_key_file_get_string (kf, conns[i], "host", NULL);
      if (!s_value)
        continue;

      conn = vinagre_protocol_new_connection (ext);
      i_value = g_key_file_get_integer (kf, conns[i], "port", NULL);
      if (i_value == 0)
        i_value = 5900;

      g_object_set (conn,
		    "name", conns[i],
		    "host", s_value,
		    "port", i_value,
		    "view-only", g_key_file_get_boolean (kf, conns[i], "view_only", NULL),
		    "fullscreen", g_key_file_get_boolean (kf, conns[i], "fullscreen", NULL),
		    "scaling", g_key_file_get_boolean (kf, conns[i], "scaling", NULL),
		    NULL);

      entries = g_slist_insert_sorted  (entries,
					vinagre_bookmarks_entry_new_conn (conn),
					(GCompareFunc)vinagre_bookmarks_entry_compare);
      g_object_unref (conn);
      g_free (s_value);
    }

  g_strfreev (conns);
  return entries;
}

void
vinagre_bookmarks_migration_migrate (const gchar *filename)
{
  gchar    *old, *dir;
  GKeyFile *kf;
  GError   *error;
  GSList   *entries;

  error  = NULL;
  if (!vinagre_utils_create_dir_for_file (filename, &error))
    {
      g_warning (_("Error while migrating bookmarks: %s"), error?error->message:_("Failed to create the directory"));
      if (error)
	g_error_free (error);
      return;
    }

  dir = vinagre_dirs_get_user_data_dir ();
  old = g_build_filename (dir,
			  VINAGRE_BOOKMARKS_FILE_OLD,
			  NULL);
  g_free (dir);
  if (!g_file_test (old, G_FILE_TEST_EXISTS))
    {
      g_free (old);
      old = g_build_filename (g_get_home_dir (),
			      ".gnome2",
			      VINAGRE_BOOKMARKS_FILE_OLD,
			      NULL);
      if (!g_file_test (old, G_FILE_TEST_EXISTS))
	{
	  g_free (old);
	  return;
	}
    }

  g_message (_("Migrating the bookmarks file to the new format. This operation is only supposed to run once."));

  kf = g_key_file_new ();
  if (!g_key_file_load_from_file (kf,
				  old,
				  G_KEY_FILE_NONE,
				  &error))
    {
      g_warning (_("Error opening old bookmarks file: %s"), error->message);
      g_warning (_("Migration cancelled"));
      g_error_free (error);
      return;
    }

  entries = create_list (kf);
  if (save_to_file (entries, filename))
    {
      if (g_unlink (old) != 0)
	g_warning (_("Could not remove the old bookmarks file"));
    }
  else
    g_warning (_("Migration cancelled"));

  g_slist_free_full (entries, g_object_unref);
  g_key_file_free (kf);
  g_free (old);
}

/* vim: set ts=8: */
