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

#include "settings.h"
#include "common.h"

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>

#ifdef WIN32
#include <shlobj.h>
#include <shlwapi.h>#endif

#ifndef WIN32
#include <dirent.h>
#endif

using namespace std;

struct langlist_tag
{
    const char *name;
    const char *description;    Uint16 lid;
};

struct langlist_tag langlist[] =
{
    {"en", "English", 0x09},
    {"pl", "Polski", 0x15},
    {"ru", "Русский", 0x19},
    {"de", "Deutsch", 0x07},
    {"es", "Español", 0x0a},
    {"hu", "Magyar", 0x0e},
    {"it", "Italiano", 0x10},
    {"fr", "Français", 0x0c},
    {"tr", "Türkçe", 0x1f},
    {NULL, NULL, 0x00}
};

vector<string> Split(string str, string sep)
{
    vector<string> ret;

    while (!str.empty())
    {
        string::size_type found = str.find_first_of(sep);

        if (found != string::npos)
        {
            string ns = str.substr(0, found);
            Strip(ns);
            ret.push_back(ns);
            str = str.substr(found + 1);
        }
        else
        {
            Strip(str);
            ret.push_back(str);
            str = "";
        }
    }

    return ret;
}

void Strip(string &str)
{
    if (str.empty())
        return;

    int start = 0;
    int end = str.length() -1;

    while ((isspace(str[start])) && (start <= end))
        start++;

    while ((isspace(str[end])) && (start <= end))
        end--;

    str = str.substr(start, (end - start) + 1);
}

namespace Scenes
{
string Settings::getDefaultDirectory(void)
{
#ifdef WIN32
    char mbpath[MAX_PATH *2];
    wchar_t path[MAX_PATH *2];
    if (SUCCEEDED(SHGetFolderPathW(NULL,
                                   CSIDL_APPDATA,
                                   NULL,
                                   0,
                                   path)))
    {
        if (WideCharToMultiByte(CP_UTF8, 0, path, -1, mbpath, MAX_PATH * 2, NULL, NULL))
        {
            return string(mbpath) + SEPARATOR + DEFAULT_DIRECTORY;
        }
        else
        {
            ERR("Error converting APPDATA to mb");
        }

    }
    else
    {
        ERR("Could not retrieve APPDATA path");
    }
#else
    string dir = getenv(ENVVAR);
    return dir + SEPARATOR + DEFAULT_DIRECTORY;
#endif
}

#ifdef ENABLE_NLS
void Settings::setLanguage(string lang)
{
    static char envString[1024];
    sprintf(envString, "%s=%s", LANGENVVAR, lang.c_str());
    putenv(envString);

    setlocale(LC_CTYPE, "");
    setlocale(LC_MESSAGES, "");
    bindtextdomain(PACKAGE_NAME, PACKAGE_LOCALE_DIR);
    bind_textdomain_codeset(PACKAGE_NAME, "UTF-8");
    textdomain(PACKAGE_NAME);

}
#endif

string Settings::getDefaultFileName(void)
{
    string retstr = DEFAULT_FILENAME;
    string envvar = ENVVAR;
    string separator = SEPARATOR;

    string dir = getenv(ENVVAR);

    return getDefaultDirectory () + SEPARATOR + retstr;
}

string Settings::getHighscoreFileName(void)
{
    return getDefaultDirectory() + SEPARATOR + HIGHSCORE_FILENAME;
}
Settings::Settings(string fileName)
        : fileNameInUse(fileName)
{
    forcedDir = NULL;

    if (!fileName.empty())
        fileNameInUse = fileName;

    Load();

    installed = true;

#ifndef WIN32
    DIR *d = opendir((string(PACKAGE_DATA_DIR) + SEPARATOR + PACKAGE_NAME).c_str());
    if (d == NULL)
    {
        installed = false;
    }
    else
    {
        closedir(d);
    }
#endif

#ifdef ENABLE_NLS
    // load language list
    languages.clear();

    for (int f = 0; langlist[f].name; f++)
    {
        languages[langlist[f].name] = langlist[f].description;
    }

    // determine the language to use

    // do we have a proper language set in environment ?
    char *cenvLang = getenv(LANGENVVAR);
    string languageFromEnv;
    char envLang[1024];

    if (cenvLang)
    {
        strcpy(envLang, cenvLang);
        for (uint f = 0; f < strlen(envLang); f++)
            envLang[f] = tolower(envLang[f]);

        uint l = strlen(envLang);

        map<string, string>::iterator iter;

        for (iter = languages.begin(); iter != languages.end(); ++iter)
        {
            string lng = iter->first;
            if (l >= lng.size())
                if (lng.compare(0, lng.size(), envLang, 0, lng.size()) == 0)
                    languageFromEnv = lng;
        }
    }
#ifdef WIN32
    LANGID lid = GetUserDefaultUILanguage();    for (int l =0; langlist[l].name != NULL; l++)    {        if (langlist[l].lid == (0xFF & lid))            languageFromEnv = langlist[l].name;    }#endif //WIN32    if (!languageFromEnv.size())        languageFromEnv = "en";    setLanguage(get("language", languageFromEnv));
#endif //ENABLE_NLS
}

void Settings::Save(string fileName)
{
    ofstream phile;

#ifdef WIN32
    phile.open(W32_CreateFile(fileName).c_str());
#else
    phile.open(fileName.c_str());
#endif

    if (!phile)
    {
        cerr << "Could not save config in " << fileName << endl;
        return;
    }

    map<string, string>::iterator iter;

    for (iter = cfg.begin(); iter != cfg.end(); ++iter)
    {
        string cfgline = iter->first;
        cfgline.append(":");
        cfgline.append(iter->second);
        cfgline.append("\n");

        phile << cfgline;
    }

    phile.close();
}

#ifdef WIN32
string Settings::W32_GetFileName(string phname)
{
    wchar_t wfn[MAX_PATH];
    wchar_t pthname[MAX_PATH];
    char shrt[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, phname.c_str(), -1, wfn, MAX_PATH);
    if (GetShortPathNameW(wfn, pthname, MAX_PATH) > 0)
    {
        WideCharToMultiByte(CP_UTF8, 0, pthname, -1, shrt, MAX_PATH, NULL, NULL);
        return string(shrt);
    }
    else return string("");
    //
}

string Settings::W32_CreateFile(string phname)
{
    string ret = W32_GetFileName(phname);

    if (ret.empty())
    {
        wchar_t wfn[MAX_PATH];
        wchar_t wb[16];

        MultiByteToWideChar(CP_UTF8, 0, phname.c_str(), -1, wfn, MAX_PATH);
        MultiByteToWideChar(CP_UTF8, 0, "wb", -1, wb, 16);

        FILE *ph = _wfopen(wfn, wb);
        fclose(ph);

        ret = W32_GetFileName(phname);
    }

    return ret;
}
#endif

void Settings::Load()
{
    ifstream phile;
#ifdef WIN32
    phile.open(W32_GetFileName(fileNameInUse).c_str());
#else
    phile.open(fileNameInUse.c_str());
#endif

    if (!phile)
    {
        return;
    }

    string cfgline;

    while (getline(phile, cfgline))
    {
        int col = cfgline.find_last_of(":");

        if (col == -1)
            continue; // format error

        string name = cfgline.substr(0, col);
        string value = cfgline.substr(col + 1);

        Strip(name);
        Strip(value);

        cfg[name] = value;
    }

    phile.close();
}

const string Settings::get(const string name, const string defval)
{
    if (!cfg.count(name))
        cfg[name] = defval;

    return cfg[name];
}

void Settings::set(const string name, const string value)
{
    cfg[name] = value;
}

bool Settings::getb(const string name, bool defval)
{
    if (!cfg.count(name))
    {
        string val = defval?"TRUE":"FALSE";
        cfg[name] = val;
    }

    bool ret = false;

    char buff[1024];
    //char buff[strlen(cfg[name].c_str()) + 1];
    strcpy(buff, cfg[name].c_str());
    int s = strlen(buff);

    for (int f = 0; f < s; f++)
    {
        buff[f] = toupper(buff[f]);
    }

    if (!strcmp(buff, "TRUE"))
    {
        return true;
    }

    return ret;
}

void Settings::setb(const string name, bool value)
{
    string val = value?"TRUE":"FALSE";
    cfg[name] = val;
}

const string Settings::getLocalDataDir()
{
    return getDefaultDirectory() + SEPARATOR + LOCAL_DATADIR_NAME;
}

const string Settings::getDataDir()
{
    if (forcedDir)
        return forcedDir;

#ifndef WIN32
    if (installed)
    {
        return string(PACKAGE_DATA_DIR) + SEPARATOR + PACKAGE_NAME;
    }
    else
    {
        return "data";
    }
#else
    return "data";
#endif
}

string Settings::getFilename(const string phname)
{
#ifdef WIN32
    string dphn = W32_GetFileName(getDataDir() + SEPARATOR + phname);
#else
    string dphn = getDataDir() + SEPARATOR + phname;
#endif

    if (forcedDir)
    {
        return dphn;
    }

#ifdef WIN32
    string lphn = W32_GetFileName(getLocalDataDir() + SEPARATOR + phname);
#else
    string lphn = getLocalDataDir() + SEPARATOR + phname;
#endif

    FILE *inph = fopen(lphn.c_str(), "rb");
    if (inph != NULL)
    {
        fclose(inph);
        return lphn;
    }

    return dphn;
}

const char *Settings::getCFilename(const string phname)
{
    static char buff[1024];
    strcpy(buff, getFilename(phname).c_str());
    return buff;
}
}

