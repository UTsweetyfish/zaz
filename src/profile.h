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

#ifndef __PROFILE_H__
#define __PROFILE_H__

#include <string>
#include <map>

#ifndef WIN32
#define ENVUSERNAME "USER"
#endif

#ifdef WIN32
#define ENVUSERNAME "USERNAME"
#endif

#define MIGRATE_SET "pro.set" // the name of the set to use when migrating from pre 0.3.1 version

using namespace std;

class Profile
{
    map<string, string> cfg;
    string name;
    string fileName;

    static string getDefaultProfileName();

public:
    const string getName()
    {
        return name;
    }

    Profile(string name = "");
    void Save();
    const string get(const string name, const string defval);
    bool getb(const string name, bool defval);
    void set(const string name, const string value);
    void setb(const string name, bool value);
};

extern Profile profile;


#endif// __PROFILE_H__
