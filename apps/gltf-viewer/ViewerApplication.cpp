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
  if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
  {
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
bool ViewerApplication::loadGltfFile(tinygltf::Model &model)
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
  if (!warn.empty())
  {
    std::cout << "Warning : " << warn.c_str() << std::endl;
  }

  if (!err.empty())
  {
    std::cout << "Error  : " << err.c_str() << std::endl;
  }

  if (!ret)
  {
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
std::vector<GLuint> ViewerApplication::createBufferObjects(const tinygltf::Model &model)
{
  std::cout << COLOR_MAGENTA << "(つ•̀ᴥ•́)つ*:･ﾟ✧ " << COLOR_RESET << " Let's create a VBO" << std::endl;

  // Create a vector of buffers objects
  std::vector<GLuint> bufferObjects(model.buffers.size(), 0);

  // Generate buffers
  glGenBuffers(bufferObjects.size(), bufferObjects.data());

  // Loop through all buffers of the model
  for (size_t i = 0; i < model.buffers.size(); ++i)
  {

    // Bind the corresponding buffer
    glBindBuffer(GL_ARRAY_BUFFER, bufferObjects[i]);

    // Fill in the datas from the model
    // Reference : http://docs.gl/gl4/glBufferStorage
    glBufferStorage(
        GL_ARRAY_BUFFER,              // GLenum target
        model.buffers[i].data.size(), // GLsizeiptr size
        model.buffers[i].data.data(), // const GLvoid * data
        0                             // GLbitfield flags
    );
  }

  // After the loop, unbind glBindBuffer
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  return bufferObjects;
}

/*
Part 3
Start by adding en empty method
std::vector<GLuint> ViewerApplication::createVertexArrayObjects(const tinygltf::Model &model,
                                                                const std::vector<GLuint> &bufferObjects,
                                                                std::vector<VaoRange> & meshIndexToVaoRange);
This method is supposed to take the model and the buffer objects we previously created,
create an array of vertex array objects and return it.
It should also fill the input vector meshIndexToVaoRange with the range of VAOs for each mesh (see below).
This vector will be used later during drawing of the scene.

Reference : https://celeborn2bealive.github.io/openglnoel/docs/gltf-viewer-02-gltf-02-initialization#creation-of-vertex-array-objects
*/

std::vector<GLuint> ViewerApplication::createVertexArrayObjects(const tinygltf::Model &model,
                                                                const std::vector<GLuint> &bufferObjects,
                                                                std::vector<VaoRange> &meshIndexToVaoRange)
{
  std::cout << COLOR_MAGENTA << "(つ•̀ᴥ•́)つ*:･ﾟ✧ " << COLOR_RESET << " Let's create a VAO" << std::endl;

  // Define vertex attribs const
  const GLuint VERTEX_ATTRIB_POSITION_IDX = 0;
  const GLuint VERTEX_ATTRIB_NORMAL_IDX = 1;
  const GLuint VERTEX_ATTRIB_TEXCOORD0_IDX = 2;
  const GLuint VERTEX_ATTRIB_TANGENT_IDX = 3;

  // Init the tangent checker
  bool hasTangent = true;

  // In the function declare std::vector<GLuint> vertexArrayObjects;
  // This vector will contain our vertex array objects.
  // Take note that I don't give a size, because we don't know it yet.
  std::vector<GLuint> vertexArrayObjects;

  // Create a loop over meshes of the glTF (model.meshes)
  for (size_t meshIdx = 0; meshIdx < model.meshes.size(); ++meshIdx)
  {

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
        primitivesSize,                // Amount
        &vertexArrayObjects[vaoOffset] // Pointer to 1st index of the array
    );

    // Create a loop over the primitives of the current mesh (so this second loop is inside the first one).
    for (size_t primitiveIdx = 0; primitiveIdx < primitivesSize; ++primitiveIdx)
    {

      // Declare the current primitive
      const auto &primitive = model.meshes[meshIdx].primitives[primitiveIdx];

      // Inside that loop, get the VAO corresponding to the primitive (vertexArrayObjects[vaoOffset + primitiveIdx])
      // and bind it (glBindVertexArray).
      glBindVertexArray(vertexArrayObjects[vaoOffset + primitiveIdx]);

      // The good news is, the code for each one is the same
      // (we will duplicate it but it can easily be factorized with a loop over ["POSITION", "NORMAL", "TEXCOORD_0", "TANGENT"]).

      // Based on https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#meshes
      // Model should provide tangent.
      std::vector<std::string> parameters{"POSITION", "NORMAL", "TANGENT", "TEXCOORD_0"};
      std::vector<GLuint> vertexAttribEnum {VERTEX_ATTRIB_POSITION_IDX,
                                            VERTEX_ATTRIB_NORMAL_IDX,
                                            VERTEX_ATTRIB_TANGENT_IDX,
                                            VERTEX_ATTRIB_TEXCOORD0_IDX};

      for (size_t i = 0; i < parameters.size(); ++i)
      {
        std::string parameter = parameters[i];
        GLuint vertexAttrib = vertexAttribEnum[i];
        // std::cout << "Parameter is " << parameter << std::endl;
        // std::cout << "Vertex Attrib  is " << vertexAttrib << std::endl;

        // Now inside that new loop we will need to enable and initialize the parameters for each vertex attribute (POSITION, NORMAL, TEXCOORD_0).
        const auto iterator = primitive.attributes.find(parameter);
        if (iterator != end(primitive.attributes))
        if (iterator != end(primitive.attributes))
        {
          // If parameter(i.e. "POSITION") has been found in the map
          // (*iterator).first is the key "POSITION",
          // (*iterator).second is the value,
          // ie. the index of the accessor for this attribute
          const auto accessorIdx = (*iterator).second;
          const auto &accessor = model.accessors[accessorIdx];             // TODO get the correct tinygltf::Accessor from model.accessors
          const auto &bufferView = model.bufferViews[accessor.bufferView]; // TODO get the correct tinygltf::BufferView from model.bufferViews. You need to use the accessor
          const auto bufferIdx = bufferView.buffer;                        // TODO get the index of the buffer used by the bufferView (you need to use it)
          const auto bufferObject = bufferObjects[bufferIdx];              // TODO get the correct buffer object from the buffer index
          assert(GL_ARRAY_BUFFER == bufferView.target);

          // TODO Enable the vertex attrib array corresponding to POSITION with glEnableVertexAttribArray
          // (you need to use VERTEX_ATTRIB_POSITION_IDX which is defined at the top of the file)
          glEnableVertexAttribArray(vertexAttrib);

          // TODO Bind the buffer object to GL_ARRAY_BUFFER
          glBindBuffer(GL_ARRAY_BUFFER, bufferObject);

          // TODO Compute the total byte offset using the accessor and the buffer view
          const auto byteOffset = bufferView.byteOffset + accessor.byteOffset;
          const auto byteStride = bufferView.byteStride ? bufferView.byteStride : 3 * sizeof(float);

          // TODO Call glVertexAttribPointer with the correct arguments.
          // Remember size is obtained with accessor.type, type is obtained with accessor.componentType.
          // The stride is obtained in the bufferView, normalized is always GL_FALSE, and pointer is the byteOffset (don't forget the cast).
          glVertexAttribPointer(
              vertexAttrib,              // GLuint index,
              accessor.type,             // GLint size,
              accessor.componentType,    // GLenum type,
              GL_FALSE,                  // GLboolean normalized,
              bufferView.byteStride,     // GLsizei stride,
              (const GLvoid *)byteOffset // const GLvoid * pointer)
          );
        }
      } // </>End vertex attribution loop
      
      // Also check if primitive contains TANGENT
      bool primitiveHasTangent = primitive.attributes.find("TANGENT") != primitive.attributes.end();
      if (!primitiveHasTangent) {
        hasTangent = false;
        std::cout << COLOR_CYAN << "(ʘᗩʘ’)" << COLOR_RESET << " Primitive don't provides tangents data." << std::endl;
      }

      // The last thing we need in our inner loop is to set the index buffer of the vertex array object, if one exists.
      // For that you need to check if primitive.indices >= 0.
      // If that's the case then you need to get the accessor of index primitive.indices,
      // its buffer view,
      // and call glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, /* TODO fill with the correct buffer object */)
      if (primitive.indices >= 0)
      {
        const auto &accessor = model.accessors[primitive.indices];
        const auto &bufferView = model.bufferViews[accessor.bufferView];
        const auto bufferIdx = bufferView.buffer;
        const auto bufferObject = bufferObjects[bufferIdx];

        assert(GL_ELEMENT_ARRAY_BUFFER == bufferView.target);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferObject);
      }
    } // </>End Primitive loop
  }   // </>End Mesh loop

  if (!hasTangent) {
    std::cout << COLOR_CYAN << "(ʘᗩʘ’)" << COLOR_RESET << " Model don't provides tangents data. Fallback to GPU computing. " << std::endl;
  }

  // End the description of our vertex array object:
  glBindVertexArray(0);
  return vertexArrayObjects;
}

/*
This method should compute a vector of texture objects. Each texture object is filled with an image and sampling parameters from the corresponding texture of the glTF file. This step basically consists of picking code examples from this section and putting them in a loop in order to initialize each texture object.
*/
std::vector<GLuint> ViewerApplication::createTextureObjects(const tinygltf::Model &model) const
{

  // When a filter is undefined, we use GL_LINEAR as default filter.
  // And when no sampler is defined for the texture (in that case we have texture.sampler == -1),
  // we use a default sampler defined as:
  tinygltf::Sampler defaultSampler;
  defaultSampler.minFilter = GL_LINEAR;
  defaultSampler.magFilter = GL_LINEAR;
  defaultSampler.wrapS = GL_REPEAT;
  defaultSampler.wrapT = GL_REPEAT;
  defaultSampler.wrapR = GL_REPEAT;

  // Gen texture
  std::vector<GLuint> textures;

  // This code example shows how to fill a texture object using an image from tinygltf
  for (const auto &texture : model.textures)
  {
    assert(texture.source >= 0);
    const auto &image = model.images[texture.source];
    GLuint textureIdx;
    glGenTextures(1, &textureIdx);

    glBindTexture(GL_TEXTURE_2D, textureIdx);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 image.width,
                 image.height,
                 0,
                 GL_RGBA,
                 image.pixel_type,
                 image.image.data());

    // The constant number here correspond to OpenGL constants such as GL_LINEAR, NEAREST_MIPMAP_NEAREST, GL_REPEAT, etc.
    // It means we can directly use the values to setup texture sampling with the OpenGL API:
    const auto &sampler = texture.sampler >= 0 ? model.samplers[texture.sampler] : defaultSampler;
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_MIN_FILTER,
                    sampler.minFilter != -1 ? sampler.minFilter : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_MAG_FILTER,
                    sampler.magFilter != -1 ? sampler.magFilter : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sampler.wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, sampler.wrapT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, sampler.wrapR);

    // Some samplers use mipmapping for their minification filter.
    // In that case, the specification tells us we need to have mipmaps computed for the texture.
    // OpenGL can compute them for us
    if (sampler.minFilter == GL_NEAREST_MIPMAP_NEAREST ||
        sampler.minFilter == GL_NEAREST_MIPMAP_LINEAR ||
        sampler.minFilter == GL_LINEAR_MIPMAP_NEAREST ||
        sampler.minFilter == GL_LINEAR_MIPMAP_LINEAR)
    {
      glGenerateMipmap(GL_TEXTURE_2D);
    }

    // Push back to the vector
    textures.push_back(textureIdx);
  }

  glBindTexture(GL_TEXTURE_2D, 0);
  return textures;
}

/*
Main run method
*/
int ViewerApplication::run()
{
  std::cout << COLOR_BOLD << std::endl
            << "==============================================" << std::endl;
  std::cout << "╰(✿˙ᗜ˙)੭━☆ﾟ.*･｡ﾟ Starting the application" << std::endl;
  std::cout << "==============================================" << COLOR_RESET << std::endl
            << std::endl;

  // Loader shaders
  const auto glslProgram =
      compileProgram({m_ShadersRootPath / m_AppName / m_vertexShader,
                      m_ShadersRootPath / m_AppName / m_fragmentShader});

  const auto uniformModelViewProjMatrix = glGetUniformLocation(glslProgram.glId(), "uModelViewProjMatrix");
  const auto uniformModelViewMatrix = glGetUniformLocation(glslProgram.glId(), "uModelViewMatrix");
  const auto uniformNormalMatrix = glGetUniformLocation(glslProgram.glId(), "uNormalMatrix");
  const auto uniformModelMatrix = glGetUniformLocation(glslProgram.glId(), "uModelMatrix");

  // We now need to send the light parameters from the application.
  // For that we need to get uniform locations with glGetUniformLocation at the begining of run() (like other uniforms).
  const auto uniformLightDirection = glGetUniformLocation(glslProgram.glId(), "uLightDirection");
  const auto uniformLightRadiance = glGetUniformLocation(glslProgram.glId(), "uLightRadiance");

  // In the C++ code, apply the modifications needed to make it work (get uniform locations, bind textures, set uniforms)
  const auto uniformEmissiveFactor = glGetUniformLocation(glslProgram.glId(), "uEmissiveFactor");
  const auto uniformEmissiveTexture = glGetUniformLocation(glslProgram.glId(), "uEmissiveTexture");

  // An occlusion map, which is just a factor to apply to the whole lighting equation, quite easy too
  const auto uniformOcclusionTexture = glGetUniformLocation(glslProgram.glId(), "uOcclusionTexture");
  const auto uniformOcclusionStrength = glGetUniformLocation(glslProgram.glId(), "uOcclusionStrength");

  // The first step is to get the uniform location of uBaseColorTexture.
  const auto uniformBaseColorTexture = glGetUniformLocation(glslProgram.glId(), "uBaseColorTexture");
  const auto uniformBaseColorFactor = glGetUniformLocation(glslProgram.glId(), "uBaseColorFactor");
  const auto uniformMetallicFactor = glGetUniformLocation(glslProgram.glId(), "uMetallicFactor");
  const auto uniformRougnessFactor = glGetUniformLocation(glslProgram.glId(), "uRougnessFactor");
  const auto uniformMetallicRoughnessTexture = glGetUniformLocation(glslProgram.glId(), "uMetallicRoughnessTexture");

  // Normal map
  const auto uniformNormalMapTexture = glGetUniformLocation(glslProgram.glId(), "uNormalMapTexture");
  const auto uniformNormalMapScale = glGetUniformLocation(glslProgram.glId(), "uNormalMapScale");
  const auto uniformNormalMapUse = glGetUniformLocation(glslProgram.glId(), "uNormalMapUse");
  

  // Declare and initialize two glm::vec3 variables lightDirection and lightIntensity.
  glm::vec3 lightDirection(1.f, 1.f, 1.f);
  glm::vec3 lightRadiance(2.f, 2.f, 2.f);

  // Imgui controllers
  bool useLightFromCamera = false;
  bool useBaseColorTexture = true;
  bool useMetallicRoughnessTexture = true;
  bool useEmissive = true;
  bool useOcclusion = true;
  bool useNormalMap = true;

  // TODO Loading the glTF file
  tinygltf::Model model;
  bool loadingModelSuccess = loadGltfFile(model);
  // Test
  if (loadingModelSuccess)
  {
    std::cout << COLOR_GREEN << "ლ ( ◕  ᗜ  ◕ ) ლ" << COLOR_RESET << " Model loaded" << COLOR_RESET << std::endl
              << std::endl;
  }
  else
  {
    std::cout << COLOR_RED << "ლ(ಥ Д ಥ )ლ " << COLOR_RESET << " Oh no !! Model failed to load" << COLOR_RESET << std::endl
              << std::endl;
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
      1.5f * maxDistance);

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

  if (m_hasUserCamera)
  {
    cameraController->setCamera(m_userCamera);
  }
  else
  {
    // TODO Use scene bounds to compute a better default camera

    // A special case we need to handle is flat scenes (like a single triangle).
    // In that case, we need to use align our view with a vector orthogonal to the scene.
    // We will only handle flat scenes on the z axis.
    if (bboxMax.z - bboxMin.z < 0.001f)
    {
      // The scene is flat
      cameraController->setCamera(Camera{center + 2.f * glm::cross(diagonal, up), center, up});
    }
    else
    {
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

  // After the call, create a single texture object with a variable GLuint whiteTexture to reference it.
  // Fill it with a single white RGBA pixel (float white[] = {1, 1, 1, 1};)
  // and set sampling parameters to GL_LINEAR and wrapping parameters to GL_REPEAT.
  // This texture will be used for the base color of objects that have no materials
  // (as specified in the glTF specification, if no base color texture is present, we should use white).
  GLuint whiteTexture;
  float white[] = {1, 1, 1, 1};
  glGenTextures(1, &whiteTexture);
  glBindTexture(GL_TEXTURE_2D, whiteTexture);
  glTexImage2D(GL_TEXTURE_2D,
               0,
               GL_RGBA,
               1, // width
               1, // height
               0,
               GL_RGBA,
               GL_FLOAT,
               &white);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
  glBindTexture(GL_TEXTURE_2D, 0);

  // TODO Creation of Buffer Objects
  std::vector<GLuint> VBO = createBufferObjects(model);
  // Test : VBO size is the same as the model
  if (VBO.size() == model.buffers.size())
  {
    std::cout << COLOR_GREEN << "ლ ( ◕  ᗜ  ◕ ) ლ " << COLOR_RESET << "VBO created" << COLOR_RESET << std::endl
              << std::endl;
  }
  else
  {
    std::cout << COLOR_RED << "ლ(ಥ Д ಥ )ლ " << COLOR_RESET << " Oh no !! VBO were not created" << COLOR_RESET << std::endl
              << std::endl;
  }

  // TODO Creation of Vertex Array Objects
  std::vector<VaoRange> meshIndexToVaoRange;
  std::vector<GLuint> VAO = createVertexArrayObjects(model, VBO, meshIndexToVaoRange);
  if (VAO.size() != 0)
  {
    std::cout << COLOR_GREEN << "ლ ( ◕  ᗜ  ◕ ) ლ " << COLOR_RESET << "VAO created" << COLOR_RESET << std::endl;
    std::cout << COLOR_GREEN << "ლ ( ◕  ᗜ  ◕ ) ლ " << COLOR_RESET << "It contains " << VAO.size() << " meshes. Not bad." << COLOR_RESET << std::endl
              << std::endl;
  }
  else
  {
    std::cout << COLOR_RED << "ლ(ಥ Д ಥ )ლ " << COLOR_RESET << " Oh no !! VAO were not created" << COLOR_RESET << std::endl
              << std::endl;
  }

  // Setup OpenGL state for rendering
  glEnable(GL_DEPTH_TEST);
  glslProgram.use();

  const auto whiteTextureCall = [&]() {
    // Default to white texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, whiteTexture);
    glUniform1i(uniformBaseColorTexture, 0);
    glUniform4f(uniformBaseColorFactor, 1, 1, 1, 1);
  };

  const auto noMetallicRoughnessCall = [&]() {
    // Default to no MetallicRoughness
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUniform1i(uniformMetallicRoughnessTexture, 1);
    glUniform1f(uniformMetallicFactor, 1.f);
    glUniform1f(uniformRougnessFactor, 1.f);
  };

  const auto noEmissiveCall = [&]() {
    // Default to no emissive
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUniform1i(uniformEmissiveTexture, 2);
    glUniform3f(uniformEmissiveFactor, 0, 0, 0);
  };

  const auto noOcclusionCall = [&]() {
    // Default to no occlusion
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUniform1i(uniformOcclusionTexture, 0);
    glUniform1f(uniformOcclusionStrength, 1.f);
  };

  const auto noNormalMapCall = [&]() {
    // Default to no Normal map
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUniform1i(uniformNormalMapTexture, 0);
    glUniform1f(uniformNormalMapScale, 1.f);
    glUniform1i(uniformNormalMapUse, false);
  };

  // In order to have a more or less clean implementation,
  // we will implement the texture binding in a specific lambda function bindMaterial(int materialIdx)
  const auto bindMaterial = [&](const auto materialIndex) {
    // Material binding
    if (materialIndex >= 0)
    {

      // Get the material
      const auto &material = model.materials[materialIndex];
      const auto &pbrMetallicRoughness = material.pbrMetallicRoughness;

      // Base color texture call
      if (useBaseColorTexture && pbrMetallicRoughness.baseColorTexture.index >= 0)
      {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureObjects[pbrMetallicRoughness.baseColorTexture.index]);
        glUniform1i(uniformBaseColorTexture, 0);
        glUniform4f(uniformBaseColorFactor,
                    (float)pbrMetallicRoughness.baseColorFactor[0] || 1.f,
                    (float)pbrMetallicRoughness.baseColorFactor[1] || 1.f,
                    (float)pbrMetallicRoughness.baseColorFactor[2] || 1.f,
                    (float)pbrMetallicRoughness.baseColorFactor[3] || 1.f);
      }
      else
      {
        // Default to white texture
        whiteTextureCall();
      }

      // Metallic roughness call
      if (useMetallicRoughnessTexture && pbrMetallicRoughness.metallicRoughnessTexture.index >= 0)
      {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textureObjects[pbrMetallicRoughness.metallicRoughnessTexture.index]);
        glUniform1i(uniformMetallicRoughnessTexture, 1);
        glUniform1f(uniformMetallicFactor, pbrMetallicRoughness.metallicFactor || 1.f);
        glUniform1f(uniformRougnessFactor, pbrMetallicRoughness.roughnessFactor || 1.f);
      }
      else
      {
        // Default to no MetallicRoughness
        noMetallicRoughnessCall();
      }

      // Emissive texture call
      if (useEmissive && material.emissiveTexture.index >= 0)
      {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, textureObjects[material.emissiveTexture.index]);
        glUniform1i(uniformEmissiveTexture, 2);
        glUniform3f(uniformEmissiveFactor,
                    (float)material.emissiveFactor[0] || 0.f,
                    (float)material.emissiveFactor[1] || 0.f,
                    (float)material.emissiveFactor[2] || 0.f);
      }
      else
      {
        noEmissiveCall();
      }

      // Occlusion call
      if (useOcclusion && material.occlusionTexture.index >= 0)
      {
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, textureObjects[material.occlusionTexture.index]);
        glUniform1i(uniformOcclusionTexture, 3);
        glUniform1f(uniformOcclusionStrength, material.occlusionTexture.strength || 1.f);
      }
      else
      {
        noOcclusionCall();
      }

      // Normal map call
      if (useNormalMap && material.normalTexture.index >= 0)
      {
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, textureObjects[material.normalTexture.index]);
        glUniform1i(uniformNormalMapTexture, 4);
        glUniform1f(uniformNormalMapScale, material.normalTexture.scale || 1.f);
        glUniform1i(uniformNormalMapUse, true);
      }
      else
      {
        noNormalMapCall();
      }

      return;
    }

    // Default to white texture
    whiteTextureCall();

    // Default to no MetallicRoughness
    noMetallicRoughnessCall();

    // Default to no emissive
    noEmissiveCall();

    // Default to no occlusion
    noOcclusionCall();

    // Default to no normal map
    noNormalMapCall();
  };

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
    if (uniformLightDirection >= 0)
    {
      if (useLightFromCamera)
      {
        glUniform3f(uniformLightDirection, 0, 0, 1);
      }
      else
      {
        glUniform3f(uniformLightDirection, lightDirectionViewSpace.x, lightDirectionViewSpace.y, lightDirectionViewSpace.z);
      }
    }
    if (uniformLightRadiance >= 0)
    {
      glUniform3f(uniformLightRadiance, lightRadiance.r, lightRadiance.g, lightRadiance.b);
    }

    // The recursive function that should draw a node
    // We use a std::function because a simple lambda cannot be recursive
    const std::function<void(int, const glm::mat4 &)> drawNode =
        [&](int nodeIdx, const glm::mat4 &parentMatrix) {
          // TODO The drawNode function

          // Now we can attack the drawNode function.
          // The first step is to get the node (as a tinygltf::Node)
          // and to compute its model matrix from the parent matrix.
          // Fortunately for you, I included a helper function getLocalToWorldMatrix(node, parentMatrix).
          // If you are interested you can take a look at the code,
          // its mostly calls to glm function to perform the right maths.
          const auto &node = model.nodes[nodeIdx];
          glm::mat4 modelMatrix = getLocalToWorldMatrix(node, parentMatrix);

          // Then we need to ensure that the node has a mesh
          if (node.mesh >= 0)
          {
            // Compute modelViewMatrix
            glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;

            // Compute modelViewProjectionMatrix
            glm::mat4 modelViewProjectionMatrix = projMatrix * modelViewMatrix;

            // Compute normalMatrix
            glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelViewMatrix));

            // Send all of these to the shaders with glUniformMatrix4fv.
            glUniformMatrix4fv(uniformModelMatrix, 1, GL_FALSE, (const GLfloat *)&modelMatrix);
            glUniformMatrix4fv(uniformModelViewMatrix, 1, GL_FALSE, (const GLfloat *)&modelViewMatrix);
            glUniformMatrix4fv(uniformNormalMatrix, 1, GL_FALSE, (const GLfloat *)&normalMatrix);
            glUniformMatrix4fv(uniformModelViewProjMatrix, 1, GL_FALSE, (const GLfloat *)&modelViewProjectionMatrix);

            // Get the mesh
            const auto &mesh = model.meshes[node.mesh];

            // To draw a primitive we need the VAO that we filled for it.
            // Remember that we computed a vector meshIndexToVaoRange with the range of vertex array objects for each mesh
            // (this range being an offset and an number of elements in the vertexArrayObjects vector).
            // Each primitive of index primIdx of the mesh should has its corresponding VAO
            // at vertexArrayObjects[vaoRange.begin + primIdx] if vaoRange is in the range of the mesh.
            const auto vaoRange = meshIndexToVaoRange[node.mesh];

            // Iterate over its primitives to draw them.
            for (size_t primitiveIdx = 0; primitiveIdx < mesh.primitives.size(); ++primitiveIdx)
            {

              // Get the VAO of the primitive (using vertexArrayObjects, the vaoRange and the primitive index) and bind it.
              const auto &vao = VAO[vaoRange.begin + primitiveIdx];

              // Get the current primitive.
              const auto &primitive = mesh.primitives[primitiveIdx];

              // Now we need to check if the primitive has indices by testing if (primitive.indices >= 0).
              // If its the case we should use glDrawElements for the drawing,
              // If not we should use glDrawArrays.

              // Implement the first case, where the primitive has indices.
              if (primitive.indices >= 0)
              {

                // You need to get the accessor of the indices (model.accessors[primitive.indices])
                const auto &accessor = model.accessors[primitive.indices];

                // And the bufferView to compute the total byte offset to use for indices
                const auto &bufferView = model.bufferViews[accessor.bufferView];
                const auto byteOffset = bufferView.byteOffset + accessor.byteOffset;

                // In the drawScene lambda function, just before drawing a specific primitive (before binding its VAO),
                // add a call to bindMaterial with the material index of the primitive as argument.
                bindMaterial(primitive.material);

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
                    (const GLvoid *)byteOffset);
              }
              else
              {

                // Implement the second case, where the primitive does not have indices.

                // For this you need the number of vertex to render.
                // The specification of glTF tells us that we can use the accessor of an arbritrary attribute of the primitive.
                const auto accessorIdx = (*begin(primitive.attributes)).second;
                const auto &accessor = model.accessors[accessorIdx];

                // And the bufferView to compute the total byte offset to use for indices
                const auto &bufferView = model.bufferViews[accessor.bufferView];
                const auto byteOffset = bufferView.byteOffset + accessor.byteOffset;

                // Then call glDrawArrays, passing it
                // the mode of the primitive,
                // 0 as second argument,
                // and accessor.count as last argument.
                glDrawArrays(
                    static_cast<GLenum>(primitive.mode),
                    static_cast<GLint>(0),
                    static_cast<GLsizei>(accessor.count));
              }
            }
          }

          // We then have one last thing to implement,
          // after the if (node.mesh >= 0) body:
          // we need to draw children recursively.

          // After the if body, add a loop over node.children
          for (const auto nodeChildIdx : node.children)
          {
            // Call drawNode on each children.
            // The matrix passed as second argument should be the modelMatrix that has been computed earlier in the function.
            drawNode(nodeChildIdx, modelMatrix);
          }
        };

    // Draw the scene referenced by gltf file
    if (model.defaultScene >= 0)
    {
      // TODO Draw all nodes
      for (const auto nodeIdx : model.scenes[model.defaultScene].nodes)
      {
        drawNode(nodeIdx, glm::mat4(1));
      }
    }

    // Unbind the vertex array
    glBindVertexArray(0);
  };

  if (!m_OutputPath.empty())
  {

    std::cout << COLOR_MAGENTA << "(つ•̀ᴥ•́)つ*:･ﾟ✧ " << COLOR_RESET << " Let's make an image !" << std::endl;

    // Render to image
    std::vector<unsigned char> pixels(m_nWindowWidth * m_nWindowHeight * 3);
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
    std::cout << COLOR_BOLD << std::endl
              << "==============================================" << std::endl;
    std::cout << "┗(＾0＾)┓ Leaving the application" << std::endl;
    std::cout << "==============================================" << COLOR_RESET << std::endl
              << std::endl;
    return 0;
  }
  else
  {

    std::cout << COLOR_MAGENTA << "(つ•̀ᴥ•́)つ*:･ﾟ✧ " << COLOR_RESET << " Let's run in interactive mode !" << std::endl;

    // Loop until the user closes the window
    for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount)
    {
      const auto seconds = glfwGetTime();
      const auto camera = cameraController->getCamera();
      drawScene(camera);

      // GUI code:
      imguiNewFrame();

      {
        ImGui::Begin("GUI");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                    1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        if (ImGui::CollapsingHeader("Camera info"))
        {
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

          if (ImGui::Button("CLI camera args to clipboard"))
          {
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
        if (ImGui::CollapsingHeader("Camera control", ImGuiTreeNodeFlags_DefaultOpen))
        {
          static int radioButtonToggleState = 0;
          ImGui::Text("Change the CameraController type");
          const auto c1 = ImGui::RadioButton("Trackball", &radioButtonToggleState, 0);
          ImGui::SameLine();
          const auto c2 = ImGui::RadioButton("First person", &radioButtonToggleState, 1);
          const auto hasChanged = c1 || c2;
          if (hasChanged)
          {
            if (radioButtonToggleState == 0)
            {
              cameraController = std::make_unique<TrackballCameraController>(m_GLFWHandle.window(), 1.5f * maxDistance);
            }
            else if (radioButtonToggleState == 1)
            {
              cameraController = std::make_unique<FirstPersonCameraController>(m_GLFWHandle.window(), 1.5f * maxDistance);
            }
            cameraController->setCamera(camera);
          }
        }

        // In the GUI, add a new section "Light" (with ImGui::CollapsingHeader())
        if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen))
        {

          // Add a checkbox "light from camera" that control a boolean.
          // If set, then send (0, 0, 1) instead of lightDirection in the drawing function for the uniform controlling the light direction.
          static bool c = false;
          if (ImGui::Checkbox("Use lighting from the camera", &c))
          {
            useLightFromCamera = c;
          }

          // containing two sliders for θ (between 0 and π) and ϕ (between 0 and 2π) angles.
          // When they are changed, compute lightDirection from them using the above formula.
          static float theta = 1.0f;
          static float phi = 1.0f;
          const auto hasChanged = ImGui::SliderFloat("Theta θ", &theta, 0, 3.14) ||
                                  ImGui::SliderFloat("Phi ϕ", &phi, 0, 6.28);
          if (hasChanged)
          {
            // ωi=(sinθcosϕ,cosθ,sinθsinϕ)
            lightDirection = glm::vec3(sin(theta) * cos(phi),
                                       cos(theta),
                                       sin(theta) * sin(phi));
          }

          // Add a ImGui::ColorEdit3 to specify a color and an input float for the intensity factor.
          // If they are changed, update lightIntensity with their product.
          static float color[3] = {lightRadiance.r, lightRadiance.g, lightRadiance.b};
          if (ImGui::ColorEdit3("Light color", color))
          {
            lightRadiance = glm::vec3(color[0], color[1], color[2]);
          }
        }

        // Control the emissive texture
        if (ImGui::CollapsingHeader("Ameliorations", ImGuiTreeNodeFlags_DefaultOpen))
        {

          // Base color texture controls
          static bool s_useBaseColorTexture = useBaseColorTexture;
          if (ImGui::Checkbox("Use base color texture", &s_useBaseColorTexture))
          {
            useBaseColorTexture = s_useBaseColorTexture;
          }

          // Metallic roughness controls
          static bool s_useMetallicRoughnessTexture = useMetallicRoughnessTexture;
          if (ImGui::Checkbox("Use metallic roughness texture", &s_useMetallicRoughnessTexture))
          {
            useMetallicRoughnessTexture = s_useMetallicRoughnessTexture;
          }

          // Emissive controls
          static bool s_useEmissive = useEmissive;
          if (ImGui::Checkbox("Use emissive texture", &s_useEmissive))
          {
            useEmissive = s_useEmissive;
          }

          // Occlusion control
          static bool s_useOcclusion = useOcclusion;
          if (ImGui::Checkbox("Use occlusion mapping", &s_useOcclusion))
          {
            useOcclusion = s_useOcclusion;
          }

          // Normal mapping control
          static bool s_useNormalMap = useNormalMap;
          if (ImGui::Checkbox("Use normal map", &s_useNormalMap))
          {
            useNormalMap = s_useNormalMap;
          }
        }

        ImGui::End();
      }

      imguiRenderFrame();

      glfwPollEvents(); // Poll for and process events

      auto ellapsedTime = glfwGetTime() - seconds;
      auto guiHasFocus = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
      if (!guiHasFocus)
      {
        cameraController->update(float(ellapsedTime));
      }

      m_GLFWHandle.swapBuffers(); // Swap front and back buffers
    }
  }

  std::cout << COLOR_MAGENTA << "ʕ༼◕  ౪  ◕✿༽ʔ    " << COLOR_RESET << "Good bye !" << std::endl;
  std::cout << COLOR_BOLD << std::endl
            << "==============================================" << std::endl;
  std::cout << "┗(＾0＾)┓ Leaving the application" << std::endl;
  std::cout << "==============================================" << COLOR_RESET << std::endl
            << std::endl;
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
    const fs::path &output) : m_nWindowWidth(width),
                              m_nWindowHeight(height),
                              m_AppPath{appPath},
                              m_AppName{m_AppPath.stem().string()},
                              m_ImGuiIniFilename{m_AppName + ".imgui.ini"},
                              m_ShadersRootPath{m_AppPath.parent_path() / "shaders"},
                              m_gltfFilePath{gltfFile},
                              m_OutputPath{output}
{
  if (!lookatArgs.empty())
  {
    m_hasUserCamera = true;
    m_userCamera =
        Camera{glm::vec3(lookatArgs[0], lookatArgs[1], lookatArgs[2]),
               glm::vec3(lookatArgs[3], lookatArgs[4], lookatArgs[5]),
               glm::vec3(lookatArgs[6], lookatArgs[7], lookatArgs[8])};
  }

  if (!vertexShader.empty())
  {
    m_vertexShader = vertexShader;
  }

  if (!fragmentShader.empty())
  {
    m_fragmentShader = fragmentShader;
  }

  ImGui::GetIO().IniFilename =
      m_ImGuiIniFilename.c_str(); // At exit, ImGUI will store its windows
                                  // positions in this file

  glfwSetKeyCallback(m_GLFWHandle.window(), keyCallback);

  printGLVersion();
}
