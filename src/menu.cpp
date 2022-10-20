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

#include <iostream>

#include "menu.h"

#define RENDERHEIGHT 5

Menu::Menu(float x, float y, float width)
        : x(x), y(y), lmx(0.0), lmy(0.0), width(width), hoverItem(0), posCalculated(false), lastClickedItem(0)
{
};

Menu::Menu()
        : lmx(0.0), lmy(0.0), hoverItem(0), posCalculated(false), lastClickedItem(0)
{

};

Menu::~Menu()
{
    Clear();
};

void Menu::Add(const std::string txt)
{
    Add(new GenericMenuItem(txt));
}

void Menu::SetDimensions(float x, float y, float width)
{
    this->x = x;
    this->y = y;
    this->width = width;
    CalculatePositions();
    posCalculated = true;

    if (items.size() > 0)
    {
        if (hoverItem < items.size() - 1)
        {
            items[hoverItem]->hover = true;
        }
        else
        {
            items[0]->hover = true;
        }
    }
}

void Menu::Render()
{
    // render items
    std::vector<MenuItem *>::iterator i;

    for (i = items.begin(); i != items.end(); ++i)
    {
        (*i)->Render();
    }
};

void Menu::CalculatePositions()
{
    std::vector<MenuItem *>::iterator i;

    int yy = 0;
    for (i = items.begin(); i != items.end(); ++i)
    {
        (*i)->x = x;
        (*i)->y = y - (itemHeight + 1) * yy;
        (*i)->width = width;
        (*i)->height = itemHeight;

        ++yy;
    }
};

void Menu::setHoverItem(uint i)
{
    hoverItem = i;
    for (uint f = 0; f < items.size(); ++f)
    {
        items[f]->hover = false;
    }

    if (items.size() > 0)
        items[hoverItem]->hover = true;
}

void Menu::Logic(Scenes::FrameEvents events)//double mx, double my, bool click)
{
    if (!posCalculated)
    {
        CalculatePositions();
        posCalculated = true;
    }

    double mx = events.mouseX;
    double my = events.mouseY;
    bool click = false;
    bool mouseMoved = false;

    if (lmx != mx)
        mouseMoved = true;

    if (lmy != my)
        mouseMoved = true;

    if (events.buttDown[0])
    {
        click = true;
        mouseMoved = true;
    }

    // fix hovers triggered by mouse
    if (mouseMoved)
        for (uint f = 0; f < items.size(); ++f)
        {
            //std::cout << items[f]->x << ":" << items[f]->y << " - " << items[f]->width << ":" << items[f]->height << " --- " << mx << ":" << my << std::endl;

            if ((items[f]->x < mx)
                    && (items[f]->x + items[f]->width > mx)
                    && (items[f]->y > my)
                    && (items[f]->y - items[f]->height < my))
            {
                hoverItem = f;
            }
        }

    // fix hovers by keyboard
    if (!events.empty)
    {
        if (events.keyDown.size() > 0)
            for (vector<SDLKey>::iterator i = events.keyDown.begin(); i != events.keyDown.end(); ++i)
            {
                bool sendKey = true;

                if (*i == SDLK_UP)
                {
                    sendKey = false;
                    if (hoverItem > 0)
                    {
                        hoverItem--;
                    }
                    else hoverItem = items.size() - 1;
                }

                if (*i == SDLK_DOWN)
                {
                    sendKey = false;
                    hoverItem++;
                    if (hoverItem == items.size())
                        hoverItem = 0;
                }

                if (!items.empty())
                    if (sendKey && hoverItem < items.size())
                        items[hoverItem]->Key(*i);
            }
    }

    for (uint f = 0; f < items.size(); ++f)
    {
        items[f]->hover = false;
    }

    if (items.size() > 0)
        items[hoverItem]->hover = true;

    if (click)
    {
        for (uint f = 0; f < items.size(); ++f)
        {
            //std::cout << items[f]->x << ":" << items[f]->y << " - " << items[f]->width << ":" << items[f]->height << " --- " << mx << ":" << my << std::endl;

            if ((items[f]->x < mx)
                    && (items[f]->x + items[f]->width > mx)
                    && (items[f]->y > my)
                    && (items[f]->y - items[f]->height < my))
                items[f]->Click((int)mx - items[f]->x, (int)my - items[f]->y);

            lastClickedItem = f;
        }
    }

    lmx = mx;
    lmy = my;
};

void OptionMenuItem::Render()
{
    if (!show)
        return;

    GenericMenuItem::Render();
    glPushMatrix();
    const char *val = values[v].c_str();

    if (useDescriptions)
    {
        val = descriptions[v].c_str();
    }

    FTBBox b = font->BBox(val);
    glTranslated(x + (width - (b.Upper().X() / 6.67)) - 2, y - height / 2 - 1, RENDERZHEIGHT + 1);
    glScaled(0.15, 0.15, 0.15);
    font->Render(val);
    glPopMatrix();
}

int OptionMenuItem::GetVNum()
{
    if (!settings)
        return 0;

    std::string opt = settings->get(option, "");

    for (uint f = 0; f < values.size(); ++f)
    {
        if (values[f] == opt)
            return f;
    }

    return 0;
}

void BooleanMenuItem::Render()
{
    if (!show)
        return;

    GenericMenuItem::Render();

    // draw tickbox
    glLineWidth(2.0);
    glPushMatrix();
    glTranslatef(x + (width - 5), y - 1, RENDERZHEIGHT + 1);
    glBegin(GL_LINE_STRIP);
    glVertex3d(0, 0, 0);
    glVertex3d(0, -3, 0);
    glVertex3d(3, -3, 0);
    glVertex3d(3, 0, 0);
    glVertex3d(0, 0, 0);
    glEnd();

    if (value)
    {
        glBegin(GL_LINE_STRIP);
        glVertex3d(0.5, -1.5, 0);
        glVertex3d(1.5, -2.5, 0);
        glVertex3d(2.5, 0, 0);
        glEnd();
    }

    glPopMatrix();
};

void ValueMenuItem::Render()
{
    if (!show)
        return;

    GenericMenuItem::Render();
    glLoadIdentity();
    glLineWidth(2.0);
    glPushMatrix();
    glTranslatef(x + (width / 2), y - 1, RENDERZHEIGHT + 1);
    glBegin(GL_LINE_STRIP);
    glVertex3d(0, 0, 0);
    glVertex3d(0, -3, 0);
    glVertex3d(width / 2 - 2, -3, 0);
    glVertex3d(width / 2 - 2, 0, 0);
    glVertex3d(0, 0, 0);
    glEnd();

    double xx = (double)value / (double)maximum * (width / 2 - 2);

    glBegin(GL_QUADS);
    glVertex3d(0, 0, 0);
    glVertex3d(0, -3, 0);
    glVertex3d(xx, -3, 0);
    glVertex3d(xx, 0, 0);
    glEnd();
    glPopMatrix();
};

void ValueMenuItem::Click(float x, float y)
{
    std::stringstream vs;

    double xx = width / 2 - 3;
    if (x < width / 2)
        return;

    value = (int)iround(((x - width / 2.0f) / xx) * (float)maximum);

    if (value < minimum)
        value = minimum;

    if (value > maximum)
        value = maximum;

    vs << value;

    settings->set(option, vs.str());
};

void ValueMenuItem::Key(SDLKey k)
{
    std::stringstream vs;

    if (k == SDLK_RIGHT)
        value++;

    if (k == SDLK_LEFT)
        value--;

    if (k == SDLK_END)
        value = maximum;

    if (k == SDLK_HOME)
        value = minimum;

    if (k == SDLK_PAGEUP)
        value += (maximum - minimum) / 10;

    if (k == SDLK_PAGEDOWN)
        value -= (maximum - minimum) / 10;

    if (value < minimum)
        value = minimum;

    if (value > maximum)
        value = maximum;

    vs << value;

    settings->set(option, vs.str());
}

void GenericMenuItem::Render()
{
    if (!show)
        return;

    glLoadIdentity();
    if (hover)
    {
        glPushMatrix();
        glTranslatef(x, y, RENDERZHEIGHT);
        glBegin(GL_QUADS);
        glColor3f( .3, .3, 1.0);
        glVertex3d(0, 0, 0);
        glVertex3d(0, -height, 0);
        glColor3d( .1, .1, .7);
        glVertex3d(width, -height, 0);
        glVertex3d(width, 0, 0);
        glEnd();

        glTranslatef(0.5, -0.5, -1);
        glBegin(GL_QUADS);
        glColor3f( 0, 0, .2);
        glVertex3d(0, 0, 0);
        glVertex3d(0, -height, 0);
        glVertex3d(width, -height, 0);
        glVertex3d(width, 0, 0);
        glEnd();


        glPopMatrix();
    }
    else
    {
        glPushMatrix();
        glTranslatef(x, y, RENDERZHEIGHT);
        glBegin(GL_QUADS);
        glColor3d( 0, 0, 0.5);
        glVertex3d(0, 0, 0);
        glVertex3d(0, -height, 0);
        glColor3d( .1, .1, .7);
        glVertex3d(width, -height, 0);
        glVertex3d(width, 0, 0);
        glEnd();

        glTranslatef(0.5, -0.5, -1);
        glBegin(GL_QUADS);
        glColor3f( 0, 0, .2);
        glVertex3d(0, 0, 0);
        glVertex3d(0, -height, 0);
        glVertex3d(width, -height, 0);
        glVertex3d(width, 0, 0);
        glEnd();


        glPopMatrix();
    }

    if (hover)
    {
        glColor3d(1.0, 1.0, 1.0);
    }
    else
    {
        glColor3d(0.5, 0.5, 0.5);
    }
    if (!text.empty())
    {
        glPushMatrix();
        glTranslated(x + 1, y - height / 2 - 1, RENDERZHEIGHT + 1);
        glScaled(0.15, 0.15, 0.15);
        font->Render(text.c_str());
        glPopMatrix();
    }
};

