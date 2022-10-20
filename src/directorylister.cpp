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

#include "directorylister.h"
#ifndef WIN32

#include <sys/stat.h>

vector<string> ListFiles(string dir, const string extension)
{
    vector<string> ret;
    DIR *d = opendir(dir.c_str());

    if (!d)
        return ret;

    struct dirent *file = readdir(d);
    while (file)
    {
        bool isFile = false;

        if (file->d_type == DT_REG)
            isFile = true;

        if (!isFile)
        { // do additional test
            struct stat s;
            string pth = dir + "/" + file->d_name;

            if (!stat(pth.c_str(), &s))
                isFile = S_ISREG(s.st_mode);
        }

        if (isFile)
        {
            string ext = file->d_name;
            size_t extpos = ext.find(extension);

            if (extpos != string::npos)
            {
                if ((extpos == ext.size() - extension.size()) || extension.empty())
                {
                    ret.push_back(file->d_name);
                }
            }
        }

        file = readdir(d);
    }

    closedir(d);
    return ret;
};
#endif

#ifdef WIN32

#include <windows.h>

vector<string> ListFiles(string dir, const string extension)
{
    vector<string> ret;
    WIN32_FIND_DATAW FindFileData;
    HANDLE hFind;

    wchar_t ff[1024];
    string s;

    s = dir + "\\*";

    if (extension.empty())
    {
        s+= ".*";
    }
    else
    {
        s+= extension;
    }

    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, ff, MAX_PATH);

    hFind = FindFirstFileW(ff, &FindFileData);
    bool found = (hFind != INVALID_HANDLE_VALUE);
    while (found)
    {
        char utf8name[1024];

        WideCharToMultiByte(CP_UTF8, 0, FindFileData.cFileName, -1, utf8name, MAX_PATH, NULL, NULL);
        ret.push_back(utf8name);


        found = FindNextFileW(hFind, &FindFileData);
    }

    if (hFind != INVALID_HANDLE_VALUE)
    {
        FindClose(hFind);
    }

    return ret;
}
#endif

