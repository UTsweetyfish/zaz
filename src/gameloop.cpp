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

#include "gameloop.h"
#include "level.h"
#include "textureloader.h"
#include "lineeditor.h"
#include "profile.h"
#include "hiscores.h"
#ifdef WIN32
#include <shlobj.h>
#include <shlwapi.h>
#endif
#include <sstream>
#include <ctime>

GameLoop::GameLoop(SDL_Surface *surf, GLuint *gameTextures, uint startLevel, uint randomSeed, bool survivalMode)
        : Scene(surf), score(0), lastscore(0), currentLevelName(startLevel), randomSeed((uint)time(0)), gameTextures(gameTextures),
        showMenu(false), gameOver(false), showPauseMenu(false), continueGame(false), showLifeLostMenu(false), showSureQuitMenu(false),
        pwned(false), survival(survivalMode), survivalHiScore(false), timeHiScore(false), adventureHiScore(false), game(0),
        logoTexture(LoadTexture ("logo.png")),screenShotTexture(0), pwnedr(0), editor(profile.getName(), 60, 50, 16),
        livesLeft(nLives), livesLeftInReplay(nLives), startLevel(startLevel)
{
    pauseMenu.Add(new GenericMenuItem(_("End game"), pauseMenuEndGameHandler, this));
    pauseMenu.Add(new GenericMenuItem(_("Continue"), pauseMenuContinueHandler, this));

    pauseMenu.SetDimensions(30, 40, 40);

    nextLevelMenu.Add(new GenericMenuItem(_("Next level"), nextLevelMenuNextLevelHandler, this));
    nextLevelMenu.Add(new GenericMenuItem(_("End game"), pauseMenuEndGameHandler, this));
    nextLevelMenu.Add(new GenericMenuItem(_("View replay"), nextLevelMenuViewReplayHandler, this));
    nextLevelMenu.Add(new GenericMenuItem(_("Export video"), nextLevelMenuExportVideoHandler, this));

    nextLevelMenu.SetDimensions(30, 40, 40);

    gameOverMenu.Add(new GenericMenuItem(_("Back to main menu"), gameOverMenuEndGameHandler, this));
    gameOverMenu.Add(new GenericMenuItem(_("Restart game"), gameOverMenuRestartHandler, this));
    gameOverMenu.Add(new GenericMenuItem(_("View replay"), nextLevelMenuViewReplayHandler, this));
    gameOverMenu.Add(new GenericMenuItem(_("Export video"), nextLevelMenuExportVideoHandler, this));

    gameOverMenu.SetDimensions(30, 40, 40);

    lifeLostMenu.Add(new GenericMenuItem(_("Retry level"), lifeLostMenuRetryLevelHandler, this));
    lifeLostMenu.Add(new GenericMenuItem(_("End game"), lifeLostMenuEndGameHandler, this));
    lifeLostMenu.Add(new GenericMenuItem(_("View replay"), nextLevelMenuViewReplayHandler, this));
    lifeLostMenu.Add(new GenericMenuItem(_("Export video"), nextLevelMenuExportVideoHandler, this));

    lifeLostMenu.SetDimensions(30, 40, 40);

    sureQuitMenu.Add(new GenericMenuItem(_("Yes"), sureQuitMenuYesHandler, this));
    sureQuitMenu.Add(new GenericMenuItem(_("No"), sureQuitMenuNoHandler, this));

    sureQuitMenu.SetDimensions(30, 45, 40);

    pwned = false;
    playedFromStart = !startLevel;
#ifdef WIN32
    tempRecording << " "; // braindead !!!
#endif
}

void GameLoop::ClearGame()
{
    if (game)
    {
        delete game;
        game = 0;
    }
}

void sureQuitMenuNoHandler(void *ptr)
{
    GameLoop *p = (GameLoop*)ptr;
    p->showSureQuitMenu = false;
}

void sureQuitMenuYesHandler(void *ptr)
{
    GameLoop *p = (GameLoop*)ptr;
    if (p->lifeLostSaveHiScoreOnQuit)
        p->SaveAdventureHiScore();

    p->quit = true;
    p->ClearGame();
}

void gameOverMenuEndGameHandler(void *ptr)
{
    GameLoop *p = (GameLoop*)ptr;
    p->quit = true;
    p->ClearGame();
}

void lifeLostMenuEndGameHandler(void *ptr)
{
    GameLoop *p = (GameLoop*)ptr;
    p->lifeLostSaveHiScoreOnQuit = true;
    p->showSureQuitMenu = true;
}

void gameOverMenuRestartHandler(void *ptr)
{
    GameLoop *p = (GameLoop*)ptr;
    p->score = 0;

    p->currentLevelName = p->startLevel;
    //p->currentLevelName = 0;

    p->gameOver = false;
    p->pwned = false;
};

void GameLoop::SaveAdventureHiScore()
{
    hiScores.SubmitHiScore(HiScoreEntry(score,
                                        currentLevelSet.filename,
                                        HS_ADVENTURE,
                                        profile.getName(),
                                        level.name));
}

void GameLoop::ViewReplay()
{
    stringstream rec;
    rec << recording;

    ClearGame();
    level = currentLevelSet.levels[currentLevelName];
    //level.LoadTex();
    game = new Game(surface, level, gameTextures, randomSeed, livesLeftInReplay, lastscore, false, false, survival);
    game->Play(rec);
    ClearGame();
}

void lifeLostMenuRetryLevelHandler(void *ptr)
{
    GameLoop *p = (GameLoop*)ptr;
    p->showLifeLostMenu = false;
}

void GameLoop::NextLevel()
{
    currentLevelName++;
    showMenu = false;
}

void nextLevelMenuNextLevelHandler(void *ptr)
{
    GameLoop *p = (GameLoop*)ptr;
    p->NextLevel();
}

void nextLevelMenuViewReplayHandler(void *ptr)
{
    GameLoop *p = (GameLoop*)ptr;
    p->ViewReplay();
}

void GameLoop::ExportReplay()
{
    ClearGame();

    level = currentLevelSet.levels[currentLevelName];
    //level.LoadTex();
    stringstream rec;
    rec << recording;
    game = new Game(surface, level, gameTextures, randomSeed, livesLeftInReplay, lastscore, false, false, survival);

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

    sprintf(tempbuff, "%s%szaz-%s.ogv", exportpath.c_str(), SEPARATOR, datebuff);
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
            sprintf(tempbuff, "%s%szaz-%s-%d.ogv", exportpath.c_str(), SEPARATOR, datebuff, f);
            ++f;
        }
    }

    string exportphname = tempbuff;

    game->Export(exportphname, rec, 4);
    ClearGame();
}

void nextLevelMenuExportVideoHandler(void *ptr)
{
    GameLoop *p = (GameLoop*)ptr;
    p->ExportReplay();
}

void pauseMenuContinueHandler(void *ptr)
{
    GameLoop *p = (GameLoop*)ptr;
    p->continueGame = true;
    p->showPauseMenu = false;
};

void pauseMenuEndGameHandler(void *ptr)
{
    GameLoop *p = (GameLoop*)ptr;
    p->lifeLostSaveHiScoreOnQuit = false;
    p->showSureQuitMenu = true;
};


GameLoop::~GameLoop()
{
    if (screenShotTexture)
        glDeleteTextures(1, &screenShotTexture);

    glDeleteTextures(1, &logoTexture);

    ClearGame();
}

void GameLoop::GLSetup()
{
    SDL_WM_GrabInput(SDL_GRAB_OFF);

    int width = surface->w;
    int height = surface->h;

    //    float ratio = (float) width / (float) height;

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

    glClearColor( .1f, .1f, .7f, 1.0f );
    glViewport( 0, 0, width, height);
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );

    glOrtho(vleft, vwidth + vleft, 0, 100, -100, 100);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

void GameLoop::Logic(ulong frame)
{
    pointerFrame++;
    if (pointerFrame >= BALLTEXTURECOUNT*BALLTEXTURECOUNT*2)
        pointerFrame = 0;

    pointerRot = pointerRot + 0.1;

    if (!events.empty)
    {
        if (events.keyDown.size() > 0)
            for (vector<SDLKey>::iterator i = events.keyDown.begin(); i != events.keyDown.end(); ++i)
            {
                if (*i == SDLK_ESCAPE)
                {
                    if (!showSureQuitMenu)
                    {
                        if (showMenu)
                        {
                            NextLevel();
                        }

                        if (showPauseMenu)
                        {
                            continueGame = true;
                            showPauseMenu =	false;
                        }
                    }

                    if (gameOver)
                    {
                        quit = true;
                        ClearGame();
                    }

                    if (showSureQuitMenu)
                        showSureQuitMenu = false;

                }
            }

        bool click = false;

        // we have to translate mouse events to local coords for the menu
        FrameEvents tempEvents = events;
        tempEvents.mouseX = mx;
        tempEvents.mouseY = my;

        if (events.buttDown[0])
            click = true;

        if (showSureQuitMenu)
        {
            sureQuitMenu.Logic(tempEvents);
            return;
        }

        if (showLifeLostMenu && !showSureQuitMenu)
            lifeLostMenu.Logic(tempEvents);

        if ((showPauseMenu) && (!showSureQuitMenu))
        {
            pauseMenu.Logic(tempEvents);
        }

        if (showMenu)
            nextLevelMenu.Logic(tempEvents);

        if (gameOver)
            gameOverMenu.Logic(tempEvents);
    }

    if (quit)
    {
        return;
    }

    if (!showLifeLostMenu && !showMenu && !gameOver && !showPauseMenu)
    {
        if (!continueGame)
        {
            ClearGame();
            adventureHiScore = timeHiScore = survivalHiScore = false;

            level = currentLevelSet.levels[currentLevelName];
            randomSeed = (uint)time(0);
            tempRecording.clear();
            tempRecording.seekp(0);
            game = new Game(surface, level, gameTextures, randomSeed, livesLeft, score, false, false, survival);
        }

        if (continueGame)
        {
            game->quit = false;
            game->escaped = false;

            int sx, sy;
            SDL_GetMouseState(&sx, &sy);
            game->Record(tempRecording, pausedFrame);
            SDL_WarpMouse(sx, sy);
            RecalculateMousePos();
            continueGame = false;
            GenScreenShotTexture();
            GLSetup();
        }
        else
        {
            livesLeftInReplay = livesLeft;
            int sx, sy;
            SDL_GetMouseState(&sx, &sy);
            game->Record(tempRecording, 0);
            SDL_WarpMouse(sx, sy);
            RecalculateMousePos();
            GenScreenShotTexture();
            GLSetup();
        }

        if (game->escaped)
        {
            showPauseMenu = true;
            pausedFrame = game->getLastLogicFrame();
        }
        else
        {
            if (!game->gameOver)
            { // finished level
                lastscore = score;
                score=game->score;
                livesLeft = game->lives;

                stringstream cfgs;
                int f = currentLevelName + 1;

                cfgs << currentLevelSet.filename << ":" << f << ":completed";

                profile.setb(cfgs.str(), true);

                // save time score
                if (hiScores.GoodEnough (HiScoreEntry(game->levelSeconds, currentLevelSet.filename, HS_TIME, profile.getName(), level.name)))
                {
                    timeHiScore = true;
                    hiScores.SubmitHiScore(HiScoreEntry(game->levelSeconds,
                                                        currentLevelSet.filename,
                                                        HS_TIME,
                                                        profile.getName(),
                                                        level.name));
                }

                if (currentLevelName < currentLevelSet.levels.size() - 1)
                {
                    showMenu = true;
                }
                else   // pwned the game !!!
                {
                    gameOver = true;
                    pwned = true;
                    if (playedFromStart && livesLeft)
                    {
                        score += Game::pointsForRemainingLife * livesLeft;
                    }

                    if (hiScores.GoodEnough (HiScoreEntry(score, currentLevelSet.filename, HS_ADVENTURE, profile.getName(), level.name)))
                    {
                        adventureHiScore = true;
                        SaveAdventureHiScore ();
                    }

                }
                GenScreenShotTexture();
                GLSetup();
                recording = tempRecording.str();
            }
            else
            { // lost a life
                lastscore = score;
                score=game->score;
                livesLeft = (int)game->lives;
                livesLeft--;

                if (!survival)
                {
                    if (livesLeft >= 0)
                    {
                        showLifeLostMenu = true;
                        recording = tempRecording.str();
                        return;
                    }
                }
                else
                {
                    // save survival hiscore
                    if (hiScores.GoodEnough (HiScoreEntry(score, currentLevelSet.filename, HS_SURVIVAL, profile.getName(), level.name)))
                    {
                        adventureHiScore = true;
                        hiScores.SubmitHiScore(HiScoreEntry(score,
                                                            currentLevelSet.filename,
                                                            HS_SURVIVAL,
                                                            profile.getName(),
                                                            level.name));
                    }

                }

                gameOver = true;

                if (!survival)
                    if (hiScores.GoodEnough (HiScoreEntry(score, currentLevelSet.filename, HS_ADVENTURE, profile.getName(), level.name)))
                    {
                        adventureHiScore = true;
                        SaveAdventureHiScore ();
                    }

                livesLeft = nLives;
                recording = tempRecording.str();
            }
        }
    }

    if (pwned)
        pwnedr+=0.01;
}

void GameLoop::CenterMsg(string msg, double y, FTFont *font)
{
    glLoadIdentity( );
    FTBBox b = font->BBox(msg.c_str());
    double tw = b.Upper().X() / 5;
    glTranslated((100 - tw) / 2, y, 5);
    glScaled(0.2, 0.2, 0.2);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    font->Render(msg.c_str());
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}


void GameLoop::Render(ulong frame)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(1.0, 1.0, 1.0);
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity( );

    if (screenShotTexture != 0)
    {
        // render level background
        glPushMatrix();
        glTranslatef(0.0, 0.0, -5);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, screenShotTexture);
        glBegin(GL_QUADS);
        glTexCoord2d(0, 0);
        glVertex3d(vleft, 100, 0);
        glTexCoord2d(0, 1);
        glVertex3d(vleft, 0, 0);
        glTexCoord2d(1, 1);
        glVertex3d(vleft + vwidth, 0, 0);
        glTexCoord2d(1, 0);
        glVertex3d(vleft + vwidth, 100, 0);
        glEnd();
        glDisable(GL_TEXTURE_2D);
        glPopMatrix();
    }

    if (showLifeLostMenu && !showSureQuitMenu)
    {
        lifeLostMenu.Render();
        glLoadIdentity( );
        glColor3d(1.0, 1.0, 1.0);

        char msgscore[256];
        sprintf(msgscore, _("Lives left: %d"), livesLeft);
        CenterMsg(msgscore, 50, font);

        sprintf(msgscore, _("Current score: %06d"), score);

        CenterMsg(msgscore, 45, font);
    }

    if (showSureQuitMenu)
    {
        sureQuitMenu.Render();
        string msg = _("Sure you want to end game?");
        glColor3d(1.0, 0.0, 0.0);

        CenterMsg(msg, 50, font);
    }

    if ((showPauseMenu) && (!showSureQuitMenu))
    {
        pauseMenu.Render();
        string msg = _("Game paused");
        glColor3d(1.0, 0.0, 0.0);

        CenterMsg(msg, 45, font);
    }

    if (showMenu && !showSureQuitMenu)
    {
        nextLevelMenu.Render();

        glLoadIdentity( );

        glColor3d(1.0, 1.0, 1.0);
        CenterMsg(_("Level cleared"), 50, font);

        char msgscore[256];
        sprintf(msgscore, _("Current score: %06d"), score);

        CenterMsg(msgscore, 45, font);
    }


    if (gameOver)
    {
        gameOverMenu.Render();

        glLoadIdentity( );
        string msg = _("Game Over");

        glColor3d(1.0, 0.0, 0.0);

        CenterMsg(msg, 50, font);
        char msgscore[256];
        sprintf(msgscore, _("Score: %06d"), score);
        glColor3d(1.0, 1.0, 1.0);
        CenterMsg(msgscore, 45, font);
    }

    if (pwned)
    {
        glColor3d(1.0, 1.0, 1.0);
        CenterMsg(_("Congratulations !!! You just pwned"), 90, font);

        if (playedFromStart && livesLeft)
        {
            stringstream s;
            s << _("Extra points for remaining lives : ") << Game::pointsForRemainingLife << "*" << livesLeft << "=" << (Game::pointsForRemainingLife * livesLeft);
            CenterMsg(s.str(), 85, font);
        }

        // logo
        glLoadIdentity();
        glPopMatrix();
        glEnable(GL_TEXTURE_2D);
        glTranslated(15 + 30 * cos(pwnedr), 90, 5);
        glBindTexture(GL_TEXTURE_2D, logoTexture);
        glScalef(70, 41, 5);
        glBegin(GL_QUADS);
        glTexCoord2d(0, 0);
        glVertex3d(0, 0, 0);
        glTexCoord2d(0, 1);
        glVertex3d(0, -1, 0);
        glTexCoord2d(1, 1);
        glVertex3d(1, -1, 0);
        glTexCoord2d(1, 0);
        glVertex3d(1, 0, 0);
        glEnd();
        glDisable(GL_TEXTURE_2D);

    }

    if (timeHiScore && !adventureHiScore)
    {
        glColor3d(1.0, 1.0, 1.0);
        /// Those 3 are shown in the bottom after each level
        CenterMsg(_("Shortest time for this level!"), 10, font);
    }

    if (adventureHiScore && timeHiScore)
    {
        glColor3d(1.0, 1.0, 1.0);
        CenterMsg(_("Shortest time for this level and a hi score!"), 10, font);
    }

    if ((adventureHiScore && !timeHiScore) || (survivalHiScore))
    {
        glColor3d(1.0, 1.0, 1.0);
        CenterMsg(_("You have a hi score!"), 10, font);
    }

    // render pointer
    glLoadIdentity();
    glEnable(GL_TEXTURE_2D);
    glTranslatef(mx, my, 20);
    glColor4f(1.0, 1.0, 1.0, 1.0);
    glBindTexture(GL_TEXTURE_2D, pointer);
    glScalef(5, 5, 5);
    glBegin(GL_QUADS);
    glTexCoord2d(0, 0);
    glVertex3d(0, 0, 0);
    glTexCoord2d(0, 1);
    glVertex3d(0, -1, 0);
    glTexCoord2d(1, 1);
    glVertex3d(1, -1, 0);
    glTexCoord2d(1, 0);
    glVertex3d(1, 0, 0);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    int pf = pointerFrame / 2;
    double ts = 1.0 / double(BALLTEXTURECOUNT);
    double tx = (double)((pf % BALLTEXTURECOUNT)) * ts;
    double ty = (double)((pf / BALLTEXTURECOUNT)) * ts;

    glLoadIdentity();
    glEnable(GL_TEXTURE_2D);
    glTranslated(mx, my, 20);
    glBindTexture(GL_TEXTURE_2D, gameTextures[1]);
    glTranslated(3.5, -3.5, 0);
    glScalef(3, 3, 3);
    glRotated(pointerRot, 0, 0, 1.0);

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
    glDisable(GL_TEXTURE_2D);
}

void GameLoop::GenScreenShotTexture()
{
    if (screenShotTexture)
    {
        glDeleteTextures(1, &screenShotTexture);
    }

    unsigned char *pixels;
    unsigned char *temp;

    glFlush();

    pixels = (unsigned char *)malloc(3 * surface->w * surface->h);
    temp = (unsigned char *)malloc(3 * screenShotTextureSize * screenShotTextureSize);

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, surface->w, surface->h,
                 GL_RGB, GL_UNSIGNED_BYTE, pixels);

    double sx = (double)surface->w / (double)screenShotTextureSize;
    double sy = (double)surface->h / (double)screenShotTextureSize;

    for (uint y = 0; y < screenShotTextureSize; ++y)
        for (uint x = 0; x < screenShotTextureSize; ++x)
        {
            uint yy = (uint)iround(double(y) * sy);
            uint xx = (uint)iround(double(x) * sx);

            if (xx >= (uint)surface->w)
                xx = surface->w - 1;

            if (yy >= (uint)surface->h)
                yy = surface->h - 1;

            unsigned char *p = pixels + (((surface->h - 1) - yy) * 3 * surface->w) + (xx * 3);
            unsigned char *pt = temp + (y * 3 * screenShotTextureSize) + (x * 3);

            unsigned char r = p[0];
            unsigned char g = p[1];
            unsigned char b = p[2];

            int newcol = (((int)r + (int)g + (int)b) / 3) - 10;
            if (newcol < 0)
                newcol = 0;

            unsigned char nc = (unsigned char)newcol;

            pt[0] = 0;
            pt[1] = 0;
            pt[2] = nc;
        };

    glGenTextures( 1, &screenShotTexture);

    glBindTexture( GL_TEXTURE_2D, screenShotTexture);

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

    glTexImage2D( GL_TEXTURE_2D, 0, 3, screenShotTextureSize, screenShotTextureSize, 0,
                  GL_RGB, GL_UNSIGNED_BYTE, temp);

    free(pixels);
    free(temp);
}

