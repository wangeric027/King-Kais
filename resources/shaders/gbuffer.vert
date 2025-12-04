#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uvCoords;
out vec3 norm;
out vec3 worldPos;
out vec2 texCoords;

uniform mat4 modelMVP;
uniform mat4 normalMatrix;
uniform mat4 ctm;


void main() {
   vec4 homogObjectPos = vec4(position, 1.0f);
   gl_Position = modelMVP * homogObjectPos;
   norm = vec3(normalMatrix * vec4(normal, 0.0f)); //world norm
   worldPos = vec3(ctm * homogObjectPos);
   texCoords = uvCoords;
}
