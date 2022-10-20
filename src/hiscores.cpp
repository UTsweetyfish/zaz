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

#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include "hiscores.h"
#include "common.h"

using namespace std;

namespace Scenes
{

HiScores::HiScores(string filename)
        : filename(filename)
{
    ifstream inph;
#ifdef WIN32
    inph.open(Settings::W32_GetFileName(filename).c_str());
#else
    inph.open(filename.c_str());
#endif
    if (!inph) // may be empty or not exist
        return;

    string l;
    while (!getline(inph, l).fail())
    {
        int score;
        HiScoreType type;
        string set;
        string name;
        string level;
        istringstream sl;

        Strip(l);
        vector<string> sc = Split(l, ":");

        if (sc.empty())
            continue;

        if (sc.size() == 2)
        {
            sl.clear();
            sl.str(sc[1]);
            sl >> score;

            sl.clear();
            sl.str(sc[0]);
            int t;
            sl >> t;
            type = HiScoreType(t);
        }
        else
        {
            sl.clear();
            sl.str(sc[0]);
            sl >> score;

            type = HiScoreType(0);
        }

        bool ok = (!getline(inph, l).fail());
        Strip(l);
        name = l;

        ok = ok && (!getline(inph, l).fail());
        Strip(l);
        vector<string> lev = Split(l, ":");
        if (lev.size() == 2)
        {
            level = lev[1];
            set = lev[0];
        }
        else
        {
            level = lev[0];
            set = "pro.set";
        }

        if (ok)
            SubmitHiScore(HiScoreEntry(score, set, type, name, level));
    }
    inph.close();
}

HiScores::~HiScores()
{
    ofstream oph;
#ifdef WIN32
    oph.open(Settings::W32_CreateFile(filename).c_str());
#else
    oph.open(filename.c_str());
#endif
    if (!oph) // very bad
    {
        cout << filename + ": could not save hi scores";
        return;
        
    }

    vector<HiScoreEntry>::iterator i;

    map<string, HiScoreSet>::iterator iter;

    for (iter = scoreSets.begin(); iter != scoreSets.end(); ++iter)
        for (int g = 0; g < 3; g++)
        {
            int f = 0;

            for (i = iter->second.scores[g].begin(); i != iter->second.scores[g].end(); ++i)
            {
                if (((f < 10) || (g != 0)) && (i->score > 0))
                {
                    oph << i->type << ":" << i->score << std::endl;
                    oph << i->name << std::endl;
                    oph << i->set << ":" << i->level << std::endl;
                }
                ++f;
            }
        }

    oph.close();
}

void HiScores::SubmitHiScore(HiScoreEntry ent)
{
    vector<HiScoreEntry>::iterator i;
    vector<HiScoreEntry>::iterator ii;

    HiScoreSet s = scoreSets[ent.set];
    s.name = ent.set;

    if (ent.type == 0)
    {
        ii = s.scores[ent.type].begin();
        for (i = s.scores[ent.type].begin(); i != s.scores[ent.type].end(); ++i)
        {
            if (i->score > ent.score)
            {
                ii = i + 1;
            }
        }

        s.scores[ent.type].insert(ii, ent);
    }

    if (ent.type == HS_TIME) // best times
    {
        bool found = false;
        // update a previous hiscore for a level or create a new one ?
        ii = s.scores[ent.type].begin();
        for (i = s.scores[ent.type].begin(); i != s.scores[ent.type].end(); ++i)
        {
            if (i->level == ent.level)
            {
                found = true;
                if  (i->score > ent.score)
                {
                    ii = i;
                    s.scores[ent.type].erase(i);
                    s.scores[ent.type].insert(i, ent);
                }
            }
        }

        if (!found)
            s.scores[ent.type].insert(ii, ent);
    }

    if (ent.type == HS_SURVIVAL) // best scores in survival
    {
        bool found = false;
        // update a previous hiscore for a level or create a new one ?
        ii = s.scores[ent.type].begin();
        for (i = s.scores[ent.type].begin(); i != s.scores[ent.type].end(); ++i)
        {
            if (i->level == ent.level)
            {
                found = true;
                if  (i->score < ent.score)
                {
                    ii = i;
                    s.scores[ent.type].erase(i);
                    s.scores[ent.type].insert(i, ent);
                }
            }
        }

        if (!found)
            s.scores[ent.type].insert(ii, ent);
    }

    scoreSets[ent.set] = s;
}

bool HiScores::GoodEnough(HiScoreEntry ent)
{
    vector<HiScoreEntry>::iterator i;

    if (ent.score == 0)
        return false;

    if (ent.type == HS_ADVENTURE)
    {
        vector<HiScoreEntry> s = scoreSets[ent.set].scores[ent.type];

        if (s.size() < 10)
            return true;

        int f = 0;
        for (i = s.begin(); i != s.end(); ++i)
        {
            if (f < 10)
            {
                if (i->score < ent.score)
                    return true;
            }
            ++f;
        }

    }

    if (ent.type == HS_TIME)
    {
        vector<HiScoreEntry> s = scoreSets[ent.set].scores[ent.type];
        for (i = s.begin(); i != s.end(); ++i)
        {
            if ((i->level == ent.level) && (i->score <= ent.score))
                return false;
        }

        return true;
    }

    if (ent.type == HS_SURVIVAL)
    {
        vector<HiScoreEntry> s = scoreSets[ent.set].scores[ent.type];
        for (i = s.begin(); i != s.end(); ++i)
        {
            if ((i->level == ent.level) && (i->score >= ent.score))
                return false;
        }

        return true;
    }


    return false;
}

bool operator<(const HiScoreEntry &e1, const HiScoreEntry &e2)
{
    return (e1.score < e2.score);
}

bool sortLev(const HiScoreEntry &e1, const HiScoreEntry &e2)
{
    return (e1.level < e2.level);
}

template <class T> bool invSort(const T &t1, const T &t2)
{
    return !(t1 < t2);
}


uint utf8_strlen(const char *s)
{
    int i = 0;
    int j = 0;

    while (s[i])
    {
        if ((s[i] & 0xC0) != 0x80)
            j++;
        i++;
    }
    return (j);
}

std::string HiScores::trimUtf8(string s, unsigned int l)
{
    std::string txt = s;
    while (utf8_strlen(txt.c_str()) > l)
    {
        if (txt[txt.size() -1] < 0)
            txt = txt.substr(0, txt.size() - 1);

        txt = txt.substr(0, txt.size() - 1);
    }

    while (utf8_strlen(txt.c_str()) < l)
    {
        txt.append(" ");
    }

    return txt;
}

vector<ReportPage> HiScores::GenerateReport(int entriesPerPage)
{
    vector<ReportPage> rep;

    map<string, HiScoreSet>::iterator iter;

    for (iter = scoreSets.begin(); iter != scoreSets.end(); ++iter)
    {
        // figure out set name
        string setname = iter->first;
        std::ifstream in(settings.getFilename(setname).c_str());

        if (in)
        {
            string l;
            getline(in, l);
            Strip(l);
            setname = _(l.c_str());
        }

        for (int i = 0; i < 3; i++)
        {
            ReportPage pag;
            int lines = 0;
            string header;
            switch (i)
            {
            case 0:
                /// Next 3 are shown in highscores on main menu
                header = _("Sequential");
                break;
            case 1:
                header = _("Best times");
                break;
            case 2:
                header = _("Survival");
                break;
            }

            pag.header = header + " - " + setname;

            if (i == 0)
            { // Adventure mode
                vector<HiScoreEntry>::iterator s;
                sort(iter->second.scores[i].begin(), iter->second.scores[i].end(), invSort<HiScoreEntry>);
                for (s = iter->second.scores[i].begin(); s != iter->second.scores[i].end(); s++)
                {
                    stringstream ln;
                    ln.clear();
                    ln << setw(7) << s->score;
                    ln << " " << setw(12) << left << trimUtf8(s->name, 12) << " " << _(s->level.c_str());
                    pag.lines.push_back(ln.str());
                    lines++;

                    if (lines > entriesPerPage)
                    {
                        lines = 0;
                        rep.push_back(pag);
                        pag.lines.clear();
                    }
                }
            }

            if (i == 1)
            { // Best times
                vector<HiScoreEntry>::iterator s;
                sort(iter->second.scores[i].begin(), iter->second.scores[i].end());
                for (s = iter->second.scores[i].begin(); s != iter->second.scores[i].end(); s++)
                {
                    char tm[256];
                    stringstream ln;
                    ln.clear();
                    sprintf(tm, "%02d:%02d", s->score / 60, s->score%60);
                    ln << setw(7) << tm;
                    ln << " " << setw(12) << left << trimUtf8(s->name, 12) << " " << _(s->level.c_str());
                    pag.lines.push_back(ln.str());
                    lines++;

                    if (lines > entriesPerPage)
                    {
                        lines = 0;
                        rep.push_back(pag);
                        pag.lines.clear();
                    }
                }
            }

            if (i == 2)
            { // Survival
                vector<HiScoreEntry>::iterator s;
                sort(iter->second.scores[i].begin(), iter->second.scores[i].end(), invSort<HiScoreEntry>);
                for (s = iter->second.scores[i].begin(); s != iter->second.scores[i].end(); s++)
                {
                    stringstream ln;
                    ln.clear();
                    ln << setw(7) << s->score;
                    ln << " " << setw(12) << left << trimUtf8(s->name, 12) << " " << _(s->level.c_str());
                    pag.lines.push_back(ln.str());
                    lines++;

                    if (lines > entriesPerPage)
                    {
                        lines = 0;
                        rep.push_back(pag);
                        pag.lines.clear();
                    }
                }
            }

            if (!pag.lines.empty())
                rep.push_back(pag);
        }
    }
    return rep;
}
}
