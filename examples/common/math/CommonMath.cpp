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

#include "CommonMath.h"

using namespace difont::examples;

float aglDot(const vec3_t a, const vec3_t b) {
    return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}


void Math::MatrixIdentity(float *mOut) {
    mOut[ 0] = 1; mOut[ 4] = 0; mOut[ 8] = 0; mOut[12] = 0;
    mOut[ 1] = 0; mOut[ 5] = 1; mOut[ 9] = 0; mOut[13] = 0;
    mOut[ 2] = 0; mOut[ 6] = 0; mOut[10] = 1; mOut[14] = 0;
    mOut[ 3] = 0; mOut[ 7] = 0; mOut[11] = 0; mOut[15] = 1;
}


void Math::MatrixTranslation(float *mOut, const float fX, const float fY, const float fZ) {
    mOut[ 0] = 1; mOut[ 4] = 0; mOut[ 8] = 0; mOut[12] = fX;
    mOut[ 1] = 0; mOut[ 5] = 1; mOut[ 9] = 0; mOut[13] = fY;
    mOut[ 2] = 0; mOut[ 6] = 0; mOut[10] = 1; mOut[14] = fZ;
    mOut[ 3] = 0; mOut[ 7] = 0; mOut[11] = 0; mOut[15] = 1;
}


void Math::MatrixRotationZ(float *mOut, const float fAngle) {
    float fsin = sinf(fAngle);
    float fcos = cosf(fAngle);

    mOut[ 0] = fcos;    mOut[ 4] = fsin;    mOut[ 8] = 0; mOut[12] = 0;
    mOut[ 1] = -fsin;   mOut[ 5] = fcos;    mOut[ 9] = 0; mOut[13] = 0;
    mOut[ 2] = 0;       mOut[ 6] = 0;       mOut[10] = 1; mOut[14] = 0;
    mOut[ 3] = 0;       mOut[ 7] = 0;       mOut[11] = 0; mOut[15] = 1;
}


void Math::MatrixPerspectiveFovRH(float *mOut,
                                  const float fov_y,
                                  const float aspect,
                                  const float near,
                                  const float far) {
    float f, n, realAspect;

    realAspect = aspect;

    // cotangent(a) == 1.0f / tan(a);
    f = 1.0f / (float)tan(fov_y * 0.5f);
    n = 1.0f / (near - far);

    mOut[ 0] = f / realAspect; mOut[ 4] = 0;   mOut[ 8] = 0;                 mOut[12] = 0;
    mOut[ 1] = 0;              mOut[ 5] = f;   mOut[ 9] = 0;                 mOut[13] = 0;
    mOut[ 2] = 0;              mOut[ 6] = 0;   mOut[10] = (far + near) * n;  mOut[14] = (2 * far * near) * n;
    mOut[ 3] = 0;              mOut[ 7] = 0;   mOut[11] = -1;                mOut[15] = 0;
}


void Math::Cross3(vec3_t vOut, const vec3_t a, const vec3_t b) {
    vec3Set(vOut,
            (a[1] * b[2]) - (a[2] * b[1]),
            (a[2] * b[0]) - (a[0] * b[2]),
            (a[0] * b[1]) - (a[1] * b[0]));
}


void Math::Normalize3(vec3_t vOut, const vec3_t vec) {
    float f;
    double temp;

    temp = (double)(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
    temp = 1.0 / sqrt(temp);
    f = (float)temp;
    vec3Set(vOut, vec[0] * f, vec[1] * f, vec[2] * f);
}


void Math::MatrixLookAtRH(float *mOut, const vec3_t pos, const vec3_t target, const vec3_t up) {
    vec3_t f, x_axis, y_axis, z_axis;

    f[0] = pos[0] - target[0];
    f[1] = pos[1] - target[1];
    f[2] = pos[2] - target[2];

    Math::Normalize3(z_axis, f);
    Math::Cross3(x_axis, up, z_axis);
    Math::Normalize3(x_axis, x_axis);
    Math::Cross3(y_axis, z_axis, x_axis);

    mOut[ 0] = x_axis[0];   mOut[ 4] = x_axis[1];   mOut[ 8] = x_axis[2];   mOut[12] = -aglDot(x_axis, pos);
    mOut[ 1] = y_axis[0];   mOut[ 5] = y_axis[1];   mOut[ 9] = y_axis[2];   mOut[13] = -aglDot(y_axis, pos);
    mOut[ 2] = z_axis[0];   mOut[ 6] = z_axis[1];   mOut[10] = z_axis[2];   mOut[14] = -aglDot(z_axis, pos);
    mOut[ 3] = 0;           mOut[ 7] = 0;           mOut[11] = 0;           mOut[15] = 1;
}


void Math::MatrixMultiply(float *mOut,
                          const float *mA,
                          const float *mB) {
    for(int i = 0; i < 4; i++) {
        mOut[i*4] =   mA[i*4] * mB[ 0] + mA[i*4+1] * mB[ 4] + mA[i*4+2] * mB[ 8] + mA[i*4+3] * mB[12];
        mOut[i*4+1] = mA[i*4] * mB[ 1] + mA[i*4+1] * mB[ 5] + mA[i*4+2] * mB[ 9] + mA[i*4+3] * mB[13];
        mOut[i*4+2] = mA[i*4] * mB[ 2] + mA[i*4+1] * mB[ 6] + mA[i*4+2] * mB[10] + mA[i*4+3] * mB[14];
        mOut[i*4+3] = mA[i*4] * mB[ 3] + mA[i*4+1] * mB[ 7] + mA[i*4+2] * mB[11] + mA[i*4+3] * mB[15];
    }
}


void Math::Ortho(float *mOut, float left, float right, float bottom, float top, float near, float far) {
    Math::MatrixIdentity(mOut);

    float w = (right - left);
    float h = (top - bottom);
    float d = (far - near);

    mOut[ 0] = 2.0f / w;    mOut[ 4] = 0;           mOut[ 8] = 0;       mOut[12] = -1;
    mOut[ 1] = 0;           mOut[ 5] = 2.0f / h;    mOut[ 9] = 0;       mOut[13] = 1;
    mOut[ 2] = 0;           mOut[ 6] = 0;           mOut[10] = -2.0f/d; mOut[14] = 0;
    mOut[ 3] = 0;           mOut[ 7] = 0;           mOut[11] = 0;       mOut[15] = 1;
}
