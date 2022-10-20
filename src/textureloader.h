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

#ifndef __TEXTURELOADER_H__
#define __TEXTURELOADER_H__

int LoadTexture(const char *philename);
int LoadBallsTexture(const char *filename, unsigned char r, unsigned char g, unsigned char b, int cb);
int LoadBallsTextureFile(const char *filename, unsigned char r, unsigned char g, unsigned char b, int cb);
int LoadTextureFile(const char *philename);

#endif // __TEXTURELOADER_H__
