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

#include "levelset.h"
#include "directorylister.h"
#include "settings.h"
#include "string.h"
#include "textureloader.h"
#include <fstream>
#include <sstream>

LevelSet currentLevelSet;

LevelSet::LevelSet()
        : empty(true)
{
}

LevelSet::LevelSet(string philename)
        : filename(philename)
{
    string phname = settings.getFilename(filename);
    std::ifstream in(phname.c_str());

    if (!in)
    {
        ERR("Could not open set:" + filename);
    }

    empty = false;

    istringstream sl;
    string l;

    // name
    getline(in, l);
    Strip(l);
    description = l;

    // thumbfilename
    getline(in, l);
    Strip(l);
    thumbTexFilename = l;

    getline(in, l);
    Strip(l);
    while (l != "-")
    {
        // name
        string name = l;
        // files
        getline(in, l);
        Strip(l);
        vector<string> filelist = Split(l, ",");

        if (filelist.size() < 4)
            ERR("Format error in " + phname + " : filelist must have at least 4 entries");

        if (filelist.size() > 5)
            ERR("Format error in " + phname + " : filelist must have no more than 5 entries");

        // attributes
        getline(in, l);
        Strip(l);
        vector<string> attributes = Split(l, ",");

        if (attributes.size() != 11)
            ERR("Format error in " + phname + " : attribute list must have 11 entries");

        Level lev(settings.getCFilename(filelist[0]));

        lev.name = name;
        lev.backgroundTexFilename = settings.getFilename(filelist[1]);
        Strip(filelist[2]);
        if (!filelist[2].empty())
            lev.overlayTexFilename = settings.getFilename(filelist[2]);

        lev.thumbTexFilename = settings.getFilename(filelist[3]);

        // is music defined ?
        if (filelist.size() == 5)
        {
            Strip(filelist[4]);
            if (!filelist[4].empty())
                lev.musicFilename = filelist[4];
        }

        double value;
        uint npaths = 0;
        if ((lev.paths.size() - 1) >= 0)
            npaths = lev.paths.size() - 1;

        vector<string>::iterator i;

        // startfeedrates
        lev.startFeedRates.clear();
        vector<string> ff = Split(attributes[0], ";");
        for (i = ff.begin(); i != ff.end(); ++i)
        {
            sl.clear();
            sl.str(*i);
            sl >> value;
            lev.startFeedRates.push_back(value);
        }

        while (lev.startFeedRates.size() < npaths)
            lev.startFeedRates.push_back(value);

        // endfeedrates
        lev.endFeedRates.clear();
        ff = Split(attributes[1], ";");
        for (i = ff.begin(); i != ff.end(); ++i)
        {
            sl.clear();
            sl.str(*i);
            sl >> value;
            lev.endFeedRates.push_back(value);
        }

        while (lev.endFeedRates.size() < npaths)
            lev.endFeedRates.push_back(value);

        int val;
        // ballstodraw
        lev.ballsToDraw.clear();
        ff = Split(attributes[2], ";");
        for (i = ff.begin(); i != ff.end(); ++i)
        {
            sl.clear();
            sl.str(*i);
            sl >> val;
            lev.ballsToDraw.push_back(val);
        }

        while (lev.ballsToDraw.size() < npaths)
            lev.ballsToDraw.push_back(val);

        // ballsfromstart
        lev.ballsFromStart.clear();
        ff = Split(attributes[3], ";");
        for (i = ff.begin(); i != ff.end(); ++i)
        {
            sl.clear();
            sl.str(*i);
            sl >> val;
            lev.ballsToDraw.push_back(val);
        }

        while (lev.ballsFromStart.size() < npaths)
            lev.ballsFromStart.push_back(val);

        // colours
        sl.clear();
        sl.str(attributes[4]);
        sl >> lev.colors;

        // time
        sl.clear();
        sl.str(attributes[5]);
        sl >> lev.time;

        // bonus frequency
        sl.clear();
        sl.str(attributes[6]);
        sl >> lev.bonusFrequency;

        // kids mode
        sl.clear();
        sl.str(attributes[7]);
        sl >> lev.kidsMode;

        // ballsizes
        lev.ballSizes.clear();
        ff = Split(attributes[8], ";");
        for (i = ff.begin(); i != ff.end(); ++i)
        {
            sl.clear();
            sl.str(*i);
            sl >> value;
            lev.ballSizes.push_back(value);
        }

        while (lev.ballSizes.size() < npaths)
            lev.ballSizes.push_back(value);

        // mirror x
        sl.clear();
        sl.str(attributes[9]);
        sl >> lev.mirrorX;

        // mirror y
        sl.clear();
        sl.str(attributes[10]);
        sl >> lev.mirrorY;

        levels.push_back(lev);

        // next level
        getline(in, l);
        Strip(l);
    }
}


LevelSet::~LevelSet()
{
}

vector<string> LevelSet::getSetNames()
{
    vector<string> ret;

    vector<string> dat = ListFiles(settings.getDataDir(), LEVELSET_EXTENSION);
    vector<string> loc = ListFiles(settings.getLocalDataDir(), LEVELSET_EXTENSION);

    vector<string>::iterator i;
    vector<string>::iterator ii;

    for (i = dat.begin(); i != dat.end(); ++i)
    {
        ret.push_back(*i);
    }

    for (i = loc.begin(); i != loc.end(); ++i)
    {
        bool found = false;

        for (ii = ret.begin(); ii != ret.end(); ++ii)
        {
            if ((*ii) == (*i))
                found = true;
        }

        if (!found)
            ret.push_back(*i);
    }

    return ret;
}

