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

#ifndef __HISCORES_H__
#define __HISCORES_H__

#include <string>
#include <vector>
#include "settings.h"


namespace Scenes
{

enum HiScoreType
{
    HS_ADVENTURE = 0,
    HS_TIME = 1,
    HS_SURVIVAL = 2
};

struct ReportPage
{
    string header;
    std::vector<string> lines;
};

struct HiScoreEntry
{
    int score;
    std::string name;
    std::string set;
    std::string level;
    HiScoreType type;

    HiScoreEntry(int score, std::string set, HiScoreType type, std::string name, std::string level)
            : score(score), name(name), set(set), level(level), type(type) {};
};

struct HiScoreSet
{
    string name;
    std::vector<HiScoreEntry> scores[3];

    HiScoreSet(string name)
            : name(name) {};

    HiScoreSet()
            : name("") {};
};

struct HiScores
{
    std::string filename;
    std::map<string, HiScoreSet> scoreSets;
    std::string trimUtf8(string, unsigned int);
    HiScores(std::string filename = Settings::getHighscoreFileName ());
    ~HiScores();
    bool GoodEnough(HiScoreEntry ent);
    void SubmitHiScore(HiScoreEntry hs);
    std::vector<ReportPage> GenerateReport(int entriesPerPage = 1000);
};
}

extern Scenes::HiScores hiScores;
#endif //__HISCORES_H__
