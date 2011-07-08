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

public class Vinagre.Application : Gtk.Application {
    // FIXME: Place in optionstate struct.
    static string geometry;
    static bool fullscreen;
    static bool new_window;
    static string filenames[];
    static string uris[];

    private const string application_path = "org.gnome.Vinagre";
    // FIXME: Move into Vinagre.Options?
    const OptionEntry[] options = {
        {"geometry", 0, 0, OptionArg.STRING, ref geometry,
            N_("Specify geometry of the main Vinagre window"), null},
        {"fullscreen", 'f', 0, OptionArg.NONE, ref fullscreen,
            N_("Open Vinagre in fullscreen mode"), null},
        {"new-window", 'n', 0, OptionArg.NONE, ref new_window,
            N_("Create a new toplevel window in an existing instance of Vinagre"),
            null},
        {"file", 'F', 0, OptionArg.FILENAME_ARRAY, ref filenames,
            N_("Open a file recognized by Vinagre"), null},
        // "" in this case is equivalent to G_OPTION_REMAINING.
        {"", 0, 0, OptionArg.STRING_ARRAY, ref uris, null, N_("[server:port]")},
        {null}
    };

    public Application (string app_id, ApplicationFlags flags) {
        GLib.Object (application_id: app_id, flags: flags);
    }

    construct {
        activate.connect (on_activate);
        command_line.connect (on_command_line);
    }

    public void on_activate () {
        var windows = get_windows ();
        if (windows != null) {
            windows.foreach ((window) => {
                window.present_with_time (Gdk.CURRENT_TIME);
            });
        } else {
            var window = new Gtk.Window ();

            Environment.set_application_name (_("Remote Desktop Viewer"));
            window.set_default_icon_name (Config.PACKAGE_TARNAME);

            window.destroy.connect (Gtk.main_quit);
            window.show ();
        }
    }

    public int on_command_line (ApplicationCommandLine command_line) {
        var arguments = command_line.get_arguments ();

        try {
            // FIXME: make the string the same as above, and use printf.
            var context = new OptionContext (_("— Remote Desktop Viewer"));
            context.add_main_entries (options, Config.GETTEXT_PACKAGE);
            context.add_group (Gtk.get_option_group (true));

            // TODO: Register plugin stuff with the context here.
            context.parse (ref arguments);
            process_parsed_command_line ();
        }
        catch (OptionError error) {
            command_line.printerr ("%s\n", error.message);
            command_line.printerr (_("Run ‘%s --help’ to see a full list of available command-line options.\n"), arguments[0]);
            // FIXME: EXIT_FAILURE?
            command_line.set_exit_status (1);
        }
    }

    // FIXME: Should accept a window parameter.
    private void process_parsed_command_line () {
        SList<string> errors;

        if (filenames != null) {
            foreach (string file in filenames) {
                stdout.printf ("New connection from file: %s", file);
            };
        }

        if (uris != null) {
            foreach (string uri in uris) {
                stdout.printf ("New connection from URI: %s", uri);
            };
        }

        if (/* connections != null && */ new_window) {
            // TODO: Replace with Application.new_window().
            var window = new Gtk.Window ();
            window.show_all ();
            window.set_application (this);
        }

        var windows = get_windows ();
        // Set all windows to have the supplied geometry.
        if (geometry != null) {
            windows.foreach ((window) => {
                if (!window.parse_geometry (geometry))
                    errors.append (_("Invalid argument %s for --geometry").printf (geometry));
            });
        }

        // Use the first window as the parent.
        if (errors != null)
            Utils.show_many_errors (ngettext
                ("The following error has occurred:",
                "The following errors have occurred:", errors.length ()),
                errors, windows.first ());

        windows.foreach ((window) => {window.present ();});
    }
}

// Application entry point.
public int main (string[] args) {
    Intl.bindtextdomain (Vinagre.Config.GETTEXT_PACKAGE,
        Vinagre.Config.PACKAGE_LOCALEDIR);
    Intl.bind_textdomain_codeset (Vinagre.Config.GETTEXT_PACKAGE, "UTF-8");
    Intl.textdomain (Vinagre.Config.GETTEXT_PACKAGE);

    // This used to be "org.gnome.vinagre".
    var app = new Vinagre.Application ("org.gnome.Vinagre",
        ApplicationFlags.HANDLES_COMMAND_LINE);

    // run() initialises GTK+, so there is no need to do it manually.
    var result = app.run (args);

    return result;
}
