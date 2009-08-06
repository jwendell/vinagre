/*
 * vinagre-spinner.h
 * This file is part of vinagre
 *
 * Copyright (C) 2005 - Paolo Maggi 
 * Copyright (C) 2000 - Eazel, Inc. 
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
 
/*
 * This widget was originally written by Andy Hertzfeld <andy@eazel.com> for
 * Nautilus.
 */

#ifndef __VINAGRE_SPINNER_H__
#define __VINAGRE_SPINNER_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define VINAGRE_TYPE_SPINNER		(vinagre_spinner_get_type ())
#define VINAGRE_SPINNER(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), VINAGRE_TYPE_SPINNER, VinagreSpinner))
#define VINAGRE_SPINNER_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), VINAGRE_TYPE_SPINNER, VinagreSpinnerClass))
#define VINAGRE_IS_SPINNER(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), VINAGRE_TYPE_SPINNER))
#define VINAGRE_IS_SPINNER_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), VINAGRE_TYPE_SPINNER))
#define VINAGRE_SPINNER_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), VINAGRE_TYPE_SPINNER, VinagreSpinnerClass))


/* Private structure type */
typedef struct _VinagreSpinnerPrivate	VinagreSpinnerPrivate;

/*
 * Main object structure
 */
typedef struct _VinagreSpinner		VinagreSpinner;

struct _VinagreSpinner
{
	GtkWidget parent;

	/*< private >*/
	VinagreSpinnerPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _VinagreSpinnerClass	VinagreSpinnerClass;

struct _VinagreSpinnerClass
{
	GtkWidgetClass parent_class;
};

/*
 * Public methods
 */
GType		vinagre_spinner_get_type	(void) G_GNUC_CONST;

GtkWidget      *vinagre_spinner_new	(void);

void		vinagre_spinner_start	(VinagreSpinner *throbber);

void		vinagre_spinner_stop	(VinagreSpinner *throbber);

void		vinagre_spinner_set_size	(VinagreSpinner *spinner,
					 GtkIconSize   size);

G_END_DECLS

#endif /* __VINAGRE_SPINNER_H__ */
