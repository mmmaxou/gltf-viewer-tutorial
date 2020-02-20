#version 330

in vec3 vViewSpacePosition;
in vec3 vViewSpaceNormal;
in vec2 vTexCoords;

uniform vec3 uLightDirection;
uniform vec3 uLightRadiance;

uniform sampler2D uBaseColorTexture;
uniform vec4 uBaseColorFactor;

uniform float uMetallicFactor;
uniform float uRougnessFactor;
uniform sampler2D uMetallicRoughnessTexture;

out vec3 fColor;

// Constants
const float GAMMA = 2.2;
const float INV_GAMMA = 1. / GAMMA;
const float M_PI = 3.141592653589793;
const float M_1_PI = 1.0 / M_PI;
vec3 dielectricSpecular = vec3(0.04, 0.04, 0.04);
vec3 black = vec3(0, 0, 0);

// We need some simple tone mapping functions
// Basic gamma = 2.2 implementation
// Stolen here: https://github.com/KhronosGroup/glTF-Sample-Viewer/blob/master/src/shaders/tonemapping.glsl

// linear to sRGB approximation
// see http://chilliant.blogspot.com/2012/08/srgb-approximations-for-hlsl.html
vec3 LINEARtoSRGB(vec3 color)
{
  return pow(color, vec3(INV_GAMMA));
}

// sRGB to linear approximation
// see http://chilliant.blogspot.com/2012/08/srgb-approximations-for-hlsl.html
vec4 SRGBtoLINEAR(vec4 srgbIn)
{
  return vec4(pow(srgbIn.xyz, vec3(GAMMA)), srgbIn.w);
}

void main()
{
  vec3 N = normalize(vViewSpaceNormal);
  vec3 L = uLightDirection;
  vec3 V = normalize(-vViewSpacePosition);
  vec3 H = normalize(L + V);

  // Dots
  float NdotL = clamp(dot(N, L), 0, 1);
  float VdotH = clamp(dot(V, H), 0, 1);
  float NdotV = clamp(dot(N, V), 0, 1);
  float NdotH = clamp(dot(N, H), 0, 1);
  float NdotL_2 = NdotL * NdotL;
  float NdotV_2 = NdotV * NdotV;
  float NdotH_2 = NdotH * NdotH;

  // Base texture
  vec4 baseColorFromTexture = SRGBtoLINEAR(texture(uBaseColorTexture, vTexCoords));
  vec4 baseColor = baseColorFromTexture * uBaseColorFactor;
  // vec3 diffuse = baseColor.rgb * M_1_PI * NdotL;

  // Metallic values
  vec4 metallicRoughnessTexture = texture(uMetallicRoughnessTexture, vTexCoords);
  float metallic = metallicRoughnessTexture.b * uMetallicFactor;
  float roughness = metallicRoughnessTexture.g * uRougnessFactor;
  float alpha = roughness * roughness;
  float alpha_2 = alpha * alpha;


  // Metallic-Roughness model
  vec3 C_diffuse = mix(baseColor.rgb * (1 - dielectricSpecular.r), black, metallic);
  vec3 F0 = mix(dielectricSpecular, baseColor.rgb, metallic);

  // Surface Reflection Ratio (F)
  // Fresnel Schlick
  // Shclick factor
  float baseShlickFactor = (1-VdotH);
  float shlickFactor = baseShlickFactor * baseShlickFactor; // power 2
  shlickFactor *= shlickFactor; // power 4
  shlickFactor *= baseShlickFactor; // power 5
  vec3 F = F0 + (1 - F0) * shlickFactor;

  // Geometric Occlusion (G)
  // Smith Joint GGX
  float vis_denom = (NdotL) * sqrt(NdotV_2*(1-alpha_2)+alpha_2) + (NdotV) * sqrt(NdotL_2*(1-alpha_2) + alpha_2);
  float Vis = vis_denom <= 0 ? 0 : 0.5 / vis_denom;

  // Microfacet Distribution (D)
  // Trowbridge-Reitz
  float d_factor = (NdotH_2*(alpha_2-1)+1);
  float D = alpha_2 / (M_PI * d_factor * d_factor);

  // Diffuse Term (diffuse)
  // Lambert
  vec3 diffuse = C_diffuse * M_1_PI;
  vec3 f_diffuse = (1 - F) * diffuse;
  vec3 f_specular = F * Vis * D;
  vec3 f = f_diffuse + f_specular;

  // fColor = LINEARtoSRGB(diffuse);
  fColor = LINEARtoSRGB(f * uLightRadiance * NdotL);
  //fColor = vec3(1, 0, 0) * uLightRadiance;
}
