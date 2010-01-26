/*
 * vinagre-ssh.h
 * SSH Utilities for Vinagre
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

#ifndef __VINAGRE_SSH_H__
#define __VINAGRE_SSH_H__

#define VINAGRE_SSH_CHECK "ViNagRE_CHEck"
#define VINAGRE_SSH_CHECK_LENGTH 13

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef enum 
{
  VINAGRE_SSH_ERROR_FAILED = 1,
  VINAGRE_SSH_ERROR_INVALID_CLIENT,
  VINAGRE_SSH_ERROR_TIMEOUT,
  VINAGRE_SSH_ERROR_PASSWORD_CANCELED,
  VINAGRE_SSH_ERROR_IO_ERROR,
  VINAGRE_SSH_ERROR_PERMISSION_DENIED,
  VINAGRE_SSH_ERROR_HOST_NOT_FOUND
} VinagreSshError;

#define VINAGRE_SSH_ERROR vinagre_ssh_error_quark()
GQuark vinagre_ssh_error_quark (void);

gboolean vinagre_ssh_connect (GtkWindow *parent,
			      const gchar *hostname,
			      gint port,
			      const gchar *username,
			      gchar **extra_arguments,
			      gchar **command,
			      gint *tty,
			      GError **error);

G_END_DECLS

#endif  /* __VINAGRE_SSH_H__  */
/* vim: set ts=8: */
