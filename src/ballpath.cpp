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

#include "ballpath.h"
#include "game.h"

BallPath::BallPath(Bezier path, GLuint *textures, Scenes::Mixer **mixer, bool drawPath, double ballSizeFactor)
        :  ballSize(BALLSIZE), path(path), drawPath(drawPath), hasGap(false), tex(textures), rollSound(0), playRoll(false), mixer(mixer),
        extraBallFading(false), extraBallFadeinTimeout(0)
{
//    std::vector<XY> pts = path.GenerateBalls(ballSize, stepsPerBall);
    ballSize = (int)(ballSize * ballSizeFactor);
    std::vector<XY> pts = path.GenerateUniform((double)ballSize / (double)stepsPerBall);
    std::vector<XY>::iterator i;
    for (i = pts.begin(); i != pts.end(); ++i)
        ballPath.push_back(PathStep(i->x, i->y, 0, i->under, i->hidden));

    pthLen = ballPath.size();
    GenRotation();
}

BallPath::~BallPath()
{
}

void BallPath::GenRotation()
{
    for (uint i = 1; i < pthLen - 1; ++i)
    {
        XY pt1 = XY(ballPath[i - 1].x, ballPath[i - 1].y);
        XY pt2 = XY(ballPath[i + 1].x, ballPath[i + 1].y);

        double x = pt2.x - pt1.x;
        double y = (pt2.y - pt1.y);

        ballPath[i].r = atan2(y, x) * (180.0 / PI);
    }
}

Bonus BallPath::DrawBonus()
{
    int freq = bonusFrequency;
    if (state.bonusFrequency > 0)
        freq = state.bonusFrequency;

    int b = (rand()%(int(BONUS_BOMB))) + 1;

    int d = rand()%100;

    if (d > 100 - freq)
    {
        if (b == BONUS_BOMB)
        {
            if (state.survival)
                return BONUS_NONE;

            if (!state.hadBonusBomb || state.kidsMode)
            {
                state.hadBonusBomb = true;
                return BONUS_BOMB;
            }

            return BONUS_NONE;
        }

        if (b == BONUS_REVERSE)
        {
            if (state.survival)
                return BONUS_NONE;

            if (!state.hadBonusReverse || state.kidsMode)
            {
                state.hadBonusReverse = true;
                return BONUS_REVERSE;
            }

            return BONUS_NONE;
        }

        if (b == BONUS_ACCURACY && state.kidsMode)
            return BONUS_NONE;

        return Bonus(b);
    }

    return BONUS_NONE;
}

Ball BallPath::DrawBall()
{
    if (!reversedBalls.empty())
    {
        Ball db = reversedBalls.top();
        reversedBalls.pop();
        return db;
    }

    if (balls.size() < 2)
    {
        if (state.ballsToDraw > 0)
            state.ballsToDraw--;

        return Ball(rand() % state.colors, DrawBonus(), 0);
    }

    bool ok = false;
    int b = 0;

    while (!ok)
    {
        ok = true;
        b = rand() % state.colors;

        if ((b == balls[0].col)
                && (b == balls[1].col))
            ok = false;
    }

    if (state.ballsToDraw > 0)
        state.ballsToDraw--;

    return Ball(b, DrawBonus(), 0);
}


void BallPath::Logic()
{
    if (extraBallFading)
    {
        if (!balls.empty())
        {
            extraBallFadeinTimeout--;
            if (extraBallFadeinTimeout == 0)
            {
                extraBallFading = false;
                balls.push_front(Ball(state.extraBall, BONUS_NONE, balls[0].upos() - (stepsPerBall)));
                balls[0].size = stepsPerBall;
                state.extraBall = -1;
            }
        }
        else
        {
            extraBallFading = false;
            state.extraBall = -1;
        }
    }

    if (!explosions.empty())
    {
        vector<Explosion>::iterator i;
        bool clear = true;

        for (i = explosions.begin(); i != explosions.end(); ++i)
        {
            i->fc++;
            if (i->fc > 2)
            {
                i->frame++;
                i->fc = 0;
            }
            if (i->frame <= 16)
            {
                clear = false;
            }
        }

        if (clear)
            explosions.clear();
    }

    if (!pointSprites.empty())
    {
        vector<PointSprite>::iterator i;
        bool clear = true;

        for (i = pointSprites.begin(); i != pointSprites.end(); ++i)
        {
            if (i->time > 0)
            {
                clear = false;
                i->time--;
                i->Recalc();
            }

            if (i->time <= 0)
            {
                i->time = 0;
            }
        }

        if (clear)
            pointSprites.clear();
    }

    if (state.bonusPause)
    {
        state.bonusPauseTime--;
        if (state.bonusPauseTime < 0)
            state.bonusPause = false;
    }

    if (state.bonusSlow)
    {
        state.bonusSlowTime--;
        if (state.bonusSlowTime < 0)
            state.bonusSlow = false;
    }

    if (playRoll || state.speedUp)
    {
        if (rollSound == 0)
            if (mixer != NULL)
                rollSound = (*mixer)->EnqueueSample (sfx_roll, sfxVol, 0, true);
    }
    else
    {
        if (rollSound != 0)
        {
            rollSound->Stop();
            rollSound = 0;
        }
    }

    if ((state.bonusReverseBallsLeft > 0) && (!state.speedUp))
    {
        if (balls.size() == 0)
        {
            state.bonusReverseBallsLeft = 0;
            state.bonusPause = false;
            state.bonusSlow = false;
        }
        else
        {
            playRoll = true;

            if (balls[0].pos == 0)
            {
                reversedBalls.push(balls[0]);
                balls.erase(balls.begin());
                state.bonusReverseBallsLeft--;
                return;
            }
            else
            {
                balls[0].pos-=maxPullSpeed;
                if (balls[0].pos < 0)
                    balls[0].pos = 0;
            }

            Attract();
            Eliminate();
            return;
        }
    }

    if (state.ballsFromStart > 0)
    {
        if (balls.size() > 0)
        {
            if (balls[0].pos >= stepsPerBall)
            {
                Ball db = DrawBall();
                balls.push_front(Ball(db.col, db.bonus, 0));
                --state.ballsFromStart;
            }
        }
        else
        {
            Ball db = DrawBall();
            balls.push_front(Ball(db.col, db.bonus, 0));
            --state.ballsFromStart;
        }
    }

    if (!state.ballOut)
    {
        if (balls.size() == 0)
        {
            if (state.ballsToDraw == -1 || state.ballsToDraw > 0 || !reversedBalls.empty())
            {
                Ball db = DrawBall();
                balls.push_front(Ball(db.col, db.bonus, 0));
            }
        }

        if (!balls.empty())
            if (balls[0].pos >= stepsPerBall)
            {
                if (state.ballsToDraw == -1 || state.ballsToDraw > 0 || !reversedBalls.empty())
                {
                    Ball db = DrawBall();
                    balls.push_front(Ball(db.col, db.bonus, 0));
                }
                else
                {
                    if (state.extraBall != -1)
                        if (balls[0].pos > STEPSPERBALL * 2)
                            if (!extraBallFading)
                            {
                                extraBallFadeinTimeout = extraBallFadeinTime;
                                extraBallFading = true;
                                if (mixer != NULL)
                                    (*mixer)->EnqueueSample(sfx_extraball, sfxVol);
                            }
                }
            }
    }


    // this could happen...
    if (state.extraBall != -1 && balls.empty())
    {
        extraBallFading = false;
        state.extraBall = -1;
    }

    if (state.ballsFromStart > 0)
    {
        Drive(stepsPerBall / 4);
    }
    else
    {
        if (state.speedUp)
        {
            Drive(state.feedRate * speedUpMultiplier);
            state.bonusPause = false;
            state.bonusSlow = false;
            state.bonusReverseBallsLeft = 0;
        }
        else
        {
            Drive(state.feedRate);
        }
        Attract();
        Eliminate();
    }
}

void BallPath::Drive(double d)

{
    if (state.bonusPause)
        d = 0.0;

    if (state.bonusSlow)
        d/=2.0;

    if (balls.empty())
        return;

    if (d > 10)
        playRoll = true;

    balls[0].pos+=d;
    balls[0].frame+= ((double)ballAnimFrames/(double)STEPSPERBALL/2.0) * (double)d;

    if (!balls[0].elim)
    {
        if (balls[0].pos < stepsPerBall)
        {
            if (balls[0].size < stepsPerBall)
                balls[0].size += (int)d; //ballGrowSpeed;
        }
        else
        {
            balls[0].size += ballGrowSpeed;
        }
    }

    if (balls[0].size > stepsPerBall)
        balls[0].size = stepsPerBall;

    if (balls[0].frame > (double)ballAnimFrames)
        balls[0].frame -= (double)ballAnimFrames;

    for (uint b = 1; b < balls.size(); b++)
    {
        if (!balls[b].elim)
            if (balls[b].size < stepsPerBall)
                balls[b].size += ballGrowSpeed;

        if (balls[b].size > stepsPerBall)
            balls[b].size = stepsPerBall;

        if ((balls[b].pos - balls[b - 1].pos) < balls[b - 1].size)
        {
            balls[b].				pos = balls[b - 1].pos + balls[b - 1].size;
            balls[b].frame += ((double)ballAnimFrames/(double)STEPSPERBALL/2.0) * (double)d;
            if (balls[b].frame > (double)ballAnimFrames)
                balls[b].frame -= (double)ballAnimFrames;
        }
    }

    // is a ball outside
    int lball = balls.size() - 1;
    if (balls[lball].upos() >= pthLen)
    {
        state.ballOut = true;
        deque<Ball>::iterator i;
        i = balls.begin();
        i+=lball;

        balls.erase(i);
    }
}

void BallPath::InsertBall(ShotAddr addr, int col, Bonus bonus)
{
    state.comboCnt = 0;
    state.score -= 5;

    if (addr.pos == -1)
    {
        balls.push_front(Ball(col, bonus, (uint)balls[0].pos - ballGrowSpeed));
        balls[0].size = ballGrowSpeed;
        return;
    }

    if (addr.front && addr.pos == 0)
        addr.front = false;

    if ((uint)addr.pos < balls.size())
    {
        if (addr.front)
        {
            balls[addr.pos].pos+=ballGrowSpeed;

            for (int b = addr.pos; (uint)b < balls.size(); b++)
            {
                if ((balls[b].pos - balls[b - 1].pos) < stepsPerBall)
                    balls[b].pos = balls[b - 1].pos + stepsPerBall;
            }

            balls.push_back(balls[balls.size() - 1]);

            for (int b = balls.size() - 1; b > addr.pos; --b)
                balls[b] = balls[b - 1];

            balls[addr.pos].col = col;
            balls[addr.pos].bonus = bonus;
            balls[addr.pos].size = ballGrowSpeed;
            balls[addr.pos].elim = false;
        }
        else
        {
            balls.push_back(balls[balls.size() - 1]);

            for (int b = balls.size() - 1; b > addr.pos; --b)
                balls[b] = balls[b - 1];

            addr.pos++;
            balls[addr.pos].pos+=ballGrowSpeed;

            for (int b = addr.pos + 1; (uint)b < balls.size(); b++)
            {
                if ((balls[b].pos - balls[b - 1].pos) < stepsPerBall)
                    balls[b].pos = balls[b - 1].pos + stepsPerBall;
            }

            balls[addr.pos].col = col;
            balls[addr.pos].bonus = bonus;
            balls[addr.pos].size = ballGrowSpeed;
            balls[addr.pos].elim = false;
        }
    }
    else
    {
        balls.push_back(balls[balls.size() - 1]);
        balls[addr.pos].pos+=ballGrowSpeed;
        balls[addr.pos].col = col;
        balls[addr.pos].bonus = bonus;
        balls[addr.pos].elim = false;

        balls[addr.pos].size = ballGrowSpeed;
    }
}

void BallPath::Eliminate()
{
    bool updateCombo = false;

    for (uint b = 0; b < balls.size(); ++b)
    {
        int score = 0;

        if (balls[b].elim)
            balls[b].size -= ballGrowSpeed;

        if (balls[b].size < 0)
        {
            deque<Ball>::iterator i;
            i = balls.begin();
            i += b;

            int pan = (int)ballPath[balls[b].upos()].x - 50;

            if (balls[b].bonus == BONUS_PAUSE)
            {
                state.bonusPauseTime = bonusPauseTimeout;
                state.bonusPause = true;
                if (mixer != NULL)
                    (*mixer)->EnqueueSample(sfx_bonus, sfxVol, pan);
            }

            if (balls[b].bonus == BONUS_SLOW)
            {
                state.bonusSlowTime = bonusSlowTimeout;
                state.bonusSlow = true;
                if (mixer != NULL)
                    (*mixer)->EnqueueSample(sfx_bonus, sfxVol, pan);
            }

            if (balls[b].bonus == BONUS_REVERSE)
            {
                state.bonusReverseBallsLeft += bonusReverseBalls;
                if (mixer != NULL)
                    (*mixer)->EnqueueSample(sfx_bonus, sfxVol, pan);
            }

            if (balls[b].bonus == BONUS_ACCURACY)
            {
                state.accuracyShotTriggered = true;
                if (mixer != NULL)
                    (*mixer)->EnqueueSample(sfx_bonus, sfxVol, pan);
            }

            if (balls[b].bonus == BONUS_BOMB)
            {
                int bdist = bombDistance;
//				if (state.kidsMode)
//					bdist *= 1.5;

                int bs = (int)balls[b].pos - bdist * stepsPerBall;
                int be = (int)balls[b].pos + bdist * stepsPerBall;

                if (bs < 0)
                    bs = 0;

                if (be >= (int)pthLen)
                    be = pthLen;

                for (uint f = 0; f < balls.size(); ++f)
                {
                    if (balls[f].pos >= bs && balls[f].pos <= be)
                    {
                        balls[f].elim = true;
                        balls[f].elbomb = true;
                        balls[f].explode = true;
                    }
                }

                if (mixer != NULL)
                    (*mixer)->EnqueueSample(sfx_bonus, sfxVol, pan);
            }

            if (!balls[b].elbomb) // don't count bombs for combos
            {
                updateCombo = true;
                state.comboCnt++;
            }
            else
            {
                state.comboCnt = 0;
            }

            if ((state.comboCnt > 3) && (!balls[b].elbomb))
            {
                score = scoreElimination * (state.comboCnt - 2);
                state.score += score;
            }
            else
            {
                score = scoreElimination;
                state.score += score;
            }

            double x = ballPath[balls[b].upos()].x;
            double y = ballPath[balls[b].upos()].y;

            pointSprites.push_back(PointSprite(score, x, y));
            balls.erase(i);
        }
    }

    if (updateCombo)
    {
        if (state.comboCnt > 3)
            if (mixer != NULL)
                (*mixer)->EnqueueSample(sfx_combo, sfxVol);
    }

    if (balls.size() < 3)
        return;

    for (uint b = 0; b < balls.size() - 2; ++b)
    {
        int col = balls[b].col;

        if ((balls[b + 1].col == col)
                && (balls[b + 2].col == col)
                /*                && (balls[b].size == stepsPerBall)
                                && (balls[b + 1].size == stepsPerBall)
                                && (balls[b + 2].size == stepsPerBall)
                */                && (balls[b + 2].pos - balls[b + 1].pos < (double)STEPSPERBALL * 1.01)
                && (balls[b + 1].pos - balls[b].pos < (double)STEPSPERBALL * 1.01))
        {
            if (!balls[b].elim)
                balls[b].explode = true;

            if (!balls[b + 1].elim)
                balls[b + 1].explode = true;

            if (!balls[b + 2].elim)
                balls[b + 2].explode = true;

            balls[b].elim = true;
            balls[b + 1].elim = true;
            balls[b + 2].elim = true;
        }
    }

    // trigger explosions
    for (uint b = 0; b < balls.size(); ++b)
    {
        if (balls[b].explode)
        {
            balls[b].explode = false;
            double x = ballPath[balls[b].upos()].x;
            double y = ballPath[balls[b].upos()].y;

            explosions.push_back(Explosion(x, y));

            int pan = (int)x - 50;

            if (mixer != NULL)
                (*mixer)->EnqueueSample(sfx_eliminate, sfxVol, pan);
        }
    }

}

void BallPath::Attract()
{
    bool fullSpeed = false;

    if (state.bonusReverseBallsLeft > 0)
        fullSpeed = true;

    playRoll = false;

    if (balls.size() < 2)
        return;

    if (hasGap)
    {
        waitAttract--;
    }

    // do we have a gap ?
    bool gap = false;
    int gapball = 0;

    for (uint b = 0; b < balls.size() - 1; ++b)
    {
        int d = (int)iround(balls[b + 1].pos - balls[b].pos);
        if ((d > stepsPerBall) && (d < stepsPerBall * maxBallDistanceToAttract))
        {
            gap = true;
            gapball = b + 1;
        }
    }

    if (!gap)
    {
        hasGap = false;
        return;
    }

    if (!hasGap)
    {
        waitAttract = attractDelay;
        hasGap = true;
        pullSpeed = 2.0;
    }

    if (!fullSpeed)
        if (waitAttract > 0)
            return;

    // has a gap and it's time to do some magnetic action
    playRoll = true;
    pullSpeed *= 1.02;
    if (pullSpeed > maxPullSpeed)
        pullSpeed = maxPullSpeed;

    if (fullSpeed)
        pullSpeed = maxPullSpeed;

    for (uint b = gapball; b < balls.size(); ++b)
    {
        balls[b].pos -= pullSpeed;
        balls[b].frame -= pullSpeed / 2;
        if (balls[b].frame < 0)
            balls[b].frame += ballAnimFrames;
    }

    if ((balls[gapball].pos - balls[gapball - 1].pos) < (double)stepsPerBall)
    {
        int push = (int)(stepsPerBall - (balls[gapball].pos - balls[gapball - 1].pos));
        for (uint b = gapball; b < balls.size(); ++b)
        {
            balls[b].pos += push;
            balls[b].frame += push / 2;
            if (balls[b].frame > ballAnimFrames)
                balls[b].frame -= ballAnimFrames;
        }
    }
}

void BallPath::Render(bool render_balls, bool render_sprites, bool render_under, bool render_over)
{
    glLoadIdentity();
    // balls
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    if (render_balls)
        for (uint i = 0; i < balls.size(); ++i)
        {
            uint pos = balls[i].upos();

            int fr = (int)balls[i].frame;
            if (fr > ballAnimFrames - 1)
                fr = 0;

//            double ts = (ballOneTextureSize * 8.0) / double(ballTextureSize);
            double ts = 1.0 / double(ballTextureCount);
            double tx = (double)((fr % ballTextureCount)) * ts;
            double ty = (double)((fr / ballTextureCount)) * ts;
            //ts *= 0.1;

            double bs = ((double)balls[i].size / (stepsPerBall)) * ballSize;

            if (balls[i].pos < stepsPerBall)
            {
                bs = ((double)balls[i].pos / (stepsPerBall / 4)) * ballSize;
                if (bs > ballSize)
                    bs = ballSize;
            }

            if (balls[i].pos > pthLen - stepsPerBall)
            {
                bs = ((double)(balls[i].pos - (pthLen - stepsPerBall))/ (stepsPerBall / 4)) * ballSize;
                bs = ballSize - bs;
                if (bs < 0.0)
                    bs = 0.0;
            }

            bool render_this_ball = false;

            if (pos < pthLen)
            {
                render_this_ball = true;

                if ((ballPath[pos].under || ballPath[pos].hidden) && !render_under)
                    render_this_ball = false;

                if ((!ballPath[pos].under && !ballPath[pos].hidden) && !render_over)
                    render_this_ball = false;
            }

            if (render_this_ball)
            {
                glLoadIdentity();
                glTranslated(ballPath[pos].x, ballPath[pos].y, 2);
                glBindTexture(GL_TEXTURE_2D, tex[balls[i].col]);
                if (ballPath[pos].under || ballPath[pos].hidden)
                    glTranslated(0.0, 0.0, -1);

                glScaled(bs, bs, bs);

                glRotated(ballPath[pos].r, 0, 0, 1.0);

                glBegin(GL_QUADS);
                glTexCoord2d(tx, ty);
                glVertex3d(-0.5, 0.5, 0);
                glTexCoord2d(tx, ty + ts);
                glVertex3d(-0.5, -0.5, 0);
                glTexCoord2d(tx + ts, ty + ts);
                glVertex3d(0.5, -0.5, 0);
                glTexCoord2d(tx + ts, ty);
                glVertex3d(0.5, 0.5, 0);
                glEnd();

                if (balls[i].bonus != BONUS_NONE)
                {
                    glLoadIdentity();
                    glTranslated(ballPath[pos].x, ballPath[pos].y, 2.1);

                    if (ballPath[pos].under || ballPath[pos].hidden)
                        glTranslated(0.0, 0.0, -1);

                    glScaled(bs, bs, bs);
                    glRotated(ballPath[pos].r, 0, 0, 1.0);
                    glBindTexture(GL_TEXTURE_2D, tex[NBALLCOLORS + (balls[i].bonus - 1) ]);
                    glBegin(GL_QUADS);
                    glTexCoord2d(0, 0);
                    glVertex3d(-0.5, 0.5, 0);
                    glTexCoord2d(0, 1);
                    glVertex3d(-0.5, -0.5, 0);
                    glTexCoord2d(1, 1);
                    glVertex3d(0.5, -0.5, 0);
                    glTexCoord2d(1, 0);
                    glVertex3d(0.5, 0.5, 0);
                    glEnd();
                }
            }
        }

    // extraball
    if (render_balls)
        if (!balls.empty())
            if (extraBallFading && balls[0].pos > stepsPerBall)
            {
                float alpha = float((extraBallFadeinTime - extraBallFadeinTimeout)) / float(extraBallFadeinTime);
                int pos = (int)(alpha * (balls[0].pos - stepsPerBall));

                if (pos < 0)
                    pos = 0;

                double bs = ((double)balls[0].size / (stepsPerBall)) * ballSize;
                double x = ballPath[pos].x;
                double y = ballPath[pos].y;
                double ts = 1.0 / double(ballTextureCount);

                glLoadIdentity();
                glTranslated(x, y, 2);

                bool render_this_ball = true;

                if ((ballPath[pos].under || ballPath[pos].hidden) && !render_under)
                    render_this_ball = false;

                if ((!ballPath[pos].under && !ballPath[pos].hidden) && !render_over)
                    render_this_ball = false;

                if (ballPath[pos].under || ballPath[pos].hidden)
                    glTranslated(0.0, 0.0, -1);

                if (render_this_ball)
                {
                    glRotated(ballPath[pos].r, 0, 0, 1.0);
                    glScaled(bs, bs, bs);
                    glColor4f(1.0f, 1.0f, 1.0f, alpha);
                    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
                    glBindTexture(GL_TEXTURE_2D, tex[state.extraBall]);
                    glBegin(GL_QUADS);
                    glTexCoord2d(0, 0);
                    glVertex3d(-0.5, 0.5, 0);
                    glTexCoord2d(0, ts);
                    glVertex3d(-0.5, -0.5, 0);
                    glTexCoord2d(ts, ts);
                    glVertex3d(0.5, -0.5, 0);
                    glTexCoord2d(ts, 0);
                    glVertex3d(0.5, 0.5, 0);
                    glEnd();
                    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
                }
            }

    // explosions
    vector<Explosion>::iterator expl;

    double z = 20;
    if (render_sprites)
        for (expl = explosions.begin(); expl != explosions.end(); ++expl)
        {
            if (expl->frame < 17)
            {
                double tx = (expl->frame % 4);
                double ty = (expl->frame / 4);

                tx*=0.25;
                ty*=0.25;

                glLoadIdentity();
                glTranslated(expl->x, expl->y, z);
                glRotated(expl->r, 0.0, 0.0, 1.0);
                glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
                glBindTexture(GL_TEXTURE_2D, tex[NBALLCOLORS + BONUS_BOMB]);
//            glBindTexture(GL_TEXTURE_2D, tex[NBALLCOLORS + 1]);
                glColor4d(1.0, 1.0, 1.0, 0.7);
                glScaled(expl->s, expl->s, 1.0);

                glBegin(GL_QUADS);
                glTexCoord2d(tx, ty);
                glVertex3d(-0.5, 0.5, 0);
                glTexCoord2d(tx, ty + 0.25);
                glVertex3d(-0.5, -0.5, 0);
                glTexCoord2d(tx + 0.25, ty + 0.25);
                glVertex3d(0.5, -0.5, 0);
                glTexCoord2d(tx + 0.25, ty);
                glVertex3d(0.5, 0.5, 0);

                glEnd();
                glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            }
            z+=1.0;
        }

    // pointsprites
    vector<PointSprite>::iterator psi;
    if (render_sprites)
        for (psi = pointSprites.begin(); psi != pointSprites.end(); ++psi)
        {
            if (psi->time)
            {
                float alpha = float(psi->time) / float(pointSpriteFadeoutTime);

                glColor4f(1.0, 1.0, 1.0, alpha);
                glLoadIdentity();
                glTranslated(psi->x, psi->y, 50);
                glScaled(0.1, 0.1, 0.1);

                char scoretxt[256];
                sprintf(scoretxt, "%d", psi->points);
                glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
                font3->Render(scoretxt);
                glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            }
        }

    glDisable(GL_TEXTURE_2D);
}

int BallPath::Pick(double x, double y, double vx, double vy)
{
    int ret = -1;
    double ldist = 200;
    bool outside = false;
    double t = 1.0;
    uint nballs = balls.size();

    while (!outside)
    {
        double px = x + t * vx;
        double py = y + t * vy;

        if (px > 130 || px < -30 || py > 130 || py < -30)
        {
            outside = true;
            continue;
        }

        for (uint b = 0; b < nballs; ++b)
        {
            if (balls[b].elim)
                continue;

            if (ballPath[balls[b].upos()].hidden)
                continue;

            double bx = ballPath[balls[b].upos()].x;
            double by = ballPath[balls[b].upos()].y;

            if (fabs(px - bx) < ((double)ballSize / 1.5) && fabs(py - by) < ((double)ballSize / 1.5))
            {
                double dist = sqrt(pow(fabs(bx - x), 2.0) + pow(fabs(by - y), 2.0));

                if (dist < ldist)
                {
                    return b;
                    /*ret = b;
                    ldist = dist;*/
                }
            }
        }

        t+=2.0;
    }

    return ret;
}

ShotAddr BallPath::PickShot(double x, double y)
{
    int ret = -1;
    double ldist = 200;
    bool front = true;

    for (uint b = 0; b < balls.size(); ++b)
    {
        double bx = ballPath[balls[b].upos()].x;
        double by = ballPath[balls[b].upos()].y;

        if (ballPath[balls[b].upos()].hidden)
            continue;

        if (fabs(x - bx) < ((double)ballSize) && fabs(y - by) < ((double)ballSize))
        {
            double dist = sqrt(pow(fabs(bx - x), 2.0) + pow(fabs(by - y), 2.0));

            double distf;
            if (balls[b].upos() > 0)
            {
                bx = ballPath[balls[b].upos() - 1].x;
                by = ballPath[balls[b].upos() - 1].y;
                distf = sqrt(pow(fabs(bx - x), 2.0) + pow(fabs(by - y), 2.0));
            }
            else
            {
                bx = ballPath[balls[b].upos()].x;
                by = ballPath[balls[b].upos()].y;
                distf = sqrt(pow(fabs(bx - x), 2.0) + pow(fabs(by - y), 2.0));
            }

            bx = ballPath[balls[b].upos() + 1].x;
            by = ballPath[balls[b].upos() + 1].y;
            double distb = sqrt(pow(fabs(bx - x), 2.0) + pow(fabs(by - y), 2.0));

            if (dist < ldist)
            {
                ret = b;
                ldist = dist;

                if (distf < distb)
                {
                    front = true;
                }
                else
                {
                    front = false;
                }
            }
        }
    }

    return ShotAddr(ret, front);
}
