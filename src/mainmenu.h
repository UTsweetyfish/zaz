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

#ifndef __MAINMENU_H__
#define __MAINMENU_H__

#include <vector>

#include "common.h"
#include "menu.h"
#include "scene.h"
#include "level.h"
#include "ballpath.h"
#include "game.h"
#include "lineeditor.h"
#include "levelset.h"
#include "hiscores.h"

struct LevelDesc
{
    Level lev;
    GLuint tex;
    bool locked;

    LevelDesc(const char *philename)
            : lev(philename, true), tex(0), locked(true) {};

    LevelDesc()
            : tex(0), locked(true) {};

    ~LevelDesc()
    {
        if (tex)
        {
            glDeleteTextures(1, &tex);
        }
    }
};

struct CreditsLine
{
    bool fat;
    string txt;

    CreditsLine(string txt, bool fat = false)
            : fat(fat), txt(txt) {};
};

class MainMenu : public Scenes::Scene
{
    friend void menuBackHandler(void *ptr);
    friend void optionsMenuCancelHandler(void *ptr);
    friend void optionsMenuApplyHandler(void *ptr);
    friend void startMenuOptionsHandler(void *ptr);
    friend void startMenuExitHandler(void *ptr);
    friend void startMenuStartHandler(void *ptr);
    friend void startMenuCreditsHandler(void *ptr);
    friend void browserMenuLeftHandler(void *ptr);
    friend void browserMenuRightHandler(void *ptr);
    friend void startMenuShowProfilesHandler(void *ptr);
    friend void profileActionsMenuNewHandler(void *ptr);
    friend void profileActionsMenuDeleteHandler(void *ptr);
    friend void profileActionsMenuUseHandler(void *ptr);

    const static int browserNLevelsPerPage = 3;
    const static uint browserThumbSpacing = 5;
    const static uint hiScoreWaitPageSec = 10;
    const static int linesPerHiScorePage = 10;

    double lmx;
    double lmy;

    Menu startMenu;
    Menu optionsMenu;
    Menu creditsMenu;
    Menu browserMenu;
    Menu profileActionsMenu;
    Menu profileListMenu;
    LineEditor *newProfileEditor;
    Menu *currentMenu;

    GLuint logoTex;

    Level menuLev;

    GLuint ballText[NBALLCOLORS + BONUS_BOMB + 2];
    std::vector<BallPath> bp;
    vector<LevelSet> sets;
    vector<string> setNames;
    vector<GLuint> setTex;

    int nBallPaths;

    Scenes::Sample *music;
    bool renderHiscores;
    bool showCredits;
    bool showBrowser;
    bool showProfiles;
    bool showNewProfile;
    bool showSets;

    void StartMusic();
    void StopMusic();

    bool oldFullscreen;
    string oldRes;
    bool oldColourHints;
    string oldLanguage;
    int oldMusicVol;
    void CenterMsg(string msg, double y, FTFont *font, double size = 0.2);
    void LoadTextures();
    void LoadLevelSets();
    void FixLevelLocks();
    void CreateOptionsMenu();
    void CreateCredits();

    GLuint logoGpl;
    LevelDesc *levels;
    int nleveldesc;
    vector<string> profileNames;
    vector<CreditsLine> credits;

    bool startup;
    float startupProgress;
    float startupProgressSteps;
    void RenderStartupProgress();

    double browserScrollOffset;
    int browserScrollOffsetDest;
    int browserSelectedLevel;
    int browserMouseOverLevel;
    bool browserUsedKeyboard;

    uint selectedSet;
    void FillLevelDesc();

    std::vector<ReportPage> hiScoreRep;
    uint hiScorePage;
    uint hiScoreTimeout;
    int creditsScroll;
    const static int creditsScrollClearance = 900;

public:
    MainMenu(SDL_Surface *surf, uint fps = Scenes::DEFAULT_FPS);
    ~MainMenu();
    void GLSetup();
    void Render(ulong frame);


    void Logic(ulong frame);
};

void menuBackHandler(void *ptr);
void optionsMenuCancelHandler(void *ptr);
void optionsMenuApplyHandler(void *ptr);
void startMenuOptionsHandler(void *ptr);
void startMenuExitHandler(void *ptr);
void startMenuStartHandler(void *ptr);
void startMenuCreditsHandler(void *ptr);
void startMenuShowProfilesHandler(void *ptr);
void profileActionsMenuNewHandler(void *ptr);
void profileActionsMenuDeleteHandler(void *ptr);
void profileActionsMenuUseHandler(void *ptr);

void browserMenuLeftHandler(void *ptr);
void browserMenuRightHandler(void *ptr);

#endif //__MAINMENU_H__
