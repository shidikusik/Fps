#pragma once

#include "raylib.h"
#include "rlgl.h"

// Minimal directional lambert shader for the whole 3D pass. raylib's
// immediate-mode cubes are flat-colored; without this, a wall or an enemy
// up close is a single unreadable slab. rlgl transforms normals into world
// space for batched geometry, so the shader shades with vertexNormal as-is.
// GLSL 330 on desktop GL, GLSL 100 on OpenGL ES (Android).
inline Shader LoadShadingShaderES() {
    const char* vs = R"(
        #version 100
        attribute vec3 vertexPosition;
        attribute vec2 vertexTexCoord;
        attribute vec3 vertexNormal;
        attribute vec4 vertexColor;
        uniform mat4 mvp;
        varying vec4 fragColor;
        varying vec3 fragNormal;
        varying float fragDist;
        void main() {
            fragColor = vertexColor;
            fragNormal = vertexNormal;
            gl_Position = mvp * vec4(vertexPosition, 1.0);
            fragDist = gl_Position.w;
        }
    )";
    const char* fs = R"(
        #version 100
        precision mediump float;
        varying vec4 fragColor;
        varying vec3 fragNormal;
        varying float fragDist;
        uniform vec4 colDiffuse;
        void main() {
            vec3 n = normalize(fragNormal);
            vec3 keyLight = normalize(vec3(0.35, 0.9, 0.2));
            vec3 fillLight = normalize(vec3(-0.5, 0.25, -0.65));
            float b = 0.55 + 0.38 * max(dot(n, keyLight), 0.0)
                           + 0.20 * max(dot(n, fillLight), 0.0);
            vec3 lit = fragColor.rgb * colDiffuse.rgb * b;
            float fog = clamp((fragDist - 25.0) / 90.0, 0.0, 0.55);
            vec3 fogCol = vec3(0.04, 0.015, 0.03);
            gl_FragColor = vec4(mix(lit, fogCol, fog), fragColor.a * colDiffuse.a);
        }
    )";
    return LoadShaderFromMemory(vs, fs);
}

inline Shader LoadShadingShader() {
    int glv = rlGetVersion();
    if (glv == RL_OPENGL_ES_20 || glv == RL_OPENGL_ES_30)
        return LoadShadingShaderES();
    const char* vs = R"(
        #version 330
        in vec3 vertexPosition;
        in vec2 vertexTexCoord;
        in vec3 vertexNormal;
        in vec4 vertexColor;
        uniform mat4 mvp;
        out vec4 fragColor;
        out vec3 fragNormal;
        out float fragDist;
        void main() {
            fragColor = vertexColor;
            fragNormal = vertexNormal;
            gl_Position = mvp * vec4(vertexPosition, 1.0);
            fragDist = gl_Position.w; // ~view-space distance
        }
    )";
    const char* fs = R"(
        #version 330
        in vec4 fragColor;
        in vec3 fragNormal;
        in float fragDist;
        uniform vec4 colDiffuse;
        out vec4 finalColor;
        void main() {
            vec3 n = normalize(fragNormal);
            vec3 keyLight = normalize(vec3(0.35, 0.9, 0.2));
            vec3 fillLight = normalize(vec3(-0.5, 0.25, -0.65));
            float b = 0.55 + 0.38 * max(dot(n, keyLight), 0.0)
                           + 0.20 * max(dot(n, fillLight), 0.0);
            vec3 lit = fragColor.rgb * colDiffuse.rgb * b;
            // distance fog toward the void color, for depth
            float fog = clamp((fragDist - 25.0) / 90.0, 0.0, 0.55);
            vec3 fogCol = vec3(0.04, 0.015, 0.03);
            finalColor = vec4(mix(lit, fogCol, fog), fragColor.a * colDiffuse.a);
        }
    )";
    return LoadShaderFromMemory(vs, fs);
}
