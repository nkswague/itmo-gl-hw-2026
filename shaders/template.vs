#version 330 core

layout(location=0) in vec2 pos;

uniform mat4 mvp;
uniform float halfWidth;

out vec2 local_pos;

void main() {
    local_pos = pos * (1.0 + halfWidth);
    gl_Position = mvp * vec4(local_pos, 0.0, 1.0);
}
