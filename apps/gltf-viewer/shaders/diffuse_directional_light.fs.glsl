#version 330

in vec3 vViewSpacePosition;
in vec3 vViewSpaceNormal;
in vec2 vTexCoords;

uniform vec3 uLightDirection;
uniform vec3 uLightRadiance;

out vec3 fColor;

void main()
{

    // Let ωi be the lighting direction (a normalized vector of R3) and 
    // Let ωo be the view direction ("i" stands for "incoming" and "o" stands for "outgoing"). 
    // Let Li be the radiance emitted by the directional light (a color also).
    // The radiance L(ωo) in the view direction is then expressed by:

    // L(ωo) = fr(ωi,ωo) * Li * cos(θ){n,ωi}
    vec3 viewNormals = normalize(vViewSpaceNormal);
    float invertPi = 1 / 3.14;
    fColor = vec3(invertPi) * uLightRadiance * dot(uLightDirection, viewNormals) * 1.5;
}