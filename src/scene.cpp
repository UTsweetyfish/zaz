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

#include "scene.h"
#include "mixer.h"

#include <ctime>
#include <cstdlib>
#include <sstream>

#include <GL/gl.h>

namespace Scenes
{
Mixer *mixer = NULL;

Scene::Scene(SDL_Surface *surf, uint fps)
{
    surface = surf;
    desiredFPS = fps;
    quit = resync = false;
    logicInputFrame = 0;
    renderframedecimator = 1;
    show_fps = settings.getb("showFps", false);
    restartFromFrame = 0;
    screenshotRequested = false;

    vwidth = 100 * (640.0/480.0);
    vleft = (100 - vwidth) / 2;
    vheight = 100.0;
}

void Scene::RestartTimers()
{
    tickStart = SDL_GetTicks() - ((restartFromFrame * 1000) / desiredFPS);
    logicFrame = restartFromFrame;

    logicInterval = 1000.0 / desiredFPS;
}

void Scene::Run()
{
    mode = RUN;
    mixerMode = Realtime;
    Go();
}

void Scene::Record(ostream &outstr, uint fromFrame)
{
    mode = RECORD;
    mixerMode = Realtime;
    out_stream = &outstr;
    restartFromFrame = fromFrame;
    Go();
}

void Scene::Play(istream &instr)
{
    mode = PLAY;
    mixerMode = Realtime;
    in_stream = &instr;
    Go();
}

void Scene::Export(string philename, istream &instr, uint skipframes)
{
    string phn;
#ifdef WIN32
    phn = Settings::W32_CreateFile(philename);
#else
    phn = philename;
#endif
    ogvexp = new OgvExport(phn, surface->w, surface->h, desiredFPS / skipframes);

    mode = RENDER;
    renderframedecimator = skipframes;
    mixerMode = Nonrealtime;
    in_stream = &instr;
    Mixer *oldmixer = mixer;
    mixer = NULL;
    Go();
    mixer = oldmixer;
    delete (ogvexp);
}

void Scene::Go()
{
    bool ownMixer = false;

    if (mixer == NULL)
    {
        mixer = new Mixer(mixerMode);
        ownMixer = true;
    }

    void *pixels;
    uint exportedFrame = 0;
    void *pcmbuffer;

    int pcmbuffersize = (44100 * 2 * 2) / (desiredFPS / renderframedecimator);

    if (mode == RENDER)
    {
        pixels = malloc(3 * surface->w * surface->h);
        pcmbuffer = malloc(pcmbuffersize);
    }

    GLSetup();
    RestartTimers();
    uint last_sec = tickStart / 1000;
    uint lfpsc = 0;
    uint rfpsc = 0;

    mixer->Play();

    while (!quit)
    {
        int currTick = SDL_GetTicks() - tickStart;
        if (currTick < 0)
        {

            currTick = 0;
        }

        uint currFrame = (currTick / 1000) * desiredFPS;
        uint now_sec = currTick / 1000;

        currFrame += (uint)(double(currTick % 1000) / 1000.0 * desiredFPS);

        if (mode == RENDER)
            currFrame = logicFrame+1;

        // are we ahead of time ?
        if (!resync)
            if (logicFrame >= currFrame)
            {
                SDL_Delay((Uint32)double(((logicFrame + 1) * logicInterval) - currTick));
                continue;
            }

        while (!resync && (logicFrame < currFrame))
        {
            if ((mode == RUN) || (mode == RECORD))
            {
                events = CreateFrameEvents();

                if (mode == RECORD)
                {
                    SaveFrameEvents();
                }
            }

            if ((mode == PLAY) || (mode == RENDER))
            {
                events = LoadFrameEvents();
                /*if (!events.empty)
                    SDL_WarpMouse(events.mouseX, events.mouseY);
                 */
            }

            Logic(logicFrame);
            ++logicInputFrame;
            ++logicFrame;
            lfpsc++;
        }

        if (mode != RENDER)
        {
            Render(logicFrame);

            if (screenshotRequested)
            {
                SaveScreenshot(screenshotFilename);
                screenshotRequested = false;
            }

            if (show_fps)
            {
                stringstream fpsmsg;
                fpsmsg << "fps: " << renderFPS;
                glLoadIdentity( );
                double vleft = (100 - 100 * (640.0/480.0)) / 2;

                glTranslated(vleft, 98, 50);
                glScaled(0.1, 0.1, 0.1);

                glColor3f(1.0f, 1.0f, 1.0f);
                glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
                if ((renderFPS < 200) && (renderFPS > 0))
                    font->Render(fpsmsg.str().c_str());
            }

            SDL_GL_SwapBuffers();
            rfpsc++;
        }

        if (resync)
        {
            resync = false;
            GLSetup();
            RestartTimers();
            continue;
        }

        if (mode == PLAY)
        {
            events = CreateFrameEvents();
            if (events.keyDown.size() > 0)
                for (vector<SDLKey>::iterator i = events.keyDown.begin(); i != events.keyDown.end(); ++i)
                {
                    if (*i == SDLK_ESCAPE)
                        quit = true;
                }
        }

        if (mode == RENDER)
        {
            events = CreateFrameEvents();

            if (events.keyDown.size() > 0)
                for (vector<SDLKey>::iterator i = events.keyDown.begin(); i != events.keyDown.end(); ++i)
                {
                    if (*i == SDLK_ESCAPE)
                        quit = true;
                }

            if (logicFrame%renderframedecimator == 0)
            {
                Render(logicFrame);

                glFlush();
                glPixelStorei(GL_PACK_ALIGNMENT, 1);
                glReadPixels(0, 0, surface->w, surface->h,
                             GL_RGB, GL_UNSIGNED_BYTE, pixels);
                mixer->Fill((Uint8 *)pcmbuffer, pcmbuffersize);

                ogvexp->FeedFrame(pixels, pcmbuffer);

                exportedFrame++;

                // add progress before showing the buffer
                char msg[256];
                sprintf(msg, "%s", _("Exporting video [press ESC to cancel]"));
                glLoadIdentity( );
                FTBBox b = font->BBox(msg);
                double tw = b.Upper().X() / 7;

                glTranslated((100 - tw) / 2, 40, 50);
                glScaled(1.0/7.0, 1.0/7.0, 1.0/7.0);

                glColor3f(1.0f, 1.0f, 1.0f);
                glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
                font->Render(msg);
                SDL_GL_SwapBuffers();
                rfpsc++;
            }
        }

        if (now_sec != last_sec)
        {
            last_sec = now_sec;
            renderFPS = rfpsc;
            logicFPS = lfpsc;
            lfpsc = rfpsc = 0;
        }
    }

    if (mode == RENDER)
    {
        free(pixels);
        free(pcmbuffer);
    }

    if (ownMixer)
    {
        delete mixer;
        mixer = 0;
    }
}

void Scene::Branch(Scene *scn)
{
    // no branching while recording/playing/rendering
    if (mode != RUN)
        return;

    //cout << "Leaving on frame :" << logicInputFrame << endl;
    scn->logicInputFrame = logicInputFrame + 1;
    scn->GLSetup();
    scn->Run();
    logicInputFrame = scn->logicInputFrame;
    resync = true;
}

FrameEvents Scene::CreateFrameEvents()
{
    SDL_Event event;
    FrameEvents ev;

    bool mmotion = false;
    int mmx = 0;
    int mmy = 0;
    int rmx = 0;
    int rmy = 0;


    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_KEYDOWN:
            ev.keyDown.push_back(event.key.keysym.sym);
            ev.empty = false;
            ev.unicode.push_back(event.key.keysym.unicode);
            break;
        case SDL_KEYUP:
            ev.keyUp.push_back(event.key.keysym.sym);
            ev.empty = false;
            break;
        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT)
                ev.buttDown[0] = true;
            if (event.button.button == SDL_BUTTON_RIGHT)
                ev.buttDown[1] = true;
            if (event.button.button == SDL_BUTTON_MIDDLE)
                ev.buttDown[2] = true;

            ev.empty = false;
            break;

        case SDL_MOUSEBUTTONUP:
            if (event.button.button == SDL_BUTTON_LEFT)
                ev.buttUp[0] = true;
            if (event.button.button == SDL_BUTTON_RIGHT)
                ev.buttUp[1] = true;
            if (event.button.button == SDL_BUTTON_MIDDLE)
                ev.buttUp[2] = true;

            ev.empty = false;
            break;

        case SDL_MOUSEMOTION:
            mmx = event.motion.x;
            mmy = event.motion.y;
            rmx = event.motion.xrel;
            rmy = event.motion.yrel;

            mmotion = true;
            break;
        }
    }

    if (mmotion)
    {
        if (logicInputFrame == 0)
            rmx = rmy = 0;

        ev.relmouseX = rmx;
        ev.relmouseY = rmy;

        ev.mouseX = (double)mmx / (double)surface->w;
        ev.mouseY = (double)mmy / (double)surface->h;

        // convert to int and back exactly as recording/playback would have done
        ev.mouseX = (double)((int)(ev.mouseX * MOUSE_RESOLUTION)) / MOUSE_RESOLUTION;
        ev.mouseY = (double)((int)(ev.mouseY * MOUSE_RESOLUTION)) / MOUSE_RESOLUTION;

        RecalculateMousePos(mmx, mmy);
        ev.empty = false;
    }

    return ev;
}

void Scene::RecalculateMousePos(int mmx, int mmy)
{
    if ((mmx == mmy) && (mmx == -1))
    {
        SDL_GetMouseState(&mmx, &mmy);
    }

    double mouseX = (double)mmx / (double)surface->w;
    double mouseY = (double)mmy / (double)surface->h;

    // convert to int and back exactly as recording/playback would have done
    mouseX = (double)((int)(mouseX * MOUSE_RESOLUTION)) / MOUSE_RESOLUTION;
    mouseY = (double)((int)(mouseY * MOUSE_RESOLUTION)) / MOUSE_RESOLUTION;

    mx = (vwidth * mouseX) + vleft;
    my = vheight - (vheight * mouseY);
}


FrameEvents Scene::LoadFrameEvents()
{
    FrameEvents ev;

    if (nextEventLine.empty())
        *in_stream >> nextEventLine;

    uint rec_frame = atoi(nextEventLine.c_str());

    if (rec_frame == logicInputFrame)
    {
        //cout << nextEventLine << endl;

        // split line in fields
        vector<string> fields;
        string::size_type startpos = 0;
        string::size_type endpos = string::npos;

        while ((endpos = nextEventLine.find_first_of(':', startpos)) != string::npos)
        {
            fields.push_back(nextEventLine.substr(startpos, (endpos - startpos)));
            startpos = endpos + 1;
        }

        fields.push_back(nextEventLine.substr(startpos));

        // interpret fields
        ev.empty = false;

        // keydown
        startpos = 0;
        endpos = string::npos;
        while ((endpos = fields[1].find_first_of(',', startpos)) != string::npos)
        {
            ev.keyDown.push_back((SDLKey)atoi(fields[1].substr(startpos, (endpos - startpos)).c_str()));
            startpos = endpos + 1;
        }

        if (fields[1].size() - startpos)
            ev.keyDown.push_back((SDLKey)atoi(fields[1].substr(startpos).c_str()));

        // keyup
        startpos = 0;
        endpos = string::npos;
        while ((endpos = fields[2].find_first_of(',', startpos)) != string::npos)
        {
            ev.keyUp.push_back((SDLKey)atoi(fields[2].substr(startpos, (endpos - startpos)).c_str()));
            startpos = endpos + 1;
        }

        if (fields[2].size() - startpos)
            ev.keyUp.push_back((SDLKey)atoi(fields[2].substr(startpos).c_str()));

        ev.mouseX = atoi(fields[3].c_str());
        ev.mouseY = atoi(fields[4].c_str());

        ev.mouseX = (double)ev.mouseX / (double) MOUSE_RESOLUTION;
        ev.mouseY = (double)ev.mouseY / (double) MOUSE_RESOLUTION;
        ev.relmouseX = atoi(fields[5].c_str());
        ev.relmouseY = atoi(fields[6].c_str());


        ev.buttDown[0] = fields[7][0] == '1';
        ev.buttDown[1] = fields[7][1] == '1';
        ev.buttDown[2] = fields[7][2] == '1';

        ev.buttUp[0] = fields[8][0] == '1';
        ev.buttUp[1] = fields[8][1] == '1';
        ev.buttUp[2] = fields[8][2] == '1';

        *in_stream >> nextEventLine;
    }

    return ev;
}

void Scene::SaveFrameEvents()
{
    if (!events.empty)
    {
        bool escape_hit = false;
        for (vector<SDLKey>::iterator i = events.keyDown.begin(); i != events.keyDown.end(); ++i)
        {
            if (*i == SDLK_ESCAPE)
                escape_hit = true;
        }

        if (escape_hit) // don't save the event
            return;

        *out_stream << logicInputFrame;
        // keydown
        *out_stream << ":";
        for (vector<SDLKey>::iterator i = events.keyDown.begin(); i != events.keyDown.end(); ++i)
        {
            if (i != events.keyDown.begin())
                *out_stream << ",";

            *out_stream << *i;
        }

        // keyup
        *out_stream << ":";
        for (vector<SDLKey>::iterator i = events.keyUp.begin(); i != events.keyUp.end(); ++i)
        {
            if (i != events.keyUp.begin())
                *out_stream << ",";

            *out_stream << *i;
        }

        // mouse position
        *out_stream << ":" << (int)(events.mouseX * MOUSE_RESOLUTION) << ":" << (int)(events.mouseY * MOUSE_RESOLUTION);
        *out_stream << ":" << events.relmouseX << ":" << events.relmouseY;
        // buttons
        *out_stream << ":" << events.buttDown[0] << events.buttDown[1] << events.buttDown[2];
        *out_stream << ":" << events.buttUp[0] << events.buttUp[1] << events.buttUp[2];

        *out_stream << endl;
    }
}

void Scene::SaveScreenshot(string philename)
{
    unsigned char *pixels;

    pixels = (unsigned char *)malloc(3 * surface->w * surface->h);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, surface->w, surface->h,
                 GL_RGB, GL_UNSIGNED_BYTE, pixels);


    Uint32 rmask, gmask, bmask;//, amask;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    //amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    //amask = 0xff000000;
#endif

    SDL_Surface *temp = SDL_CreateRGBSurface(SDL_SWSURFACE, surface->w, surface->h, 24, rmask, gmask, bmask, 0);

    for (int i=0; i<surface->h; i++)
        memcpy(((char *) temp->pixels) + temp->pitch * i,
               pixels + 3*surface->w * (surface->h-i-1),
               surface->w*3);
    free(pixels);
#ifdef WIN32
    SDL_SaveBMP(temp, Settings::W32_CreateFile(philename).c_str());
#else
    SDL_SaveBMP(temp, philename.c_str());
#endif
    SDL_FreeSurface(temp);
}

void Scene::Screenshot(string philename)
{
    screenshotFilename = philename;
    screenshotRequested = true;
}
}
