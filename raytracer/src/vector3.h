#ifndef VECTOR3
#define VECTOR3

/////////////////////////////////////////////////////////////////
//
// class Vecto3 - a simple 3D vector class
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
		float magSqrd = x * x + y * y + z * z;
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

	float operator *(const Vector3& v) const
	{
		return x * v.x + y * v.y + z * v.z;
	}

	// Math
	void zero() 
	{
		x = y = z = 0.f;
	}

	Vector3 operator -() const
	{
		return Vector3(-x, -y, -z);
	}

	Vector3 operator +(const Vector3& v) const
	{
		return Vector3(v.x + x, v.y + y, v.z + z);
	}

	Vector3 operator -(const Vector3& v) const
	{
		return Vector3(v.x - x, v.y - y, v.z - z);
	}

	Vector3 operator *(float s) const
	{
		return Vector3(x * s, y * s, z * s);
	}

	Vector3 operator /(float d) const
	{	
		float oneOver = 1.f / d;
		return Vector3(x * oneOver, y * oneOver, z * oneOver);
	}

	Vector3 &operator +=(const Vector3& v)
	{
		x += v.x; y += v.y; z += v.z;
		return *this;
	}

	Vector3 &operator -=(const Vector3& v)
	{
		x -= v.x; y -= v.y; z -= v.z;
		return *this;
	}

	Vector3 &operator *=(float s)
	{
		x *= s; y *= s; z *= s;
		return *this;
	}

	Vector3 &operator /=(float d)
	{
		float oneOver = 1.f / d;
		x *= oneOver; y *= oneOver; z *= oneOver;
		return *this;
	}

	~Vector3() {}
};

#endif