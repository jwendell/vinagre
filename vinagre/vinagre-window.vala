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

// Handles everything related to the toplevel window(s).
class Vinagre.Window : Gtk.Window {
    // Child widgets.
    private Vinagre.Notebook notebook;
    private Gtk.StatusBar statusbar;
    private Gtk.UIManager ui_manager;
    private Gtk.Toolbar toolbar;
    private Gtk.MenuBar menubar;
    private Gtk.Box main_box;

    // Action groups for merging into UIManager.
    private Gtk.ActionGroup always_sensitive_actions;
    private Gtk.ActionGroup remote_connected_actions;
    private Gtk.ActionGroup remote_initialized_actions;
    private Gtk.ActionGroup bookmarks_actions;
    private Gtk.ActionGroup recent_connections_actions;

    // Window state.
    private bool fullscreen = false;
    private bool toolbar_visible;
    private bool statusbar_visible;
    private SList<Vinagre.Connection> connections;

    // Signal connection and context IDs.
    private ulong update_recent_menu_id;
    private uint statusbar_tooltip_id;

    // Actions for action groups.
    private const Gtk.ActionEntry[] always_sensitive_entries = {
        // Toplevel (menu) actions.
        {null, "Remote", null, N_("_Remote"), null, null},
        {null, "Edit", null, N_("_Edit"), null, null},
        {null, "View", null, N_("_View"), null, null},
        {null, "Bookmarks", null, N_("_Bookmarks"), null, null},
        {null, "Help", null, N_("_Help"), null, null},

        // Remote menu.
        {Command.remote_connect, "RemoteConnect", Gtk.Stock.CONNECT,
            "<control>N", N_("Connect to a remote desktop")},
        {Command.remote_open, "RemoteOpen", Gtk.Stock.OPEN, "<control>O",
            N_("Open a .VNC file")},
        {Command.remote_vnc_listener, "VNCListener", null,
            N_("_Reverse Connections…"), null,
            N_("Configure incoming VNC connections")},
        {Command.remote_quit, "RemoteQuit", Gtk.Stock.QUIT, "<control>Q",
            N_("Quit the program")},

        // Edit menu.
        {Command.edit_preferences, "EditPreferences", Gtk.Stock.PREFERENCES,
            null, null, N_("Edit the application preferences")},

        // Help menu.
        {Command.help_contents, "HelpContents", Gtk.Stock.HELP, null, "F1",
            N_("Open the Vinagre help")},
        {Command.help_about, "HelpAbout", Gtk.Stock.ABOUT, null, null,
            N_("About this application")}
    };

    private const Gtk.ActionEntry[] remote_connected_entries = {
        // Remote menu.
        {Command.remote_disconnect, "RemoteDisconnect", Gtk.Stock.DISCONNECT,
            "<control>W", N_("Disconnect the current connection")},
        {Command.remote_disconnect_all, "RemoteDisconnectAll", null,
            "<control><shift>W", N_("Disconnect all connections")},

        // Bookmarks menu.
        {Command.bookmarks_add, "BookmarksAdd", Gtk.Stock.ADD, "<control>D",
            N_("Add the current connection to your bookmarks")}
    };

    private const Gtk.ActionEntry[] remote_initialized_entries = {
        // Remote menu.
        {Command.remote_take_screenshot, "RemoteTakeScreenshot",
            "applets-screenshooter", N_("_Take Screenshot"), null,
            N_("Take a screenshot of the current remote desktop")},

        // View menu.
        {Command.view_fullscreen, "ViewFullscreen", Gtk.Stock.FULLSCREEN, null,
            "F11", N_("View the current remote desktop in fullscreen mode")}
    };

    private const Gtk.ToggleActionEntry[] always_sensitive_toggle_entries = {
        // View menu.
        {Command.view_show_toolbar, "ViewToolbar", null, N_("_Toolbar"), null,
            N_("Show or hide the toolbar")},
        {Command.view_show_statusbar, "ViewStatusbar", null, N_("_Statusbar"),
            null, N_("Show or hide the statusbar")}
    };

    public Window () {
        set_title (GLib.get_application_name ());

        create_box ();
        create_menubar_and_toolbar ();
        create_statusbar ();
        create_notebook ();
    }

    public ~Window () {
        recent_manager.disconnect (update_recent_menu_id);
    }

    private void create_box () {
        main_box = new Gtk.Box (Gtk.Orientation.VERTICAL, 0);
        window.add (main_box);
        main_box.show ();
    }

    private void create_menubar_and_toolbar () {
        ui_manager = new UIManager ();

        // For showing tooltips in the status bar.
        ui_manager.connect_proxy.connect (on_ui_manager_connect_proxy);
        ui_manager.disconnect_proxy.connect (on_ui_manager_disconnect_proxy);

        add_accel_group (ui_manager.accel_group);

        // Actions which are always sensitive.
        always_sensitive_actions = new Gtk.ActionGroup
            ("VinagreWindowAlwaysSensitiveActions");
        always_sensitive_actions.set_translation_domain (null);
        always_sensitive_actions.add_actions (always_sensitive_entries, this);
        always_sensitive_actions.add_toggle_actions
            (always_sensitive_toggle_entries, this);
        ui_manager.insert_action_group (always_sensitive_actions, 0);

        // Only set the connect action to be important, but 591369.
        var connect_action = always_sensitive_actions.get_action
            ("RemoteConnect");
        connect_action.is_important = true;

        // Actions which are sensitive when connected to a remote.
        remote_connected_actions = new Gtk.ActionGroup
            ("VinagreWindowRemoteConnectedActions");
        remote_connected_actions.set_translation_domain (null);
        remote_connected_actions.add_actions (remote_connected_entries, this);
        remote_connected_actions.sensitive = false;
        ui_manager.insert_action_group (remote_connected_actions, 0);

        // Actions which are sensitive when the remote connection is active.
        remote_initialized_actions = new GtkActionGroup
            ("VinagreWindowRemoteInitializedActions");
        remote_initialized_actions.set_translation_domain (null);
        remote_initialized_actions.add_actions
            (remote_initialized_entries, this);
        remote_initialized_actions.sensitive = false;
        ui_manager.insert_action_group (remote_initialized_actions, 0);

        // Load the UI definition.
        try {
            ui_manager.add_ui_from_file (get_uimanager_filename ());
        } catch (Error error) {
            error ("Error while loading UI manager file: %s", error.message);
        }

        // Bookmarks.
        bookmarks_actions = new Gtk.ActionGroup ("BookmarksActions");
        bookmarks_actions.set_translation_domain (null);
        ui_manager.insert_action_group (bookmarks_actions);

        // Menubar and toolbar.
        menubar = ui_manager.get_widget ("/Menubar");
        main_box.pack_start (menubar, false, false, 0);

        toolbar = ui_manager.get_widget ("/Toolbar");
        Gtk.StyleContext.add_class (toolbar.get_style_context (),
            Gtk.StyleClass.PRIMARY_TOOLBAR);
        toolbar.hide ();
        main_box.pack_start (toolbar, false, false, 0);

        // Recent connections.
        recent_action = new Gtk.RecentAction ("recent_connections",
            _("_Recent Connections"), null, null);
        recent_action.show_not_found = true;
        recent_action.local_only = false;
        recent_action.show_private = true;
        recent_action.show_tips = true;
        recent_action.sort_type - Gtk.Recent.SORT_MRU;
        recent_action.item_activated.connect (on_activate_recent);

        recent_action_group = new Gtk.ActionGroup
            ("VinagreRecentConnectionsActions");
        recent_action_group.set_translation_domain (null);
        ui_manager.insert_action_group (recent_action_group, 0);
        recent_action_group.add_action (recent_action);

        var filter = new Gtk.RecentFilter ();
        filter.add_group (Config.PACKAGE_TARNAME);
        recent_action.add_filter (filter);

        update_recent_connections ();

        var recent_manager = Gtk.RecentManager.get_default ();
        update_recent_menu_id = recent_manager.changed.connect
            (on_recent_manager_changed);

        var default_settings = Prefs.get_default_gsettings ();
        default_settings.changed["show-accels"].connect
		(on_show_accels_changed);
        on_show_accels_changed ();
    }

    private void create_statusbar () {
        statusbar = new Gtk.Statusbar ();
        statusbar_tooltip_id = statusbar.get_context_id ("tooltip_message");

        main_box.pack_end (statusbar, false, true, 0);
    }

    private void create_notebook () {
        notebook = new Vinagre.Notebook (this);
        main_box.pack_start (notebook, true, true, 0);
        notebook.show ();
    }

    // Signal handlers.
    private void on_uimanager_connect_proxy () {
        if (proxy is Gtk.MenuItem) {
            proxy.select.connect (on_menu_item_select);
            proxy.deselect.connect (on_menu_item_deselect);
        }
    }

    private void on_uimanager_disconnect_proxy () {
        if (proxy is Gtk.MenuItem) {
            proxy.select.disconnect (on_menu_item_select);
            proxy.deselect.disconnect (on_menu_item_deselect);
        }
    }

    private void on_menu_item_select () {
        var action = proxy.gtk_action;
        var message = action.message;
        if (message)
            statusbar.push (tooltip_message_id, message);
    }

    private void on_menu_item_deselect () {
        statusbar.pop (tooltip_message_id);
    }
}
