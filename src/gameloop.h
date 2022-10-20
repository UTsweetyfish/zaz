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

#ifndef __GAMELOOP_H__
#define __GAMELOOP_H__

#include <vector>
#include <string>
#include <sstream>

#include "common.h"
#include "scene.h"
#include "game.h"
#include "menu.h"
#include "lineeditor.h"
#include "levelset.h"

class GameLoop : public Scenes::Scene
{
    friend void pauseMenuContinueHandler(void *ptr);
    friend void pauseMenuEndGameHandler(void *ptr);
    friend void nextLevelMenuNextLevelHandler(void *ptr);
    friend void nextLevelMenuViewReplayHandler(void *ptr);
    friend void nextLevelMenuExportVideoHandler(void *ptr);
    friend void gameOverMenuRestartHandler(void *ptr);
    friend void gameOverMenuEndGameHandler(void *ptr);
    friend void lifeLostMenuRetryLevelHandler(void *ptr);
    friend void lifeLostMenuEndGameHandler(void *ptr);
    friend void sureQuitMenuNoHandler(void *ptr);
    friend void sureQuitMenuYesHandler(void *ptr);

    static const uint nLives = 3;
    static const uint screenShotTextureSize = 512;

    uint score;
    uint lastscore;
    uint currentLevelName;
    uint randomSeed;

    GLuint *gameTextures;

    bool showMenu;
    bool gameOver;
    bool showPauseMenu;
    bool continueGame;
    bool showLifeLostMenu;
    bool showSureQuitMenu;
    bool pwned;
    bool survival;
    bool survivalHiScore;
    bool timeHiScore;
    bool adventureHiScore;
    bool playedFromStart;
    bool lifeLostSaveHiScoreOnQuit;

    Game *game;
    Level level;

    GLuint logoTexture;
    GLuint screenShotTexture;

    double pwnedr;

    Menu pauseMenu;
    Menu nextLevelMenu;
    Menu gameOverMenu;
    Menu lifeLostMenu;
    Menu sureQuitMenu;
    LineEditor editor;

    stringstream tempRecording;
    string recording;

    uint pausedFrame;

    int livesLeft;
    int livesLeftInReplay;
    int startLevel;

    void CenterMsg(string msg, double y, FTFont *font);

public:
    GameLoop(SDL_Surface *surf, GLuint *gameTextures, uint startLevel, uint fps = Scenes::DEFAULT_FPS, bool survivalMode = false);
    ~GameLoop();
    void GenScreenShotTexture();
    void GLSetup();
    void Render(ulong frame);
    void Logic(ulong frame);
    void ViewReplay();
    void ExportReplay();
    void NextLevel();
    void ClearGame();
    void CenterMsg();
    void SaveAdventureHiScore();
};

void sureQuitMenuNoHandler(void *ptr);
void sureQuitMenuYesHandler(void *ptr);
void pauseMenuContinueHandler(void *ptr);
void pauseMenuEndGameHandler(void *ptr);
void lifeLostMenuEndGameHandler(void *ptr);
void nextLevelMenuNextLevelHandler(void *ptr);
void nextLevelMenuViewReplayHandler(void *ptr);
void nextLevelMenuExportVideoHandler(void *ptr);
void gameOverMenuRestartHandler(void *ptr);
void gameOverMenuEndGameHandler(void *ptr);
void lifeLostMenuRetryLevelHandler(void *ptr);

#endif //__GAMELOOP_H__
