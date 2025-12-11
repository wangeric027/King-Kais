#version 330 core
in vec2 uv;
out vec4 fragColor;

uniform sampler2D colorTexture;
uniform sampler2D depthTexture;
uniform sampler3D lutTexture;
uniform vec2 texelSize;

// Effect toggles
uniform bool useGrayscale;
uniform bool useInvert;
uniform bool useEdgeDetection;
uniform bool useVignette;
uniform bool useDepthVisualization;
uniform bool usePixelation;
uniform bool useToonShading;
uniform int lutIndex;

// Edge detection (Sobel)
float detectEdge(vec2 uv, vec2 texelSize) {
    float tl = length(texture(colorTexture, uv + vec2(-texelSize.x,  texelSize.y)).rgb);
    float t  = length(texture(colorTexture, uv + vec2( 0.0,          texelSize.y)).rgb);
    float tr = length(texture(colorTexture, uv + vec2( texelSize.x,  texelSize.y)).rgb);
    float l  = length(texture(colorTexture, uv + vec2(-texelSize.x,  0.0)).rgb);
    float r  = length(texture(colorTexture, uv + vec2( texelSize.x,  0.0)).rgb);
    float bl = length(texture(colorTexture, uv + vec2(-texelSize.x, -texelSize.y)).rgb);
    float b  = length(texture(colorTexture, uv + vec2( 0.0,         -texelSize.y)).rgb);
    float br = length(texture(colorTexture, uv + vec2( texelSize.x, -texelSize.y)).rgb);

    float sobelX = -tl - 2.0*l - bl + tr + 2.0*r + br;
    float sobelY = -tl - 2.0*t - tr + bl + 2.0*b + br;
    return sqrt(sobelX * sobelX + sobelY * sobelY);
}

// Toon shading
vec3 toonShade(vec3 color) {
    float intensity = dot(color, vec3(0.299, 0.587, 0.114));

    float quantized;
    if (intensity > 0.8) quantized = 1.0;
    else if (intensity > 0.5) quantized = 0.7;
    else if (intensity > 0.3) quantized = 0.4;
    else quantized = 0.2;

    vec3 normalizedColor = color / max(intensity, 0.001);
    return normalizedColor * quantized;
}

// LUT
vec3 applyLUT(vec3 color) {
    // Clamp color to [0, 1] range
    color = clamp(color, 0.0, 1.0);
    // Sample the 3D LUT texture
    return texture(lutTexture, color).rgb;
}

void main() {
    // Determine which UV to use (pixelated or normal)
    vec2 sampleUV = uv;

    if (usePixelation) {
        float pixelSize = 8.0;
        sampleUV = floor(uv / (texelSize * pixelSize)) * (texelSize * pixelSize);
    }

    vec3 color;

    // Start with either depth or color (using the correct UV)
    if (useDepthVisualization) {
        float depth = texture(depthTexture, sampleUV).r;
        depth = pow(depth, 50.0);
        color = vec3(depth);
    } else {
        color = texture(colorTexture, sampleUV).rgb;
    }

    // Apply effects in order

    // 1. Toon Shading
    if (useToonShading) {
        color = toonShade(color);
    }

    // 2. LUT Color Grading
    if (lutIndex > 0) {
        color = applyLUT(color);
    }

    // 3. Edge detection
    if (useEdgeDetection) {
        float edge = detectEdge(sampleUV, texelSize);
        color = color * (1.0 - edge);
    }

    // 4. Grayscale
    if (useGrayscale) {
        float gray = dot(color, vec3(0.299, 0.587, 0.114));
        color = vec3(gray);
    }

    // 5. Invert
    if (useInvert) {
        color = 1.0 - color;
    }

    // 6. Vignette
    if (useVignette) {
        vec2 center = uv - 0.5;
        float dist = length(center);
        float vignette = smoothstep(0.8, 0.3, dist);
        color *= vignette;
    }

    fragColor = vec4(color, 1.0);
}
