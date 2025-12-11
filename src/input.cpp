#include "input.hpp"

#include <fmt/core.h>

namespace input {
    void Input::update() {
        mouse_delta = mouse_pos - last_mouse_pos;
        just_updated_keys.clear();
        last_mouse_pos = mouse_pos;
    }

    void Input::register_move_event(glm::vec2 new_mouse_pos) {
        mouse_pos = new_mouse_pos;
    }

    void Input::register_event(Key key, KeyState state) {
        keyboard_state.insert_or_assign(key, state);
        just_updated_keys.emplace(key);
    }

    Input::Input() : last_mouse_pos(0.0f), mouse_pos(0.0f), mouse_delta(0.0f) {}

    glm::vec2 Input::get_mouse_position() const {
        return mouse_pos;
    }

    glm::vec2 Input::get_mouse_delta() const {
        return mouse_delta;
    }

    bool Input::is_key(Key key, KeyState state, bool should_be_just_updated) const {
        auto it = keyboard_state.find(key);
        if (it != keyboard_state.end()) {
            if (should_be_just_updated == false) {
                return state == it->second;
            } else {
                auto it_updated = just_updated_keys.find(key);
                return it_updated != just_updated_keys.end();
            }
        }

        return false;
    }

    bool Input::is_key_pressed(Key key) const {
        return is_key(key, KeyState::PRESSED);
    }

    bool Input::is_key_just_pressed(Key key) const {
        return is_key(key, KeyState::PRESSED, true);
    }

    bool Input::is_key_just_released(Key key) const {
        return is_key(key, KeyState::RELEASED, true);
    }
}
