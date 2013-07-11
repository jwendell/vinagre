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

using Gtk;

namespace Vinagre.Utils {
    public bool parse_boolean (string str) {
        if (str == "true" || str == "1")
            return true;
        else
            return false;
    }

    public void set_widget_visible (Widget widget, bool visible) {
        if (visible)
            widget.show_all ();
        else
            widget.hide ();
    }

    public void show_error_dialog (string? title,
                                   string? message,
                                   Window? parent) {
        if (title == null)
            title = _("An error occurred");

        var dialog = new MessageDialog (parent,
                                        DialogFlags.DESTROY_WITH_PARENT |
                                        DialogFlags.MODAL,
                                        MessageType.ERROR,
                                        ButtonsType.CLOSE,
                                        "%s",
                                        title);

        if (message != null)
            dialog.format_secondary_markup ("%s", message);

        dialog.response.connect ((d, response) => { dialog.destroy (); });
        dialog.show_all ();
    }

    public void show_many_errors (string?       title,
                                  SList<string> items,
                                  Window        parent) {
        var messages = "";
        items.foreach ((message) => messages += "%s\n".printf (message));
        show_error_dialog (title, messages, parent);
    }

    public bool create_dir_for_file (string filename) throws Error {
        var file = File.new_for_path (filename);
        var parent = file.get_parent ();
        var parent_path = parent.get_path ();

        if (!FileUtils.test (parent_path, FileTest.EXISTS))
            return parent.make_directory_with_parents ();
        else
            return true;
    }

    public Builder get_builder () {
        var filename = Vinagre.Dirs.get_package_data_file ("vinagre.ui");

        var builder = new Builder ();
        try {
            builder.add_from_file (filename);
        } catch (Error err) {
            var subtitle =
                _("Vinagre failed to open a UI file, with the error message:");
            var closing = _("Please check your installation.");
            var message = "%s\n\n%s\n\n%s".printf (subtitle,
                                                   err.message,
                                                   closing);
            show_error_dialog (_("Error loading UI file"), message, null);
        }

        return builder;
    }

    public bool request_credential (Window     parent,
                                    string     protocol,
                                    string     host,
                                    bool       need_username,
                                    bool       need_password,
                                    int        password_limit,
                                    out string username,
                                    out string password,
                                    out bool   save_in_keyring)
    {
        var xml = get_builder ();

        var password_dialog = xml.get_object ("auth_required_dialog") as Dialog;
        password_dialog.set_transient_for (parent);

        var auth_label = xml.get_object ("auth_required_label") as Label;
        // Translators: %s is a protocol, like VNC or SSH
        auth_label.label = _("%s authentication is required").printf (protocol);

        var host_label = xml.get_object ("host_label") as Label;
        host_label.label = host;

        var password_label = xml.get_object ("password_label") as Label;
        var username_label = xml.get_object ("username_label") as Label;
        var save_credential_check = xml.get_object ("save_credential_check")
                                    as CheckButton;

        var ok_button = xml.get_object ("ok_button") as Button;
        var image = new Image.from_stock (Stock.DIALOG_AUTHENTICATION,
                                          IconSize.BUTTON);
        ok_button.image = image;

        var username_entry = xml.get_object ("username_entry") as Entry;
        var password_entry = xml.get_object ("password_entry") as Entry;

        username_entry.changed.connect (() => {
            var enabled = true;

            if (username_entry.visible)
                enabled = enabled && username_entry.text_length > 0;

            if (password_entry.visible)
                enabled = enabled && password_entry.text_length > 0;

            ok_button.sensitive = enabled;
        });

        if (!need_username) {
            username_label.hide ();
            username_entry.hide ();
        }

        password_entry.changed.connect (() => {
            var enabled = true;

            if (username_entry.visible)
                enabled = enabled && username_entry.text_length > 0;

            if (password_entry.visible)
                enabled = enabled && password_entry.text_length > 0;

            ok_button.sensitive = enabled;
        });

        if (need_password) {
            password_entry.max_length = password_limit;
        } else {
            password_label.hide ();
            password_entry.hide ();
        }

        var result = password_dialog.run ();
        if (result == ResponseType.OK) {
            if (username_entry.text_length > 0)
                username = username_entry.text;

            if (password_entry.text_length > 0)
                password = password_entry.text;

            save_in_keyring = save_credential_check.active;
        }

        password_dialog.destroy ();

        return result == ResponseType.OK;
    }

    public void show_help (Window window, string? page)
    {
        string uri;
        if (page != null)
            uri = "help:" + Vinagre.Config.PACKAGE_TARNAME + "/" + page;
        else
            uri = "help:" + Vinagre.Config.PACKAGE_TARNAME;

        try {
            show_uri (window.get_screen (), uri, Gdk.CURRENT_TIME);
        } catch (Error error) {
            show_error_dialog (_("Error showing help"), error.message, window);
        }
    }

    public void show_help_about (Window parent) {
        var builder = get_builder ();
        var dialog = builder.get_object ("about_dialog") as Gtk.AboutDialog;

        dialog.logo_icon_name = "preferences-desktop-remote-desktop";
        dialog.version = Vinagre.Config.PACKAGE_VERSION;
        dialog.website = Vinagre.Config.PACKAGE_URL;
        dialog.set_transient_for (parent);

        dialog.run ();
        dialog.hide ();
    }
} // namespace Vinagre.Utils
