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

#ifndef __ERROR_H__
#define __ERROR_H__

class Error
{
public:
    std::string msg;
    int line;
    std::string file;

    Error(std::string s, int l, std::string f)
            : msg(s), line(l), file(f)
    {
        file = file.substr(file.find_last_of("\\/") + 1);
    };
};


#define ERR(a) throw(Error(a, __LINE__, __FILE__))

#endif // __ERROR_H__
