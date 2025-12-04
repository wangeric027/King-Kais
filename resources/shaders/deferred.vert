#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uvCoords;

out vec2 texCoords;

void main() {
   vec4 homogObjectPos = vec4(position, 1.0f);
   gl_Position = homogObjectPos;
   texCoords = uvCoords;
}
