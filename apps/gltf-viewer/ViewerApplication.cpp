#include "ViewerApplication.hpp"
#include "cout_colors.hpp"

#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>

#include "utils/gltf.hpp"
#include "utils/cameras.hpp"
#include "utils/images.hpp"
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

Reference : https://celeborn2bealive.github.io/openglnoel/docs/gltf-viewer-02-gltf-02-initialization#loading-the-gltf-file
*/
bool ViewerApplication::loadGltfFile(tinygltf::Model & model)
{
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

Reference : https://celeborn2bealive.github.io/openglnoel/docs/gltf-viewer-02-gltf-02-initialization#creation-of-buffer-objects
*/
std::vector<GLuint> ViewerApplication::createBufferObjects( const tinygltf::Model &model)
{
  std::cout << COLOR_MAGENTA << "(つ•̀ᴥ•́)つ*:･ﾟ✧ " << COLOR_RESET << " Let's create a VBO" << std::endl;

  // Create a vector of buffers objects
  std::vector<GLuint> bufferObjects(model.buffers.size(), 0);

  // Generate buffers
  glGenBuffers(bufferObjects.size(), bufferObjects.data());

  // Loop through all buffers of the model
  for (size_t i = 0; i < model.buffers.size(); ++i)  {

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
Part 3
Start by adding en empty method
std::vector<GLuint> ViewerApplication::createVertexArrayObjects( const tinygltf::Model &model,
                                                                 const std::vector<GLuint> &bufferObjects,
                                                                 std::vector<VaoRange> & meshIndexToVaoRange);
This method is supposed to take the model and the buffer objects we previously created,
create an array of vertex array objects and return it.
It should also fill the input vector meshIndexToVaoRange with the range of VAOs for each mesh (see below).
This vector will be used later during drawing of the scene.

Reference : https://celeborn2bealive.github.io/openglnoel/docs/gltf-viewer-02-gltf-02-initialization#creation-of-vertex-array-objects
*/

std::vector<GLuint> ViewerApplication::createVertexArrayObjects( const tinygltf::Model &model,
                                                                 const std::vector<GLuint> &bufferObjects,
                                                                 std::vector<VaoRange> &meshIndexToVaoRange)
{
  std::cout << COLOR_MAGENTA << "(つ•̀ᴥ•́)つ*:･ﾟ✧ " << COLOR_RESET << " Let's create a VAO" << std::endl;

  // Define vertex attribs const
  const GLuint VERTEX_ATTRIB_POSITION_IDX = 0;
  const GLuint VERTEX_ATTRIB_NORMAL_IDX = 1;
  const GLuint VERTEX_ATTRIB_TEXCOORD0_IDX = 2;

  // In the function declare std::vector<GLuint> vertexArrayObjects;
  // This vector will contain our vertex array objects.
  // Take note that I don't give a size, because we don't know it yet.
  std::vector<GLuint> vertexArrayObjects;


  // Create a loop over meshes of the glTF (model.meshes)
  for (size_t meshIdx = 0; meshIdx < model.meshes.size(); ++meshIdx) {

    // Extend the size of vertexArrayObjects using resize(),
    // by adding the number of primitives of the current mesh of the loop
    // (model.meshes[meshIdx].primitives.size()).
    const auto vaoOffset = vertexArrayObjects.size();
    const auto primitivesSize = model.meshes[meshIdx].primitives.size();

    // Resize VAO to store our identifiers
    vertexArrayObjects.resize(vaoOffset + primitivesSize);
    meshIndexToVaoRange.push_back(VaoRange{static_cast<GLsizei>(vaoOffset), static_cast<GLsizei>(primitivesSize)});    
    // std::cout << "Size is now " << vaoOffset + primitivesSize << std::endl;

    // Call glGenVertexArrays to create new vertex arrays, one for each primitives.
    // For that you will need to pass a pointer to the correct index vertexArrayObjects,
    // which is the size of vertexArrayObjects before it was extended.
    // So you need to store it in a variable before extending its size.
    glGenVertexArrays(
      primitivesSize, // Amount
      &vertexArrayObjects[vaoOffset]// Pointer to 1st index of the array
    );

    // Create a loop over the primitives of the current mesh (so this second loop is inside the first one).
    for (size_t primitiveIdx = 0; primitiveIdx < primitivesSize; ++primitiveIdx) {

      // Declare the current primitive 
      const auto &primitive = model.meshes[meshIdx].primitives[primitiveIdx];

      // Inside that loop, get the VAO corresponding to the primitive (vertexArrayObjects[vaoOffset + primitiveIdx])
      // and bind it (glBindVertexArray).
      glBindVertexArray(vertexArrayObjects[vaoOffset + primitiveIdx]);

      // The good news is, the code for each one is the same 
      // (we will duplicate it but it can easily be factorized with a loop over ["POSITION", "NORMAL", "TEXCOORD_0"]).
      std::vector<std::string> parameters {"POSITION", "NORMAL", "TEXCOORD_0"};
      std::vector<GLuint> vertexAttribEnum {VERTEX_ATTRIB_POSITION_IDX, VERTEX_ATTRIB_NORMAL_IDX, VERTEX_ATTRIB_TEXCOORD0_IDX};

      for (size_t i = 0; i < 3; ++i) {
        std::string parameter = parameters[i];
        GLuint vertexAttrib = vertexAttribEnum[i];
        // std::cout << "Parameter is " << parameter << std::endl;
        // std::cout << "Vertex Attrib  is " << vertexAttrib << std::endl;

        // Now inside that new loop we will need to enable and initialize the parameters for each vertex attribute (POSITION, NORMAL, TEXCOORD_0).
        const auto iterator = primitive.attributes.find(parameter);
        if (iterator != end(primitive.attributes)) {
          // If parameter(i.e. "POSITION") has been found in the map
          // (*iterator).first is the key "POSITION",
          // (*iterator).second is the value,
          // ie. the index of the accessor for this attribute
          const auto accessorIdx = (*iterator).second;
          const auto &accessor = model.accessors[accessorIdx]; // TODO get the correct tinygltf::Accessor from model.accessors
          const auto &bufferView = model.bufferViews[accessor.bufferView]; // TODO get the correct tinygltf::BufferView from model.bufferViews. You need to use the accessor
          const auto bufferIdx = bufferView.buffer; // TODO get the index of the buffer used by the bufferView (you need to use it)
          const auto bufferObject = bufferObjects[bufferIdx]; // TODO get the correct buffer object from the buffer index
          assert(GL_ARRAY_BUFFER == bufferView.target); 

          // Verify
          // std::cout << "Buffer object data len is " << bufferObject.data.size() << std::endl; 

          // TODO Enable the vertex attrib array corresponding to POSITION with glEnableVertexAttribArray
          // (you need to use VERTEX_ATTRIB_POSITION_IDX which is defined at the top of the file)
          glEnableVertexAttribArray(vertexAttrib);

          // TODO Bind the buffer object to GL_ARRAY_BUFFER
          glBindBuffer(GL_ARRAY_BUFFER, bufferObject);

          // TODO Compute the total byte offset using the accessor and the buffer view
          const auto byteOffset = bufferView.byteOffset + accessor.byteOffset; 

          // TODO Call glVertexAttribPointer with the correct arguments. 
          // Remember size is obtained with accessor.type, type is obtained with accessor.componentType. 
          // The stride is obtained in the bufferView, normalized is always GL_FALSE, and pointer is the byteOffset (don't forget the cast).
          glVertexAttribPointer(
            vertexAttrib,               // GLuint index, 
            accessor.type,              // GLint size,
            accessor.componentType,     // GLenum type,
            GL_FALSE,                   // GLboolean normalized,
            bufferView.byteStride,      // GLsizei stride, 
            (const GLvoid*) byteOffset  // const GLvoid * pointer)
          ); 

        }
      } // </>End vertex attribution loop

      // The last thing we need in our inner loop is to set the index buffer of the vertex array object, if one exists.
      // For that you need to check if primitive.indices >= 0.
      // If that's the case then you need to get the accessor of index primitive.indices,
      // its buffer view,
      // and call glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, /* TODO fill with the correct buffer object */)
      if (primitive.indices >= 0) {
        const auto &accessor = model.accessors[primitive.indices];
        const auto &bufferView = model.bufferViews[accessor.bufferView];
        const auto bufferIdx = bufferView.buffer;
        const auto bufferObject = bufferObjects[bufferIdx];

        assert(GL_ELEMENT_ARRAY_BUFFER == bufferView.target); 
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferObject);
      }
    } // </>End Primitive loop
  } // </>End Mesh loop
      
  // End the description of our vertex array object:
  glBindVertexArray(0);
  return vertexArrayObjects;
}
/*
Main run method
*/
int ViewerApplication::run()
{
  std::cout << COLOR_BOLD << std::endl << "==============================================" << std::endl;
  std::cout << "╰(✿˙ᗜ˙)੭━☆ﾟ.*･｡ﾟ Starting the application" << std::endl;
  std::cout << "==============================================" << COLOR_RESET << std::endl << std::endl;

  // Loader shaders
  const auto glslProgram =
      compileProgram({m_ShadersRootPath / m_AppName / m_vertexShader,
          m_ShadersRootPath / m_AppName / m_fragmentShader});

  const auto modelViewProjMatrixLocation = glGetUniformLocation(glslProgram.glId(), "uModelViewProjMatrix");
  const auto modelViewMatrixLocation = glGetUniformLocation(glslProgram.glId(), "uModelViewMatrix");
  const auto normalMatrixLocation = glGetUniformLocation(glslProgram.glId(), "uNormalMatrix");

  // We now need to send the light parameters from the application.
  // For that we need to get uniform locations with glGetUniformLocation at the begining of run() (like other uniforms).
  const auto uniformLightDirection = glGetUniformLocation(glslProgram.glId(), "uLightDirection");
  const auto uniformLightRadiance = glGetUniformLocation(glslProgram.glId(), "uLightRadiance");

  // Declare and initialize two glm::vec3 variables lightDirection and lightIntensity.
  glm::vec3 lightDirection(1.f, 1.f, 1.f);
  glm::vec3 lightRadiance(0.1f, 0.2f, 0.8f);

  // Shall use lighting from camera
  bool useLightFromCamera = false;

  // TODO Loading the glTF file
  tinygltf::Model model;
  bool loadingModelSuccess = loadGltfFile(model);
  // Test
  if ( loadingModelSuccess ) {
    std::cout << COLOR_GREEN << "ლ ( ◕  ᗜ  ◕ ) ლ" << COLOR_RESET << " Model loaded" << COLOR_RESET << std::endl << std::endl;
  } else {
    std::cout << COLOR_RED << "ლ(ಥ Д ಥ )ლ " << COLOR_RESET << " Oh no !! Model failed to load" << COLOR_RESET << std::endl << std::endl;
  }

  /*
    Assuming bboxMin and bboxMax are glm::vec3
    Center of the bounding box using bboxMin and bboxMax is

    center = (bboxMin + bboxMax) * 0.5;

    Diagonal vector from bboxMin to bboxMax is

    diagonal = bboxMax - bboxMin
  */

  // void computeSceneBounds(const tinygltf::Model &model, glm::vec3 &bboxMin, glm::vec3 &bboxMax);
  glm::vec3 bboxMin, bboxMax; 
  computeSceneBounds(model, bboxMin, bboxMax);
  glm::vec3 center = (bboxMin + bboxMax) * 0.5f;
  glm::vec3 diagonal = bboxMax - bboxMin;
  glm::vec3 up = glm::vec3(0, 1, 0);

  // Use the diagonal vector to compute a maximum distance between two points of the scene.
  float maxDistance = glm::length(diagonal);
  maxDistance = maxDistance > 0.f ? maxDistance : 100.f;

  // Build projection matrix
  // Use near = 0.001f * maxDistance and far = 1.5f * maxDistance to compute the project matrix (the call to glm::perspective).
  const auto projMatrix = glm::perspective(
    70.f, 
    float(m_nWindowWidth) / m_nWindowHeight,
    0.001f * maxDistance,
    1.5f * maxDistance
  );

  // TODO Implement a new CameraController model and use it instead. Propose the
  // choice from the GUI
  // FirstPersonCameraController cameraController{m_GLFWHandle.window(), 1.5f * maxDistance};

  // TODO In ViewerApplication::run() replace the FirstPersonCameraController by a TrackballCameraController.
  // TrackballCameraController cameraController{m_GLFWHandle.window(), 0.5f * maxDistance};

  // If we want to easily switch between our two controllers,
  // we need to manipulate a pointer to a CameraController instead of a variable to a concrete type in our run() method.
  // We will use dynamic allocation and std::unique_ptr for that.

  std::unique_ptr<CameraController> cameraController = 
    std::make_unique<TrackballCameraController>(m_GLFWHandle.window(), 1.5f * maxDistance);

  if (m_hasUserCamera) {
    cameraController->setCamera(m_userCamera);
  } else {
    // TODO Use scene bounds to compute a better default camera

    // A special case we need to handle is flat scenes (like a single triangle).
    // In that case, we need to use align our view with a vector orthogonal to the scene.
    // We will only handle flat scenes on the z axis.
    if ( bboxMax.z - bboxMin.z < 0.001f ) {
      // The scene is flat
      cameraController->setCamera(Camera{center + 2.f * glm::cross(diagonal, up), center, up});
    } else {
      // The scene is not flat
      cameraController->setCamera(Camera{center + diagonal, center, up});
    }
  }


  // Add a call to your method in the run() method,
  // in the initialization step,
  // before the drawing loop
  // (for example, before the call to createBufferObjects).
  // Store the result in a vector textureObjects.
  std::vector<GLuint> textureObjects = createTextureObjects(model);



  // TODO Creation of Buffer Objects
  std::vector<GLuint> VBO = createBufferObjects(model);
  // Test : VBO size is the same as the model 
  if (VBO.size() == model.buffers.size()) {
    std::cout << COLOR_GREEN << "ლ ( ◕  ᗜ  ◕ ) ლ " << COLOR_RESET << "VBO created" << COLOR_RESET << std::endl << std::endl;
  } else {
    std::cout << COLOR_RED << "ლ(ಥ Д ಥ )ლ " << COLOR_RESET << " Oh no !! VBO were not created" << COLOR_RESET << std::endl << std::endl;
  }


  // TODO Creation of Vertex Array Objects
  std::vector<VaoRange> meshIndexToVaoRange;
  std::vector<GLuint> VAO = createVertexArrayObjects(model, VBO, meshIndexToVaoRange);
  // Test NOT KNOWN ??
  if (VAO.size() != 0 ) {
    std::cout << COLOR_GREEN << "ლ ( ◕  ᗜ  ◕ ) ლ " << COLOR_RESET << "VAO created" << COLOR_RESET << std::endl;
    std::cout << COLOR_GREEN << "ლ ( ◕  ᗜ  ◕ ) ლ " << COLOR_RESET << "It contains " << VAO.size() << " meshes. Not bad." << COLOR_RESET << std::endl << std::endl;
  } else {
    std::cout << COLOR_RED << "ლ(ಥ Д ಥ )ლ " << COLOR_RESET << " Oh no !! VAO were not created" << COLOR_RESET << std::endl << std::endl;
  }

  // Setup OpenGL state for rendering
  glEnable(GL_DEPTH_TEST);
  glslProgram.use();

  // Lambda function to draw the scene
  const auto drawScene = [&](const Camera &camera) {
    glViewport(0, 0, m_nWindowWidth, m_nWindowHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const auto viewMatrix = camera.getViewMatrix();

    // Then in the render loop we need to set our uniforms with glUniform3f.
    // For the light direction, we must be careful to
    //    Muliply it with the view matrix,
    //    And normalize it,
    // before sending it to the shader.
    // Otherwise, we will see the light move as we move the camera, and that's not what we want of course.

    const auto lightDirectionViewSpace = glm::normalize(glm::vec3(viewMatrix * glm::vec4(lightDirection, 0.)));
    if ( uniformLightDirection >= 0 ) {
      if ( useLightFromCamera ) {
        glUniform3f(uniformLightDirection, 0, 0, 1);
      } else {
        glUniform3f(uniformLightDirection, lightDirectionViewSpace.x, lightDirectionViewSpace.y, lightDirectionViewSpace.z);
      }
    }
    if ( uniformLightRadiance >= 0 )  {
      glUniform3f(uniformLightRadiance, lightRadiance.r, lightRadiance.g, lightRadiance.b);
    }

    // The recursive function that should draw a node
    // We use a std::function because a simple lambda cannot be recursive
    const std::function<void(int, const glm::mat4 &)> drawNode =
        [&](int nodeIdx, const glm::mat4 &parentMatrix)
    {
      // TODO The drawNode function

      // Now we can attack the drawNode function.
      // The first step is to get the node (as a tinygltf::Node)
      // and to compute its model matrix from the parent matrix.
      // Fortunately for you, I included a helper function getLocalToWorldMatrix(node, parentMatrix).
      // If you are interested you can take a look at the code,
      // its mostly calls to glm function to perform the right maths.
      const auto & node = model.nodes[nodeIdx];
      glm::mat4 modelMatrix = getLocalToWorldMatrix(node, parentMatrix);

      // Then we need to ensure that the node has a mesh
      if (node.mesh >= 0) {
        // Compute modelViewMatrix
        glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;

        // Compute modelViewProjectionMatrix
        glm::mat4 modelViewProjectionMatrix = projMatrix * modelViewMatrix;

        // Compute normalMatrix
        glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelViewMatrix));

        // Send all of these to the shaders with glUniformMatrix4fv.
        glUniformMatrix4fv(modelViewMatrixLocation, 1, GL_FALSE, (const GLfloat*) &modelViewMatrix);
        glUniformMatrix4fv(modelViewProjMatrixLocation, 1, GL_FALSE, (const GLfloat*) &modelViewProjectionMatrix);
        glUniformMatrix4fv(normalMatrixLocation, 1, GL_FALSE, (const GLfloat*) &normalMatrix);

        // Get the mesh
        const auto & mesh = model.meshes[node.mesh];

        // To draw a primitive we need the VAO that we filled for it.
        // Remember that we computed a vector meshIndexToVaoRange with the range of vertex array objects for each mesh
        // (this range being an offset and an number of elements in the vertexArrayObjects vector).
        // Each primitive of index primIdx of the mesh should has its corresponding VAO
        // at vertexArrayObjects[vaoRange.begin + primIdx] if vaoRange is in the range of the mesh.
        const auto vaoRange = meshIndexToVaoRange[node.mesh];

        // Iterate over its primitives to draw them.
        for (size_t primitiveIdx = 0; primitiveIdx < mesh.primitives.size(); ++primitiveIdx) {

          // Get the VAO of the primitive (using vertexArrayObjects, the vaoRange and the primitive index) and bind it.
          const auto & vao = VAO[vaoRange.begin + primitiveIdx];

          // Get the current primitive.
          const auto & primitive = mesh.primitives[primitiveIdx];

          // Now we need to check if the primitive has indices by testing if (primitive.indices >= 0).
          // If its the case we should use glDrawElements for the drawing,
          // If not we should use glDrawArrays.

          // Implement the first case, where the primitive has indices. 
          if (primitive.indices >= 0) {

            // You need to get the accessor of the indices (model.accessors[primitive.indices]) 
            const auto & accessor = model.accessors[primitive.indices];
            
            // And the bufferView to compute the total byte offset to use for indices
            const auto & bufferView = model.bufferViews[accessor.bufferView];
            const auto byteOffset = bufferView.byteOffset + accessor.byteOffset; 

            // You should then call glDrawElements with
            // the mode of the primitive,
            // the number of indices (accessor.count),
            // the component type of indices (accessor.componentType)
            // the byte offset as last argument (with a cast to const GLvoid*).
            glBindVertexArray(vao);
            glDrawElements(
              static_cast<GLenum>(primitive.mode),
              static_cast<GLsizei>(accessor.count),
              static_cast<GLenum>(accessor.componentType),
              (const GLvoid*)byteOffset
            );
          } else {

            // Implement the second case, where the primitive does not have indices.

            // For this you need the number of vertex to render.
            // The specification of glTF tells us that we can use the accessor of an arbritrary attribute of the primitive.
            const auto accessorIdx = (*begin(primitive.attributes)).second;
            const auto &accessor = model.accessors[accessorIdx];

            // And the bufferView to compute the total byte offset to use for indices
            const auto & bufferView = model.bufferViews[accessor.bufferView];
            const auto byteOffset = bufferView.byteOffset + accessor.byteOffset;

            // Then call glDrawArrays, passing it
            // the mode of the primitive,
            // 0 as second argument,
            // and accessor.count as last argument.
            glDrawArrays(
              static_cast<GLenum>(primitive.mode),
              static_cast<GLint>(0),
              static_cast<GLsizei>(accessor.count)
            );
          }
        }
      }

      // We then have one last thing to implement,
      // after the if (node.mesh >= 0) body:
      // we need to draw children recursively.

      // After the if body, add a loop over node.children
      for (const auto nodeChildIdx : node.children) {
        // Call drawNode on each children.
        // The matrix passed as second argument should be the modelMatrix that has been computed earlier in the function.
        drawNode(nodeChildIdx, modelMatrix);
      }

    };

    // Draw the scene referenced by gltf file
    if (model.defaultScene >= 0) {
      // TODO Draw all nodes
      for (const auto nodeIdx : model.scenes[model.defaultScene].nodes) {
        drawNode(nodeIdx, glm::mat4(1));
      }
    }

    // Unbind the vertex array
    glBindVertexArray(0);
  };

  if ( !m_OutputPath.empty() ) {

    std::cout << COLOR_MAGENTA << "(つ•̀ᴥ•́)つ*:･ﾟ✧ " << COLOR_RESET << " Let's make an image !" << std::endl;

    // Render to image
    std::vector<unsigned char> pixels( m_nWindowWidth * m_nWindowHeight * 3);
    renderToImage(m_nWindowWidth, m_nWindowHeight, 3, pixels.data(), [&]() {
      drawScene(cameraController->getCamera());
    });

    // Flip the image vertically, because OpenGL does not use the same convention for that than png files.
    flipImageYAxis<unsigned char>(m_nWindowWidth, m_nWindowHeight, 3, pixels.data());

    // Write the png file with stb_image_write library which is included in the third-parties.
    const auto strPath = m_OutputPath.string();
    stbi_write_png(strPath.c_str(), m_nWindowWidth, m_nWindowHeight, 3, pixels.data(), 0);

    // Finally at the end of the if statement, returns 0. So in that mode, our application just render an image in a file and leave.
    std::cout << COLOR_MAGENTA << "╰[✿•̀o•́✿]╯       " << COLOR_RESET << "Image was rendered ! Yay !" << std::endl;
    std::cout << COLOR_MAGENTA << "ʕ༼◕  ౪  ◕✿༽ʔ    " << COLOR_RESET << "Good bye !" << std::endl;
    std::cout << COLOR_BOLD << std::endl << "==============================================" << std::endl;
    std::cout << "┗(＾0＾)┓ Leaving the application" << std::endl;
    std::cout << "==============================================" << COLOR_RESET << std::endl << std::endl;
    return 0;
  } else {

    std::cout << COLOR_MAGENTA << "(つ•̀ᴥ•́)つ*:･ﾟ✧ " << COLOR_RESET << " Let's run in interactive mode !" << std::endl;

    // Loop until the user closes the window
    for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount) {      
      const auto seconds = glfwGetTime();
      const auto camera = cameraController->getCamera();
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

        // Finally, we will add the possibility to switch between the two controllers using radio buttons in the GUI
        static int radioButtonToggleState = 0;
        ImGui::Text("Change the CameraController type");
        const auto c1 = ImGui::RadioButton("Trackball", &radioButtonToggleState, 0);
        ImGui::SameLine();
        const auto c2 = ImGui::RadioButton("First person", &radioButtonToggleState, 1);
        const auto hasChanged = c1 || c2;
        if ( hasChanged ) {
          if (radioButtonToggleState == 0) {
            cameraController = std::make_unique<TrackballCameraController>(m_GLFWHandle.window(), 1.5f * maxDistance);
          } else if (radioButtonToggleState == 1) {
            cameraController = std::make_unique<FirstPersonCameraController>(m_GLFWHandle.window(), 1.5f * maxDistance);
          }
        }
        cameraController->setCamera(camera);

        // Add a checkbox "light from camera" that control a boolean.
        // If set, then send (0, 0, 1) instead of lightDirection in the drawing function for the uniform controlling the light direction.
        static bool c = false;
        if ( ImGui::Checkbox("Use lighting from the camera", &c) ) {
          useLightFromCamera = c;
          std::cout << "Updated checkbox is now " << useLightFromCamera << std::endl;
        }

        // In the GUI, add a new section "Light" (with ImGui::CollapsingHeader())
        if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen)) {

          // containing two sliders for θ (between 0 and π) and ϕ (between 0 and 2π) angles.
          // When they are changed, compute lightDirection from them using the above formula.
            static float theta = 1.0f;
            static float phi = 1.0f;
            const auto hasChanged = ImGui::SliderFloat("Theta θ", &theta, 0, 3.14) ||
                                    ImGui::SliderFloat("Phi ϕ", &phi, 0, 6.28);
            if ( hasChanged ) {
              // ωi=(sinθcosϕ,cosθ,sinθsinϕ)
              lightDirection = glm::vec3( sin(theta) * cos(phi),
                                            cos(theta),
                                            sin(theta) * sin(phi));
            }

            // Add a ImGui::ColorEdit3 to specify a color and an input float for the intensity factor.
            // If they are changed, update lightIntensity with their product.
            static float color[3] = { lightRadiance.r, lightRadiance.g, lightRadiance.b };
            if ( ImGui::ColorEdit3("Light color", color) ) {
              lightRadiance = glm::vec3(color[0], color[1], color[2]);
            }
        }


        ImGui::End();
      }

      imguiRenderFrame();

      glfwPollEvents(); // Poll for and process events

      auto ellapsedTime = glfwGetTime() - seconds;
      auto guiHasFocus = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
      if (!guiHasFocus) {
        cameraController->update(float(ellapsedTime));
      }

      m_GLFWHandle.swapBuffers(); // Swap front and back buffers
    }
  }

  // TODO clean up allocated GL data


  std::cout << COLOR_MAGENTA << "ʕ༼◕  ౪  ◕✿༽ʔ    " << COLOR_RESET << "Good bye !" << std::endl;
  std::cout << COLOR_BOLD << std::endl << "==============================================" << std::endl;
  std::cout << "┗(＾0＾)┓ Leaving the application" << std::endl;
  std::cout << "==============================================" << COLOR_RESET << std::endl << std::endl;
  return 0;
}

/*
Constructor
*/
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


/*
This method should compute a vector of texture objects. Each texture object is filled with an image and sampling parameters from the corresponding texture of the glTF file. This step basically consists of picking code examples from this section and putting them in a loop in order to initialize each texture object.
*/
std::vector<GLuint> ViewerApplication::createTextureObjects(const tinygltf::Model &model) const {

}