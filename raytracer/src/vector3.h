#pragma once

/////////////////////////////////////////////////////////////////
//
// class Vector3 - a simple 3D vector class
//
/////////////////////////////////////////////////////////////////

class Vector3
{
public:

	float x, y, z;

	// Constructors
	Vector3() {}

	Vector3(const Vector3& v) : x(v.x), y(v.y), z(v.z) {}

	Vector3(float nx, float ny, float nz) : x(nx), y(ny), z(nz) {}

	Vector3(float f) : x(f), y(f), z(f) {}

	// Assignment
	Vector3 &operator =(const Vector3 &v) {
		x = v.x; y = v.y; z = v.z;
		return *this;
	}

	bool operator ==(const Vector3& v) const {
		return x == v.x && y == v.y && z == v.z;
	}

	bool operator !=(const Vector3& v) const {
		return x != v.x || y != v.y || z != v.z;
	}

	// Geometric
	void normalize()
	{
		float magSqrd = magnitudeSquared();
		if (magSqrd > 0.0f)
		{
			float oneOverMag = 1.f / sqrt(magSqrd);
			x *= oneOverMag;
			y *= oneOverMag;
			z *= oneOverMag;
		}
	}

	Vector3 & normalized()
	{
		normalize();
		return *this;
	}

	float dot(const Vector3& v) const
	{
		return x * v.x + y * v.y + z * v.z;
	}

	inline float magnitudeSquared()
	{
		return x * x + y * y + z * z;
	}

	inline float magnitude()
	{
		return sqrtf(magnitudeSquared());
	}

	// Math
	inline Vector3 & reset()
	{
		x = y = z = 0.f;
		return *this;
	}

	static inline Vector3 zero()
	{
		return Vector3().reset();
	}


	// Note: *this is pointing away from angle of incidence
	//Vector3 reflect(const Vector3& v) const
	//{
	//	return v * 2.f * v.dot(*this) - *this;
	//}

	// Note: *this is pointing towards angle of incidence
	inline Vector3 reflect(const Vector3& v) const
	{
		return *this - (v * 2.f * v.dot(*this));
	}

	inline Vector3 operator -() const
	{
		return Vector3(-x, -y, -z);
	}

	inline Vector3 operator +(const Vector3& v) const
	{
		return Vector3(v.x + x, v.y + y, v.z + z);
	}

	inline Vector3 operator -(const Vector3& v) const
	{
		return Vector3(x - v.x, y - v.y, z - v.z);
	}

	inline Vector3 operator *(float s) const
	{
		return Vector3(x * s, y * s, z * s);
	}

	inline Vector3 operator /(float d) const
	{	
		float oneOver = 1.f / d;
		return Vector3(x * oneOver, y * oneOver, z * oneOver);
	}

	inline Vector3 &operator +=(const Vector3& v)
	{
		x += v.x; y += v.y; z += v.z;
		return *this;
	}

	inline Vector3 &operator -=(const Vector3& v)
	{
		x -= v.x; y -= v.y; z -= v.z;
		return *this;
	}

	inline Vector3 &operator *=(float s)
	{
		x *= s; y *= s; z *= s;
		return *this;
	}

	inline Vector3 &operator /=(float d)
	{
		float oneOver = 1.f / d;
		x *= oneOver; y *= oneOver; z *= oneOver;
		return *this;
	}

	~Vector3() {}
};

inline Vector3 operator *(float t, const Vector3& rhs)
{
	return Vector3(t * rhs.x, t * rhs.y, t * rhs.z);
}

inline Vector3 operator /(float t, const Vector3& rhs)
{
	return Vector3(t / rhs.x, t / rhs.y, t / rhs.z);
}
