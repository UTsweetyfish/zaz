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

#include "game.h"

#ifdef WIN32
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#endif
#include <sstream>
#include <ctime>

Sample *sfx_pull;
Sample *sfx_push;
Sample *sfx_eliminate;
Sample *sfx_ouch;
Sample *sfx_combo;
Sample *sfx_roll;
Sample *sfx_bonus;
Sample *sfx_extraball;
Sample *sfx_extralife;
Sample *sfx_timebonus;
Sample *sfx_newcolor;
Sample *sfx_failedlevel;

BallDesc BallDescriptions[] =
{
    {"balls1.png", 255, 0, 0, 1},
    {"balls1.png", 255, 255, 255, 0},
    {"balls1.png", 0, 170, 0, 2},
    {"balls1.png", 0, 0, 255, 3},
    {"balls1.png", 255, 0, 255, 4},
    {"balls1.png", 255, 255, 0, 5},
    {"balls1.png", 30, 30, 30, 0},
    {"balls1.png", 189, 108, 28, 6},
    {"balls2.png", 255, 0, 0, 1},
    {"balls2.png", 255, 255, 255, 0},
    {"balls2.png", 0, 170, 0, 2},
    {"balls2.png", 0, 0, 255, 3},
    {"balls2.png", 255, 0, 255, 4},
    {"balls2.png", 255, 255, 0, 5},
    {"balls2.png", 30, 30, 30, 0},
    {"balls2.png", 189, 108, 28, 6}
};

Mixer *mix;
int sfxVol;
int musicVol;

Game::Game(SDL_Surface *surf, Level &level, GLuint *textures, uint randomSeed, uint lives, int score, bool fromEditor, bool alwaysAccuracy, bool survivalMode)
        : Scene(surf), surface(surf),
        dying(false), randomSeed(randomSeed), randomSeedSet(false), finishCountdown(false), level(level),
        editor(fromEditor), ownTextures(false), lives(lives), displayExtraLife(false), extraLifeLastScore(0),
        stopTimer(false), timeBonus(false), timeBonusSecondsAdded(0), renderingForThumb(false), survival(survivalMode), dyingSound(false), score(score), gameOver(false), escaped(false)
{
    if (textures == NULL)
    {
        bool useColourHints = settings.getb("colourHints", false);

        for (int b = 0; b < 16; b++)
        {
            BallDesc bd = BallDescriptions[b];
            ballText[b] = LoadBallsTexture(bd.fileName, bd.r, bd.g, bd.b, useColourHints?bd.overlay:0);
        }

        ballText[16] = LoadTexture("bonus1.png");
        ballText[17] = LoadTexture("bonus2.png");
        ballText[18] = LoadTexture("bonus3.png");
        ballText[19] = LoadTexture("bonus4.png");
        ballText[20] = LoadTexture("bonus5.png");
        ballText[21] = LoadTexture("explosion.png");
        ownTextures = true;
    }
    else
    {
        for (int f = 0; f < 22; f++)
            ballText[f] = textures[f];
    }

    // load the sounds
    mix = NULL;

    srand(randomSeed);

    if (!editor)
    {
        try
        {
            try
            {
                music = (Sample *)new Scenes::StreamingOggSample(level.musicFilename);
            }
            catch (Error e)
            {
                music = (Sample *)new Scenes::StreamingOggSample(getRandomMusic());
            }
        }
        catch (Error e)
        {
            music = 0;
        }
    }

    mbuff = 0;

    sfx_pull = (Sample *)new Scenes::OggSample("pull.ogg");
    sfx_push = (Sample *)new Scenes::OggSample("push.ogg");
    sfx_eliminate = (Sample *)new Scenes::OggSample("eliminate.ogg");
    sfx_ouch = (Sample *)new Scenes::OggSample("ouch.ogg");
    sfx_combo = (Sample *)new Scenes::OggSample("combo.ogg");
    sfx_roll = (Sample *)new Scenes::OggSample("roll.ogg");
    sfx_bonus = (Sample *)new Scenes::OggSample("bonus.ogg");
    sfx_extraball = (Sample *)new Scenes::OggSample("extraball.ogg");
    sfx_extralife = (Sample *)new Scenes::OggSample("extralife.ogg");
    sfx_timebonus = (Sample *)new Scenes::WaveSample("timebonus.wav");
    sfx_newcolor = (Sample *)new Scenes::OggSample("newcolor.ogg");
    sfx_failedlevel = (Sample *)new Scenes::OggSample("failedlevel.ogg");

    // create paths
    nPaths = level.paths.size() - 1;

    ballPaths = new BallPath*[nPaths + 1];
    for (int b = 0; b < nPaths; ++b)
    {
        Bezier nb = level.paths[b + 1];

        if (level.mirrorX)
        {
            for (uint p = 0; p < nb.points.size(); p++)
            {
                nb.points[p].x = 50.0 + (50.0 - nb.points[p].x);
                nb.points[p].cx = 50.0 + (50.0 - nb.points[p].cx);
            }
        }

        if (level.mirrorY)
        {
            for (uint p = 0; p < nb.points.size(); p++)
            {
                nb.points[p].y = 50.0 + (50.0 - nb.points[p].y);
                nb.points[p].cy = 50.0 + (50.0 - nb.points[p].cy);
            }
        }

        ballPaths[b] = new BallPath(nb, ballText, &mix, false, level.ballSizes[b]);
        ballPaths[b]->state.ballsToDraw = level.ballsToDraw[b];
        ballPaths[b]->state.colors = level.colors;
        ballPaths[b]->state.lastColors = level.colors;
        ballPaths[b]->state.feedRate = level.startFeedRates[b];
        ballPaths[b]->state.startFeedRate = level.startFeedRates[b];
        ballPaths[b]->state.ballsFromStart = level.ballsFromStart[b];
        ballPaths[b]->state.kidsMode = level.kidsMode;
        ballPaths[b]->state.bonusFrequency = level.bonusFrequency;

        if (survival)
        {
            ballPaths[b]->state.colors = survivalColorsStart;
            ballPaths[b]->state.lastColors = survivalColorsStart;
            ballPaths[b]->state.survival = survival;
        }
    }

    ballPaths[nPaths] = 0;

    // create the player
    Bezier nb = level.paths[0];

    if (level.mirrorX)
    {
        for (uint p = 0; p < nb.points.size(); p++)
        {
            nb.points[p].x = 50.0 + (50.0 - nb.points[p].x);
            nb.points[p].cx = 50.0 + (50.0 - nb.points[p].cx);
        }
    }

    if (level.mirrorY)
    {
        for (uint p = 0; p < nb.points.size(); p++)
        {
            nb.points[p].y = 50.0 + (50.0 - nb.points[p].y);
            nb.points[p].cy = 50.0 + (50.0 - nb.points[p].cy);
        }
    }

    bool inv = level.invert;

    if (level.mirrorX && !level.mirrorY)
    {
        inv = !inv;
    }

    if (level.mirrorY && !level.mirrorX)
    {
        inv = !inv;
    }

    pl = new Player(nb, ballPaths, level.loop, inv, fromEditor, level.kidsMode || alwaysAccuracy);
    dying = false;

    sfxVol = atoi(settings.get("sfxVolume", "50").c_str());
    musicVol = atoi(settings.get("musicVolume", "50").c_str());

    displayLevelName = true;
    levelNameDisplayTime = levelNameDisplayTimeout;

    extraLifeLastScore = (score/pointsForNewLife) * pointsForNewLife;

    level.LoadTex();

    // create an empty cursor

    Uint8 cursdata[] = {0, 0, 0, 0, 0, 0, 0, 0};
}

const string Game::getRandomMusic()
{
    char phname[1024];
    int maxM = 0;

    phname[0] = 0;

    for (int f = 1; f < 100; f++)
    {
        sprintf(phname, "mus%d.ogg", f);

        ifstream inph(settings.getCFilename (phname));
        if (inph)
            maxM=f;
    }

    if (maxM > 0)
    {
        sprintf(phname, "mus%d.ogg", (rand()%maxM) + 1);
    }
    else
    {
        ERR("No music files installed");
    }
    return string(phname);
}

Game::~Game()
{
    CleanupSounds();

    for (int b = 0; b < nPaths; ++b)
    {
        delete ballPaths[b];
    }

    delete[] ballPaths;
    if (ownTextures)
        glDeleteTextures(14, ballText);

    delete pl;

    level.FreeTex();
}

void Game::Logic(ulong frame)
{
    if (!stopTimer)
        levelSeconds = frame / desiredFPS;

    if (!randomSeedSet)
    {
        srand(randomSeed);
        randomSeedSet = true;
    }

    if (mix == NULL)
        mix = mixer;

    if (!editor)
        if (music)
            if (mbuff == 0)
            {
                mbuff= mixer->EnqueueSample (music, musicVol, 0, true);
            }

    if (!events.empty)
    {
        if (events.keyDown.size() > 0)

            for (vector<SDLKey>::iterator i = events.keyDown.begin(); i != events.keyDown.end(); ++i)
            {
                if (*i == SDLK_ESCAPE)
                {
                    quit = true;
                    escaped = true;
                }

                /*                if (*i == SDLK_F1)
                                {
                                    ballPaths[0]->state.ballOut = true;

                                    return;
                                }

                                if (*i == SDLK_F2)
                                {
                                    gameOver = false;
                                    quit = true;
                                    return;
                                }
                */
                if (*i == SDLK_F12)
                {
                    time_t theTime;
                    time( &theTime );
                    tm *t = localtime( &theTime );
                    char tempbuff[512];
                    char datebuff[256];

                    string exportpath = ".";

#ifdef WIN32
                    wchar_t path[MAX_PATH];
                    if (SUCCEEDED(SHGetFolderPathW(NULL,
                                                   CSIDL_PERSONAL,
                                                   NULL,
                                                   0,
                                                   path)))
                    {
                        char utfpath[MAX_PATH];
                        WideCharToMultiByte(CP_UTF8, 0, path, -1, utfpath, MAX_PATH, NULL, NULL);

                        exportpath = string(utfpath);
                    }
#endif

                    sprintf(datebuff, "%04d-%02d-%02d", 1900 + t->tm_year, t->tm_mon + 1, t->tm_mday);

                    sprintf(tempbuff, "%s%szaz-%s.bmp", exportpath.c_str(), SEPARATOR, datebuff);
                    bool ok = false;
                    int f = 1;
                    while (!ok)
                    {
#ifdef WIN32
                        ifstream inph(Settings::W32_GetFileName(tempbuff).c_str());
#else
                        ifstream inph(tempbuff);
#endif
                        if (!inph)
                        {
                            ok = true;
                        }
                        else
                        {
                            sprintf(tempbuff, "%s%szaz-%s-%d.bmp", exportpath.c_str(), SEPARATOR, datebuff, f);
                            ++f;
                        }
                    }

                    string exportphname = tempbuff;
//                    cout << exportphname << endl;
                    Screenshot(exportphname);
                    if (editor)
                    {
                        renderingForThumb = true;
                        pl->alwaysAccuracy = false;
                        pl->drawPath = false;
                    }
                }
            }
    }

    pl->Logic(events);

    if (editor && !renderingForThumb)
    {
        for (int p = 0; p < nPaths; ++p)
        {
            ballPaths[p]->state.ballsToDraw = -1;
            ballPaths[p]->state.ballOut = false;

            int nb = ballPaths[p]->balls.size();

            if (nb > 0)
            {
                if (ballPaths[p]->balls[nb - 1].pos < (double)ballPaths[p]->pthLen * 0.9)
                {
                    ballPaths[p]->state.feedRate = 50;
                }
                else
                {
                    ballPaths[p]->state.feedRate = 2;
                }
            }

            ballPaths[p]->Logic();
        }
        return;
    }

    for (int b = 0; b < nPaths; ++b)
    {
        ballPaths[b]->Logic();
    }

    // adjust the feedrate
    if (!survival)
    {
        for (int p = 0; p < nPaths; ++p)
        {
            if (ballPaths[p]->state.ballsToDraw != -1)
            {
                double fs = level.startFeedRates[p];
                double fe = level.endFeedRates[p];

                double bd = ballPaths[p]->state.ballsToDraw - ballPaths[p]->state.ballsFromStart;
                double b2d = level.ballsToDraw[p] - level.ballsFromStart[p];

                if (bd < 0)
                    bd = 0.0;

                bd = b2d - bd;

                ballPaths[p]->state.feedRate = fs + (bd / b2d) * (fe - fs);
            }
        }
    }
    else
    {
        for (int b = 0; b < nPaths; ++b)
        {
            ballPaths[b]->state.ballsToDraw = 1;
            ballPaths[b]->state.colors = survivalColorsStart + (levelSeconds / survivalNewColorTimeout);
            if (ballPaths[b]->state.colors >= NBALLCOLORS)
                ballPaths[b]->state.colors = NBALLCOLORS;

            ballPaths[b]->state.feedRate = ballPaths[b]->state.startFeedRate + (ballPaths[b]->state.colors - survivalColorsStart);

            ballPaths[b]->state.feedRate = ballPaths[b]->state.feedRate + ((double(levelSeconds) - ((ballPaths[b]->state.colors - survivalColorsStart) * survivalNewColorTimeout * 1.4)) / double(survivalNewColorTimeout) * 1.8);
        }
    }

    // fix new color sfx
    if (ballPaths[0]->state.lastColors != ballPaths[0]->state.colors)
    {
        mixer->EnqueueSample (sfx_newcolor, sfxVol, 0, false);
        ballPaths[0]->state.lastColors = ballPaths[0]->state.colors;
    }

    // load the score
    for (int p = 0; p < nPaths; ++p)
    {
        score+= ballPaths[p]->state.score;
        ballPaths[p]->state.score = 0;
    }

    score += pl->score;
    pl->score = 0;

    if (score < 0)
        score = 0;

    // fix extra life
    if (!survival)
        if ((score > extraLifeLastScore) && ((score - extraLifeLastScore) > (int)pointsForNewLife))
        {
            extraLifeLastScore = (score/pointsForNewLife) * pointsForNewLife;
            lives++;

            extraLifeDisplayTime = extraLifeDisplayTimeout;
            displayExtraLife = true;

            mixer->EnqueueSample (sfx_extralife, sfxVol, 0, false);
        }

    if (displayExtraLife)
    {
        extraLifeDisplayTime--;

        if (extraLifeDisplayTime <= 0)
            displayExtraLife = false;
    }

    // do we need extra balls to finish this level ?
    bool finishedDrawing = true;
    for (int b = 0; b < nPaths; ++b)
    {
        if (ballPaths[b]->state.ballsToDraw != 0)
            finishedDrawing = false;

        if (!ballPaths[b]->reversedBalls.empty())
            finishedDrawing = false;
    }

    if (pl->shooting || pl->picking)
        finishedDrawing = false;

    if (!dying)
        if (finishedDrawing)
        {
            int cols[NBALLCOLORS];

            for (int f = 0; f < NBALLCOLORS; f++)
                cols[f] = 0;

            for (int p = 0; p < nPaths; ++p)
            {
                for (uint b = 0; b < ballPaths[p]->balls.size(); ++b)
                {
                    if (!ballPaths[p]->balls[b].elim)
                        cols[ballPaths[p]->balls[b].col]++;
                }

                if (ballPaths[p]->state.extraBall != -1)
                    cols[ballPaths[p]->state.extraBall]++;
            }

            int extraCol = -1;
            for (int f = 0; f < NBALLCOLORS; f++)
            {
                if ((cols[f] > 0) && (cols[f] < 3))
                    extraCol = f;
            }

            if (extraCol != -1)
            {
                int p = rand() % nPaths;
                if (!ballPaths[p]->balls.empty())
                    if (ballPaths[p]->reversedBalls.empty())
                        if (ballPaths[p]->state.extraBall == -1)
                            ballPaths[p]->state.extraBall = extraCol;
            }
        }

    // are we dying ?
    bool dying = false;
    for (int p = 0; p < nPaths; ++p)
    {
        if (ballPaths[p]->state.ballOut)
            dying = true;
    }

    dying = dying && !editor;

    if (dying)
    {
        for (int p = 0; p < nPaths; ++p)
        {
            ballPaths[p]->state.feedRate = 50;
            ballPaths[p]->state.bonusPause = false;
            ballPaths[p]->state.bonusSlow = false;
        }

        if (!dyingSound)
        {
            mixer->EnqueueSample (sfx_failedlevel, sfxVol, 0, false);
            dyingSound = true;
        }
    }

    bool dead = dying;
    for (int p = 0; p < nPaths; ++p)
    {
        if (ballPaths[p]->balls.size() != 0)
            dead = false;
    }

    if (dead)
    {
        gameOver = true;
        quit = true;
    }

    // did we finish the level ?
    bool finished = !dying;
    if (!dying)
    {
        for (int p = 0; p < nPaths; ++p)
        {
            if (!ballPaths[p]->balls.empty())
                finished = false;

            if (!ballPaths[p]->reversedBalls.empty())
                finished = false;

            if (ballPaths[p]->state.ballsToDraw != 0)
                finished = false;
        }
    }

    if (finished)
    {
        stopTimer = true;

        if (!finishCountdown)
        {
            finishedTimer = waitAfterFinished;
            finishCountdown = true;
        }

        if (levelSeconds + timeBonusSecondsAdded < level.time)
        {
            timeBonus = true;
//            levelSeconds++;
            timeBonusSecondsAdded++;
            score+=timeBonusScorePerSecond;
            if (timeBonusSecondsAdded%4 == 0)
                mixer->EnqueueSample (sfx_timebonus, sfxVol, 0, false);
        }

        if (timeBonusSecondsAdded + levelSeconds >= level.time)
            finishedTimer--;

        if (finishedTimer < 0)
        {
            gameOver = false;
            quit = true;
        }
    }

    // fix level display
    if (displayLevelName)
    {
        levelNameDisplayTime--;
        if (levelNameDisplayTime <= 0)
            displayLevelName = false;
    }

    /*SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);
    SDL_WarpMouse(surface->w / 2, surface->h / 2);
    SDL_EventState(SDL_MOUSEMOTION, SDL_ENABLE);*/
}

void Game::Render(ulong frame)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(1.0, 1.0, 1.0);

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity( );

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    // render level background
    XY tl(0,0);
    XY tr(1,0);
    XY bl(0,1);
    XY br(1,1);

    if (level.mirrorX)
    {
        XY t = tl;
        tl = tr;
        tr = t;

        t = bl;
        bl = br;
        br = t;
    }

    if (level.mirrorY)
    {
        XY t = tl;
        tl = bl;
        bl = t;

        t = br;
        br = tr;
        tr = t;
    }

    glPushMatrix();
    glTranslatef(0.0, 0.0, -5);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, level.backgroundTex);
    glBegin(GL_QUADS);
    glTexCoord2d(tl.x, tl.y);
    glVertex3d(vleft, 100, 0);
    glTexCoord2d(bl.x, bl.y);
    glVertex3d(vleft, 0, 0);
    glTexCoord2d(br.x, br.y);
    glVertex3d(vleft + vwidth, 0, 0);
    glTexCoord2d(tr.x, tr.y);
    glVertex3d(vleft + vwidth, 100, 0);

    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

    for (int b = 0; b < nPaths; ++b)
    {
        ballPaths[b]->Render(true, false, true, false);
    }

    glLoadIdentity();
    // render overlay
    if (level.overlayTex)
    {
        glPushMatrix();
        glTranslatef(0.0, 0.0, 1.5);
        glEnable(GL_TEXTURE_2D);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glBindTexture(GL_TEXTURE_2D, level.overlayTex);
        glColor4f(1.0, 1.0, 1.0, 1.0);
        glBegin(GL_QUADS);

        glTexCoord2d(tl.x, tl.y);
        glVertex3d(vleft, 100, 0);
        glTexCoord2d(bl.x, bl.y);
        glVertex3d(vleft, 0, 0);
        glTexCoord2d(br.x, br.y);
        glVertex3d(vleft + vwidth, 0, 0);
        glTexCoord2d(tr.x, tr.y);
        glVertex3d(vleft + vwidth, 100, 0);
        glEnd();
        glDisable(GL_TEXTURE_2D);
        glPopMatrix();
    }

    glLoadIdentity();
    for (int b = 0; b < nPaths; ++b)
    {
        ballPaths[b]->Render(true, false, false, true);
    }

    glLoadIdentity();
    for (int b = 0; b < nPaths; ++b)
    {
        ballPaths[b]->Render(false, true);
    }

    pl->Render();

    // render score
    glLoadIdentity( );
    if (!editor && !renderingForThumb)
    {
        glColor3f(1.0, 1.0, 1.0);
        glPushMatrix();
        glTranslatef(95, 95, 50);
        glScaled(0.2, 0.2, 0.2);

        char scoretxt[256];
        sprintf(scoretxt, "%06d", score);

        font3->Render(scoretxt);
        glPopMatrix();
    }

    if (editor && !renderingForThumb)
    {
        for (int b = 0; b < nPaths; ++b)
        {
            glPushMatrix();
            glPointSize(2.5);
            glColor3f(0.0, 0.0, 0.0);
            glBegin(GL_POINTS);

            for (std::vector<PathStep>::iterator i = ballPaths[b]->ballPath.begin(); i != ballPaths[b]->ballPath.end(); ++i)
                glVertex3d(i->x, i->y, 0.0);

            glEnd();
            glPopMatrix();
        }
    }

    // render lives
    if (!survival && !editor && !renderingForThumb)
        for (uint l = 0; l < lives; l++)
        {
            glPushMatrix();
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, pl->tex);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            glTranslated(5 + (l * 5) + vleft, 95, 50);
            glScaled(6.0, 6.0, 6.0);
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
            glPopMatrix();
        }

    if (displayExtraLife)
    {
        char extraLifeTxt[256];
        sprintf(extraLifeTxt, "%s", _("Extra Life !"));
        glLoadIdentity( );
        FTBBox b = font->BBox(extraLifeTxt);
        double tw = b.Upper().X() / 5;

        glTranslated((100 - tw) / 2, 50, 50);
        if (timeBonus) // make sure extra life does not blend with time bonus
            glTranslated(0.0, 10.0, 0);

        glScaled(0.2, 0.2, 0.2);

        double alpha = 1.0;
        if (extraLifeDisplayTime < extraLifeDisplayTimeout / 3)
        {
            alpha = (double)extraLifeDisplayTime / ((double)extraLifeDisplayTimeout / 3.0);
        }

        glColor4d(1.0, 0.0, 0.0, alpha);

        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        font->Render(extraLifeTxt);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    };

    // render level name
    if (!editor && !renderingForThumb)
        if (displayLevelName)
        {
            glLoadIdentity( );

            FTBBox b = font->BBox(gettext(level.name.c_str()));
            double tw = b.Upper().X() / 5;

            glTranslated((100 - tw) / 2, 50, 50);
            glScaled(0.2, 0.2, 0.2);

            double alpha = 1.0;
            if (levelNameDisplayTime < levelNameDisplayTimeout / 3)
            {
                alpha = (double)levelNameDisplayTime / ((double)levelNameDisplayTimeout / 3.0);
            }

            glColor4d(1.0, 1.0, 1.0, alpha);

            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            font->Render(gettext(level.name.c_str()));
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        }

    // time
    char tbuff[256];
    if (!survival)
    {
        sprintf(tbuff, "%02d:%02d / %02d:%02d ", levelSeconds/60, levelSeconds%60, level.time/60, level.time%60);
    }
    else
    {
        sprintf(tbuff, "%02d:%02d", levelSeconds/60, levelSeconds%60);
    }

    glLoadIdentity( );

    FTBBox b = font3->BBox(tbuff);
    double tw = b.Upper().X() / 5;
    glTranslated((100 - tw) / 2, 95, 50);
    glScaled(0.2, 0.2, 0.2);
    glColor4d(1.0, 1.0, 1.0, 1.0);

    if (!editor && !renderingForThumb)
        font3->Render(tbuff);

    if (timeBonus) // we're adding time bonus !
    {
        char buff[256];

        sprintf(buff, _("Time Bonus ! (%d sec * %d = %d)"), timeBonusSecondsAdded, timeBonusScorePerSecond,
                timeBonusScorePerSecond * timeBonusSecondsAdded);

        glLoadIdentity( );
        FTBBox b = font->BBox(buff);
        double tw = b.Upper().X() / 5;

        glTranslated((100 - tw) / 2, 50, 50);
        glScaled(0.2, 0.2, 0.2);

        glColor4d(1.0, 0.0, 0.0, 1.0);

        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        font->Render(buff);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    }

    if (renderingForThumb)
    {
        renderingForThumb = false;
        pl->drawPath = false;
    }
}

void Game::GLSetup()
{
    if ((mode == RUN) || (mode == RECORD))
    {
        SDL_WM_GrabInput(SDL_GRAB_ON);
    }

    int width = surface->w;
    int height = surface->h;

    /* Our shading model--Gouraud (smooth). */
    glShadeModel( GL_SMOOTH );

    /* Culling. */
    glCullFace( GL_BACK );
    glFrontFace( GL_CCW );
    glEnable( GL_CULL_FACE );
    glEnable(GL_DEPTH_TEST);
    glEnable( GL_ALPHA_TEST );
    glAlphaFunc(GL_GREATER, 0.0);
    glEnable(GL_LINE_SMOOTH);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

    glClearColor( 0, 0, 0, 0 );
    glViewport( 0, 0, width, height);
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );

    vwidth = 100 * (640.0/480.0);
    vleft = (100 - vwidth) / 2;
    vheight = 100.0;

    glOrtho(vleft, vwidth + vleft, 0, 100, -100, 100);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

void Game::CleanupSounds()
{
    mixer->DisposeSample(sfx_pull);
    mixer->DisposeSample(sfx_push);
    mixer->DisposeSample(sfx_eliminate);
    mixer->DisposeSample(sfx_ouch);
    mixer->DisposeSample(sfx_combo);
    mixer->DisposeSample(sfx_roll);
    mixer->DisposeSample(sfx_bonus);
    mixer->DisposeSample(sfx_extraball);
    mixer->DisposeSample(sfx_extralife);
    mixer->DisposeSample(sfx_timebonus);
    mixer->DisposeSample(sfx_newcolor);
    mixer->DisposeSample(sfx_failedlevel);


    if (!editor)
        if (music)
            mixer->DisposeSample(music);
}
