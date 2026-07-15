#pragma once

#include "raylib.h"

// Minimal directional lambert shader for the whole 3D pass. raylib's
// immediate-mode cubes are flat-colored; without this, a wall or an enemy
// up close is a single unreadable slab. rlgl transforms normals into world
// space for batched geometry, so the shader shades with vertexNormal as-is.
inline Shader LoadShadingShader() {
    const char* vs = R"(
        #version 330
        in vec3 vertexPosition;
        in vec2 vertexTexCoord;
        in vec3 vertexNormal;
        in vec4 vertexColor;
        uniform mat4 mvp;
        out vec4 fragColor;
        out vec3 fragNormal;
        void main() {
            fragColor = vertexColor;
            fragNormal = vertexNormal;
            gl_Position = mvp * vec4(vertexPosition, 1.0);
        }
    )";
    const char* fs = R"(
        #version 330
        in vec4 fragColor;
        in vec3 fragNormal;
        uniform vec4 colDiffuse;
        out vec4 finalColor;
        void main() {
            vec3 n = normalize(fragNormal);
            vec3 keyLight = normalize(vec3(0.35, 0.9, 0.2));
            vec3 fillLight = normalize(vec3(-0.5, 0.25, -0.65));
            float b = 0.55 + 0.38 * max(dot(n, keyLight), 0.0)
                           + 0.20 * max(dot(n, fillLight), 0.0);
            finalColor = vec4(fragColor.rgb * colDiffuse.rgb * b,
                              fragColor.a * colDiffuse.a);
        }
    )";
    return LoadShaderFromMemory(vs, fs);
}
