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

namespace Vinagre.Dirs {
    public string get_user_cache_dir () {
        return Path.build_filename (Environment.get_user_cache_dir (),
            Config.PACKAGE_TARNAME);
    }

    public string get_user_data_dir () {
        return Path.build_filename (Environment.get_user_data_dir (),
            Config.PACKAGE_TARNAME);
    }

    // Only used by get_ui_file ().
    private string get_vinagre_data_dir () {
#if ! G_OS_WIN32
        // TODO: const string[] data_dirs = Environment.get_system_data_dirs ();
        return Path.build_filename (Config.DATADIR,
            "%s-%s".printf (Config.PACKAGE_TARNAME,
                Config.VINAGRE_ABI_VERSION));
#else
        return Path.build_filename (
            Win32.get_package_installation_directory_of_module (null), "share",
            "%s-%s".printf (Config.PACKAGE_TARNAME,
                Config.VINAGRE_ABI_VERSION));
#endif
    }

    public string get_ui_file (string filename) {
        return Path.build_filename (get_vinagre_data_dir (), filename);
    }
}
