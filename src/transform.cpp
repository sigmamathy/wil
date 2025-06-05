#include <wil/transform.hpp>

namespace wil {

Fmat4 TranslateModel(Fvec2 position)
{
	return TranslateModel(Fvec3(position.x, position.y, 0.0f));
}

Fmat4 TranslateModel(Fvec3 position)
{
	// position = {1,1,1};
	return {
		Fvec4(1, 0, 0, position.x),
		Fvec4(0, 1, 0, position.y),
		Fvec4(0, 0, 1, position.z),
		Fvec4(0, 0, 0, 1),
	};
}

Fmat4 RotateModel(float rad, Fvec3 axis)
{
	if (rad == 0)
		return Fmat4(1);
	axis = Normalize(axis);
	float c = std::cos(rad), s = std::sin(rad);
	auto fun1 = [&](int i) -> float { return c + axis[i] * axis[i] * (1 - c); };
	auto fun2 = [&](int i, int j, int k) -> float { return (1 - c) * axis[i] * axis[j] + s * axis[k]; };
	auto fun3 = [&](int i, int j, int k) -> float { return (1 - c) * axis[i] * axis[j] - s * axis[k]; };

	return {
		Fvec4(fun1(0), fun3(0, 1, 2), fun2(0, 2, 1), 0),
		Fvec4(fun2(0, 1, 2), fun1(1), fun3(1, 2, 0), 0),
		Fvec4(fun3(0, 2, 1), fun2(1, 2, 0), fun1(2), 0),
		Fvec4(0, 0, 0, 1),
	};
}

Fmat4 ScaleModel(Fvec2 scale)
{
	return ScaleModel(Fvec3(scale.x, scale.y, 1.0f));
}

Fmat4 ScaleModel(Fvec3 scale)
{
	return {
		Fvec4(scale.x, 0, 0, 0),
		Fvec4(0, scale.y, 0, 0),
		Fvec4(0, 0, scale.z, 0),
		Fvec4(0, 0, 0, 1)
	};
}

Fmat4 LookAtView(Fvec3 position, Fvec3 orientation, Fvec3 up)
{
	Fmat4 res(1.0f);
	Fvec3 f = Normalize(orientation);
	Fvec3 s = Normalize(Cross(f, up));
	Fvec3 u = Cross(s, f);
	res[0][0] = s.x;
	res[0][1] = s.y;
	res[0][2] = s.z;
	res[1][0] = u.x;
	res[1][1] = u.y;
	res[1][2] = u.z;
	res[2][0] = -f.x;
	res[2][1] = -f.y;
	res[2][2] = -f.z;
	res[0][3] = -Dot(s, position);
	res[1][3] = -Dot(u, position);
	res[2][3] = Dot(f, position);
	return res;
}

Fmat4 OrthogonalProjection(float left, float right, float bottom, float top, float near, float far)
{
	float x = right - left,
		y = top - bottom,
		z = far - near;

	return {
		Fvec4(2 / x, 0, 0, -(right+left)/x),
		Fvec4(0, 2 / y, 0, -(top+bottom)/y),
		Fvec4(0, 0, -2 / z, -(far+near)/z),
		Fvec4(0, 0, 0, 1)
	};
}

Fmat4 PerspectiveProjection(float fovy, float aspect, float near, float far)
{
	Fmat4 res(0.0f);
	float const tanHalfFovy = tan(fovy / 2.0f);
	res[0][0] = 1.0f / (-aspect * tanHalfFovy);
	res[1][1] = 1.0f / (tanHalfFovy);
	res[2][2] = -(far + near) / (far - near);
	res[3][2] = -1.0f;
	res[2][3] = -(2.0f * far * near) / (far - near);
	return res;
}

}
