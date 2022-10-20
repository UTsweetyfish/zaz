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

#include "player.h"
#include "game.h"
#include <deque>

Player::Player(Bezier path, BallPath **ballPaths, bool pathLooped, bool invert, bool drawPath, bool alwaysAccuracy)
        : tex(LoadTexture("player.png")), path(path), drawPath(drawPath), pathLooped(pathLooped), invert(invert),
        ballPaths(ballPaths),  pickedCol(-1), pickedBonus(BONUS_NONE), picking(false), shooting(false), score(0),
        accuracyShot(false), alwaysAccuracy(alwaysAccuracy), disableSpeedup(settings.getb("disableSpeedup", false))
{
    BezierPoint p0 = path.points[0];

    p0.cx = p0.x + (p0.x - p0.cx);
    p0.cy = p0.y + (p0.y - p0.cy);

    if (pathLooped)
        path.points.push_back(p0);

    pts = path.GenerateUniform(0.05);

    ptsLen = pts.size() -1;
    currPos = 0.5;
    Move(0.0);

    double mouseSens = atof(settings.get("mouseSensivity", "5").c_str());
    if (mouseSens < 1)
        mouseSens = 1;

    mouseDiv = (mouseSens / 10.0) / 500;

    keyVelocity = 0.0;
    keySensivity = atof(settings.get("keyboardSensivity", "5").c_str());
    leftPressed = rightPressed = false;
    buttPressed = false;
}

Player::~Player()
{
    glDeleteTextures(1, &tex);
}

void Player::UpdateRotation()
{
    XY pt1 = pts[currPt - 1];
    XY pt2 = pts[currPt + 1];

    double x;
    double y;

    if (!invert)
    {
        x = pt2.x - pt1.x;
        y = (pt2.y - pt1.y);
    }
    else
    {
        x = pt1.x - pt2.x;
        y = (pt1.y - pt2.y);
    }

    rot = atan2(y, x) * (180.0 / PI);
}

void Player::Move(double t)
{
    currPos += t;

    if (pathLooped)
    {
        while ( currPos < 0.0)
            currPos+=1.0;

        while (currPos > 1.0)
            currPos-=1.0;
    }
    else
    {
        if (currPos < 0.0)
            currPos = 0.0;

        if (currPos > 1.0)
            currPos = 1.0;
    }

    currPt = (int)((ptsLen - 2) * currPos);

    if (currPt < 1)
        currPt = 1;

    if (currPt > ptsLen - 2)
        currPt = ptsLen - 2;

    UpdateRotation();
}

void Player::Logic(Scenes::FrameEvents events)
{
    int p = 0;
    while (ballPaths[p] != NULL)
    {
        if (ballPaths[p]->state.accuracyShotTriggered)
        {
            accuracyShot = true;
            accuracyShotTime = accuracyShotTimeout;
            ballPaths[p]->state.accuracyShotTriggered = false;
        }

        p++;
    }

    if (accuracyShot)
    {
        accuracyShotTime--;
        if (accuracyShotTime < 0)
            accuracyShot = false;
    }

    if (shooting)
    {
        int p = 0;
        double ldist = 100;
        ShotAddr shotBall(-1);
        int shotPath = -1;
        ShotAddr nBall(-1);

        while (ballPaths[p] != NULL)
        {
            nBall = ballPaths[p]->PickShot(shootPoint.x, shootPoint.y);

            if (nBall.pos != -1)
            {
                double bx = ballPaths[p]->ballPath[ballPaths[p]->balls[nBall.pos].upos()].x;
                double by = ballPaths[p]->ballPath[ballPaths[p]->balls[nBall.pos].upos()].y;

                double dist = sqrt(pow(fabs(bx - shootPoint.x), 2.0) + pow(fabs(by - shootPoint.y), 2.0));
                if (dist < ldist)
                {
                    shotBall = nBall;
                    shotPath = p;
                    ldist = dist;
                }
            }
            ++p;
        }

        // is this the first ball ?
        if (shotBall.pos == 0)
        {
            // do we have space for a ball before that one ?
            if (ballPaths[shotPath]->balls[shotBall.pos].pos >= BallPath::stepsPerBall)
            {
                // if yes.... was the shot on the right side ?
                double bx = ballPaths[shotPath]->ballPath[ballPaths[shotPath]->balls[shotBall.pos].upos()].x;
                double by = ballPaths[shotPath]->ballPath[ballPaths[shotPath]->balls[shotBall.pos].upos()].y;

                double distBall = sqrt(pow(fabs(bx - shootPoint.x), 2.0) + pow(fabs(by - shootPoint.y), 2.0));

                bx = ballPaths[shotPath]->ballPath[ballPaths[shotPath]->balls[shotBall.pos].upos() - BallPath::stepsPerBall].x;
                by = ballPaths[shotPath]->ballPath[ballPaths[shotPath]->balls[shotBall.pos].upos() - BallPath::stepsPerBall].y;

                double distFirst = sqrt(pow(fabs(bx - shootPoint.x), 2.0) + pow(fabs(by - shootPoint.y), 2.0));

                if (distFirst < distBall)
                {
                    ballPaths[shotPath]->InsertBall(ShotAddr(-1), shootCol, shootBonus);
                    shotBall.pos= -1;
                    shooting = false;
                }
            }
        }


        if (shotBall.pos != -1)
        {
            ballPaths[shotPath]->InsertBall(shotBall, shootCol, shootBonus);
            shooting = false;
        }

        if (shooting)
            if (shootPoint.x > 120 || shootPoint.x < -20 || shootPoint.y > 120 || shootPoint.y < -20)
            {
                mix->EnqueueSample(sfx_ouch, sfxVol);
                score -= 25;
                shooting = false;
                picking = true;
                pickT = 1.0;
            }

        shootPoint.x = shootPoint.x + shootVel.x * shootingSpeed;
        shootPoint.y = shootPoint.y + shootVel.y * shootingSpeed;
    }

    if (picking)
    {
        pickT += pickV;
        if (pickT > 1.0)
            pickT = 1.0;
    }

    // fix keyboard
    bool movedByKeyboard = false;
    bool buttReleased = false;
    bool enterMouseEvents = false;
    int speedUp = 0;

    if (!events.empty)
    {
        if (events.keyDown.size() > 0)
            for (vector<SDLKey>::iterator i = events.keyDown.begin(); i != events.keyDown.end(); ++i)
            {
                if (*i == SDLK_LEFT)
                    leftPressed = true;
                if (*i == SDLK_RIGHT)
                    rightPressed = true;
                if (*i == SDLK_SPACE || *i == SDLK_RETURN || *i == SDLK_DOWN)
                {
                    buttPressed = true;
                    enterMouseEvents = true;
                }
                if (*i == SDLK_UP)
                {
                    speedUp = 1;
                }
            }

        if (events.keyUp.size() > 0)
            for (vector<SDLKey>::iterator i = events.keyUp.begin(); i != events.keyUp.end(); ++i)
            {
                if (*i == SDLK_LEFT)
                    leftPressed = false;
                if (*i == SDLK_RIGHT)
                    rightPressed = false;

                if (*i == SDLK_SPACE || *i == SDLK_RETURN || *i == SDLK_DOWN)
                {
                    buttPressed = false;
                    buttReleased = true;
                    enterMouseEvents = true;
                }

                if (*i == SDLK_UP)
                {
                    speedUp = -1;
                }
            }
    }

    if (leftPressed)
        keyVelocity -= 0.5 * keySensivity;

    if (rightPressed)
        keyVelocity += 0.5 * keySensivity;

    if (keyVelocity != 0.0)
    {
        movedByKeyboard = true;
        if (!invert)
        {
            Move(keyVelocity * 0.001);
        }
        else Move(keyVelocity * -0.001);
    }

    keyVelocity = keyVelocity * 0.8;

    if (abs(keyVelocity) < 0.01)
        keyVelocity = 0.0;

    if (!buttPressed && !movedByKeyboard && !buttReleased)
        enterMouseEvents = true;

    if (enterMouseEvents)
        if (!events.empty)
        {
            if (!invert)
            {

                Move(double(events.relmouseX) * mouseDiv);
            }
            else
                Move(double(-1 * events.relmouseX) * mouseDiv);

            if ((events.buttDown[0] || buttPressed) && picking && !shooting)
            {
                shootCol = pickedCol;
                shootTex = pickedTex;
                shootBonus = pickedBonus;
                shootBonusTex = pickedBonusTex;

                shootVel.x = -sin(rot / (180.0 / PI));
                shootVel.y = cos(rot / (180.0 / PI));

                shootPoint.x = pts[currPt].x + holdingDist * shootVel.x;
                shootPoint.y = pts[currPt].y + holdingDist * shootVel.y;

                shooting = true;
                picking = false;
                reshooting = true;

                mix->EnqueueSample(sfx_push, sfxVol);
            }

            if (events.buttDown[1])
            {
                speedUp = 1;
            }

            if (events.buttUp[1])
            {
                speedUp = -1;
            }


            if ((events.buttDown[0] || buttPressed) && !shooting && !picking)
            { // we're shoootin :)
                int p = 0;
                XY pt1 = pts[currPt - 1];
                XY pt2 = pts[currPt + 1];

                double vx = -sin(rot / (180.0 / PI));
                double vy = cos(rot / (180.0 / PI));

                double x = pts[currPt].x;
                double y = pts[currPt].y;

                int nBall = -1;
                pickedPath = -1;
                double ldist = 200.0;
                pickedBall = -1;

                while (ballPaths[p] != NULL)
                {
                    nBall = ballPaths[p]->Pick(x, y, vx, vy);

                    if (nBall != -1)
                    {
                        double bx = ballPaths[p]->ballPath[ballPaths[p]->balls[nBall].upos()].x;
                        double by = ballPaths[p]->ballPath[ballPaths[p]->balls[nBall].upos()].y;
                        double dist = sqrt(pow(fabs(bx - x), 2.0) + pow(fabs(by - y), 2.0));

                        if (dist < ldist)
                        {
                            pickedBall = nBall;
                            pickedPath = p;
                            ldist = dist;
                        }
                    }

                    ++p;
                }

                if (pickedBall != -1)
                {
                    pickedCol = ballPaths[pickedPath]->balls[pickedBall].col;
                    pickedBonus = ballPaths[pickedPath]->balls[pickedBall].bonus;
                    pickedTex = ballPaths[pickedPath]->tex[pickedCol];
                    pickedBonusTex = ballPaths[pickedPath]->tex[NBALLCOLORS + (pickedBonus) - 1];
                    pickPoint.x = ballPaths[pickedPath]->ballPath[ballPaths[pickedPath]->balls[pickedBall].upos()].x;
                    pickPoint.y = ballPaths[pickedPath]->ballPath[ballPaths[pickedPath]->balls[pickedBall].upos()].y;

                    picking = true;
                    reshooting = false;
                    pickT = 0.0;
                    pickV = 1.0 / (double)pickSpeed;

                    deque<Ball>::iterator i;
                    i = ballPaths[pickedPath]->balls.begin();
                    i += pickedBall;

                    ballPaths[pickedPath]->balls.erase(i);
                    ballPaths[pickedPath]->state.comboCnt = 0;

                    mix->EnqueueSample(sfx_pull, sfxVol);
                }
            }

            if (events.buttUp[0] || buttReleased)
            {
                if (picking && !shooting && !reshooting)
                {
                    shootCol = pickedCol;
                    shootTex = pickedTex;
                    shootBonus = pickedBonus;
                    shootBonusTex = pickedBonusTex;

                    shootVel.x = -sin(rot / (180.0 / PI));
                    shootVel.y = cos(rot / (180.0 / PI));

                    shootPoint.x = pts[currPt].x + holdingDist * shootVel.x;
                    shootPoint.y = pts[currPt].y + holdingDist * shootVel.y;

                    shooting = true;
                    picking = false;
                    reshooting = false;
                    mix->EnqueueSample(sfx_push, sfxVol);
                }
            }
        }

    if (!disableSpeedup)
        if (speedUp != 0)
        {
            bool s = speedUp==1?true:false;

            int p = 0;
            while (ballPaths[p] != NULL)
            {
                ballPaths[p]->state.speedUp = s;
                ++p;
            }
        }
}

void Player::Render()
{
    double ts = ((BallPath::ballOneTextureSize * 10.0)/ double(BallPath::ballTextureSize)) * 0.1; // size of one ball in texture coordinates

    glLoadIdentity();

    glEnable(GL_TEXTURE_2D);
    glTranslated(pts[currPt].x, pts[currPt].y, 40);
    glBindTexture(GL_TEXTURE_2D, tex);
    glScaled(10, 10, 10);
    glRotated(rot, 0, 0, 1.0);
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
    glDisable(GL_TEXTURE_2D);

    if (shooting)
    {
        glLoadIdentity();
        glEnable(GL_TEXTURE_2D);
        glTranslated(shootPoint.x, shootPoint.y, 40);
        glBindTexture(GL_TEXTURE_2D, shootTex);
        glScaled(5, 5, 5);
        glBegin(GL_QUADS);
        glTexCoord2d(0.0, 0.0);
        glVertex3d(-0.5, 0.5, 0);
        glTexCoord2d(0.0, ts);
        glVertex3d(-0.5, -0.5, 0);
        glTexCoord2d(ts, ts);
        glVertex3d(0.5, -0.5, 0);
        glTexCoord2d(ts, 0.0);
        glVertex3d(0.5, 0.5, 0);
        glEnd();

        if (pickedBonus)
        {
            glLoadIdentity();
            glTranslated(shootPoint.x, shootPoint.y, 45);
            glBindTexture(GL_TEXTURE_2D, shootBonusTex);
            glScaled(5, 5, 5);
            glBegin(GL_QUADS);
            glTexCoord2d(0.0, 0.0);
            glVertex3d(-0.5, 0.5, 0);
            glTexCoord2d(0.0, 1);
            glVertex3d(-0.5, -0.5, 0);
            glTexCoord2d(1, 1);
            glVertex3d(0.5, -0.5, 0);
            glTexCoord2d(1, 0.0);
            glVertex3d(0.5, 0.5, 0);
            glEnd();
        }

        glDisable(GL_TEXTURE_2D);
    }

    if (picking)
    {
        double bx = pts[currPt].x + holdingDist * -sin(rot / (180.0 / PI));
        double by = pts[currPt].y + holdingDist * cos(rot / (180.0 / PI));
        bx = bx + (pickPoint.x - bx) * (1.0 - pickT);
        by = by + (pickPoint.y - by) * (1.0 - pickT);

        double bps = (double)BALLSIZE / 1.5;
        double bs = ballPaths[pickedPath]->ballSize - ((ballPaths[pickedPath]->ballSize - bps) * pickT);

        glLoadIdentity();
        glEnable(GL_TEXTURE_2D);
        glTranslated(bx, by, 40);
        glRotated(rot, 0, 0, 1.0);
        glBindTexture(GL_TEXTURE_2D, pickedTex);
        glScaled(bs, bs, bs);
        glBegin(GL_QUADS);
        glTexCoord2d(0.0, 0.0);
        glVertex3d(-0.5, 0.5, 0);
        glTexCoord2d(0.0, ts);
        glVertex3d(-0.5, -0.5, 0);
        glTexCoord2d(ts, ts);
        glVertex3d(0.5, -0.5, 0);
        glTexCoord2d(ts, 0.0);
        glVertex3d(0.5, 0.5, 0);
        glEnd();

        if (pickedBonus)
        {
            glLoadIdentity();
            glTranslated(bx, by, 45);
            glRotated(rot, 0, 0, 1.0);
            glBindTexture(GL_TEXTURE_2D, pickedBonusTex);
            glScaled(bs, bs, bs);
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

        glDisable(GL_TEXTURE_2D);
    }

    if (accuracyShot || alwaysAccuracy)
    {
        glLoadIdentity();
        glLineWidth(3.0);
        glTranslated(pts[currPt].x, pts[currPt].y, 30);
        glRotated(rot, 0, 0, 1.0);
        glTranslatef(0, 5, 0);
        glBegin(GL_TRIANGLES);

        if (!alwaysAccuracy)
        {
            glColor4d(1, 0, 0, 0.5 * double(accuracyShotTime) / double(accuracyShotTimeout));
        }
        else
        {
            glColor4d(1, 0, 0, 0.5);
        }

        glVertex3d(-1,0,0);
        glVertex3d(1,0,0);
        glVertex3d(0, 100, 0);

        glColor4d(0, 0, 0, 0);
        glEnd();
    }

    if (drawPath)
    {
        glLoadIdentity();
        glPointSize(2.5);
        glColor3f(0.0, 0.0, 0.0);
        glBegin(GL_POINTS);

        for (std::vector<XY>::iterator i = pts.begin(); i != pts.end(); ++i)
            glVertex3d(i->x, i->y, 0.0);

        glEnd();
    }
}

