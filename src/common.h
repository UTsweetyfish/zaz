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

#pragma once
#ifndef __COMMON_H__

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef WIN32
#include <windows.h>
#include <direct.h>
#endif

typedef unsigned int uint;
typedef unsigned long ulong;

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <SDL/SDL.h>
#include <FTGL/ftgl.h>
#include <GL/gl.h>

#include "gettext.h"
#include "error.h"
#include "settings.h"

extern SDL_Surface *screen;
extern FTFont *font;
//extern FTFont *font2;
extern FTFont *font3;
extern FTFont *font4;

extern Scenes::Settings settings;
extern bool wantReinit;
extern bool resReset;

extern SDL_Rect **screenModes;
extern GLuint pointer;
extern int pointerFrame;
extern double pointerRot;

// some useful tools for text processing - defined in settings.cpp
vector<string> Split(string str, string sep);
void Strip(string &str);

#define PI 3.1415926535

#define iround(x) floor((x)+0.5)

#define _(string) gettext(string)

#define clamp(a, b, c) if (a < b) a = b; if (a > c) a = c;

#ifndef VERSION
#define VERSION "1.0.1"
#endif

#ifdef ENABLE_SPLASH
#define SHOW_SPLASH ENABLE_SPLASH
#else
#define SHOW_SPLASH true
#endif

#define __COMMON_H__
#endif // __COMMON_H__
