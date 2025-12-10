
#version 330 core

in vec2 uv;
out vec4 fragColor;

uniform sampler2D colorTexture;
uniform sampler2D depthTexture;
uniform vec2 texelSize;

// Effect toggles
uniform bool useGrayscale;
uniform bool useInvert;
uniform bool useEdgeDetection;
uniform bool useVignette;
uniform bool useDepthVisualization;
uniform bool usePixelation;

// Edge detection (Sobel) -> only one that needed its own because everything is simple
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

void main() {
    vec3 color;

    // Start with either depth or color
    if (useDepthVisualization) {
        float depth = texture(depthTexture, uv).r;
        depth = pow(depth, 50.0);
        color = vec3(depth);
    } else {
        color = texture(colorTexture, uv).rgb;
    }

    // Apply effects

    //0. Pixelation - added this last
    if (usePixelation) {
        float pixelSize = 8.0;  // this will affect size of pixels 8 is good
        vec2 pixelUV = floor(uv / (texelSize * pixelSize)) * (texelSize * pixelSize);
        color = texture(colorTexture, pixelUV).rgb;
    }

    // 1. Edge detection
    if (useEdgeDetection) {
        float edge = detectEdge(uv, texelSize);
        color = color * (1.0 - edge);
    }

    // 2. Grayscale
    if (useGrayscale) {
        float gray = dot(color, vec3(0.299, 0.587, 0.114));
        color = vec3(gray);
    }

    // 3. Invert
    if (useInvert) {
        color = 1.0 - color;
    }

    // 4. Vignette - Stylized to get that oldish burnt edges look
    if (useVignette) {
        vec2 center = uv - 0.5;
        float dist = length(center);
        float vignette = smoothstep(0.8, 0.3, dist);
        color *= vignette;
    }

    fragColor = vec4(color, 1.0);
}
