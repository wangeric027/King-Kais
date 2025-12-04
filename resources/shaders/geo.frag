#version 330 core
out vec4 FragColor;

in vec2 texCoords;
uniform sampler2D buffer;


void main() {
   FragColor = texture(buffer, texCoords);
}
