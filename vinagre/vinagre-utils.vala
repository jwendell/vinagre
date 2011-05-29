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

static bool parse_boolean(string str)
{
    if(str == "true" || str == "1")
        return true;
    else
        return false;
}

static void toggle_widget_visible(Gtk.Widget widget)
{
    if(widget.visible)
        widget.hide();
    else
        widget.show_all();
}

static void show_error_dialog(string? title, string? message, Gtk.Window? parent)
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

static void show_many_errors(string? title, GLib.SList<string> items, Gtk.Window parent)
{
    string messages = "";
    items.foreach((message) => messages.printf("%s\n", message));
    show_error_dialog(title, messages, parent);
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

static Gtk.Builder get_builder()
{
    string filename = GLib.Path.build_filename(Vinagre.Config.VINAGRE_DATADIR,
        "vinagre.ui");

    var builder = new Gtk.Builder();
    try
    {
        builder.add_from_file(filename);
    }
    catch(GLib.Error err)
    {
        string subtitle = _("Vinagre failed to open a UI file, with the error message:");
	string closing = _("Please check your installation.");
	string message = "%s\n\n%s\n\n%s".printf(subtitle, err.message,
            closing);
        show_error_dialog(_("Error loading UI file"), message, null);
    }

    return builder;
}

static bool request_credential(Gtk.Window parent, string protocol, string host,
    bool need_username, bool need_password, int password_limit,
    out string username, out string password, out bool save_in_keyring)
{
    var xml = get_builder();

    var password_dialog = xml.get_object("auth_required_dialog") as Gtk.Dialog;
    password_dialog.set_transient_for(parent);
    string auth_label_message =
        // Translators: %s is a protocol, like VNC or SSH.
        _("%s authentication is required").printf(protocol);

    var auth_label = xml.get_object("auth_required_label") as Gtk.Label;
    auth_label.label = auth_label_message;

    var host_label = xml.get_object("host_label") as Gtk.Label;
    host_label.label = host;

    var password_label = xml.get_object("password_label") as Gtk.Label;
    var username_label = xml.get_object("username_label") as Gtk.Label;
    var save_credential_check = xml.get_object("save_credential_check")
        as Gtk.CheckButton;

    var ok_button = xml.get_object("ok_button") as Gtk.Button;
    var image = new Gtk.Image.from_stock(Gtk.Stock.DIALOG_AUTHENTICATION,
        Gtk.IconSize.DIALOG);
    ok_button.image = image;

    var username_entry = xml.get_object("username_entry") as Gtk.Entry;
    var password_entry = xml.get_object("password_entry") as Gtk.Entry;
    username_entry.changed.connect(() => {
        var enabled = true;

        if(username_entry.visible)
            enabled = enabled && username_entry.text_length > 0;

        if(password_entry.visible)
            enabled = enabled && password_entry.text_length > 0;

        ok_button.sensitive = enabled;
    });

    if(need_username)
        username_entry.text = username;
    else
    {
        username_label.hide();
	username_entry.hide();
    }

    password_entry.changed.connect(() => {
        var enabled = true;

        if(username_entry.visible)
            enabled = enabled && username_entry.text_length > 0;

        if(password_entry.visible)
            enabled = enabled && password_entry.text_length > 0;

        ok_button.sensitive = enabled;
    });
    if(need_password)
    {
        password_entry.max_length = password_limit;
        password_entry.text = password;
    }
    else
    {
        password_label.hide();
        password_entry.hide();
    }

    var result = password_dialog.run();
    if(result == Gtk.ResponseType.OK)
    {
        if(username_entry.text_length > 0)
            username = username_entry.text;

        if(password_entry.text_length > 0)
            password = password_entry.text;

        if(save_in_keyring)
            save_in_keyring = save_credential_check.active;
    }

    password_dialog.destroy();
    return result == Gtk.ResponseType.OK;
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
