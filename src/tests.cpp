#include "tests.h"
#include "game.h"

Tests::Tests(SDL_Surface *surf)
        : Scene(surf), ed("test", 20, 45, 10)
{
    music = NULL;
}

Tests::~Tests()
{
    mixer->DisposeSample(music);
};

void Tests::Logic(ulong frame)
{
    if (music == NULL)
    {
        music = (Scenes::Sample *)new Scenes::StreamingOggSample(Game::getRandomMusic());
        mixer->EnqueueSample(music, 100, 0, true);
    }

    if (!events.empty)
    {
        if (events.keyDown.size() > 0)
            for (vector<SDLKey>::iterator i = events.keyDown.begin(); i != events.keyDown.end(); ++i)
            {
                if (*i == SDLK_ESCAPE)
                {
                    quit = true;
                }
            }
    }

    if (!quit)
        ed.Logic(events);

    if (frame > 100)
        quit = true;
}

void Tests::Render(ulong frame)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(1.0, 1.0, 1.0);
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity( );

    ed.Render();
}

void Tests::GLSetup()
{
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

    glClearColor( .1f, .1f, .7f, 1.0f );
    glViewport( 0, 0, width, height);
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );

    vwidth = 100 * (640.0/480.0);
    vleft = (100 - vwidth) / 2;
    vheight = 100.0;

    glOrtho(vleft, vwidth + vleft, 0, 100, -100, 100);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}
