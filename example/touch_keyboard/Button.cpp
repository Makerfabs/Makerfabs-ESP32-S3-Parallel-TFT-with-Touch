#include "Button.h"

Button::Button()
{
    this->b_x = 0;
    this->b_y = 0;
    this->b_w = 0;
    this->b_h = 0;
    this->text = "";
    this->textSize = DEFAULT_TEXT_SIZE;
    this->value = UNABLE;
}

Button::Button(int x, int y, int w, int h, String text, int value, int textSize)
{
    this->b_x = x;
    this->b_y = y;
    this->b_w = w;
    this->b_h = h;
    this->text = text;
    this->textSize = textSize;
    this->value = value;
}

void Button::set(int x, int y, int w, int h, String text, int value, int textSize)
{
    this->b_x = x;
    this->b_y = y;
    this->b_w = w;
    this->b_h = h;
    this->text = text;
    this->textSize = textSize;
    this->value = value;
}

void Button::getFoDraw(int *x, int *y, int *w, int *h, String *text, int *textSize)
{
    *x = this->b_x;
    *y = this->b_y;
    *w = this->b_w;
    *h = this->b_h;
    *text = this->text;
    *textSize = this->textSize;
}

int Button::checkTouch(int x, int y)
{
    if (value == UNABLE)
    {
        return UNABLE;
    }
    else if (x > b_x && x < b_x + b_w && y > b_y && y < b_y + b_h)
    {
        return value;
    }
    else
        return UNABLE;
}

void Button::setText(String t)
{
    text = t;
}

String Button::getText()
{
    return text;
}

void Button::setText2(String t)
{
    text2 = t;
}

String Button::getText2()
{
    return text2;
}

void Button::setText3(String t)
{
    text3 = t;
}

String Button::getText3()
{
    return text3;
}

void Button::setValue(int v)
{
    value = v;
}

int Button::getValue()
{
    return value;
}

void Button::setTextSize(int textSize)
{
    this->textSize = textSize;
    ;
}