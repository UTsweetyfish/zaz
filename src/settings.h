/*
 * Zaz
 * Copyright (C) Remigiusz Dybka 2009 <remigiusz.dybka@gmail.com>
 *
 Zaz is free software: you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by the
 Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Zaz is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along
 with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include <vector>
#include <string>
#include <map>

#ifdef WIN32
#define ENVVAR	  "APPDATA"
#define SEPARATOR "\\"
#else
#define ENVVAR	  "HOME"
#define SEPARATOR "/"
#endif

#define LANGENVVAR	"LANGUAGE"

#define DEFAULT_FILENAME "settings"
#define HIGHSCORE_FILENAME "hiscores"
#define DEFAULT_DIRECTORY ".zaz"
#define LOCAL_DATADIR_NAME "data"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

using namespace std;

namespace Scenes
{
class Settings
{
public:
    string fileNameInUse;
    Settings(string fileName = getDefaultFileName());

    const string get(const string name, const string defval);
    bool getb(const string name, bool defval);
    void set(const string name, const string value);
    void setb(const string name, bool value);
    void Save(string fileName = getDefaultFileName());
    const string getDataDir();
    const string getLocalDataDir();
    string getFilename(string phname);
    const char *getCFilename(string phname);
    static string getDefaultDirectory(void);
    static string getDefaultFileName(void);
    static string getHighscoreFileName(void);
    bool installed;
    char *forcedDir; // holds directory specified by -d option

#ifdef ENABLE_NLS
    map<string, string> languages;
    void setLanguage(string lang);
#endif

#ifdef WIN32
    // converts given filename in UTF-8 to a short filename
    // if the file does not exist - return empty string
    static string W32_GetFileName(string);

    // creates a file from UTF-8 string and returns it short form
    static string W32_CreateFile(string);
#endif

private:
    void Load();

    map<string, string> cfg;
};
}

#endif //__SETTINGS_H__
