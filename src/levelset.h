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

#ifndef __LEVELSET_H__
#define __LEVELSET_H__

#define LEVELSET_EXTENSION ".set"

#include <vector>
#include "level.h"

using namespace std;

class LevelSet
{
    string description;
    string thumbTexFilename;
    string filename;
    bool empty;
    vector<Level> levels;

    friend class MainMenu;
    friend class GameLoop;
public:
    LevelSet(string filename);
    LevelSet();
    ~LevelSet();
    static vector<string> getSetNames();
    const string getDesc() const
    {
        return description;
    };
    const bool getEmpty() const
    {
        return empty;
    };
};

extern LevelSet currentLevelSet;
#endif //__LEVELSET_H__
