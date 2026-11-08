#version 330 core

in vec2 local_pos;

out vec4 out_col;

uniform float halfWidth;
uniform float circleRadius;
uniform float circleCurvature;
uniform float paletteRange;
uniform float palettePower;

vec3 circleColor(float radius) {
    const vec3 darkOrange = vec3(0.70, 0.28, 0.06);
    const vec3 darkViolet = vec3(0.30, 0.08, 0.46);

    float position = clamp(log2(1.0 / radius) / paletteRange, 0.0, 1.0);
    position = pow(position, palettePower);
    return mix(darkOrange, darkViolet, position);
}

float innerShadowAlpha(float radialDistance, float radius) {
    if (circleCurvature < 0.0 || radialDistance >= 0.0) {
        return 0.0;
    }

    const float shadowWidth = 0.60;
    float fade = 1.0 - smoothstep(0.0, shadowWidth, -radialDistance);
    float strength = mix(0.15, 0.55, sqrt(clamp(radius, 0.0, 1.0)));
    return strength * fade;
}

void main() {
    float d = length(local_pos) - 1.0;
    float outlineAlpha = abs(d) < halfWidth ? 1.0 : 0.0;
    float shadowAlpha = innerShadowAlpha(d, circleRadius);
    float alpha = max(outlineAlpha, shadowAlpha);

    if (alpha <= 0.0) {
        discard;
    }

    out_col = vec4(circleColor(circleRadius), alpha);
}
