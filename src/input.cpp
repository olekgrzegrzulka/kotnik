#include "input.hpp"
#include <GLFW/glfw3.h>
#include "common.hpp"

namespace Input {

namespace detail {
static GLFWwindow* glfw_window{};
static int mouse_x;
static int mouse_y;
static int last_mouse_x;
static int last_mouse_y;

enum class ButtonState {
  RELEASED,
  PRESSED,
  JUST_RELEASED,
  JUST_PRESSED,
};

std::array<ButtonState, (size_t)Mouse::MOUSE_BUTTON_SIZE> mouse_states;
std::array<ButtonState, (size_t)Key::KEY_SIZE> key_states;
} // namespace detail

void glfw_cursor_position_callback(GLFWwindow*, double, double) {
  // detail::last_mouse_x = detail::mouse_x;
  // detail::last_mouse_y = detail::mouse_y;
  // detail::mouse_x = x;
  // detail::mouse_y = y;
}

void glfw_mouse_button_callback(GLFWwindow*, i32, i32, i32) {
}

void glfw_scroll_button_callback(GLFWwindow*, double, double) {
}

void init(GLFWwindow* window) {
  detail::glfw_window = window;
  glfwSetCursorPosCallback(window, glfw_cursor_position_callback);
  glfwSetMouseButtonCallback(window, glfw_mouse_button_callback);
  glfwSetScrollCallback(window, glfw_scroll_button_callback);
}

void update() {
  double x, y;
  glfwGetCursorPos(detail::glfw_window, &x, &y);
  detail::last_mouse_x = detail::mouse_x;
  detail::last_mouse_y = detail::mouse_y;
  detail::mouse_x = (i32)x;
  detail::mouse_y = (i32)y;

  for (size_t i = 0; i < detail::mouse_states.size(); i += 1) {
    using enum detail::ButtonState;
    bool is_pressed = glfwGetMouseButton(detail::glfw_window, i) == GLFW_PRESS;

    if (detail::mouse_states[i] == RELEASED && is_pressed) {
      detail::mouse_states[i] = JUST_PRESSED;
    } else if (detail::mouse_states[i] == PRESSED && !is_pressed) {
      detail::mouse_states[i] = JUST_RELEASED;
    } else if (detail::mouse_states[i] == JUST_RELEASED) {
      if (!is_pressed) {
        detail::mouse_states[i] = RELEASED;
      } else {
        detail::mouse_states[i] = JUST_PRESSED;
      }
    } else if (detail::mouse_states[i] == JUST_PRESSED) {
      if (is_pressed) {
        detail::mouse_states[i] = PRESSED;
      } else {
        detail::mouse_states[i] = JUST_RELEASED;
      }
    }
  }

  for (size_t i = 0; i < detail::key_states.size(); i += 1) {
    using enum detail::ButtonState;
    bool is_pressed = glfwGetKey(detail::glfw_window, i) == GLFW_PRESS;

    if (detail::key_states[i] == RELEASED && is_pressed) {
      detail::key_states[i] = JUST_PRESSED;
    } else if (detail::key_states[i] == PRESSED && !is_pressed) {
      detail::key_states[i] = JUST_RELEASED;
    } else if (detail::key_states[i] == JUST_RELEASED) {
      if (!is_pressed) {
        detail::key_states[i] = RELEASED;
      } else {
        detail::key_states[i] = JUST_PRESSED;
      }
    } else if (detail::key_states[i] == JUST_PRESSED) {
      if (is_pressed) {
        detail::key_states[i] = PRESSED;
      } else {
        detail::key_states[i] = JUST_RELEASED;
      }
    }
  }
  // clang-format on
}

int get_mouse_x() {
  return detail::mouse_x;
}

int get_mouse_y() {
  return detail::mouse_y;
}

bool mouse_pressed(Mouse button) {
  using enum detail::ButtonState;
  auto state = detail::mouse_states[(size_t)button];
  return state == JUST_PRESSED || state == PRESSED;
}

bool mouse_just_pressed(Mouse button) {
  return detail::mouse_states[(size_t)button] == detail::ButtonState::JUST_PRESSED;
}

bool mouse_just_released(Mouse button) {
  return detail::mouse_states[(size_t)button] == detail::ButtonState::JUST_RELEASED;
}

bool key_pressed(Key button) {
  using enum detail::ButtonState;
  auto state = detail::key_states[(size_t)button];
  return state == JUST_PRESSED || state == PRESSED;
}

bool key_just_pressed(Key button) {
  return detail::key_states[(size_t)button] == detail::ButtonState::JUST_PRESSED;
}

bool key_just_released(Key button) {
  return detail::key_states[(size_t)button] == detail::ButtonState::JUST_RELEASED;
}

glm::vec<2, i32> get_mouse_delta() {
  return {
      detail::mouse_x - detail::last_mouse_x,
      detail::mouse_y - detail::last_mouse_y};
}

}; // namespace Input
