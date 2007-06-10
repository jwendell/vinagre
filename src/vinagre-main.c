#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include <config.h>
#include "vinagre-main.h"
#include "vinagre-prefs-manager.h"

int main (int argc, char **argv) {
  
  bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);

  g_thread_init (NULL);

  gtk_init (&argc, &argv);

  g_set_application_name (_("Vinagre"));
  vinagre_prefs_manager_init ();

  main_window = vinagre_window_new ();

  gtk_widget_show (GTK_WIDGET(main_window));
  gtk_main ();

  vinagre_prefs_manager_shutdown ();

  return 0;
}
