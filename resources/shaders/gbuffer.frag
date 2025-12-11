#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gDiffuse;
layout (location = 3) out vec4 gSpec;

in vec2 texCoords;
in vec3 worldPos;
in vec3 norm;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

void main()
{
    // store the fragment position vector in the first gbuffer texture
    gPosition = worldPos;
    // also store the per-fragment normals into the gbuffer
    gNormal = normalize(norm);
    // and the diffuse per-fragment color
    gDiffuse = texture(texture_diffuse1, texCoords);
    gSpec = texture(texture_specular1, texCoords);


    //at this moment, shininess is stored in gSpec and gDiffuse

}
