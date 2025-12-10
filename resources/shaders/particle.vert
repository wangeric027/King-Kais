#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in float life;
layout(location = 3) in float size;

out vec4 fragColor;

uniform mat4 viewProj;

void main() {
    gl_Position = viewProj * vec4(position, 1.0);
    gl_PointSize = size;  // Scale with distance
    fragColor = color;
    fragColor.a *= life;  // Fade based on life
}
