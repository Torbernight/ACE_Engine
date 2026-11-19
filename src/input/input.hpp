#pragma once 

#include <map>
#include <SDL_events.h>

typedef unsigned int Key;

enum MouseButton {
    MouseButton_Left,
    MouseButton_Right,
    MouseButton_Center
};

/* 
 * Singleton class for input handling
**/
class InputHandler
{
    public:

    static InputHandler& get()
    {
        static InputHandler singleton = InputHandler();
        return singleton;
    }

    // check wether a given key is pressed
    bool is_key_pressed(Key k) const;

    bool is_moused_button_pressed(MouseButton b) const;

    void get_mouse_position(int *x, int *y) const;

    void get_mouse_delta(int *dx, int *dy) const;

    // enable or disable input handling
    void set_enabled(bool c){
        m.enabled = c;
    }

    // returns true if input handling is enabled
    bool get_enabled() {
        return m.enabled;
    }

    void process_keyboard_event(const SDL_KeyboardEvent &event);
    void process_mouse_event(const SDL_Event &event);

    private:

    InputHandler() = default;

    struct {
        bool enabled = true;
        std::map<Key, bool> keys;
        int mouse_x, mouse_y;
        int mouse_delta_x, mouse_delta_y;
        int mouse_button_left, mouse_button_right, mouse_button_center;
        int mouse_scroll_x, mouse_scroll_y;
    } m;
};