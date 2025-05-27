#pragma once

#include <cmath>
#include <type_traits>

namespace wil {

// ematical vectors
template<class T, unsigned Dim>
class Vector
{
public:

	// Value uninitialized.
	constexpr Vector() noexcept = default;

	// Copy constructor.
	template<class T2, unsigned Dim2> requires (Dim2 >= Dim)
	constexpr Vector(Vector<T2, Dim2> const& vec) noexcept
	{
		for (unsigned i = 0; i < Dim; i++)
			entries_[i] = static_cast<T>(vec[i]);
	}

	// Value fill initialization.
	explicit constexpr Vector(auto value) noexcept
	{
		for (unsigned i = 0; i < Dim; i++)
			entries_[i] = static_cast<T>(value);
	}

	// Construct with Dim number of arguments.
	template<class... Ts>
		requires (sizeof...(Ts) == Dim && (std::is_convertible_v<Ts, T> && ...))
	constexpr Vector(Ts... args) noexcept
		: entries_{ static_cast<T>(args)... }
	{
	}

	// Access elements.
	constexpr T& operator[](unsigned i)
	{
		return entries_[i];
	}

	// Access elements const.
	constexpr T const& operator[](unsigned i) const
	{
		return entries_[i];
	}

	// Calculate the magnitude.
	auto Norm() const
	{
		using common = std::common_type_t<T, float>;
		common sum = 0;
		for (unsigned i = 0; i < Dim; i++)
			sum += static_cast<common>(entries_[i] * entries_[i]);
		return std::sqrt(sum);
	}

	template<class T2>
	Vector& operator+=(Vector<T2, Dim> const& vec)
	{
		for (unsigned i = 0; i < Dim; i++)
			entries_[i] += static_cast<T>(vec[i]);
		return *this;
	}

	template<class T2>
	Vector& operator*=(Vector<T2, Dim> const& vec)
	{
		for (unsigned i = 0; i < Dim; i++)
			entries_[i] -= static_cast<T>(vec[i]);
		return *this;
	}

	Vector& operator*=(auto scale)
	{
		for (unsigned i = 0; i < Dim; i++)
			entries_[i] *= scale;
		return *this;
	}

private:
	// array implementation
	std::array<T, Dim> entries_;
};

// 1D vector variation (x)
template<class T>
class Vector<T, 1>
{
public:

	T x;

	constexpr Vector() noexcept = default;

	constexpr Vector(auto x) noexcept
	{
		this->x = static_cast<T>(x);
	}

	template<class T2, unsigned Dim2> requires (Dim2 >= 1)
	constexpr Vector(Vector<T2, Dim2> const& vec) noexcept
	{
		x = static_cast<T>(vec[0]);
	}

	constexpr T& operator[](unsigned i)
	{
		return x;
	}

	constexpr T const& operator[](unsigned i) const
	{
		return x;
	}

	auto Norm() const
	{
		return x;
	}

	template<class T2>
	Vector& operator+=(Vector<T2, 1> const& vec)
	{
		x += static_cast<T>(vec.x);
		return *this;
	}

	template<class T2>
	Vector& operator-=(Vector<T2, 1> const& vec)
	{
		x -= static_cast<T>(vec.x);
		return *this;
	}

	Vector& operator*=(auto scale)
	{
		x *= static_cast<T>(scale);
		return *this;
	}
};

// 2D vector variation (x, y)
template<class T>
class Vector<T, 2>
{
public:

	T x, y;

	constexpr Vector() noexcept = default;

	explicit constexpr Vector(auto value) noexcept
	{
		x = y = static_cast<T>(value);
	}

	constexpr Vector(auto x, auto y) noexcept
	{
		this->x = static_cast<T>(x);
		this->y = static_cast<T>(y);
	}

	template<class T2, unsigned Dim2> requires (Dim2 >= 2)
	constexpr Vector(Vector<T2, Dim2> const& vec) noexcept
	{
		x = static_cast<T>(vec[0]);
		y = static_cast<T>(vec[1]);
	}

	constexpr T& operator[](unsigned i)
	{
		return i ? y : x;
	}

	constexpr T const& operator[](unsigned i) const
	{
		return i ? y : x;
	}

	auto Norm() const
	{
		return std::sqrt(x * x + y * y);
	}

	template<class T2>
	Vector& operator+=(Vector<T2, 2> const& vec)
	{
		x += static_cast<T>(vec.x);
		y += static_cast<T>(vec.y);
		return *this;
	}

	template<class T2>
	Vector& operator-=(Vector<T2, 2> const& vec)
	{
		x -= static_cast<T>(vec.x);
		y -= static_cast<T>(vec.y);
		return *this;
	}

	Vector& operator*=(auto scale)
	{
		x *= static_cast<T>(scale);
		y *= static_cast<T>(scale);
		return *this;
	}
};

// 3D vector variation (x, y, z)
template<class T>
class Vector<T, 3>
{
public:

	T x, y, z;

	constexpr Vector() noexcept = default;

	explicit constexpr Vector(auto value) noexcept
	{
		x = y = z = static_cast<T>(value);
	}

	constexpr Vector(auto x, auto y, auto z) noexcept
	{
		this->x = static_cast<T>(x);
		this->y = static_cast<T>(y);
		this->z = static_cast<T>(z);
	}

	template<class T2, unsigned Dim2> requires (Dim2 >= 3)
	constexpr Vector(Vector<T2, Dim2> const& vec) noexcept
	{
		x = static_cast<T>(vec[0]);
		y = static_cast<T>(vec[1]);
		z = static_cast<T>(vec[2]);
	}

	constexpr T& operator[](unsigned i)
	{
		return i == 2 ? z : (i ? y : x);
	}

	constexpr T const& operator[](unsigned i) const
	{
		return i == 2 ? z : (i ? y : x);
	}

	auto Norm() const
	{
		return std::sqrt(x * x + y * y + z * z);
	}

	template<class T2>
	Vector& operator+=(Vector<T2, 3> const& vec)
	{
		x += static_cast<T>(vec.x);
		y += static_cast<T>(vec.y);
		z += static_cast<T>(vec.z);
		return *this;
	}

	template<class T2>
	Vector& operator-=(Vector<T2, 3> const& vec)
	{
		x -= static_cast<T>(vec.x);
		y -= static_cast<T>(vec.y);
		z -= static_cast<T>(vec.z);
		return *this;
	}

	Vector& operator*=(auto scale)
	{
		x *= static_cast<T>(scale);
		y *= static_cast<T>(scale);
		z *= static_cast<T>(scale);
		return *this;
	}
};

// 4D vector variation (x, y, z, w)
template<class T>
class Vector<T, 4>
{
public:

	T x, y, z, w;

	constexpr Vector() noexcept = default;

	explicit constexpr Vector(auto value) noexcept
	{
		x = y = z = w = static_cast<T>(value);
	}

	constexpr Vector(auto x, auto y, auto z, auto w) noexcept
	{
		this->x = static_cast<T>(x);
		this->y = static_cast<T>(y);
		this->z = static_cast<T>(z);
		this->w = static_cast<T>(w);
	}

	template<class T2, unsigned Dim2> requires (Dim2 >= 4)
	constexpr Vector(Vector<T2, Dim2> const& vec) noexcept
	{
		x = static_cast<T>(vec[0]);
		y = static_cast<T>(vec[1]);
		z = static_cast<T>(vec[2]);
		w = static_cast<T>(vec[3]);
	}

	constexpr T& operator[](unsigned i)
	{
		return i == 3 ? w : (i == 2 ? z : (i ? y : x));
	}

	constexpr T const& operator[](unsigned i) const
	{
		return i == 3 ? w : (i == 2 ? z : (i ? y : x));
	}

	auto Norm() const
	{
		return std::sqrt(x * x + y * y + z * z + w * w);
	}

	template<class T2>
	Vector& operator+=(Vector<T2, 4> const& vec)
	{
		x += static_cast<T>(vec.x);
		y += static_cast<T>(vec.y);
		z += static_cast<T>(vec.z);
		w += static_cast<T>(vec.w);
		return *this;
	}

	template<class T2>
	Vector& operator-=(Vector<T2, 4> const& vec)
	{
		x -= static_cast<T>(vec.x);
		y -= static_cast<T>(vec.y);
		z -= static_cast<T>(vec.z);
		w -= static_cast<T>(vec.w);
		return *this;
	}

	Vector& operator*=(auto scale)
	{
		x *= static_cast<T>(scale);
		y *= static_cast<T>(scale);
		z *= static_cast<T>(scale);
		w *= static_cast<T>(scale);
		return *this;
	}
};

// -------------------- Tpedefs ---------------------- //

using Ivec2 = Vector<int, 2>;
using Ivec3 = Vector<int, 3>;
using Ivec4 = Vector<int, 4>;

using Uvec2 = Vector<unsigned, 2>;
using Uvec3 = Vector<unsigned, 3>;
using Uvec4 = Vector<unsigned, 4>;

using Fvec2 = Vector<float, 2>;
using Fvec3 = Vector<float, 3>;
using Fvec4 = Vector<float, 4>;

// ------------------------------------ Functions ------------------------------------ //

// Returns true if 2 vector are equal
template<class T1, class T2, unsigned Dim>
constexpr bool operator==(Vector<T1, Dim> const& vec1, Vector<T2, Dim> const& vec2)
{
	for (unsigned i = 0; i < Dim; i++)
		if (vec1[i] != vec2[i])
			return false;
	return true;
}

// Addition operation bewteen 2 vectors
template<class T1, class T2, unsigned Dim>
constexpr auto operator+(Vector<T1, Dim> const& vec1, Vector<T2, Dim> const& vec2)
{
	Vector<decltype(vec1[0] + vec2[0]), Dim> res;
	for (unsigned i = 0; i < Dim; i++)
		res[i] = vec1[i] + vec2[i];
	return res;
}

// Subtraction operation bewteen 2 vectors
template<class T1, class T2, unsigned Dim>
constexpr auto operator-(Vector<T1, Dim> const& vec1, Vector<T2, Dim> const& vec2)
{
	Vector<decltype(vec1[0] - vec2[0]), Dim> res;
	for (unsigned i = 0; i < Dim; i++)
		res[i] = vec1[i] - vec2[i];
	return res;
}

// Negate operation of vector
template<class T, unsigned Dim>
constexpr auto operator-(Vector<T, Dim> const& vec)
{
	Vector<decltype(-vec[0]), Dim> res;
	for (unsigned i = 0; i < Dim; i++)
		res[i] = -vec[i];
	return res;
}

// Scaler multiplication operation of vector
template<class T, class ScT, unsigned Dim> requires std::is_arithmetic_v<ScT>
constexpr auto operator*(Vector<T, Dim> const& vec, ScT scale)
{
	Vector<decltype(vec[0] * scale), Dim> res;
	for (unsigned i = 0; i < Dim; i++)
		res[i] = vec[i] * scale;
	return res;
}

// Scaler multiplication operation of vector
template<class T, class ScT, unsigned Dim> requires std::is_arithmetic_v<ScT>
constexpr auto operator*(ScT scale, Vector<T, Dim> const& vec)
{
	return vec * scale;
}

// Scaler division operation of vector
template<class T, class ScT, unsigned Dim> requires std::is_arithmetic_v<ScT>
constexpr auto operator/(Vector<T, Dim> const& vec, ScT scale)
{
	Vector<decltype(vec[0] / scale), Dim> res;
	for (unsigned i = 0; i < Dim; i++)
		res[i] = vec[i] / scale;
	return res;
}

// Concatenate two vectors into a larger vector
template<class T1, unsigned Dim1, class T2, unsigned Dim2>
constexpr auto operator&(Vector<T1, Dim1> const& vec1, Vector<T2, Dim2> const& vec2)
{
	Vector<std::common_type_t<T1, T2>, Dim1 + Dim2> res;
	unsigned n = 0;
	for (unsigned i = 0; i < Dim1; i++) res[n++] = vec1[i];
	for (unsigned i = 0; i < Dim2; i++) res[n++] = vec2[i];
	return res;
}

// Concatenate a vector and a number into a larger vector
template<class T, unsigned Dim, class ScT> requires std::is_arithmetic_v<ScT>
constexpr auto operator&(Vector<T, Dim> const& vec, ScT num)
{
	Vector<std::common_type_t<T, ScT>, Dim + 1> res;
	for (unsigned i = 0; i < Dim; i++) res[i] = vec[i];
	res[Dim] = static_cast<T>(num);
	return res;
}

// Concatenate a vector and a number into a larger vector
template<class T, unsigned Dim, class ScT> requires std::is_arithmetic_v<ScT>
constexpr auto operator&(ScT num, Vector<T, Dim> const& vec)
{
	Vector<std::common_type_t<T, ScT>, Dim + 1> res;
	res[0] = static_cast<T>(num);
	for (unsigned i = 0; i < Dim; i++) res[i + 1] = vec[i];
	return res;
}

// Dot product operation
template<class T1, class T2, unsigned Dim>
constexpr auto Dot(Vector<T1, Dim> const& vec1, Vector<T2, Dim> const& vec2)
{
	decltype(vec1[0] * vec2[0]) res = 0;
	for (unsigned i = 0; i < Dim; i++)
		res += vec1[i] * vec2[i];
	return res;
}

// Cross product operation
template<class T1, class T2>
constexpr auto Cross(Vector<T1, 3> const& vec1, Vector<T2, 3> const& vec2)
{
	return Vector<decltype(vec1[0] * vec2[0]), 3>
	{
		vec1[1] * vec2[2] - vec1[2] * vec2[1],
		vec1[2] * vec2[0] - vec1[0] * vec2[2],
		vec1[0] * vec2[1] - vec1[1] * vec2[0]
	};
}

// Compute a normalized vector
template<class T, unsigned Dim>
constexpr auto Normalize(Vector<T, Dim> const& vec)
{
	using common_type = std::common_type_t<T, float>;
	T sum = 0;
	for (unsigned i = 0; i < Dim; i++)
		sum += vec[i] * vec[i];
	auto invsqrt = 1.0f / std::sqrt(static_cast<common_type>(sum));
	Vector<common_type, Dim> res;
	for (unsigned i = 0; i < Dim; i++)
		res[i] = vec[i] * invsqrt;
	return res;
}

// ematical matrix
template<class T, unsigned Rw, unsigned Cn>
class Matrix
{
public:
	// Value uninitialized construction
	constexpr Matrix() noexcept = default;

	// Identity matrix multiplied by value
	explicit constexpr Matrix(T value) noexcept
	{
		std::fill(rows_.begin(), rows_.end(), Vector<T, Cn>(0));
		constexpr unsigned max = Rw > Cn ? Rw : Cn;
		for (unsigned i = 0; i < max; i++)
			rows_[i][i] = value;
	}

	// Construct with Rw * Cn number of arguments
	template<class... Ts> requires (sizeof...(Ts) == Rw)
	constexpr Matrix(Vector<Ts, Cn> const&... rows) noexcept
		: rows_ { static_cast<Vector<T, Cn>>(rows)... }
	{
	}

	// Copy constructor
	template<class T2>
	constexpr Matrix(Matrix<T2, Rw, Cn> const& mat) noexcept
	{
		for (unsigned i = 0; i < Rw; i++)
			rows_[i] = mat.rows_[i];
	}

	// Access modifiers
	constexpr Vector<T, Cn>& operator[](unsigned i)
	{
		return rows_[i];
	}

	// Constant access
	constexpr Vector<T, Cn> const& operator[](unsigned i) const
	{
		return rows_[i];
	}

	template<class T2>
	Matrix& operator+=(Matrix<T2, Rw, Cn> const& mat)
	{
		for (unsigned i = 0; i < Rw; i++)
			rows_[i] += mat[i];
		return *this;
	}

	template<class T2>
	Matrix& operator-=(Matrix<T2, Rw, Cn> const& mat)
	{
		for (unsigned i = 0; i < Rw; i++)
			rows_[i] -= mat[i];
		return *this;
	}

	Matrix& operator*=(auto scale)
	{
		for (unsigned i = 0; i < Rw; i++)
			rows_[i] *= scale;
		return *this;
	}

private:
	// array of column vectors
	std::array<Vector<T, Cn>, Rw> rows_;
};

// -------------------- Tpedefs ---------------------- //

using Imat2x2 = Matrix<int, 2, 2>;
using Imat2x3 = Matrix<int, 2, 3>;
using Imat2x4 = Matrix<int, 2, 4>;
using Imat3x2 = Matrix<int, 3, 2>;
using Imat3x3 = Matrix<int, 3, 3>;
using Imat3x4 = Matrix<int, 3, 4>;
using Imat4x2 = Matrix<int, 4, 2>;
using Imat4x3 = Matrix<int, 4, 3>;
using Imat4x4 = Matrix<int, 4, 4>;

using Umat2x2 = Matrix<unsigned, 2, 2>;
using Umat2x3 = Matrix<unsigned, 2, 3>;
using Umat2x4 = Matrix<unsigned, 2, 4>;
using Umat3x2 = Matrix<unsigned, 3, 2>;
using Umat3x3 = Matrix<unsigned, 3, 3>;
using Umat3x4 = Matrix<unsigned, 3, 4>;
using Umat4x2 = Matrix<unsigned, 4, 2>;
using Umat4x3 = Matrix<unsigned, 4, 3>;
using Umat4x4 = Matrix<unsigned, 4, 4>;

using Fmat2x2 = Matrix<float, 2, 2>;
using Fmat2x3 = Matrix<float, 2, 3>;
using Fmat2x4 = Matrix<float, 2, 4>;
using Fmat3x2 = Matrix<float, 3, 2>;
using Fmat3x3 = Matrix<float, 3, 3>;
using Fmat3x4 = Matrix<float, 3, 4>;
using Fmat4x2 = Matrix<float, 4, 2>;
using Fmat4x3 = Matrix<float, 4, 3>;
using Fmat4x4 = Matrix<float, 4, 4>;

using Imat2 = Imat2x2;
using Imat3 = Imat3x3;
using Imat4 = Imat4x4;

using Umat2 = Umat2x2;
using Umat3 = Umat3x3;
using Umat4 = Umat4x4;

using Fmat2 = Fmat2x2;
using Fmat3 = Fmat3x3;
using Fmat4 = Fmat4x4;

// ------------------------------------ Functions ------------------------------------ //

// Compare equality of 2 matrices
template<class T1, class T2, unsigned Rw, unsigned Cn>
constexpr auto operator==(Matrix<T1, Rw, Cn> const& mat1, Matrix<T2, Rw, Cn> const& mat2)
{
	for (unsigned i = 0; i < Rw; i++)
		if (mat1[i] != mat2[i])
			return false;
	return true;
}

// Addition operation between 2 matrices
template<class T1, class T2, unsigned Rw, unsigned Cn>
constexpr auto operator+(Matrix<T1, Rw, Cn> const& mat1, Matrix<T2, Rw, Cn> const& mat2)
{
	Matrix<decltype(mat1[0][0] + mat2[0][0]), Rw, Cn> res;
	for (unsigned i = 0; i < Rw; i++)
		res[i] = mat1[i] + mat2[i];
	return res;
}

// Subtraction operation between 2 matrices
template<class T1, class T2, unsigned Rw, unsigned Cn>
constexpr auto operator-(Matrix<T1, Rw, Cn> const& mat1, Matrix<T2, Rw, Cn> const& mat2)
{
	Matrix<decltype(mat1[0][0] - mat2[0][0]), Rw, Cn> res;
	for (unsigned i = 0; i < Rw; i++)
		res[i] = mat1[i] - mat2[i];
	return res;
}

// Negate operation of matrices
template<class T, unsigned Rw, unsigned Cn>
constexpr auto operator-(Matrix<T, Rw, Cn> const& mat)
{
	Matrix<decltype(-mat[0][0]), Rw, Cn> res;
	for (unsigned i = 0; i < Rw; i++)
		res[i] = -mat[i];
	return res;
}

// Scalar mulitplication operation of matrices
template<class T1, class ScT, unsigned Rw, unsigned Cn>
constexpr auto operator*(Matrix<T1, Rw, Cn> const& mat, ScT scale)
{
	Matrix<decltype(mat[0][0] * scale), Rw, Cn> res;
	for (unsigned i = 0; i < Rw; i++)
		res[i] = mat * scale;
	return res;
}

// Scalar mulitplication operation of matrices
template<class T1, class ScT, unsigned Rw, unsigned Cn>
constexpr auto operator*(ScT scale, Matrix<T1, Rw, Cn> const& mat)
{
	return mat * scale;
}

// Scalar division operation of matrices
template<class T1, class ScT, unsigned Rw, unsigned Cn>
constexpr auto operator/(Matrix<T1, Rw, Cn> const& mat, ScT scale)
{
	Matrix<decltype(mat[0][0] / scale), Rw, Cn> res;
	for (unsigned i = 0; i < Rw; i++)
		res[i] = mat[i] / scale;
	return res;
}

// Matrix transpose operation
template<class T, unsigned Rw, unsigned Cn>
constexpr auto Transpose(Matrix<T, Rw, Cn> const& mat)
{
	Matrix<T, Cn, Rw> res;
	for (unsigned j = 0; j < Cn; j++)
		for (unsigned i = 0; i < Rw; i++)
			res[j][i] = mat[i][j];
	return res;
}

// Matrix multiplication operation
template<class T1, class T2, unsigned Rw1, unsigned Rw2Cn1, unsigned Cn2>
constexpr auto operator*(Matrix<T1, Rw1, Rw2Cn1> const& mat1, Matrix<T2, Rw2Cn1, Cn2> const& mat2)
{
	Matrix<std::common_type_t<T1, T2>, Rw1, Cn2> res;
	auto m2t = Transpose(mat2);
	
	for (unsigned i = 0; i < Rw1; i++)
		for (unsigned j = 0; j < Cn2; j++)
			res[i][j] = Dot(mat1[i], m2t[j]);
	return res;
}

// Matrix multiplication operation (with vector outcome)
template<class T1, class T2, unsigned Rw1, unsigned Dim>
constexpr auto operator*(Matrix<T1, Rw1, Dim> const& mat, Vector<T2, Dim> const& vec)
{
	Vector<std::common_type_t<T1, T2>, Rw1> res;
	for (unsigned j = 0; j < Rw1; j++)
		res[j] = Dot(mat[j], vec);
	return res;
}

}
