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
#include "profile.h"
#include "common.h"
#include "error.h"
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>

Profile profile;

Profile::Profile(string _name)
        : name(_name)
{
    Strip(name);
    bool migrate = false;

    if (name.empty())
    {
        name = getDefaultProfileName();
        migrate = true;
    }

    fileName = Scenes::Settings::getDefaultDirectory() + SEPARATOR + name + ".profile";

    ifstream phile;
#ifdef WIN32
    phile.open(Scenes::Settings::W32_GetFileName(fileName).c_str());
#else
    phile.open(fileName.c_str());
#endif

    if (!phile)
    {
        // if we create a new default profile, let's read in level progress
        // from the settings file
        if (migrate)
        {
            Scenes::Settings sets;
            for (int f = 1; f < 100; f++)
            {
                stringstream n;
                n << "level" << f << "_completed";
                if (sets.getb(n.str() ,false))
                {
                    n.str("");
                    n << MIGRATE_SET << ":" << f << ":completed";
                    setb(n.str(), true);
                }
            }
        }

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

void Profile::Save()
{
    ofstream phile;

#ifdef WIN32
    phile.open(Scenes::Settings::W32_CreateFile(fileName).c_str());
#else
    phile.open(fileName.c_str());
#endif

    if (!phile)
    {
        ERR("Could not save profile in " + fileName);
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

const string Profile::get(const string name, const string defval)
{
    if (!cfg.count(name))
        cfg[name] = defval;

    return cfg[name];
}

void Profile::set(const string name, const string value)
{
    cfg[name] = value;
}

bool Profile::getb(const string name, bool defval)
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

void Profile::setb(const string name, bool value)
{
    string val = value?"TRUE":"FALSE";
    cfg[name] = val;
}

string Profile::getDefaultProfileName()
{
#ifndef WIN32
    char *r = getenv(ENVUSERNAME);
    string n;
    if (r)
    {
        n = r;
    }
    else
    {
        n = _("Anonymous");
    }

    return n;
#else
    wchar_t wu[256];

    DWORD n = 256;
    if (GetUserNameW(wu, &n))
    {
        char userName[n * 2];
        if (WideCharToMultiByte(CP_UTF8, 0, wu, -1, userName, MAX_PATH * 2, NULL, NULL))
            return string(userName);
    }

    return string(_("Anonymous"));
#endif
}
