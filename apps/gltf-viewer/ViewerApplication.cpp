#include "ViewerApplication.hpp"
#include "cout_colors.hpp"

#include <iostream>
#include <numeric>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>

#include "utils/cameras.hpp"

#include <stb_image_write.h>
#include <tiny_gltf.h>

void keyCallback(
    GLFWwindow *window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) {
    glfwSetWindowShouldClose(window, 1);
  }
}

/*
Part 1 : 
Implement a method bool loadGltfFile(tinygltf::Model & model);
that should use m_gltfFilePath.string() to load the correct file into the model structure.
The method return value should correspond to what gltf returns.
If there is errors or warning, print them.
*/
bool ViewerApplication::loadGltfFile(tinygltf::Model & model) {
  std::cout << COLOR_MAGENTA << "(つ•̀ᴥ•́)つ*:･ﾟ✧ " << COLOR_RESET << " Let's load some Models" << std::endl;

  // Define a loader
  tinygltf::TinyGLTF loader;

  // Define outputs strings
  std::string err;
  std::string warn;

  // Load the model from our source string
  bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, m_gltfFilePath.string());

  // Display errors if required
  if (!warn.empty()) {
    std::cout << "Warning : " << warn.c_str() << std::endl;
  }

  if (!err.empty()) {
    std::cout << "Error  : " << err.c_str() << std::endl;
  }

  if (!ret) {
    std::cout << "Failed to parse glTF : " << std::endl;
    return -1;
  }

  // Return the command value
  return ret;
}


/*
Part 2 :
Implement a method std::vector<GLuint> ViewerApplication::createBufferObjects( const tinygltf::Model &model);
that compute the vector of buffer objects from a model and returns it.
Call this functions in run() after loading the glTF.
*/
std::vector<GLuint> ViewerApplication::createBufferObjects( const tinygltf::Model &model) {
  std::cout << COLOR_MAGENTA << "(つ•̀ᴥ•́)つ*:･ﾟ✧ " << COLOR_RESET << " Let's create a VBO" << std::endl;

  // Create a vector of buffers objects
  std::vector<GLuint> bufferObjects(model.buffers.size(), 0);

  // Generate buffers
  glGenBuffers(bufferObjects.size(), bufferObjects.data());

  // Loop through all buffers of the model
  for (size_t i = 0; i < model.buffers.size(); i++)  {

    // Bind the corresponding buffer
    glBindBuffer(GL_ARRAY_BUFFER, bufferObjects[i]);

    // Fill in the datas from the model
    // Reference : http://docs.gl/gl4/glBufferStorage
    glBufferStorage(
      GL_ARRAY_BUFFER, // GLenum target
      model.buffers[i].data.size(), // GLsizeiptr size
      model.buffers[i].data.data(), // const GLvoid * data
      0 // GLbitfield flags
    );
  }

  // After the loop, unbind glBindBuffer
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  return bufferObjects;
}

/*
Main run method
*/
int ViewerApplication::run()
{
  // Loader shaders
  const auto glslProgram =
      compileProgram({m_ShadersRootPath / m_AppName / m_vertexShader,
          m_ShadersRootPath / m_AppName / m_fragmentShader});

  const auto modelViewProjMatrixLocation =
      glGetUniformLocation(glslProgram.glId(), "uModelViewProjMatrix");
  const auto modelViewMatrixLocation =
      glGetUniformLocation(glslProgram.glId(), "uModelViewMatrix");
  const auto normalMatrixLocation =
      glGetUniformLocation(glslProgram.glId(), "uNormalMatrix");

  // Build projection matrix
  auto maxDistance = 500.f; // TODO use scene bounds instead to compute this
  maxDistance = maxDistance > 0.f ? maxDistance : 100.f;
  const auto projMatrix =
      glm::perspective(70.f, float(m_nWindowWidth) / m_nWindowHeight,
          0.001f * maxDistance, 1.5f * maxDistance);

  // TODO Implement a new CameraController model and use it instead. Propose the
  // choice from the GUI
  FirstPersonCameraController cameraController{
      m_GLFWHandle.window(), 0.5f * maxDistance};
  if (m_hasUserCamera) {
    cameraController.setCamera(m_userCamera);
  } else {
    // TODO Use scene bounds to compute a better default camera
    cameraController.setCamera(
        Camera{glm::vec3(0, 0, 0), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0)});
  }

  // TODO Loading the glTF file
  tinygltf::Model model;
  bool loadingModelSuccess = loadGltfFile(model);
  // Test
  if ( loadingModelSuccess ) {
    std::cout << COLOR_GREEN << "ლ ( ◕  ᗜ  ◕ ) ლ" << COLOR_RESET << " Model loaded" << COLOR_RESET << std::endl << std::endl;
  } else {
    std::cout << COLOR_RED << "ლ(ಥ Д ಥ )ლ " << COLOR_RESET << " Oh no !! Model failed to load" << COLOR_RESET << std::endl << std::endl;
  }


  // TODO Creation of Buffer Objects
  std::vector<GLuint> VBO = createBufferObjects(model);

  // Test : VBO size is the same as the model 
  if (VBO.size() == model.buffers.size()) {
    std::cout << COLOR_GREEN << "ლ ( ◕  ᗜ  ◕ ) ლ " << COLOR_RESET << " VBO created" << COLOR_RESET << std::endl << std::endl;
  } else {
    std::cout << COLOR_RED << "ლ(ಥ Д ಥ )ლ " << COLOR_RESET << " Oh no !! VBO were not created" << COLOR_RESET << std::endl << std::endl;
  }


  // TODO Creation of Vertex Array Objects

  // Setup OpenGL state for rendering
  glEnable(GL_DEPTH_TEST);
  glslProgram.use();

  // Lambda function to draw the scene
  const auto drawScene = [&](const Camera &camera) {
    glViewport(0, 0, m_nWindowWidth, m_nWindowHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const auto viewMatrix = camera.getViewMatrix();

    // The recursive function that should draw a node
    // We use a std::function because a simple lambda cannot be recursive
    const std::function<void(int, const glm::mat4 &)> drawNode =
        [&](int nodeIdx, const glm::mat4 &parentMatrix) {
          // TODO The drawNode function
        };

    // Draw the scene referenced by gltf file
    if (model.defaultScene >= 0) {
      // TODO Draw all nodes
    }
  };

  // Loop until the user closes the window
  for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose();
       ++iterationCount) {
    const auto seconds = glfwGetTime();

    const auto camera = cameraController.getCamera();
    drawScene(camera);

    // GUI code:
    imguiNewFrame();

    {
      ImGui::Begin("GUI");
      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
          1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
      if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("eye: %.3f %.3f %.3f", camera.eye().x, camera.eye().y,
            camera.eye().z);
        ImGui::Text("center: %.3f %.3f %.3f", camera.center().x,
            camera.center().y, camera.center().z);
        ImGui::Text(
            "up: %.3f %.3f %.3f", camera.up().x, camera.up().y, camera.up().z);

        ImGui::Text("front: %.3f %.3f %.3f", camera.front().x, camera.front().y,
            camera.front().z);
        ImGui::Text("left: %.3f %.3f %.3f", camera.left().x, camera.left().y,
            camera.left().z);

        if (ImGui::Button("CLI camera args to clipboard")) {
          std::stringstream ss;
          ss << "--lookat " << camera.eye().x << "," << camera.eye().y << ","
             << camera.eye().z << "," << camera.center().x << ","
             << camera.center().y << "," << camera.center().z << ","
             << camera.up().x << "," << camera.up().y << "," << camera.up().z;
          const auto str = ss.str();
          glfwSetClipboardString(m_GLFWHandle.window(), str.c_str());
        }
      }
      ImGui::End();
    }

    imguiRenderFrame();

    glfwPollEvents(); // Poll for and process events

    auto ellapsedTime = glfwGetTime() - seconds;
    auto guiHasFocus =
        ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
    if (!guiHasFocus) {
      cameraController.update(float(ellapsedTime));
    }

    m_GLFWHandle.swapBuffers(); // Swap front and back buffers
  }

  // TODO clean up allocated GL data

  return 0;
}

ViewerApplication::ViewerApplication(
    const fs::path &appPath,
    uint32_t width,
    uint32_t height,
    const fs::path &gltfFile,
    const std::vector<float> &lookatArgs,
    const std::string &vertexShader,
    const std::string &fragmentShader,
    const fs::path &output) :
    m_nWindowWidth(width),
    m_nWindowHeight(height),
    m_AppPath{appPath},
    m_AppName{m_AppPath.stem().string()},
    m_ImGuiIniFilename{m_AppName + ".imgui.ini"},
    m_ShadersRootPath{m_AppPath.parent_path() / "shaders"},
    m_gltfFilePath{gltfFile},
    m_OutputPath{output}
{
  if (!lookatArgs.empty()) {
    m_hasUserCamera = true;
    m_userCamera =
        Camera{glm::vec3(lookatArgs[0], lookatArgs[1], lookatArgs[2]),
            glm::vec3(lookatArgs[3], lookatArgs[4], lookatArgs[5]),
            glm::vec3(lookatArgs[6], lookatArgs[7], lookatArgs[8])};
  }

  if (!vertexShader.empty()) {
    m_vertexShader = vertexShader;
  }

  if (!fragmentShader.empty()) {
    m_fragmentShader = fragmentShader;
  }

  ImGui::GetIO().IniFilename =
      m_ImGuiIniFilename.c_str(); // At exit, ImGUI will store its windows
                                  // positions in this file

  glfwSetKeyCallback(m_GLFWHandle.window(), keyCallback);

  printGLVersion();
}