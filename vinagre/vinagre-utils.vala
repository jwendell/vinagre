/*  Vinagre - GNOME Remote Desktop viewer
 *
 *  Copyright (C) 2011  David King <amigadave@amigadave.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

namespace Vinagre.Utils {

static void show_error_dialog(string? title, string? message, Gtk.Window parent)
{
    if(title != null)
        title = _("An error occurred");

    var dialog = new Gtk.MessageDialog(parent,
        Gtk.DialogFlags.DESTROY_WITH_PARENT | Gtk.DialogFlags.MODAL,
        Gtk.MessageType.ERROR, Gtk.ButtonsType.CLOSE, "%s", title);

    if(message != null)
        dialog.format_secondary_markup("%s", message);

    dialog.response.connect((d, response) => { dialog.destroy(); });
    dialog.show_all();
}

static bool create_dir_for_file(string filename) throws GLib.Error
{
    var file = GLib.File.new_for_path(filename);
    var parent = file.get_parent();
    var parent_path = parent.get_path();

    if(!GLib.FileUtils.test(parent_path, GLib.FileTest.EXISTS))
        return parent.make_directory_with_parents();
    else
        return true;
}

static void show_help(Gtk.Window window, string? page)
{
    string uri;
    if(page != null)
        uri = "ghelp:" + Vinagre.Config.PACKAGE_TARNAME + "?" + page;
    else
        uri = "ghelp:" + Vinagre.Config.PACKAGE_TARNAME;

    try
    {
        Gtk.show_uri(window.get_screen(), uri, Gdk.CURRENT_TIME);
    }
    catch(GLib.Error error)
    {
        show_error_dialog(_("Error showing help"), error.message, window);
    }
}

// TODO: Move this into the GtkBuilder file.
static void show_help_about(Gtk.Window parent)
{
    string[] authors = { "David King <amigadave@amigadave.com>",
        "Jonh Wendell <jwendell@gnome.org>" };

    string[] artists = { "Vinicius Depizzol <vdepizzol@gmail.com>" };

    const string copyright = "Copyright \xc2\xa9 2007-2011 Jonh Wendell\nCopyright \xc2\xa9 2011 David King";

    const string comments = N_("Vinagre is a remote desktop viewer for the GNOME Desktop");

    const string license = """Vinagre is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

Vinagre is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see <http://www.gnu.org/licenses/>.""";

    Gtk.show_about_dialog(parent, "authors", authors, "artists", artists,
        "comments", _(comments), "copyright", copyright, "license", license,
        "wrap-license", true, "logo-icon-name", Vinagre.Config.PACKAGE_TARNAME,
        "translator-credits", _("translator-credits"),
        "version", Vinagre.Config.PACKAGE_VERSION,
        "website", Vinagre.Config.PACKAGE_URL,
        "website-label", _("Vinagre Website"));
}

} // namespace Vinagre.Utils
