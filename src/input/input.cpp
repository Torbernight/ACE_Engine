#include "input.hpp"
#include "SDL_assert.h"


void InputHandler::begin_new_frame()
{
    for(auto const& kv : m.keys){
        unsigned int key = kv.first;
        KeyState val = kv.second;
        if (val == KEY_RELEASED){
            m.keys[key] = KEY_NOT_PRESSED;
        } else if (val == KEY_PRESSED){
            m.keys[key] = KEY_HELD;
        }
    }
    m.mouse_delta_x = m.mouse_delta_y = 0;
}

bool InputHandler::is_key_pressed(Key k) const
{
    if (m.keys.count(k) == 0){
        return false;
    }

    KeyState state = m.keys.at(k);
    return state == KEY_PRESSED || state == KEY_HELD;
}

bool InputHandler::is_key_held(Key k) const 
{
    if (m.keys.count(k) == 0){
        return false;
    }

    KeyState state = m.keys.at(k);
    return state == KEY_HELD;
}

bool InputHandler::is_key_just_pressed(Key k) const 
{
    if (m.keys.count(k) == 0){
        return false;
    }

    KeyState state = m.keys.at(k);
    return state == KEY_PRESSED;
}


bool InputHandler::is_key_just_released(Key k) const 
{
    if (m.keys.count(k) == 0){
        return false;
    }

    KeyState state = m.keys.at(k);
    return state == KEY_RELEASED;
}


bool InputHandler::is_key_not_pressed(Key k) const 
{
    if (m.keys.count(k) == 0){
        return false;
    }

    KeyState state = m.keys.at(k);
    return state == KEY_NOT_PRESSED;
}



bool InputHandler::is_moused_button_pressed(MouseButton b)  const
{
    switch(b)
    {
        case MouseButton_Center:
            return m.mouse_button_center;
        case MouseButton_Left:
            return m.mouse_button_left;
        case MouseButton_Right:
            return m.mouse_button_right;
    }
    
    return false; // shouldn't get here ever
}

void InputHandler::get_mouse_position(int *x, int *y) const
{
    SDL_assert(x && y);
    *x = m.mouse_x;
    *y = m.mouse_y;
}

void InputHandler::get_mouse_delta(int *dx, int *dy) const
{
    SDL_assert(dx && dy);
    *dx = m.mouse_delta_x;
    *dy = m.mouse_delta_y;
}

void InputHandler::process_keyboard_event(const SDL_KeyboardEvent &event)
{
    if (!m.enabled)
        return;

    bool pressed = event.type == SDL_KEYDOWN;
    int key = event.keysym.sym;

    m.keys[key] = pressed ? KEY_PRESSED : KEY_RELEASED;
    
}

void InputHandler::process_mouse_event(const SDL_Event &event)
{
    if (!m.enabled)
        return;

    switch(event.type)
    {
        case SDL_MOUSEMOTION:
            m.mouse_x = event.motion.x;
            m.mouse_y = event.motion.y;
            m.mouse_delta_x = event.motion.xrel;
            m.mouse_delta_y = event.motion.yrel;
            break;
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
            switch (event.button.button)
            {
                case SDL_BUTTON_LEFT:
                    m.mouse_button_left = (event.type == SDL_MOUSEBUTTONDOWN);
                    break;
                case SDL_BUTTON_RIGHT:
                    m.mouse_button_right = (event.type == SDL_MOUSEBUTTONDOWN);
                    break;
                case SDL_BUTTON_MIDDLE:
                    m.mouse_button_center = (event.type == SDL_MOUSEBUTTONDOWN);
                    break;
            }
            break;
        case SDL_MOUSEWHEEL:
            m.mouse_scroll_x = event.wheel.x;
            m.mouse_scroll_y = event.wheel.y;
            break;
        default:
            break;
    }
}

// returns true if input handling is enabled