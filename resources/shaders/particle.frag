#version 330 core
in vec4 fragColor;
out vec4 outColor;

void main() {
    // Make particles circular with soft edges
    vec2 coord = gl_PointCoord - vec2(0.5);
    float dist = length(coord);

    if (dist > 0.5)
        discard;

    // Soft falloff for glow effect
    float alpha = (1.0 - dist * 2.0) * fragColor.a;

    outColor = vec4(fragColor.rgb, alpha);
}
