#version 330 core
out vec4 FragColor;

in vec2 texCoords;
uniform sampler2D depthTex;
uniform float near;
uniform float far;

float linearizeDepth(float depth) {
    return (2.0 * near * far) / (far + near - (depth * 2.0 - 1.0) * (far - near));
}

void main()
{
    float d = texture(depthTex, texCoords).r;
    d = linearizeDepth(d);
    FragColor = vec4(vec3(d/ far), 1.0);
}
