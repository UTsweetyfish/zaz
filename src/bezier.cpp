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

#include "bezier.h"
#include "error.h"

#define MIDP(a, b, t) XY(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.under, a.hidden)

std::vector<XY> Bezier::Generate(int steps)
{
    double stepping = 1.0 / (double)steps;

    std::vector<XY> ret;
    std::vector<BezierPoint>::iterator iter;
    std::vector<BezierPoint>::iterator iter2;

    bool first = true;
    for (iter = points.begin(), iter2 = points.begin(), ++iter2; iter2 != points.end(); ++iter, ++iter2)
    {
        XY p0(iter->x, iter->y, iter->under, iter->hidden);
        XY p1(iter->cx, iter->cy, iter->under, iter->hidden);
        XY p2(iter2->cx, iter2->cy, iter2->under, iter2->hidden);
        XY p3(iter2->x, iter2->y, iter2->under, iter2->hidden);

        // if not first point, the controls for in p1 are inverse

        if (!first)
        {
            p1 = XY((iter->x - iter->cx) + iter->x,
                    (iter->y - iter->cy) + iter->y, iter->under, iter->hidden);
        }
        first = false;

        for (double t = 0; t < 1.0; t+= stepping)
        {
            XY l1 = MIDP(p0, p1, t);
            XY l2 = MIDP(p1, p2, t);
            XY l3 = MIDP(p2, p3, t);

            XY m1 = MIDP(l1, l2, t);
            XY m2 = MIDP(l2, l3, t);

            ret.push_back(MIDP(m1, m2, t));
        }

        ret.push_back(XY(iter2->x, iter2->y, iter2->under, iter2->hidden));
    }

    return ret;
}

double veclen(XY a, XY b)
{
    return sqrt(pow(fabs(b.x - a.x), 2.0) + pow(fabs(b.y - a.y), 2.0));
}

std::vector<XY> Bezier::GenerateUniform(double step)
{
    if (step == 0.0)
        ERR("0 step in bezier !");

    std::vector<XY> ret;
    std::vector<BezierPoint>::iterator iter;
    std::vector<BezierPoint>::iterator iter2;

    bool first = true;
    for (iter = points.begin(), iter2 = points.begin(), ++iter2; iter2 != points.end(); ++iter, ++iter2)
    {
        XY p0(iter->x, iter->y, iter->under, iter->hidden);
        XY p1(iter->cx, iter->cy, iter->under, iter->hidden);
        XY p2(iter2->cx, iter2->cy, iter2->under, iter2->hidden);
        XY p3(iter2->x, iter2->y, iter2->under, iter2->hidden);

        // if not first point, the controls for in p1 are inverse
        if (!first)
        {
            p1 = XY((iter->x - iter->cx) + iter->x,
                    (iter->y - iter->cy) + iter->y, iter->under, iter->hidden);
        }
        first = false;


        double stepping = 0.01 / (((veclen(p1, p0) +
                                    veclen(p2, p1) +
                                    veclen(p3, p2)) / 3.0) / step);

        XY lp = p0;
        for (double t = 0; t < 1.0; t+= stepping)
        {
            XY l1 = MIDP(p0, p1, t);
            XY l2 = MIDP(p1, p2, t);
            XY l3 = MIDP(p2, p3, t);

            XY m1 = MIDP(l1, l2, t);
            XY m2 = MIDP(l2, l3, t);

            XY np = MIDP(m1, m2, t);

            double len = veclen(np, lp);
            if (len >= step)
            {
                ret.push_back(np);
                lp = np;
            }
        }

        ret.push_back(XY(iter2->x, iter2->y, iter2->under, iter2->hidden));
    }

    return ret;
}

std::vector<XY> Bezier::GenerateBalls(double dist, int steps)
{
    std::vector<XY> ret;
    std::vector<BezierPoint>::iterator iter;
    std::vector<BezierPoint>::iterator iter2;

    bool first = true;
    bool firstDist = true;
    int s = 0;
    for (iter = points.begin(), iter2 = points.begin(), ++iter2; iter2 != points.end(); ++iter, ++iter2)
    {
        XY p0(iter->x, iter->y, iter->under, iter->hidden);
        XY p1(iter->cx, iter->cy, iter->under, iter->hidden);
        XY p2(iter2->cx, iter2->cy, iter2->under, iter2->hidden);
        XY p3(iter2->x, iter2->y, iter2->under, iter2->hidden);

        // if not first point, the controls for in p1 are inverse
        if (!first)
        {
            p1 = XY((iter->x - iter->cx) + iter->x,
                    (iter->y - iter->cy) + iter->y, iter->under, iter->hidden);
        }



        double step = dist / (double)steps;
        double stepping = 0.01 / (((veclen(p1, p0) +
                                    veclen(p2, p1) +
                                    veclen(p3, p2)) / 3.0) / step);

        XY lp = p0;
        first = false;

        for (double t = 0; t < 1.0; t+= stepping)
        {
            XY l1 = MIDP(p0, p1, t);
            XY l2 = MIDP(p1, p2, t);
            XY l3 = MIDP(p2, p3, t);

            XY m1 = MIDP(l1, l2, t);
            XY m2 = MIDP(l2, l3, t);

            XY np = MIDP(m1, m2, t);

            if ((firstDist) && (s <= dist * steps))
            {
                if (veclen(lp, np) >= step)
                {
                    ret.push_back(np);
                    lp = np;
                    ++s;
                }
            }
            else
            {
                double len = veclen(np, ret[s - steps]);
                if (len >= dist)
                {
                    ret.push_back(np);
                    ++s;
                    lp = np;
                }
            }
        }

        firstDist = false;

        ret.push_back(XY(iter2->x, iter2->y, iter2->under, iter2->hidden));
    }

    return ret;
}
