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

#include <fstream>
#include <sstream>
#include "level.h"
#include "error.h"
#include "common.h"
#include "textureloader.h"

Level::Level(Bezier paths[], int npaths, bool loop, bool invert, string name)
        :loop(loop), mirrorX(false), mirrorY(false), name(name), invert(invert), ballsToDraw(10), ballsFromStart(0), colors(3),
        bonusFrequency(-1), kidsMode(false), backgroundTex(0), overlayTex(0), thumbTex(0)
{

}

Level::~Level()
{
    FreeTex();

    if (thumbTex)
    {
        glDeleteTextures(1, &thumbTex);
    }

    thumbTex = 0;
}

void Level::LoadTex()
{
    FreeTex();

    backgroundTex = LoadTextureFile (backgroundTexFilename.c_str());

    if (!overlayTexFilename.empty())
        overlayTex = LoadTextureFile (overlayTexFilename.c_str());
}

void Level::NewLevel()
{
    loop = false;
    invert = false;
    kidsMode = false;
    mirrorX = false;
    mirrorY = false;
    bonusFrequency = -1;
    name = "New Level";
    startFeedRates.push_back(1.0);
    endFeedRates.push_back(1.0);
    ballsToDraw.push_back(50);
    ballsFromStart.push_back(2);
    ballSizes.push_back(1.0);
    colors = 5;
    backgroundTex = 0;
    overlayTex = 0;
    time = 180;
};

void Level::FreeTex()
{
    if (backgroundTex)
    {
        glDeleteTextures(1, &backgroundTex);
    }

    if (overlayTex)
    {
        glDeleteTextures(1, &overlayTex);
    }

    backgroundTex = overlayTex = 0;
}

void Level::Save()
{
    Save(savePhilename.c_str());
}

void Level::Save(const char *philename)
{
    std::ofstream out(philename);

    // number of paths
    out << paths.size() << endl;
    // paths
    for (uint i = 0; i < paths.size(); ++i)
    {
        // number of points
        out << paths[i].points.size() << endl;
        for (uint p = 0; p < paths[i].points.size(); ++p)
        {
            if (paths[i].points[p].hidden)
            {
                out << 'H';
            }
            else if (paths[i].points[p].under)
            {
                out << 'U';
            }

            BezierPoint pt = paths[i].points[p];
            out << pt.x << " " << pt.y << " " << pt.cx << " " << pt.cy << endl;
        }
    }

    out << loop << endl;
    out << invert << endl;

    return;

    for (uint p = 0; p < startFeedRates.size(); p++)
    {
        out << startFeedRates[p];
        if (p < startFeedRates.size() - 1)
            out << ":";
    }
    out << endl;

    for (uint p = 0; p < endFeedRates.size(); p++)
    {
        out << endFeedRates[p];
        if (p < endFeedRates.size() - 1)
            out << ":";
    }
    out << endl;

    for (uint p = 0; p < ballsToDraw.size(); p++)
    {
        out << ballsToDraw[p];
        if (p < ballsToDraw.size() - 1)
            out << ":";
    }
    out << endl;

    for (uint p = 0; p < ballsFromStart.size(); p++)
    {
        out << ballsFromStart[p];
        if (p < ballsFromStart.size() - 1)
            out << ":";
    }
    out << endl;

    out << colors << endl;
    out << time << endl;
    out << bonusFrequency << endl;
    out << kidsMode << endl;

    for (uint p = 0; p < ballSizes.size(); p++)
    {
        out << ballSizes[p];
        if (p < ballSizes.size() - 1)
            out << ":";
    }
    out << endl;
}

void Level::FixData()
{
    uint npaths = paths.size();

    while (ballSizes.size() < npaths - 1)
        ballSizes.push_back(ballSizes[0]);

    while (startFeedRates.size() < npaths - 1)
        startFeedRates.push_back(startFeedRates[0]);

    while (endFeedRates.size() < npaths - 1)
        endFeedRates.push_back(endFeedRates[0]);

    while (ballsFromStart.size() < npaths - 1)
        ballsFromStart.push_back(ballsFromStart[0]);

    while (ballsToDraw.size() < npaths - 1)
        ballsToDraw.push_back(ballsToDraw[0]);
}

void Level::LoadData(const char *philename)
{
    savePhilename = philename;

    std::ifstream in(philename);

    if (!in)
    {
        return;
//		ERR(string("Could not open level : ") + philename);
    }

    paths.clear();
    uint npaths;

    istringstream sl;
    string l;

    // number of paths
    getline(in, l);
    sl.str(l);
    sl >> npaths;

    // paths
    for (uint i = 0; i < npaths; ++i)
    {
        paths.push_back(Bezier());

        int pl = 0;

        getline(in, l);
        Strip(l);
        sl.clear();
        sl.str(l);
        sl >> pl;

        for (int p = 0; p < pl; ++p)
        {
            getline(in, l);
            bool under = false;
            bool hidden = false;
            if (l[0] == 'H')
            {
                hidden = true;
                under = true;
            }

            if (l[0] == 'U')
                under = true;

            if (hidden || under)
            {
                l = l.substr(1);
            }

            sl.clear();
            sl.str(l);

            double x, y, cx, cy;
            sl >> x >> y >> cx >> cy;
            paths[i].points.push_back(BezierPoint (x, y, cx, cy, under, hidden));
        }

    }

    // loop
    getline(in, l);
    sl.clear();
    sl.str(l);
    sl >> loop;

    // invert
    getline(in, l);
    sl.clear();
    sl.str(l);
    sl >> invert;

    FixData();

    /*while (ballSizes.size() < npaths - 1)
        ballSizes.push_back(ballSizes[0]);

    while (startFeedRates.size() < npaths - 1)
        startFeedRates.push_back(startFeedRates[0]);

    while (endFeedRates.size() < npaths - 1)
        endFeedRates.push_back(endFeedRates[0]);

    while (ballsFromStart.size() < npaths - 1)
        ballsFromStart.push_back(ballsFromStart[0]);

    while (ballsToDraw.size() < npaths - 1)
        ballsToDraw.push_back(ballsToDraw[0]);
    */
    return;

    // startFeedRate
    double startFeedRate;
    startFeedRates.clear();
    getline(in, l);
    sl.clear();
    sl.str(l);
    sl >> startFeedRate;
    startFeedRates.push_back(startFeedRate);

    size_t found;
    while ((found = l.find_first_of(":")) != string::npos)
    {
        l = l.substr(found + 1);
        Strip(l);
        if (l.size() > 0)
        {
            sl.clear();
            sl.str(l);
            sl >> startFeedRate;
            startFeedRates.push_back(startFeedRate);
        }
    }

    while (startFeedRates.size() < npaths - 1)
        startFeedRates.push_back(startFeedRate);

    // endFeedRate
    double endFeedRate;
    endFeedRates.clear();
    getline(in, l);
    sl.clear();
    sl.str(l);
    sl >> endFeedRate;
    endFeedRates.push_back(endFeedRate);

    while ((found = l.find_first_of(":")) != string::npos)
    {
        l = l.substr(found + 1);
        Strip(l);
        if (l.size() > 0)
        {
            sl.clear();
            sl.str(l);
            sl >> endFeedRate;
            endFeedRates.push_back(endFeedRate);
        }
    }

    while (endFeedRates.size() < npaths - 1)
        endFeedRates.push_back(endFeedRate);

    // // ballsToDraw
    int balls2d;
    ballsToDraw.clear();
    getline(in, l);
    sl.clear();
    sl.str(l);
    sl >> balls2d;
    clamp(balls2d, 1, 9999);
    ballsToDraw.push_back(balls2d);

    while ((found = l.find_first_of(":")) != string::npos)
    {
        l = l.substr(found + 1);
        Strip(l);
        if (l.size() > 0)
        {
            sl.clear();
            sl.str(l);
            sl >> balls2d;
            clamp(balls2d, 1, 9999);
            ballsToDraw.push_back(balls2d);
        }
    }

    while (ballsToDraw.size() < npaths - 1)
        ballsToDraw.push_back(balls2d);


    // ballsFromStart
    int ballsfs;
    ballsFromStart.clear();
    getline(in, l);
    sl.clear();
    sl.str(l);
    sl >> ballsfs;
    clamp(ballsfs, 0, 9999);
    ballsFromStart.push_back(ballsfs);

    while ((found = l.find_first_of(":")) != string::npos)
    {
        l = l.substr(found + 1);
        Strip(l);
        if (l.size() > 0)
        {
            sl.clear();
            sl.str(l);
            sl >> ballsfs;
            clamp(ballsfs, 0, 9999);
            ballsFromStart.push_back(ballsfs);
        }
    }

    while (ballsFromStart.size() < npaths - 1)
        ballsFromStart.push_back(ballsfs);

    // colours
    getline(in, l);
    sl.clear();
    sl.str(l);
    sl >> colors;

    // time
    getline(in, l);
    sl.clear();
    sl.str(l);
    sl >> time;

    // bonus frequency
    if (getline(in, l))
    {
        sl.clear();
        sl.str(l);
        sl >> bonusFrequency;
    }

    // kids mode
    if (getline(in, l))
    {
        sl.clear();
        sl.str(l);
        sl >> kidsMode;
    }

    // ballsizes
    double ballSize;
    ballSizes.clear();
    if (getline(in, l))
    {
        sl.clear();
        sl.str(l);
        sl >> ballSize;
        ballSizes.push_back(ballSize);

        while ((found = l.find_first_of(":")) != string::npos)
        {
            l = l.substr(found + 1);
            Strip(l);
            if (l.size() > 0)
            {
                sl.clear();
                sl.str(l);
                sl >> ballSize;
                ballSizes.push_back(ballSize);
            }
        }
    }
    else
    {
        ballSize = 1.0;
    }

    while (ballSizes.size() < npaths - 1)
        ballSizes.push_back(ballSize);
    // boundary checks
    clamp(startFeedRate, 0.0, 10.0);
    clamp(endFeedRate, 0.0, 10.0);
    clamp(colors, 2, 8);
    clamp(time, 10, 9999);
}

Level::Level(const char *philename, bool editor)
{
    NewLevel();
    backgroundTex = 0;
    overlayTex = 0;
    thumbTex = 0;

    if (editor)
    {
        string phn = philename;
        string lvlPhilename = phn + ".lvl";
        LoadData(lvlPhilename.c_str());

        string bckPhname = phn + ".png";
        backgroundTex = LoadTextureFile(bckPhname.c_str());
        backgroundTexFilename = bckPhname;

        string ovlPhname = phn + "_overlay.png";
        try
        {
            overlayTex = LoadTextureFile(ovlPhname.c_str());
            overlayTexFilename = ovlPhname;
        }
        catch (Error e)
        {
            overlayTex = 0;
        };

        return;
    }

    LoadData(philename);
    FixData();
}
