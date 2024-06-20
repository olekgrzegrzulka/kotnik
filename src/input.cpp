#include "input.hpp"
#include <GLFW/glfw3.h>
#include "common.hpp"
#include "glm/glm.hpp"

namespace Input {

namespace detail {
static GLFWwindow* glfw_window{};
static int mouse_x;
static int mouse_y;
static int last_mouse_x;
static int last_mouse_y;
} // namespace detail

void glfw_cursor_position_callback(GLFWwindow* window, double x, double y) {
  // detail::last_mouse_x = detail::mouse_x;
  // detail::last_mouse_y = detail::mouse_y;
  // detail::mouse_x = x;
  // detail::mouse_y = y;
}

void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
}

void glfw_scroll_button_callback(GLFWwindow* window, double xoffset, double yoffset) {
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
  detail::mouse_x = (int)x;
  detail::mouse_y = (int)y;
}

bool mouse_pressed(Mouse button) {
  return glfwGetMouseButton(detail::glfw_window, (int)button) == GLFW_PRESS;
}

bool key_pressed(Key key) {
  return glfwGetKey(detail::glfw_window, (int)key) == GLFW_PRESS;
}

glm::vec<2, int> get_mouse_delta() {
  return {
      detail::mouse_x - detail::last_mouse_x,
      detail::mouse_y - detail::last_mouse_y};
}

}; // namespace Input
