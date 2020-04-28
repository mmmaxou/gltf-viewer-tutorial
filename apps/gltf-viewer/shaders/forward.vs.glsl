#version 330

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

out vec3 vViewSpacePosition;
out vec3 vViewSpaceNormal;
out vec2 vTexCoords;

uniform mat4 uModelViewProjMatrix;
uniform mat4 uModelViewMatrix;
uniform mat4 uNormalMatrix;

void main()
{
  // Tutorial used
  // https://learnopengl.com/Advanced-Lighting/Normal-Mapping
  // Modified to compute everything on the gpu
  // We first calculate the first triangle's edges and delta UV coordinates



  // Compute the TBN matrix
  //vec3 T = normalize(vec3(uModelViewMatrix * vec4(aTangent,   0.0)));
  //vec3 B = normalize(vec3(uModelViewMatrix * vec4(aBitangent, 0.0)));
  //vec3 N = normalize(vec3(uModelViewMatrix * vec4(aNormal,    0.0)));
  //mat3 TBN = mat3(T, B, N);

  vViewSpacePosition = vec3(uModelViewMatrix * vec4(aPosition, 1));
  vViewSpaceNormal = normalize(vec3(uNormalMatrix * vec4(aNormal, 0)));
  vTexCoords = aTexCoords;
  gl_Position =  uModelViewProjMatrix * vec4(aPosition, 1);
}