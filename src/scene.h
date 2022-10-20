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

#ifndef __SCENE_H__
#define __SCENE_H__

#include "common.h"
#include "frame_events.h"
#include "mixer.h"
#include "ogvexport.h"
//#include "gameloop.h"

#include <fstream>

namespace Scenes
{
const int DEFAULT_FPS = 100;
const int MOUSE_RESOLUTION = 10000;

enum SceneMode {RUN, PLAY, RECORD, RENDER};

extern Mixer *mixer;

class Scene
{
protected:
    void RestartTimers();
    void SaveScreenshot(string philename);

    uint renderFPS;
    uint logicFPS;
    uint logicFrame;
    uint logicInputFrame;
    uint tickStart;
    uint restartFromFrame;
    uint renderframedecimator; // render every n'th frame when exporting

    double mx, my;
    double vwidth;
    double vleft;
    double vheight;

    double logicInterval;
    bool resync;
    bool screenshotRequested;
    string screenshotFilename;

    FrameEvents CreateFrameEvents();
    FrameEvents LoadFrameEvents();
    void SaveFrameEvents();
    void RecalculateMousePos(int mmx = -1, int mmy = -1);

    string nextEventLine;

    void Go();
    ostream *out_stream;
    istream *in_stream;
    MixerMode mixerMode;
    OgvExport *ogvexp;

protected:
    SceneMode mode;
    SDL_Surface *surface;
    bool quit;
    FrameEvents events;
    uint desiredFPS;

public:
    Scene(SDL_Surface *surf, uint fps = DEFAULT_FPS);
    bool show_fps;
    void Run();
    void Record(ostream &outstr, uint fromFrame);
    void Play(istream &instr);
    void Export(string philename, istream &instr, uint skipframes);
    void Screenshot(string philename);
    void Branch(Scene *scn);
    uint getLastLogicFrame()
    {
        return logicFrame;
    }

    virtual void GLSetup() = 0;
    virtual void Render(ulong frame) = 0;
    virtual void Logic(ulong frame) = 0;
    virtual ~Scene() {};
};

}

#endif // __SCENE_H__
