#version 330

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in vec3 aTangent;

out vec3 vViewSpacePosition;
out vec3 vViewSpaceNormal;
out vec2 vTexCoords;

out vec3 vTangentLightPos;
out vec3 vTangentViewPos;
out vec3 vTangentFragPos;

uniform mat4 uModelViewProjMatrix;
uniform mat4 uModelViewMatrix;
uniform mat4 uNormalMatrix;
uniform bool uNormalMapUse;

void main()
{

  vViewSpacePosition = vec3(uModelViewMatrix * vec4(aPosition, 1));
  vViewSpaceNormal = normalize(vec3(uNormalMatrix * vec4(aNormal, 0)));
  vTexCoords = aTexCoords;

  if(uNormalMapUse) {
    // Tutorial used
    // https://learnopengl.com/Advanced-Lighting/Normal-Mapping
    // Compute the TBN matrix

    // Solution 2:
    // send a tangent-space light position, view position, and vertex position to the fragment shader
    mat3 normalMatrix = transpose(inverse(mat3(uModelViewMatrix)))
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    // re-orthogonalize T with respect to N
    T = normalize(T - dot(T, N) * N);
    // then retrieve perpendicular vector B with the cross product of T and N
    vec3 B = cross(N, T);

    mat3 TBN = transpose(mat3(T, B, N));


    vTangentLightPos = TBN * lightPos;
    vTangentViewPos  = TBN * viewPos;
    vTangentFragPos  = TBN * vViewSpacePosition;
  }

  gl_Position =  uModelViewProjMatrix * vec4(aPosition, 1);
}