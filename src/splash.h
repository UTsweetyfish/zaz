#ifndef __SPLASH_H__
#define __SPLASH_H__

#include "common.h"
#include "scene.h"

class Splash : public Scenes::Scene
{
public:
    Splash(SDL_Surface *surf, uint fps = 25);
    ~Splash();

    Scenes::Sample *music;

    GLuint logoTex;
    GLuint logoOverlayTex;

    double vwidth;
    double vleft;
    double vheight;

    bool showOverlay;
    double overX;
    double overY;
    double overR;

    static const ulong showFrames = 150;
    static const ulong fadeInFrames = 10;
    static const ulong fadeOutFrames = 10;
    static const int overlayShift = 4;
    static const int overlayRot = 2;

    void GLSetup();
    void Render(ulong frame);
    void Logic(ulong frame);
};

#endif //__SPLASH_H__
