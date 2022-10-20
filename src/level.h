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

#ifndef __LEVEL_H__
#define __LEVEL_H__

#include <fstream>
#include "bezier.h"
#include "common.h"

using namespace std;

struct Level
{
    string savePhilename;
    vector<Bezier> paths;
    bool loop;
    bool mirrorX;
    bool mirrorY;
    string name;
    string backgroundTexFilename;
    string overlayTexFilename;
    string thumbTexFilename;
    string musicFilename;
    bool invert;
    vector<double>startFeedRates;
    vector<double>endFeedRates;
    vector<int>ballsToDraw;
    vector<int>ballsFromStart;
    vector<double>ballSizes;
    int colors;
    uint time;
    int bonusFrequency;
    bool kidsMode;

    GLuint backgroundTex;
    GLuint overlayTex;
    GLuint thumbTex;

    Level(Bezier paths[] = NULL, int npaths = 3, bool loop = false, bool invert = false, string name = "New level");
    Level(const char *philename, bool editor = false);

    ~Level();
    void Save(const char *philename);
    void Save();
    void LoadTex();
    void LoadData(const char *philename);
    void FreeTex();
    // call after modifying paths
    void FixData();
    void NewLevel();
};

#endif //__LEVEL_H__
