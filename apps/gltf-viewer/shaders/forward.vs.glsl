#version 330

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in vec3 aTangent;

out vec3 vViewSpacePosition;
out vec3 vViewSpaceNormal;
out vec2 vTexCoords;
out vec3 vTangentLightDirection;
out vec3 vTangentPosition;

uniform mat4 uModelViewProjMatrix;
uniform mat4 uModelViewMatrix;
uniform mat4 uNormalMatrix;
uniform vec3 uLightDirection;
uniform bool uNormalMapUse;

void main()
{

  vViewSpacePosition = vec3(uModelViewMatrix * vec4(aPosition, 1));
  vTexCoords = aTexCoords;

  if(uNormalMapUse) {
    // Tutorial used
    // https://learnopengl.com/Advanced-Lighting/Normal-Mapping
    // Compute the TBN matrix

    // Solution 2:
    // send a tangent-space light position, view position, and vertex position to the fragment shader
    mat3 normalMatrix = mat3(uNormalMatrix);
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    mat3 TBN = transpose(mat3(T, B, N));

    vTangentLightDirection = TBN * uLightDirection;
    vTangentPosition  = TBN * vViewSpacePosition;
  } else {
    vViewSpaceNormal = normalize(vec3(uNormalMatrix * vec4(aNormal, 0)));
  }

  gl_Position =  uModelViewProjMatrix * vec4(aPosition, 1);
}