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

#ifndef __GAME_H__
#define __GAME_H__

#include "common.h"
#include "scene.h"
#include "bezier.h"
#include "player.h"
#include "sample.h"
#include "ballpath.h"
#include "level.h"

using namespace Scenes;

extern Sample *sfx_pull;
extern Sample *sfx_push;
extern Sample *sfx_eliminate;
extern Sample *sfx_ouch;
extern Sample *sfx_combo;
extern Sample *sfx_roll;
extern Sample *sfx_bonus;
extern Sample *sfx_extraball;
extern Sample *sfx_extralife;

extern Mixer *mix;
extern AudioBuffer *rollSoundBuff;
extern int sfxVol;
extern int musicVol;

struct BallDesc
{
    const char *fileName;
    unsigned char r, g, b;
    int overlay;
};

extern BallDesc BallDescriptions[];

class Game : public Scene
{
    friend class GameLoop;
    friend class MainMenu;

    static const uint levelNameDisplayTimeout=500;
    static const uint waitAfterFinished=300;
    static const uint pointsForNewLife = 20000;
    static const uint pointsForRemainingLife = 5000;
    static const uint extraLifeDisplayTimeout=250;
    static const uint timeBonusScorePerSecond = 50;
    static const uint survivalColorsStart = 4;
    static const uint survivalNewColorTimeout = 120;

    SDL_Surface *surface;
    void CleanupSounds();

    Player *pl;
    BallPath **ballPaths;
    int nPaths;

    GLuint ballText[NBALLCOLORS + BONUS_BOMB + 2];

    bool dying;

    uint randomSeed;
    bool randomSeedSet;

    uint levelNameDisplayTime;
    bool displayLevelName;

    int finishedTimer;
    bool finishCountdown;
    Sample *music;
    AudioBuffer *mbuff;

    double vwidth;
    double vleft;
    double vheight;
    Level &level;

    bool editor;
    bool ownTextures;


    uint lives;

    bool displayExtraLife;
    uint extraLifeDisplayTime;
    int extraLifeLastScore;

    uint levelSeconds;
    bool stopTimer;
    bool timeBonus;
    int timeBonusSecondsAdded;
    bool renderingForThumb;
    bool survival;
    bool dyingSound;

public:
    Game(SDL_Surface *surf,  Level &level, GLuint *textures, uint randomSeed, uint lives, int score = 0,
         bool fromEditor = false, bool alwaysAccuracy = false, bool survivalMode = false);
    ~Game();
    static const string getRandomMusic();
    void Render(ulong frame);
    void Logic(ulong frame);
    void GLSetup();

    int score;
    bool gameOver;
    bool escaped;
};


#endif //__GAME_H__
