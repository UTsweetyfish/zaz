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

#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <cmath>
#include "scene.h"
#include "textureloader.h"
#include "bezier.h"
#include "common.h"
#include "ballpath.h"

class Player
{
    friend class Game;
    static const int pickSpeed = 10;
    static const int holdingDist = 3;
    static const int shootingSpeed = 3;
    static const int accuracyShotTimeout = 2300;

    GLuint tex;
    Bezier path;
    bool drawPath;
    bool pathLooped;
    bool invert;
    std::vector<XY> pts;
    int ptsLen;
    int currPt;
    double currPos;
    double rot;
    double mouseDiv;
    void UpdateRotation();
    BallPath **ballPaths;

    int pickedBall;
    int pickedCol;
    Bonus pickedBonus;
    int pickedPath;
    int pickedTex;
    int pickedBonusTex;
    double pickT;
    double pickV;
    XY pickPoint;
    bool picking;

    bool reshooting;
    bool shooting;
    int shootCol;
    Bonus shootBonus;
    int shootTex;
    int shootBonusTex;
    XY shootPoint;
    XY shootVel;
    int score;

    bool accuracyShot;
    int accuracyShotTime;

    double keyVelocity;
    double keySensivity;
    bool buttPressed;
    bool leftPressed;
    bool rightPressed;
    bool alwaysAccuracy;
    bool disableSpeedup;

public:
    Player(Bezier path, BallPath **ballPaths, bool pathLooped = false, bool invert = false, bool drawPath = false, bool alwaysAccuracy = false);
    ~Player();
    void Move(double t);
    void Render();
    void Logic(Scenes::FrameEvents events);
};

#endif //__PLAYER_H__
