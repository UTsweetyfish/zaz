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

#ifndef __MENU_H__
#define __MENU_H__

#include "common.h"
#include "frame_events.h"
#include <vector>
#include <string>
#include <sstream>

#define RENDERZHEIGHT 10

class MenuItem
{
    friend class Menu;

public:
    float x;
    float y;
    float width;
    float height;
    bool hover;
    bool show;

    MenuItem()
            :hover(false), show(true) {};
    virtual void Render() = 0;
    virtual void Click(float x, float y) = 0;
    virtual void Key(SDLKey k) {};
    virtual void Revert() {};
    virtual ~MenuItem() {};
};

class GenericMenuItem : public MenuItem
{
    void (*func)(void *);
    void *ptr;
protected:
    std::string text;

public:
    GenericMenuItem(const std::string txt, void (*func)(void *) = NULL, void *ptr = NULL)
            : func(func), ptr(ptr), text(txt) {};

    virtual void Click(float x, float y)
    {
        if (!show)
            return;

        hover=false;
        if (func != NULL)
            func(ptr);
    };

    virtual void Key(SDLKey k)
    {
        if (!show)
            return;

        if (k == SDLK_RETURN || k == SDLK_KP_ENTER)
        {
            if (func != NULL)
                func(ptr);
        }
    }

    virtual void Render();
};

class BooleanMenuItem : public GenericMenuItem
{
    Scenes::Settings *settings;
    std::string option;
    bool value;
    bool original;

public:
    BooleanMenuItem(const std::string txt, Scenes::Settings *settings, const std::string option)
            : GenericMenuItem(txt),
            settings(settings), option(option), value(settings->getb(option, false)), original(value) {};

    void Click(float x, float y)
    {
        value = !value;
        settings->setb(option, value);
    };

    void Key(SDLKey k)
    {
        if (k == SDLK_RETURN || k == SDLK_KP_ENTER || k == SDLK_SPACE)
            Click(0, 0);
    }

    void Render();
    void Revert()
    {
        settings->setb(option, original);
        value = original;
    }
};

class OptionMenuItem : public GenericMenuItem
{
    Scenes::Settings *settings;
    std::string option;
    std::vector<std::string>values;
    std::vector<std::string>descriptions;
    int GetVNum();
    uint v;
    uint originalv;
    std::string original;
    bool useDescriptions;

public:
    OptionMenuItem(const std::string txt, std::vector<std::string>values, const std::string option, Scenes::Settings *settings = NULL, std::vector<std::string>descriptions = std::vector<std::string>())
            : GenericMenuItem(txt),
            settings(settings), option(option),
            values(values), v(GetVNum()), originalv(v), original(values[v]), useDescriptions(!descriptions.empty()), descriptions(descriptions) {};

    int getV()
    {
        return v;
    }

    void Click(float x, float y)
    {
        v++;
        if (v >= values.size())
            v = 0;

        if (settings)
            settings->set(option, values[v]);
    };

    void Key(SDLKey k)
    {
        if (k == SDLK_RETURN || k == SDLK_KP_ENTER || k == SDLK_RIGHT)
            Click(0,0);

        if (k == SDLK_LEFT)
        {
            if (v > 0)
            {
                v--;
            }
            else v = values.size() - 1;

            if (settings)
                settings->set(option, values[v]);
        }


    }

    void Render();
    void Revert()
    {
        settings->set(option, original);
        v = GetVNum();
    }
};

class ValueMenuItem : public GenericMenuItem
{
    Scenes::Settings *settings;
    std::string option;
    int value;
    int original;
    int minimum;
    int maximum;

public:
    ValueMenuItem(const std::string txt, int minimum, int maximum, Scenes::Settings *settings, const std::string option)
            : GenericMenuItem(txt),
            settings(settings), option(option), value(atoi(settings->get(option, "").c_str())),
            original(value), minimum(minimum), maximum(maximum) {};

    void Click(float x, float y);
    void Key(SDLKey k);

    void Render();
    void Revert()
    {
        stringstream v;
        v << original;

        settings->set(option, v.str());
        value = original;
    }
};

class Menu
{
    float x;
    float y;
    double lmx, lmy;
    float width;
    const static int itemHeight = 5;
    uint hoverItem;
    bool posCalculated;

public:
    std::vector<MenuItem *>items;
    int lastClickedItem;
    Menu();
    ~Menu();
    Menu(float x, float y, float width);
    void SetDimensions(float x, float y, float width);
    void Add(MenuItem *item)
    {
        items.push_back(item);
        CalculatePositions();
    };

    void Clear()
    {
        std::vector<MenuItem *>::iterator i;

        for (i = items.begin(); i != items.end(); ++i)
            delete (*i);

        items.clear();
    }

    uint getHoverItem()
    {
        return hoverItem;
    };
    void setHoverItem(uint i);
    void Add(const std::string txt);
    void Render();
    void Logic(Scenes::FrameEvents events);
    void CalculatePositions();
};

#endif //__MENU_H__
