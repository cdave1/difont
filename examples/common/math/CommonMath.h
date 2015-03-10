/*
 * Copyright (c) 2015 David Petrie david@davidpetrie.com
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from the
 * use of this software. Permission is granted to anyone to use this software for
 * any purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not claim
 * that you wrote the original software. If you use this software in a product, an
 * acknowledgment in the product documentation would be appreciated but is not
 * required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#ifndef DIFONT_EXAMPLES_MATH_H
#define DIFONT_EXAMPLES_MATH_H

#include <math.h>

typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];
typedef vec4_t color4_t;

#define vec2Set(__v, __x, __y) __v[0] = __x; __v[1] = __y;
#define vec3Set(__v, __x, __y, __z) __v[0] = __x; __v[1] = __y; __v[2] = __z;
#define vec4Set(__v, __x, __y, __z, __u) __v[0] = __x; __v[1] = __y; __v[2] = __z; __v[3] = __u;

typedef struct vertex {
    float xyz[3];
    float st[2];
    float rgba[4];
} vertex_t;

namespace difont {
    namespace examples {
        class Math {
        public:
            static void Cross3(vec3_t vOut, const vec3_t a, const vec3_t b);

            static void Normalize3(vec3_t vOut, const vec3_t vec);

            static void MatrixMultiply(float *mOut,
                                       const float *mA,
                                       const float *mB);

            static void MatrixIdentity(float *mOut);

            static void MatrixTranslation(float *mOut,
                                          const float fX,
                                          const float fY,
                                          const float fZ);

            static void MatrixRotationZ(float *mOut,
                                        const float fAngle);

            static void MatrixPerspectiveFovRH(float *mOut,
                                               const float fFOVy,
                                               const float fAspect,
                                               const float fNear,
                                               const float fFar);

            static void MatrixLookAtRH(float *mOut,
                                       const vec3_t vEye,
                                       const vec3_t vAt,
                                       const vec3_t vUp);

            static void Ortho(float *mOut,
                              float left,
                              float right,
                              float bottom,
                              float top,
                              float zNear,
                              float zFar);
            
        };
    }
}

#endif