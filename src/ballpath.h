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

#ifndef __BALLPATH_H__
#define __BALLPATH_H__

#include "common.h"

#include <deque>
#include <GL/gl.h>
#include <cmath>
#include <stack>
#include "scene.h"
#include "textureloader.h"
#include "bezier.h"
#include "audiobuffer.h"
#include "mixer.h"

#define NBALLCOLORS 16
#define STEPSPERBALL 200
#define POINTSPRITEFADEOUTTIME 100
#define BALLSIZE 5
#define BALLTEXTURECOUNT 8

enum Bonus {BONUS_NONE = 0, BONUS_ACCURACY, BONUS_PAUSE, BONUS_SLOW, BONUS_REVERSE, BONUS_BOMB};

struct Explosion
{
    double x;
    double y;
    int frame;
    int fc;
    double r;
    double s;

    Explosion(double x, double y, int frame = 0)
            :x(x), y(y), frame(frame), fc(0)
    {
        r = rand()%360;
        s = (rand()%60) + 30;
    };
};

struct PointSprite
{
    int points;
    double x;
    double y;
    double xx;
    double yy;
    double r;
    int time;

    void Recalc()
    {
        r+=0.1;
        x = xx + 2 * cos(r);
        y = yy;

        yy += 0.2;
    };

    PointSprite(int points, double x, double y)
            : points(points), xx(x), yy(y), r(0), time(POINTSPRITEFADEOUTTIME)
    {
        r = double(rand()%300) / 100.0;
        Recalc();
    };
};

struct ShotAddr
{
    int pos;
    bool front;

    ShotAddr(int pos, bool front = true)
            :pos(pos), front(front) {};
};

struct PathStep
{
    double x;
    double y;
    double r;
    bool under;
    bool hidden;

    PathStep(double x, double y, double r = 0.0, bool under = false, bool hidden = false)
            : x(x), y(y), r(r), under(under), hidden(hidden) {};
};

struct PathState
{
    bool ballOut;
    int score;
    double feedRate;
    double startFeedRate;
    int ballsToDraw;
    int ballsFromStart;
    int extraBall;
    int colors;
    int lastColors;
    int comboCnt;
    bool accuracyShotTriggered;
    bool bonusPause;
    int bonusPauseTime;

    bool bonusSlow;
    int bonusSlowTime;

    int bonusReverseBallsLeft;
    bool hadBonusBomb;
    bool hadBonusReverse;
    bool kidsMode;
    int bonusFrequency;
    bool speedUp;
    bool survival;

    PathState() : ballOut(false), score(0), feedRate(1), ballsToDraw(-1), extraBall(-1),
            colors(NBALLCOLORS), lastColors(NBALLCOLORS), comboCnt(0), accuracyShotTriggered(false), bonusPause(false), bonusSlow(false),
            bonusReverseBallsLeft(0), hadBonusBomb(false), hadBonusReverse(false), kidsMode(false), bonusFrequency(-1),
            speedUp(false), survival(false) {};
};

struct Ball
{
    int col;
    Bonus bonus;
    double frame;
    double pos;
    int size;
    bool elim;
    bool elbomb;
    bool explode;

    Ball(int col, Bonus bonus, uint pos)
            : col(col), bonus(bonus), frame(rand()%(BALLTEXTURECOUNT*BALLTEXTURECOUNT)), pos(pos), size(STEPSPERBALL), elim(false), elbomb(false), explode(false) {};

    uint upos()
    {
        if (pos > 0)
        {
            return (uint)iround(pos);
        }

        return 0;
    }
};

class BallPath
{
    friend class Player;
    friend class Game;
    friend struct Ball;
    const static int stepsPerBall = STEPSPERBALL;
    const static int attractDelay = 0;
    const static int maxPullSpeed = 30;
    int ballSize;
    const static int ballGrowSpeed = 10;
    const static int scoreElimination = 10;
    const static int maxBallDistanceToAttract = 200;
    const static int bombDistance = 7;
    const static int bonusPauseTimeout = 1000;
    const static int bonusSlowTimeout = 2000;
    const static int bonusReverseBalls = 20;
    const static int bonusFrequency = 5;
    const static int pointSpriteFadeoutTime = POINTSPRITEFADEOUTTIME;
    const static int extraBallFadeinTime = 150;
    const static int ballTextureSize = 512;
    const static int ballTextureCount = BALLTEXTURECOUNT;
    const static int ballAnimFrames = ballTextureCount * ballTextureCount;
    const static int ballOneTextureSize = ballTextureSize / ballTextureCount;
    const static int speedUpMultiplier = 10;

    Bezier path;
    bool drawPath;
    bool pathLooped;

    uint pthLen;
    int currPt;
    double pullSpeed;
    int waitAttract;
    bool hasGap;
    Ball DrawBall();
    Bonus DrawBonus();
    void GenRotation();
    void Attract();
    void Eliminate();
    void Anim();

    GLuint *tex;
    std::vector<PathStep> ballPath;
    std::deque<Ball> balls;

    Scenes::AudioBuffer *rollSound;
    bool playRoll;
    stack<Ball>reversedBalls;

    std::vector<PointSprite> pointSprites;
    std::vector<Explosion> explosions;

    Scenes::Mixer **mixer;
    bool extraBallFading;
    int extraBallFadeinTimeout;

public:
    PathState state;
    BallPath(Bezier path, GLuint *tex, Scenes::Mixer **mixer, bool drawPath = false, double ballSizeFactor = 1.0);
    int Pick(double x, double y, double vx, double vy);
    ShotAddr PickShot(double x, double y);
    void InsertBall(ShotAddr addr, int col, Bonus bonus = BONUS_NONE);
    ~BallPath();
    void Drive(double d);
    void NewBall();
    void Render(bool render_balls = true, bool render_sprites = true, bool render_under = true, bool render_over = true);
    void Logic();
};

#endif //__BALLPATH_H__
