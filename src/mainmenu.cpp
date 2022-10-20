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

#include "mainmenu.h"
#include "textureloader.h"
#include "game.h"
#include "sample.h"
#include "gameloop.h"
#include "profile.h"
#include "directorylister.h"
#include <cstdio>

MainMenu::MainMenu(SDL_Surface *surf, uint fps)
        :Scenes::Scene(surf, fps), currentMenu(&startMenu),
        menuLev(settings.getCFilename("menu.lvl")), nBallPaths(menuLev.paths.size() - 1), music(NULL),
        renderHiscores(true), showCredits(false), showBrowser(false), showProfiles(false), showNewProfile(false), showSets(false),
        oldFullscreen(settings.getb("fullscreen", false)),
        oldRes(settings.get("resolution", "")), oldMusicVol(atoi(settings.get("musicVolume", "50").c_str())),
        oldLanguage(settings.get("language", "")),startup(true), browserSelectedLevel(-1), browserUsedKeyboard(false)
{
    startMenu.Add(new GenericMenuItem(_("Start game"), startMenuStartHandler, this));
    startMenu.Add(new GenericMenuItem(_("Options"), startMenuOptionsHandler, this));
    startMenu.Add(new GenericMenuItem(_("Credits"), startMenuCreditsHandler, this));
    startMenu.Add(new GenericMenuItem(_("Exit"), startMenuExitHandler, this));
    startMenu.Add(new GenericMenuItem(_("Profile:") + profile.getName(), startMenuShowProfilesHandler, this));

    startMenu.SetDimensions(60, 40, 40);

    startMenu.items[4]->width = 80;
    startMenu.items[4]->y = 6;
    startMenu.items[4]->x = 10;

    creditsMenu.Add(new GenericMenuItem(_("Back"), menuBackHandler, this));
    creditsMenu.SetDimensions(30, 10, 40);

    std::vector<std::string>game_types;

    game_types.push_back(_("Sequential"));
    game_types.push_back(_("Survival"));

    browserMenu.Add(new GenericMenuItem(_("Back"), menuBackHandler, this));
    browserMenu.Add(new GenericMenuItem("<<", browserMenuRightHandler, this));
    browserMenu.Add(new GenericMenuItem(">>", browserMenuLeftHandler, this));
    browserMenu.Add(new OptionMenuItem(_("Game type:"), game_types, "game_type"));

    browserMenu.SetDimensions(30, 6, 40);

    browserMenu.items[1]->x = 10;
    browserMenu.items[1]->width = 10;
    browserMenu.items[1]->y = 11;

    browserMenu.items[2]->x = 80;
    browserMenu.items[2]->width = 10;
    browserMenu.items[2]->y = 11;

    browserMenu.items[3]->x = 20;
    browserMenu.items[3]->width = 60;
    browserMenu.items[3]->y = 45;

    profileActionsMenu.Add(new GenericMenuItem(_("New profile"), profileActionsMenuNewHandler, this));
    profileActionsMenu.Add(new GenericMenuItem(_("Delete"), profileActionsMenuDeleteHandler, this));
//    profileActionsMenu.Add(new GenericMenuItem(_("Use"), profileActionsMenuUseHandler, this));

    profileActionsMenu.SetDimensions(0, 10, 30);

    profileActionsMenu.items[0]->y = 10;
    profileActionsMenu.items[0]->x = 30;
    profileActionsMenu.items[0]->width = 40;

    for (int p = 0; p < nBallPaths; p++)
    {
        bp.push_back(BallPath(menuLev.paths[p + 1], ballText, 0));
        bp[p].state.feedRate = 2;
        bp[p].state.colors = 16;
        bp[p].state.ballsToDraw = -1;
        bp[p].state.ballsFromStart = 0;
    }

    lmx = lmy = 0;

    setNames = LevelSet::getSetNames();
    nleveldesc = 0;

    hiScoreRep = hiScores.GenerateReport(linesPerHiScorePage);
    hiScorePage = 0;
    hiScoreTimeout = hiScoreWaitPageSec * desiredFPS;

    CreateOptionsMenu();
    CreateCredits();
    creditsScroll = creditsScrollClearance * -1;
}

void MainMenu::CreateCredits()
{
    credits.push_back(CreditsLine(string("Zaz ") + VERSION, true));
    credits.push_back(CreditsLine("-"));
    credits.push_back(CreditsLine(_("code, gfx, snd & design:")));
    credits.push_back(CreditsLine(_("Remigiusz Dybka"), true));
    credits.push_back(CreditsLine(_("[remigiusz.dybka@gmail.com] Released under GPLv3 License")));
    credits.push_back(CreditsLine("-"));
    credits.push_back(CreditsLine(_("music:")));
    credits.push_back(CreditsLine(_("paniq - Leonard Ritter"), true));
    credits.push_back(CreditsLine(_("[http://www.paniq.org] Released under CC BY-SA License")));
    credits.push_back(CreditsLine("-"));
    credits.push_back(CreditsLine(_("quality assurance:")));
    credits.push_back(CreditsLine(_("Kinga Dybka"), true));
    credits.push_back(CreditsLine(_("Michael Sterrett"), true));
    credits.push_back(CreditsLine("-"));
    credits.push_back(CreditsLine("-"));
    credits.push_back(CreditsLine(_("translations:"), true));
    credits.push_back(CreditsLine("-"));
    credits.push_back(CreditsLine(_("German:")));
    credits.push_back(CreditsLine(_("Frederik Schwarzer"), true));
    credits.push_back(CreditsLine("-"));
    credits.push_back(CreditsLine(_("Spanish:")));
    credits.push_back(CreditsLine(_("Dámaso Domínguez (AmiSpaTra)"), true));
    credits.push_back(CreditsLine("-"));
    credits.push_back(CreditsLine(_("French:")));
    credits.push_back(CreditsLine(_("Nouvel Hugues"), true));
    credits.push_back(CreditsLine("-"));
    credits.push_back(CreditsLine(_("Hungarian:")));
    credits.push_back(CreditsLine(_("Gabor Kmetyko"), true));
    credits.push_back(CreditsLine("-"));
    credits.push_back(CreditsLine(_("Italian:")));
    credits.push_back(CreditsLine(_("Andrea Musuruane"), true));
    credits.push_back(CreditsLine("-"));
    credits.push_back(CreditsLine(_("Russian:")));
    credits.push_back(CreditsLine(_("Николай Рощупкин"), true));
    credits.push_back(CreditsLine("-"));
    credits.push_back(CreditsLine(_("Turkish:")));
    credits.push_back(CreditsLine(_("Anıl Özbek"), true));
    credits.push_back(CreditsLine("-"));
    credits.push_back(CreditsLine("-"));
    credits.push_back(CreditsLine(_("additional coding:")));
    credits.push_back(CreditsLine(_("Nouvel Hugues"), true));
    credits.push_back(CreditsLine("-"));
    credits.push_back(CreditsLine(_("additional testing:")));
    credits.push_back(CreditsLine(_("Irena Klon"), true));
    credits.push_back(CreditsLine(_("Kamil Krzyspiak"), true));
    credits.push_back(CreditsLine(_("Mateusz Jakubowski"), true));
    credits.push_back(CreditsLine("-"));
    credits.push_back(CreditsLine("Copyright (C) Remigiusz Dybka 2009-2010 <remigiusz.dybka@gmail.com>"));
    credits.push_back(CreditsLine("-"));
    credits.push_back(CreditsLine("Zaz is free software: you can redistribute it and/or modify it"));
    credits.push_back(CreditsLine("under the terms of the GNU General Public License as published by the"));
    credits.push_back(CreditsLine("Free Software Foundation, either version 3 of the License, or"));
    credits.push_back(CreditsLine("(at your option) any later version."));
    credits.push_back(CreditsLine("-"));
    credits.push_back(CreditsLine("Zaz is distributed in the hope that it will be useful, but"));
    credits.push_back(CreditsLine("WITHOUT ANY WARRANTY; without even the implied warranty of"));
    credits.push_back(CreditsLine("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."));
    credits.push_back(CreditsLine("See the GNU General Public License for more details."));
    credits.push_back(CreditsLine("-"));
    credits.push_back(CreditsLine("You should have received a copy of the GNU General Public License along"));
    credits.push_back(CreditsLine("with this program.  If not, see <http://www.gnu.org/licenses/>."));
}

void MainMenu::CreateOptionsMenu()
{
    optionsMenu.Clear();

    std::vector<std::string>resolutions;
    std::vector<std::string>tempRes;

    for (uint m = 0; screenModes[m]; ++m)
    {
        stringstream sm;
        sm << screenModes[m]->w << "x" << screenModes[m]->h;

        tempRes.push_back(sm.str());
    }

    // reverse the resolutions
    for (int m = (unsigned int)tempRes.size() - 1; m >= 0; m--)
    {
        resolutions.push_back(tempRes[m]);
    }

    int y = 9;

#ifdef ENABLE_NLS
    y++;
    std::vector<std::string>languages;
    std::vector<std::string>descriptions;
    std::map<std::string, std::string>::iterator iter;

    for (iter = settings.languages.begin(); iter != settings.languages.end(); ++iter)
    {
        languages.push_back(iter->first);
        descriptions.push_back(iter->second);
    }

    optionsMenu.Add(new OptionMenuItem(_("Language"), languages, "language", &settings, descriptions));
#endif

    optionsMenu.Add(new OptionMenuItem(_("Resolution"), resolutions, "resolution", &settings));
    optionsMenu.Add(new BooleanMenuItem(_("Fullscreen"), &settings, "fullscreen"));
    optionsMenu.Add(new BooleanMenuItem(_("Show FPS"), &settings, "showFps"));
    optionsMenu.Add(new BooleanMenuItem(_("Colour hints"), &settings, "colourHints"));
    optionsMenu.Add(new BooleanMenuItem(_("Disable speed-up"), &settings, "disableSpeedup"));
    optionsMenu.Add(new ValueMenuItem(_("Mouse sensivity"), 1, 10, &settings, "mouseSensivity"));
    optionsMenu.Add(new ValueMenuItem(_("Keyb. sensivity"), 1, 10, &settings, "keyboardSensivity"));
    optionsMenu.Add(new ValueMenuItem(_("SFX volume"), 0, 100, &settings, "sfxVolume"));
    optionsMenu.Add(new ValueMenuItem(_("Music volume"), 0, 100, &settings, "musicVolume"));
    optionsMenu.Add(new GenericMenuItem(_("Cancel"), optionsMenuCancelHandler, this));
    optionsMenu.Add(new GenericMenuItem(_("Apply"), optionsMenuApplyHandler, this));
    optionsMenu.SetDimensions(10, 80, 80);

    optionsMenu.items[y]->width = 29;
    optionsMenu.items[y+1]->width = 29;
    optionsMenu.items[y+1]->x = 61;
    optionsMenu.items[y+1]->y = optionsMenu.items[y]->y;
}

void MainMenu::RenderStartupProgress()
{
    float pbarwidth = 80;
    float pbarheight = 5;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity( );
    glTranslated((100 - pbarwidth) / 2, (vheight + pbarheight) / 2, 0);

    glBegin(GL_QUADS);
    glColor3d( .1, .1, .7);
    glVertex3d(0, 0, 0);
    glVertex3d(0, -pbarheight, 0);
    glColor3d( 0, 0, 0.5);
    glVertex3d((startupProgress / startupProgressSteps) * pbarwidth, -pbarheight, 0);
    glVertex3d((startupProgress / startupProgressSteps) * pbarwidth, 0, 0);
    glEnd();

    glColor3d( 0, 0, 0.5);
    glBegin(GL_LINE_STRIP);
    glVertex3d(0, 0, 0);
    glVertex3d(0, -pbarheight, 0);
    glVertex3d(pbarwidth, -pbarheight, 0);
    glVertex3d(pbarwidth, 0, 0);
    glVertex3d(0, 0, 0);
    glEnd();

    startupProgress++;
    glFlush();
    SDL_GL_SwapBuffers();
}

void MainMenu::LoadLevelSets()
{
    vector<string>::iterator i;
    for (i = setNames.begin(); i!= setNames.end(); ++i)
    {
        LevelSet s = LevelSet(*i);
        sets.push_back(s);
        setTex.push_back(LoadTextureFile (settings.getCFilename(s.thumbTexFilename)));
        RenderStartupProgress();
    }

    RenderStartupProgress();
}

void MainMenu::LoadTextures()
{
    startupProgress = 0;

    bool useColourHints = settings.getb("colourHints", false);

    for (int b = 0; b < 16; b++)
    {
        BallDesc bd = BallDescriptions[b];
        ballText[b] = LoadBallsTexture(bd.fileName, bd.r, bd.g, bd.b, useColourHints?bd.overlay:0);
        RenderStartupProgress();
    }

    ballText[16] = LoadTexture("bonus1.png");
    RenderStartupProgress();
    ballText[17] = LoadTexture("bonus2.png");
    RenderStartupProgress();
    ballText[18] = LoadTexture("bonus3.png");
    RenderStartupProgress();
    ballText[19] = LoadTexture("bonus4.png");
    RenderStartupProgress();
    ballText[20] = LoadTexture("bonus5.png");
    RenderStartupProgress();
    ballText[21] = LoadTexture("explosion.png");
    RenderStartupProgress();

    logoTex = LoadTexture("logo.png");
    RenderStartupProgress();
    logoGpl = LoadTexture ("gpl3.png");
    RenderStartupProgress();

    pointer = LoadTexture("ptr.png");
}


void MainMenu::StartMusic()
{
    int musicVol = atoi(settings.get("musicVolume", "50").c_str());

    music = (Scenes::Sample *)new Scenes::StreamingOggSample(Game::getRandomMusic());
    mixer->EnqueueSample(music, musicVol, 0, true);
}

void MainMenu::StopMusic()
{
    mixer->DisposeSample(music);
}

void profileActionsMenuNewHandler(void *ptr)
{
    MainMenu *p = (MainMenu *)ptr;
    p->showNewProfile = true;
    p->newProfileEditor = new LineEditor("", 60, 16, 30);
}

void profileActionsMenuDeleteHandler(void *ptr)
{
    MainMenu *p = (MainMenu *)ptr;
    // delete the profile file
    string ph = p->profileNames[p->profileListMenu.getHoverItem()];
    ph = settings.getDefaultDirectory() + SEPARATOR + ph + ".profile";
#ifdef WIN32
    DeleteFile(Settings::W32_GetFileName(ph).c_str());
#else
    remove(ph.c_str());
#endif

    startMenuShowProfilesHandler(ptr);
}

void profileActionsMenuUseHandler(void *ptr)
{
    MainMenu *p = (MainMenu *)ptr;
    profile.Save();
    profile = Profile(p->profileNames[p->profileListMenu.getHoverItem()]);

    p->startMenu.items[4] = new GenericMenuItem(_("Profile:") + profile.getName(), startMenuShowProfilesHandler, p);
    p->startMenu.SetDimensions(60, 40, 40);
    p->startMenu.items[4]->width = 80;
    p->startMenu.items[4]->y = 6;
    p->startMenu.items[4]->x = 10;

    settings.set("last_profile", profile.getName());

    profile.Save();
    p->showProfiles = false;
    p->renderHiscores = true;
    p->currentMenu = &p->startMenu;
}

void browserMenuLeftHandler(void *ptr)
{
    MainMenu *p = (MainMenu *)ptr;
    int sz;

    if (p->showBrowser)
    {
        sz = p->nleveldesc; //p->levels.size();
    }
    else
    {
        sz = p->sets.size();
    }

    if (p->browserScrollOffsetDest >= sz - p->browserNLevelsPerPage)
        return;

    p->browserScrollOffsetDest+=p->browserNLevelsPerPage;

    if (p->browserScrollOffsetDest > (int)(sz - p->browserNLevelsPerPage))
        p->browserScrollOffsetDest = sz - p->browserNLevelsPerPage;
}

void browserMenuRightHandler(void *ptr)
{
    MainMenu *p = (MainMenu *)ptr;
    p->browserScrollOffsetDest-=p->browserNLevelsPerPage;
    if (p->browserScrollOffsetDest < 0)
        p->browserScrollOffsetDest = 0;

}

void optionsMenuApplyHandler(void *ptr)
{
    MainMenu *p = (MainMenu *)ptr;

    menuBackHandler (ptr);

    wantReinit = false;
#ifdef ENABLE_NLS
    if (p->oldLanguage != settings.get("language", ""))
    {
        settings.setLanguage(settings.get("language", ""));
        wantReinit = true;
    }
#endif
    if (p->oldFullscreen != settings.getb("fullscreen", false))
        wantReinit = true;

    if (p->oldRes != settings.get("resolution", ""))
        wantReinit = true;

    if (p->oldColourHints != settings.getb("colourHints", false))
        wantReinit = true;

    if (p->oldMusicVol != atoi(settings.get("musicVolume", "50").c_str()))
        wantReinit = true;

    if (wantReinit)
    {
        p->quit = true;
        p->StopMusic();
    }

    ((Scene *)p)->show_fps = settings.getb("showFps", false);
};

void optionsMenuCancelHandler(void *ptr)
{
    MainMenu *p = (MainMenu *)ptr;

    menuBackHandler (ptr);

    int y = 9;

#ifdef ENABLE_NLS
    y++;
#endif
    for (int f = 0; f < y; f++)
        p->optionsMenu.items[f]->Revert();
}

void menuBackHandler(void *ptr)
{
    MainMenu *p = (MainMenu *)ptr;
    if (p->showBrowser)
    {
        p->showBrowser = false;
        p->showSets = true;
        p->browserScrollOffset = -5.0;
        p->browserScrollOffsetDest = 0;
        p->browserSelectedLevel = -1;
        return;
    }

    ((MainMenu*)ptr)->currentMenu = &((MainMenu*)ptr)->startMenu;
    ((MainMenu*)ptr)->renderHiscores = true;
    ((MainMenu*)ptr)->showCredits = false;
    ((MainMenu*)ptr)->showBrowser = false;
    ((MainMenu*)ptr)->showSets = false;
}

void startMenuOptionsHandler(void *ptr)
{
    ((MainMenu*)ptr)->oldFullscreen = settings.getb("fullscreen", false);
    ((MainMenu*)ptr)->oldRes = settings.get("resolution", "");
    ((MainMenu*)ptr)->oldColourHints = settings.getb("colourHints", "");
    ((MainMenu*)ptr)->oldLanguage = settings.get("language", "");
    ((MainMenu*)ptr)->oldMusicVol = atoi(settings.get("musicVolume", "50").c_str());

    ((MainMenu*)ptr)->currentMenu = &((MainMenu*)ptr)->optionsMenu;
    ((MainMenu*)ptr)->renderHiscores = false;
};

void startMenuCreditsHandler(void *ptr)
{
    ((MainMenu*)ptr)->currentMenu = &((MainMenu*)ptr)->creditsMenu;
    ((MainMenu*)ptr)->renderHiscores = false;
    ((MainMenu*)ptr)->showCredits = true;
    ((MainMenu*)ptr)->creditsScroll = ((MainMenu*)ptr)->creditsScrollClearance * -1;
};

void startMenuExitHandler(void *ptr)
{
    ((MainMenu*)ptr)->quit = true;
    ((MainMenu*)ptr)->StopMusic();
};

void startMenuShowProfilesHandler(void *ptr)
{
    profile.Save();
    MainMenu *p = (MainMenu *)ptr;
    p->currentMenu = &p->profileActionsMenu;

    // regenerate profile list
    p->profileListMenu.items.clear();

    p->profileNames = ListFiles(settings.getDefaultDirectory(), ".profile");

    for (uint f = 0; f < p->profileNames.size(); f++)
    {
        string n = p->profileNames[f];
        n = n.substr(0, n.find(".profile"));

        p->profileNames[f] = n;
    }

    for (uint f = 0; f < p->profileNames.size() && f < 5; f++)
        p->profileListMenu.Add(new GenericMenuItem(p->profileNames[f], profileActionsMenuUseHandler, ptr));

    p->profileListMenu.SetDimensions(25, 45, 50);

    p->profileActionsMenu.items[0]->show = true;
    if (p->profileNames.size() > 4)
        p->profileActionsMenu.items[0]->show = false;

    p->profileActionsMenu.items[1]->show = true;

    if (p->profileNames.empty())
    {
        p->profileActionsMenu.items[1]->show = false;
    }

    p->showProfiles = true;
    p->renderHiscores = false;
    if (p->profileListMenu.getHoverItem() >= p->profileListMenu.items.size())
    {
        p->profileListMenu.setHoverItem(p->profileListMenu.items.size() - 1);
    }
}

void startMenuStartHandler(void *ptr)
{
    MainMenu *p = (MainMenu *)ptr;
    p->browserScrollOffset = -5.0;
    p->browserScrollOffsetDest = 0;
    p->browserSelectedLevel = -1;
    p->showSets = true;
    p->showBrowser = false;
    p->renderHiscores = false;
    p->currentMenu = &((MainMenu*)ptr)->browserMenu;
    p->browserUsedKeyboard = false;
};

MainMenu::~MainMenu()
{
    glDeleteTextures(1, &logoTex);
    glDeleteTextures(1, &logoGpl);
    glDeleteTextures(1, &pointer);
    glDeleteTextures(14, ballText);

    for (uint f = 0; f < setTex.size(); f++)
        glDeleteTextures(1, &setTex[f]);

    if (nleveldesc)
        delete [] levels;
}

void MainMenu::GLSetup()
{
    int width = surface->w;
    int height = surface->h;

    SDL_ShowCursor(0);

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

void MainMenu::Render(ulong frame)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(1.0, 1.0, 1.0);
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity( );

    for (int p = 0; p < nBallPaths; ++p)
    {
        bp[p].Render();
    }

    // logo
    glLoadIdentity( );
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glTranslated(vleft + 5, 95, 5);
    glBindTexture(GL_TEXTURE_2D, logoTex);
    glScalef(80, 48, 5);
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
    glPopMatrix();

    currentMenu->Render();

    if (renderHiscores && !hiScoreRep.empty())
    {
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

        glPushMatrix();
        glTranslatef(18, 42, 5);
        glScalef(0.15f, 0.15f, 0.15f);
        glColor4d(1.0, 1.0, 1.0, 1.0);

        font->Render(_("Hall of Fame"), -1);
        glPopMatrix();


        double alpha = 1.0;
        int a = (hiScoreWaitPageSec * desiredFPS) - hiScoreTimeout;
        int b = (hiScoreWaitPageSec * desiredFPS) / 5;
        int c = (hiScoreWaitPageSec * desiredFPS);

        if (a < b)
        { // fadein
            alpha = (double)a / (double)b;
        }

        if (a > (c - b))
        { // fadeout
            alpha = 1.0 - ((double)(a - (c - b)) / (double)b);
        }


        float y = 37;
        glPushMatrix();
        glTranslatef(-5, y, 5);
        glScalef(0.1f, 0.1f, 0.1f);
        glColor4d(1.0, 0.0, 1.0, alpha);

        font->Render(hiScoreRep[hiScorePage].header.c_str(), -1);
        glPopMatrix();

        y-= 3;

        uint nHigh = hiScoreRep[hiScorePage].lines.size();
        for (uint f = 0; f < nHigh; f++)
        {
            glPushMatrix();
            glTranslatef(-3, y, 5);
            glScalef(0.1f, 0.1f, 0.1f);
            glColor4d(1.0, 1.0, 1.0, alpha);

            font4->Render(hiScoreRep[hiScorePage].lines[f].c_str(), -1);
            glPopMatrix();

            y-=2.5;
        }
    }

    if (showProfiles)
    {
        profileListMenu.Render();
        if (showNewProfile)
            newProfileEditor->Render();
    }

    if (showCredits)
    {
        double creditsY = 45;
        double scrollLength = 21;
        int pos = creditsScroll / 100;

        double starty = creditsY + ((creditsScroll%100) / 100.0) * 2.5;
        double y = starty;

        while (y > (starty - scrollLength))
        {
            if (pos < 0)
            {
                y = y - 2.5;
                pos++;
                continue;
            }

            if (pos >= credits.size())
            {
                y = y - 2.5;
                continue;
            }

            if (creditsY - y < (scrollLength) / 2)
            {
                glColor4f(1.0, 1.0, 1.0, (creditsY - y) / (scrollLength / 4));
            }
            else
            {
                glColor4f(1.0, 1.0, 1.0, (scrollLength - (creditsY - y)) / (scrollLength / 4) - 0.2);
            }

            double size = 0.08;
            if (credits[pos].fat)
                size = 0.12;
            if (credits[pos].txt != "-")
                CenterMsg(_(credits[pos].txt.c_str()), y, font, size);

            pos++;

            y = y - 2.5;
        }

        glLoadIdentity();
        glEnable(GL_TEXTURE_2D);
        glTranslatef(50, 18, 5);
        glBindTexture(GL_TEXTURE_2D, logoGpl);
        glScalef(20, 10, 1);
        glBegin(GL_QUADS);
        glTexCoord2d(0, 0);
        glVertex3d(-.5, .5, 0);
        glTexCoord2d(0, 1);
        glVertex3d(-.5, -.5, 0);
        glTexCoord2d(1, 1);
        glVertex3d(.5, -.5, 0);
        glTexCoord2d(1, 0);
        glVertex3d(.5, .5, 0);
        glEnd();
        glDisable(GL_TEXTURE_2D);
    }

    if (showSets)
    {
        glColor4f(1.0, 1.0, 1.0, 1.0);
        CenterMsg(_("choose a level set"), 37, font, 0.1);
        int lstart = (int)iround(browserScrollOffset) - 1;
        if (lstart < 0)
            lstart = 0;

        int lend = lstart + browserNLevelsPerPage + browserNLevelsPerPage;
        if (lend > (int)sets.size())
            lend = sets.size();

        double tw = (100.0 - double((browserThumbSpacing * (browserNLevelsPerPage - 1)))) / browserNLevelsPerPage;
        double th = tw / (640.0/480.0);

        double xmv = -(browserScrollOffset - lstart) * (tw + browserThumbSpacing);

        double xx = 0;
        double spc = 0;

        if (!browserUsedKeyboard)
            browserSelectedLevel = -1;

        browserMouseOverLevel = -1;

        for (int f = lstart; f < lend; f++)
        {
            glLoadIdentity();
            glTranslated(xx * tw + spc + xmv, 25 - (th / 2), 5);

            if ((mx > xx * tw + spc + xmv) &&
                    (mx < xx * tw + spc + xmv + tw) &&
                    (my > 25 - (th / 2)) &&
                    (my < 25 + (th / 2)))
            {
                if (!browserUsedKeyboard)
                    browserSelectedLevel = f;
                browserMouseOverLevel = f;

            }

            glColor4d(.7, .7, .7, 1.0);
            if (browserSelectedLevel == f)
            {
                glColor4d(1.0, 1.0, 1.0, 1.0);
            }

            glEnable(GL_TEXTURE_2D);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            glBindTexture(GL_TEXTURE_2D, setTex[f]);
            glBegin(GL_QUADS);
            glTexCoord2d(0, 0);
            glVertex3d(0, th, 0);
            glTexCoord2d(0, 1);
            glVertex3d(0, 0, 0);
            glTexCoord2d(1, 1);
            glVertex3d(tw, 0, 0);
            glTexCoord2d(1, 0);
            glVertex3d(tw, th, 0);
            glEnd();

            glDisable(GL_TEXTURE_2D);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

            glLoadIdentity();
            glTranslated(xx * tw + spc + xmv, 25 - (th / 2), 7);
            glColor4d(0, 0, 0, 1);
            glLineWidth(2.0);
            glBegin(GL_LINE_STRIP);
            glVertex3d(0, 0, 0);
            glVertex3d(0, th, 0);
            glVertex3d(tw, th, 0);
            glVertex3d(tw, 0, 0);
            glVertex3d(0, 0, 0);
            glEnd();

            double size = 0.08;
            glLoadIdentity( );
            glColor4d(1.0, 1.0, 1.0, 1);
            char levelName[256];
            sprintf(levelName, "%s", sets[f].getDesc().c_str());

            FTBBox b = font->BBox(gettext(levelName));
            double txtw = b.Upper().X() / (1.0 / size);
            glTranslated((xx * tw + spc) + (tw / 2) - (txtw / 2) + xmv, 11.5, 5);
            glScaled(size, size, size);

            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            font->Render(gettext(levelName));

            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

            spc += browserThumbSpacing;
            xx += 1.0;
        }
    }

    if (showBrowser)
    {
        glColor4f(1.0, 1.0, 1.0, 1.0);
        CenterMsg(_(sets[selectedSet].description.c_str()), 37, font, 0.1);

        int lstart = (int)iround(browserScrollOffset) - 1;
        if (lstart < 0)
            lstart = 0;

        int lend = lstart + browserNLevelsPerPage + browserNLevelsPerPage;
        if (lend > nleveldesc)
            lend = nleveldesc;

        double tw = (100.0 - double((browserThumbSpacing * (browserNLevelsPerPage - 1)))) / browserNLevelsPerPage;
        double th = tw / (640.0/480.0);

        double xmv = -(browserScrollOffset - lstart) * (tw + browserThumbSpacing);

        double xx = 0;
        double spc = 0;

        if (!browserUsedKeyboard)
            browserSelectedLevel = -1;

        browserMouseOverLevel = -1;

        for (int f = lstart; f < lend; f++)
        {
            glLoadIdentity();
            glTranslated(xx * tw + spc + xmv, 25 - (th / 2), 5);


            if ((mx > xx * tw + spc + xmv) &&
                    (mx < xx * tw + spc + xmv + tw) &&
                    (my > 25 - (th / 2)) &&
                    (my < 25 + (th / 2)))
            {
                if (!levels[f].locked && iround(browserScrollOffset) >= 0)
                {
                    if (!browserUsedKeyboard)
                        browserSelectedLevel = f;

                    browserMouseOverLevel = f;
                }
            }

            glColor4d(.7, .7, .7, 1.0);
            if (browserSelectedLevel == f)
            {
                glColor4d(1.0, 1.0, 1.0, 1.0);
            }

            glEnable(GL_TEXTURE_2D);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            glBindTexture(GL_TEXTURE_2D, levels[f].tex);

            XY tl(0,0);
            XY tr(1,0);
            XY bl(0,1);
            XY br(1,1);

            if (levels[f].lev.mirrorX)
            {
                XY t = tl;
                tl = tr;
                tr = t;

                t = bl;
                bl = br;
                br = t;
            }

            if (levels[f].lev.mirrorY)
            {
                XY t = tl;
                tl = bl;
                bl = t;

                t = br;
                br = tr;
                tr = t;
            }

            glBegin(GL_QUADS);
            glTexCoord2d(tl.x, tl.y);
            glVertex3d(0, th, 0);
            glTexCoord2d(bl.x, bl.y);
            glVertex3d(0, 0, 0);
            glTexCoord2d(br.x, br.y);
            glVertex3d(tw, 0, 0);
            glTexCoord2d(tr.x, tr.y);
            glVertex3d(tw, th, 0);
            glEnd();

            glDisable(GL_TEXTURE_2D);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

            glLoadIdentity();
            glTranslated(xx * tw + spc + xmv, 25 - (th / 2), 7);
            glColor4d(0, 0, 0, 1);
            glLineWidth(2.0);
            glBegin(GL_LINE_STRIP);
            glVertex3d(0, 0, 0);
            glVertex3d(0, th, 0);
            glVertex3d(tw, th, 0);
            glVertex3d(tw, 0, 0);
            glVertex3d(0, 0, 0);
            glEnd();

            double size = 0.08;
            glLoadIdentity( );
            glColor4d(1.0, 1.0, 1.0, 1);
            char levelName[256];
            sprintf(levelName, "%s", levels[f].lev.name.c_str());

            FTBBox b = font->BBox(gettext(levelName));
            double txtw = b.Upper().X() / (1.0 / size);
            glTranslated((xx * tw + spc) + (tw / 2) - (txtw / 2) + xmv, 11.5, 5);
            glScaled(size, size, size);

            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            font->Render(gettext(levelName));
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

            if (levels[f].locked)
            {
                char lockedTxt[256];

                sprintf(lockedTxt, "%s", _("locked"));
                size = 0.3;
                glLoadIdentity( );
                glColor4d(1.0, 0, 0, 1);
                b = font->BBox(lockedTxt);
                txtw = b.Upper().X() / (1.0 / size);
                glTranslated((xx * tw + spc) + (tw / 2) - (txtw / 2) + xmv, 28, 7);
                glScaled(size, size, size);

                glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
                font->Render(lockedTxt);
                glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            }

            spc += browserThumbSpacing;
            xx += 1.0;
        }
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
    glBindTexture(GL_TEXTURE_2D, ballText[1]);
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

void MainMenu::FixLevelLocks()
{
    if (currentLevelSet.getEmpty())
        return;


}

void MainMenu::Logic(ulong frame)
{
    pointerFrame++;
    if (pointerFrame >= BALLTEXTURECOUNT*BALLTEXTURECOUNT*2)
        pointerFrame = 0;

    pointerRot = pointerRot + 0.1;

    if (startup)
    {
        // handle LCTRL to reset screenmode
        if (!resReset && (SDL_GetModState() & KMOD_LCTRL))
        {
            settings.setb("fullscreen", false);
            settings.set("resolution", "640x480");

            wantReinit = true;
            quit = true;
            StopMusic();
            resReset = true;
            return;
        }

        startupProgressSteps = (float)(25 + setNames.size());
        GLSetup();
        LoadTextures();
        LoadLevelSets();
        resync = true;
        startup = false;
        return;
    }

    if (music == NULL)
        StartMusic ();

    if (showNewProfile)
    {
        newProfileEditor->Logic(events);
    }

    bool click = false;

    if (!events.empty)
    {
        if (events.keyDown.size() > 0)
            for (vector<SDLKey>::iterator i = events.keyDown.begin(); i != events.keyDown.end(); ++i)
            {
                if (*i == SDLK_ESCAPE && !showNewProfile)
                {
                    if (currentMenu == &startMenu)
                    {
                        quit = true;
                    }
                    else
                    {
                        if (currentMenu == &optionsMenu)
                        {
                            optionsMenuCancelHandler(this);
                            return;
                        }

                        if (showBrowser)
                        {
                            showBrowser = false;
                            showSets = true;
                            browserScrollOffset = -5.0;
                            browserScrollOffsetDest = 0;
                            browserSelectedLevel = -1;
                            return;
                        }
                        else
                        {
                            currentMenu = &startMenu;
                            renderHiscores = true;
                            showBrowser = false;
                            showCredits = false;
                            showSets = false;
                            if (showProfiles)
                            { // we could have deleted current profile and escaped
                                profile.Save();
                                showProfiles = false;
                                renderHiscores = true;
                            }
                        }
                    }
                }

                if (*i == SDLK_ESCAPE && showNewProfile)
                {
                    showNewProfile = false;
                    delete(newProfileEditor);
                }

                if (!showNewProfile)
                    if (showProfiles && (*i == SDLK_RETURN || *i == SDLK_KP_ENTER))
                    {
                        profileActionsMenuUseHandler(this);
                        return;
                    }

                if (showNewProfile && (*i == SDLK_RETURN || *i == SDLK_KP_ENTER))
                {
                    showNewProfile = false;
                    if (!newProfileEditor->txt.empty())
                    {
#ifndef WIN32
                        string ph = settings.getDefaultDirectory() + SEPARATOR + newProfileEditor->txt + ".profile";
                        FILE *phile = fopen(ph.c_str(), "wb");
                        if (phile)
                            fclose(phile);
#endif

#ifdef WIN32
                        Settings::W32_CreateFile(settings.getDefaultDirectory() + SEPARATOR + newProfileEditor->txt + ".profile");
#endif
                        startMenuShowProfilesHandler(this);
                    }

                    delete(newProfileEditor);
                    return;
                }

                if (showBrowser || showSets)
                {
                    if (*i == SDLK_LEFT)
                    {
                        browserSelectedLevel--;
                        browserUsedKeyboard = true;
                    }

                    if (*i == SDLK_RIGHT)
                    {
                        browserSelectedLevel++;
                        browserUsedKeyboard = true;
                    }

                    if (*i == SDLK_TAB)
                    {
                        browserMenu.items[3]->Key(SDLK_RIGHT);
                    }

                    if (showSets)
                    {
                        clamp(browserSelectedLevel, 0, (int)(sets.size() - 1))
                    }
                    else
                    {
                        clamp(browserSelectedLevel, 0, (int)(currentLevelSet.levels.size() - 1))
                    }

                    if (browserUsedKeyboard)
                    {
                        browserScrollOffsetDest=(browserSelectedLevel/browserNLevelsPerPage) * browserNLevelsPerPage;
                    }

                    if (*i == SDLK_RETURN || *i == SDLK_KP_ENTER)
                    {
                        click = true;
                    }
                }
            }

        if (lmx != mx || lmy != my)
            browserUsedKeyboard = false;

        lmx = mx;
        lmy = my;

        if (events.buttDown[0])
        {
            if (browserMouseOverLevel != -1)
                click = true;
        }

        // we have to translate mouse events to local coords for the menu
        FrameEvents tempEvents = events;
        tempEvents.mouseX = mx;
        tempEvents.mouseY = my;

        if (showProfiles)
        {
            if (!showNewProfile)
                profileListMenu.Logic(tempEvents);

            // move menuactions to hovered item in profilelist
            int h = profileListMenu.getHoverItem();

            if (profileListMenu.items.size())
            {
                profileActionsMenu.items[1]->x = 0;
                profileActionsMenu.items[1]->y = profileListMenu.items[h]->y;
                profileActionsMenu.items[1]->width = 25;

                profileActionsMenu.items[1]->show = true;
                string ph = profileNames[profileListMenu.getHoverItem()];
                if (ph == profile.getName())
                {
                    profileActionsMenu.items[1]->show = false;
                }


                /*profileActionsMenu.items[2]->x = 70;
                profileActionsMenu.items[2]->y = profileListMenu.items[h]->y;
                profileActionsMenu.items[2]->width = 30;*/
            }
        }

        // have to strip the key events from browser menu
        if (!showBrowser && !showProfiles && !showSets)
        {
            currentMenu->Logic(tempEvents);
        }
        else
        {
            tempEvents.keyUp.clear();
            tempEvents.keyDown.clear();
            if (!showNewProfile)
                currentMenu->Logic(tempEvents);
        }


        if (showBrowser && browserSelectedLevel >= 0 && click)
        {
            currentMenu = &startMenu;
            renderHiscores = true;
            showBrowser = false;

            StopMusic();
            currentLevelSet = sets[selectedSet];

            bool survival = false;
            if (((OptionMenuItem*)browserMenu.items[3])->getV() == 1)
                survival = true;

            GameLoop(surface, ballText, browserSelectedLevel, Scenes::DEFAULT_FPS, survival).Run();
            resync = true;
            RecalculateMousePos();

            StartMusic();
            GLSetup();
            SDL_ShowCursor(SDL_ENABLE);
            SDL_WM_GrabInput(SDL_GRAB_OFF);
            hiScoreRep = hiScores.GenerateReport(linesPerHiScorePage);
            hiScorePage = 0;
            hiScoreTimeout = hiScoreWaitPageSec * desiredFPS;
        }

        if (showSets && browserSelectedLevel >= 0 && click)
        {
            showSets = false;
            showBrowser = true;
            browserScrollOffset = -5.0;
            browserScrollOffsetDest = 0;
            selectedSet = browserSelectedLevel;
            browserSelectedLevel = -1;
            currentLevelSet = sets[selectedSet];
            FillLevelDesc();
        }
    }

    for (int p = 0; p < nBallPaths; ++p)
    {
        bp[p].state.ballOut = false;
        bp[p].Logic();
    }

    if (showBrowser || showSets)
    {
        browserScrollOffset = browserScrollOffset + (browserScrollOffsetDest - browserScrollOffset) / 20.0;
    }

    if (renderHiscores)
    {
        if (hiScoreTimeout)
            hiScoreTimeout--;

        if (hiScoreTimeout == 0)
        {
            hiScorePage++;
            hiScoreTimeout = hiScoreWaitPageSec * desiredFPS;
            if (hiScorePage >= hiScoreRep.size())
            {
                hiScorePage = 0;
            }
        }
    }

    if (showCredits)
    {
        creditsScroll++;
        if (creditsScroll > (int)((credits.size() * 100)))
            creditsScroll = -creditsScrollClearance;
    }
}

void MainMenu::FillLevelDesc()
{
    if (nleveldesc > 0)
        delete [] levels;

    nleveldesc = sets[selectedSet].levels.size();
    levels = new LevelDesc[nleveldesc];
    if (nleveldesc > 0)
        levels[0].locked = false;

    for (int f = 0; f < nleveldesc; f++)
    {
        levels[f].lev = sets[selectedSet].levels[f];
        levels[f].tex = LoadTextureFile(sets[selectedSet].levels[f].thumbTexFilename.c_str());

        stringstream setname;
        setname << sets[selectedSet].filename << ":" << (f + 1) << ":completed";

        if (f < nleveldesc - 1)
        {
            levels[f + 1].locked = true;

            if (profile.getb(setname.str(), false))
            {
                levels[f+1].locked = false;
            }
        }
    }
}

void MainMenu::CenterMsg(string msg, double y, FTFont *font, double size)
{
    glLoadIdentity( );
    FTBBox b = font->BBox(msg.c_str());
    double tw = b.Upper().X() / (1.0 / size);
    glTranslated((100 - tw) / 2, y, 5);
    glScaled(size, size, size);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    font->Render(msg.c_str());
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

