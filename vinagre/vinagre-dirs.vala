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

    public string get_package_data_file (string filename) {
        /* Check the compiled-in path first, so that if Vinagre is installed in
         * a custom prefix and the standard prefix, the custom prefix is
         * checked first. */
        var system_data_dirs = Environment.get_system_data_dirs ();
        string[] data_dirs = {};
        data_dirs += Config.DATADIR;
        foreach (var dir in system_data_dirs) { data_dirs += dir; };

        foreach (var dir in data_dirs) {
            var absolute_path = Path.build_filename (dir,
                Config.PACKAGE_TARNAME, filename);
            if (FileUtils.test (absolute_path, FileTest.EXISTS))
                return absolute_path;
        };

        // Filename could not be found!
        error ("Data file ‘%s’ could not be found in system data directories.",
            filename);
    }
}
