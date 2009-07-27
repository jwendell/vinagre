/*
 * vinagre-debug.c
 * This file is part of vinagre
 *
 * Based on gedit code
 * Copyright (C) 1998, 1999 Alex Roberts, Evan Lawrence
 * Copyright (C) 2000, 2001 Chema Celorio, Paolo Maggi
 * Copyright (C) 2002 - 2005 Paolo Maggi
 * 
 * vinagre-debug.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * vinagre-debug.c is distributed in the hope that it will be useful, but
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

#include <stdio.h>
#include "vinagre-debug.h"

/* #define ENABLE_PROFILING */

#ifdef ENABLE_PROFILING
static GTimer *timer = NULL;
static gdouble last = 0.0;
#endif

static VinagreDebugSection debug = VINAGRE_NO_DEBUG;

void
vinagre_debug_init ()
{
  if (g_getenv ("VINAGRE_DEBUG") != NULL)
    {
      /* enable all debugging */
      debug = ~VINAGRE_NO_DEBUG;
      goto out;
    }

  if (g_getenv ("VINAGRE_DEBUG_VIEW") != NULL)
    debug = debug | VINAGRE_DEBUG_VIEW;
  if (g_getenv ("VINAGRE_DEBUG_PREFS") != NULL)
    debug = debug | VINAGRE_DEBUG_PREFS;
  if (g_getenv ("VINAGRE_DEBUG_PRINT") != NULL)
    debug = debug | VINAGRE_DEBUG_PRINT;
  if (g_getenv ("VINAGRE_DEBUG_PLUGINS") != NULL)
    debug = debug | VINAGRE_DEBUG_PLUGINS;
  if (g_getenv ("VINAGRE_DEBUG_UTILS") != NULL)
    debug = debug | VINAGRE_DEBUG_UTILS;
  if (g_getenv ("VINAGRE_DEBUG_WINDOW") != NULL)
    debug = debug | VINAGRE_DEBUG_WINDOW;
  if (g_getenv ("VINAGRE_DEBUG_LOADER") != NULL)
    debug = debug | VINAGRE_DEBUG_LOADER;
  if (g_getenv ("VINAGRE_DEBUG_APP") != NULL)
    debug = debug | VINAGRE_DEBUG_APP;
  if (g_getenv ("VINAGRE_DEBUG_TUBE") != NULL)
    debug = debug | VINAGRE_DEBUG_TUBE;

out:		

#ifdef ENABLE_PROFILING
	if (debug != VINAGRE_NO_DEBUG)
		timer = g_timer_new ();
#endif
	return;
}

void
vinagre_debug_message (VinagreDebugSection  section,
		       const gchar       *file,
		       gint               line,
		       const gchar       *function,
		       const gchar       *format, ...)
{
  if (G_UNLIKELY (debug & section))
    {	
#ifdef ENABLE_PROFILING
      gdouble seconds;
#endif

      va_list args;
      gchar *msg;

      g_return_if_fail (format != NULL);

      va_start (args, format);
      msg = g_strdup_vprintf (format, args);
      va_end (args);

#ifdef ENABLE_PROFILING
      g_return_if_fail (timer != NULL);

      seconds = g_timer_elapsed (timer, NULL);
      g_print ("[%f (%f)] %s:%d (%s) %s\n", 
		 seconds, seconds - last,  file, line, function, msg);
      last = seconds;			 
#else
      g_print ("%s:%d (%s) %s\n", file, line, function, msg);	
#endif

      fflush (stdout);
      g_free (msg);
    }
}

void vinagre_debug (VinagreDebugSection  section,
		    const gchar       *file,
		    gint               line,
		    const gchar       *function)
{
  if (G_UNLIKELY (debug & section))
    {
#ifdef ENABLE_PROFILING
      gdouble seconds;

      g_return_if_fail (timer != NULL);

      seconds = g_timer_elapsed (timer, NULL);		
      g_print ("[%f (%f)] %s:%d (%s)\n", 
	       seconds, seconds - last, file, line, function);
      last = seconds;
#else
      g_print ("%s:%d (%s)\n", file, line, function);
#endif		
      fflush (stdout);
    }
}
/* vim: set ts=8: */
