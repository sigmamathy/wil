#pragma once

#include "algebra.hpp"

namespace wil {

// Translation matrix (2D)
Fmat4 TranslateModel(Fvec2 position);

// Translation matrix
Fmat4 TranslateModel(Fvec3 position);

// Rotation matrix (rotate about an axis)
Fmat4 RotateModel(float rad, Fvec3 axis = Fvec3(0.0f, 0.0f, -1.0f));

// Scale matrix (2D)
Fmat4 ScaleModel(Fvec2 scale);

// Scale matrix
Fmat4 ScaleModel(Fvec3 scale);

// 3 dimensional view matrix, look at a certain position
Fmat4 LookAtView(Fvec3 position, Fvec3 orientation, Fvec3 up = Fvec3(0.0f, 1.0f, 0.0f));

// Orthogonal projection matrix
Fmat4 OrthogonalProjection(float left, float right, float bottom, float top, float near = -1.0f, float far = 1.0f);

// Perspective projection matrix
Fmat4 PerspectiveProjection(float fovy, float aspect, float near, float far);

}

