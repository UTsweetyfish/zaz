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

#include "common.h"
#include "editor.h"
#include "gameloop.h"
#include "mainmenu.h"
#include "hiscores.h"
#include "tests.h"
#include "profile.h"
#include "levelset.h"
#include "splash.h"
#include <fstream>
#include <vector>
#include <sys/stat.h>
#include <ctime>
#include <sstream>
#include <cstdlib>

#include "directorylister.h"

Scenes::Settings settings;
HiScores hiScores;

SDL_Surface *screen;
FTFont *font; // the default font for the game/engine/universe
//FTFont *font2;
FTFont *font3;
FTFont *font4;

bool wantReinit;
bool resReset;
bool audioHardwareInitialized;
SDL_Rect **screenModes = NULL;
GLuint pointer = 0;
int pointerFrame = 0;
double pointerRot = 0.0;

int main (int argc, char *argv[])
{
    bool showSplash = SHOW_SPLASH;
    bool editor = false;
    bool testplay = false;
    bool tests = false;
    char *editorPhilename = 0;
    char *testplayPhilename = 0;

    resReset = false;

    if (argc > 1)
    {
        if (strcmp(argv[1], "-e") == 0)
            editor = true;

        if (editor && argc == 3)
        {
            editorPhilename = argv[2];
        }

        if (strcmp(argv[1], "-p") == 0)
            testplay = true;

        if (testplay && argc == 3)
        {
            testplayPhilename = argv[2];
        }

        if (strcmp(argv[1], "-t") == 0)
            tests = true;

        if ((strcmp(argv[1], "-d") == 0) && (argc == 3))
        {
            settings.forcedDir = argv[2];
        }
    }

    try
    {
        char err[256];
        // create a game directory
#ifndef WIN32
        string dir = Settings::getDefaultDirectory ();
        mkdir(dir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
#else
        wchar_t wd[PATH_MAX];
        MultiByteToWideChar(CP_UTF8, 0, Settings::getDefaultDirectory().c_str(), -1, wd, MAX_PATH);
        _wmkdir(wd);
#endif

        // load default settings for things we might not ask too quickly about
        settings.get("mouseSensivity", "5");
        settings.get("keyboardSensivity", "5");
        settings.get("musicVolume", "80");
        settings.get("sfxVolume", "80");
        settings.get("disableSpeedup", "FALSE");

        if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
        {
            sprintf(err, "Could not initialize SDL:%s\n", SDL_GetError());
            ERR(err);
        }

        do
        {
            wantReinit = false;

            Uint32 fullscreen_flag = 0;

            if (settings.getb("fullscreen", false))
                fullscreen_flag = SDL_FULLSCREEN;

            const SDL_VideoInfo *vi = SDL_GetVideoInfo();
            uint bpp = vi->vfmt->BitsPerPixel;

            switch (bpp)
            {
            case 16:
                SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 4 );
                SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 4 );
                SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 4 );
                SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
                SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
                break;

            case 24:
            case 32:
                SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
                SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
                SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
//		        SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );
                SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
                SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
                break;

            default:
                ERR("This program requires at least 16bit display");
            }

            if (!screenModes)
                screenModes=SDL_ListModes(NULL, SDL_OPENGL | SDL_FULLSCREEN|SDL_HWSURFACE);

            string res = settings.get("resolution", "640x480");

            int vmx = atoi(res.c_str());
            int vmy = atoi(res.substr(res.find_first_of("x") + 1).c_str());

            screen = SDL_SetVideoMode(vmx, vmy, bpp, SDL_OPENGL | fullscreen_flag);

            if (screen == NULL)
            {
                // fallback to a safe resolution
                screen = SDL_SetVideoMode(640, 480, bpp, SDL_OPENGL);
                cout << "Falling back to 640x480 resolution" << endl;
                if (!screen)
                {
                    sprintf(err, "%s \n", SDL_GetError());
                    ERR(err);
                    return 1;
                }

                // if fallback worked, let's save the changes
                settings.setb("fullscreen", false);
                settings.set("resolution", "640x480");
            }

            SDL_WM_SetCaption("Zaz", NULL);

            glColor3f(1.0, 0, 1.0);
            font = new FTTextureFont(settings.getCFilename ("FreeSans.ttf"));
            if (font->Error())
                ERR("Could not open FreeSans.ttf [" + settings.getFilename ("FreeSans.ttf") + "]");

            font3 = new FTTextureFont(settings.getCFilename ("font1.ttf"));
            if (font->Error())
                ERR("Could not open font1.ttf [" + settings.getFilename ("font1.ttf") + "]");

            font4 = new FTTextureFont(settings.getCFilename ("FreeMonoBold.ttf"));
            if (font->Error())
                ERR("Could not open FreeMonoBold.ttf [" + settings.getFilename ("FreeMonoBold.ttf") + "]");

            font->FaceSize(24);
            font3->FaceSize(24);
            font4->FaceSize(24);

            // load profile
            profile = Profile(settings.get("last_profile", ""));
            settings.set("last_profile", profile.getName());
            profile.Save();

            if (testplay)
            {
                if (testplayPhilename != 0)
                {
                    cout << "Testing:" << testplayPhilename << endl;
                    Level level(testplayPhilename, true);
                    Game game(screen, level, 0, 3, 5);
                    game.Run();
                }
                else
                {
                    ERR("Must give a level to play");
                }

                showSplash = false;
            };

            if (editor)
            {
                char defPhn[] = "default";

                if (editorPhilename == 0)
                {
                    editorPhilename = defPhn;
                }


                Level level(editorPhilename, true);
                Editor ed(screen, level);
                ed.Run();
                showSplash = false;
            }

            if (tests)
            {
                Tests t(screen);
                t.Run();
                showSplash = false;
            }

            if (showSplash)
            {
                Splash s(screen);

                s.Run();
                /*stringstream str;
                str << "0:::0:0:0:0:000:000/n";
                s.Export("phbx.ogv", str, 1);
                return 0;
                */
                showSplash = false;

            }

            if (!editor && !testplay && !tests)
            {
                MainMenu m(screen);
                m.Run();
            }

            delete font;
            delete font3;
            delete font4;
//            SDL_Quit();

            profile.Save();
        }
        while (wantReinit);
    }
    catch (Error e)
    {
        ostringstream ss;
        ss << e.file << ":" << e.line << ":" << e.msg;
#ifdef WIN32
        const char caption[] = "Zaz exception";

        int l1 = MultiByteToWideChar(CP_UTF8, 0, ss.str().c_str(), -1, 0, 0);
        wchar_t wmsg[l1];

        int l2 = MultiByteToWideChar(CP_UTF8, 0, caption, -1, 0, 0);
        wchar_t wcap[l2];

        MultiByteToWideChar(CP_UTF8, 0, ss.str().c_str(), -1, wmsg, l1);
        MultiByteToWideChar(CP_UTF8, 0, caption, -1, wcap, l2);

        MessageBoxW(0, wmsg, wcap, MB_OK | MB_ICONERROR);
#endif
        cout << ss.str() << endl;
        return(1);
    }

    settings.Save();
    profile.Save();

    if (Scenes::mixer)
        delete Scenes::mixer;

    SDL_Quit();

    return(0);
}
