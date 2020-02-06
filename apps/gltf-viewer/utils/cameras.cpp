#include "cameras.hpp"
#include "glfw.hpp"

#include <iostream>

// Good reference here to map camera movements to lookAt calls
// http://learnwebgl.brown37.net/07_cameras/camera_movement.html

using namespace glm;

struct ViewFrame
{
  vec3 left;
  vec3 up;
  vec3 front;
  vec3 eye;

  ViewFrame(vec3 l, vec3 u, vec3 f, vec3 e) : left(l), up(u), front(f), eye(e)
  {
  }
};

ViewFrame fromViewToWorldMatrix(const mat4 &viewToWorldMatrix)
{
  return ViewFrame{-vec3(viewToWorldMatrix[0]), vec3(viewToWorldMatrix[1]),
      -vec3(viewToWorldMatrix[2]), vec3(viewToWorldMatrix[3])};
}

bool FirstPersonCameraController::update(float elapsedTime)
{
  if (glfwGetMouseButton(m_pWindow, GLFW_MOUSE_BUTTON_LEFT) &&
      !m_LeftButtonPressed) {
    m_LeftButtonPressed = true;
    glfwGetCursorPos(
        m_pWindow, &m_LastCursorPosition.x, &m_LastCursorPosition.y);
  } else if (!glfwGetMouseButton(m_pWindow, GLFW_MOUSE_BUTTON_LEFT) &&
             m_LeftButtonPressed) {
    m_LeftButtonPressed = false;
  }

  const auto cursorDelta = ([&]() {
    if (m_LeftButtonPressed) {
      dvec2 cursorPosition;
      glfwGetCursorPos(m_pWindow, &cursorPosition.x, &cursorPosition.y);
      const auto delta = cursorPosition - m_LastCursorPosition;
      m_LastCursorPosition = cursorPosition;
      return delta;
    }
    return dvec2(0);
  })();

  float truckLeft = 0.f;
  float pedestalUp = 0.f;
  float dollyIn = 0.f;
  float rollRightAngle = 0.f;

  if (glfwGetKey(m_pWindow, GLFW_KEY_W)) {
    dollyIn += m_fSpeed * elapsedTime;
  }

  // Truck left
  if (glfwGetKey(m_pWindow, GLFW_KEY_A)) {
    truckLeft += m_fSpeed * elapsedTime;
  }

  // Pedestal up
  if (glfwGetKey(m_pWindow, GLFW_KEY_UP)) {
    pedestalUp += m_fSpeed * elapsedTime;
  }

  // Dolly out
  if (glfwGetKey(m_pWindow, GLFW_KEY_S)) {
    dollyIn -= m_fSpeed * elapsedTime;
  }

  // Truck right
  if (glfwGetKey(m_pWindow, GLFW_KEY_D)) {
    truckLeft -= m_fSpeed * elapsedTime;
  }

  // Pedestal down
  if (glfwGetKey(m_pWindow, GLFW_KEY_DOWN)) {
    pedestalUp -= m_fSpeed * elapsedTime;
  }

  if (glfwGetKey(m_pWindow, GLFW_KEY_Q)) {
    rollRightAngle -= 0.001f;
  }
  if (glfwGetKey(m_pWindow, GLFW_KEY_E)) {
    rollRightAngle += 0.001f;
  }

  // cursor going right, so minus because we want pan left angle:
  const float panLeftAngle = -0.01f * float(cursorDelta.x);
  const float tiltDownAngle = 0.01f * float(cursorDelta.y);

  const auto hasMoved = truckLeft || pedestalUp || dollyIn || panLeftAngle ||
                        tiltDownAngle || rollRightAngle;
  if (!hasMoved) {
    return false;
  }

  m_camera.moveLocal(truckLeft, pedestalUp, dollyIn);
  m_camera.rotateLocal(rollRightAngle, tiltDownAngle, 0.f);
  m_camera.rotateWorld(panLeftAngle, m_worldUpAxis);

  return true;
}

bool TrackballCameraController::update(float elapsedTime)
{

  // First we need to gather user inputs: is the middle button pressed ?
  // How did the mouse moved since the last update ?
  // Is shift or ctrl pressed ? 
  // All of this must be done with functions from glfw in a way very similar to FirstPersonCameraController::update(). 
  // The beginning of the function should be almost identical except that we are intersted in 
  // the middle button instead of the left button of the mouse.
  if (glfwGetMouseButton(m_pWindow, GLFW_MOUSE_BUTTON_MIDDLE) && !m_MiddleButtonPressed) {
    m_MiddleButtonPressed = true;
    glfwGetCursorPos(m_pWindow, &m_LastCursorPosition.x, &m_LastCursorPosition.y);
  } else if (!glfwGetMouseButton(m_pWindow, GLFW_MOUSE_BUTTON_MIDDLE) && m_MiddleButtonPressed) {
    m_MiddleButtonPressed = false;
  }

  // Get cursor delta
  // Copied from FirstPersonCameraController::update
  const auto cursorDelta = ([&]() {
    if (m_MiddleButtonPressed) {
      dvec2 cursorPosition;
      glfwGetCursorPos(m_pWindow, &cursorPosition.x, &cursorPosition.y);
      const auto delta = cursorPosition - m_LastCursorPosition;
      m_LastCursorPosition = cursorPosition;
      return delta;
    }
    return dvec2(0);
  })();


  // Then we need to isolate three cases: 
  // Is shift pressed ? 
  //    In that case we want to pan the camera (lateral movement orthogonal to the view direction). 
  // Is ctrl pressed ? 
  //    In that case we want to zoom/unzoom toward/backward the target. 
  // Else if no keyboard key is pressed,
  //    We just rotate aroung the target.

  if (glfwGetKey(m_pWindow, GLFW_KEY_LEFT_SHIFT) || glfwGetKey(m_pWindow, GLFW_KEY_RIGHT_SHIFT) ) {
    // Is shift pressed ? 
    //    In that case we want to pan the camera (lateral movement orthogonal to the view direction). 

    const auto truckLeft = 0.01f * float(cursorDelta.x);
    const auto pedestalUp = 0.01f * float(cursorDelta.y);
    const auto hasMoved = truckLeft || pedestalUp;
    if (!hasMoved) {
      return false;
    }

    // Implement the "pan" case.
    // This one is the easier,
    // You just need to use the moveLocal method of m_camera with the correct parameters
    // (movement only on x and y axis of the camera, no move on z).
    m_camera.moveLocal(truckLeft, pedestalUp, 0.f);
    return true;

  } 
  if (glfwGetKey(m_pWindow, GLFW_KEY_LEFT_CONTROL) || glfwGetKey(m_pWindow, GLFW_KEY_RIGHT_CONTROL) ) {
    // Is ctrl pressed ? 
    //    In that case we want to zoom/unzoom toward/backward the target. 
    
    const auto dollyIn = 0.01f * float(cursorDelta.x);
    if (dollyIn == 0.f) {
      return false;
    }

    // The zoom case one is a bit harder, 
    // The camera need to move along its view axis, but only the eye should move, not the center.
    // We cannot use moveLocal because it moves both the eye and the center.
    // Also we need to be careful to always keep eye != center (but you can ignore that initially and fix it later).
    m_camera.dollyIn(dollyIn);

    return true;

  }

  // Else if no keyboard key is pressed,
  //    We just rotate aroung the target.

  const float horizontalAngle = -0.01f * float(cursorDelta.x);
  const float verticalAngle = 0.01f * float(cursorDelta.y);
  const auto hasMoved = horizontalAngle || verticalAngle;
  if (!hasMoved) {
    return false;
  }

  // Finally, the most difficult case is the rotate case.
  // Again the center of the camera need to stay the same, and only the eye moves around.
  // For that we need to perform two rotations:
  //    one vertical rotation around the horizontal axis of the camera (which can be obtained with m_camera.left()),
  //    and then an horizontal rotation around the world up axis.
  
  const auto depthAxis = m_camera.eye() - m_camera.center();
  const auto verticalRotationMatrix = rotate(mat4(1), verticalAngle, m_camera.left());
  const auto horizontalRotationMatrix = rotate(mat4(1), horizontalAngle, m_worldUpAxis);
  const auto combinedRotationMatrix = verticalRotationMatrix * horizontalRotationMatrix;
  const auto finalDepthAxis = vec3(combinedRotationMatrix * vec4(depthAxis, 0));
  const auto newEye = m_camera.center() + finalDepthAxis;

  m_camera = Camera(newEye, m_camera.center(), glm::vec3(0, 1, 0));

  return true;
}