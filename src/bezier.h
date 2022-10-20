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

#ifndef __BEZIER_H__
#define __BEZIER_H__

#include <iostream>
#include <vector>
#include <string>
#include <cmath>

struct BezierPoint
{
    double x, y;   // centre
    double cx, cy; // control point
    bool under;
    bool hidden;


    BezierPoint() {};

    BezierPoint(double x, double y)
            : x(x), y(y), cx(0.0), cy(0.0), under(false), hidden(false) {};

    BezierPoint(double x, double y, double cx, double cy, bool under = false, bool hidden = false)
            : x(x), y(y), cx(cx), cy(cy), under(under), hidden(hidden) {};

//	BezierPoint operator~(void) { return BezierPoint(x, y, -cx, -cy);}
};

struct XY
{
    double x, y;
    bool under;
    bool hidden;

    XY()
            :x(0), y(0), under(false), hidden(false) {};

    XY(double x, double y, bool under = false, bool hidden = false)
            : x(x), y(y), under(under), hidden(hidden) {};
};

struct Bezier
{
    std::vector<BezierPoint> points;

    Bezier() {};

    std::vector<XY> Generate(int steps = 5);
    std::vector<XY> GenerateUniform(double step = 0.1);
    // dist - distance between balls
    // in -step- steps
    std::vector<XY> GenerateBalls(double dist = 5.0, int steps = 50);

};

#endif //__BEZIER_H__
