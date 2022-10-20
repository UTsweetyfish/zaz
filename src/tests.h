#ifndef __TESTS_H__
#define __TESTS_H__

#include "common.h"
#include "scene.h"
#include "lineeditor.h"

class Tests : public Scenes::Scene
{
public:
    Scenes::LineEditor ed;
    Tests(SDL_Surface *surf);
    ~Tests();

    Scenes::Sample *music;

    double vwidth;
    double vleft;
    double vheight;

    void GLSetup();
    void Render(ulong frame);
    void Logic(ulong frame);
};

#endif //__TESTS_H__
