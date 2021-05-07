#include "vector3d.h"

float dot_product(const Vector3D& le, const Vector3D& re) { return le.x * re.x + le.y * re.y + le.z * re.z; }

float length(const Vector3D& v) { return v.length(); }

float Vector3D::length() const { return sqrt(dot_product(*this, *this)); }

Vector3D normalize(const Vector3D& v) { return v / v.length(); }

Vector3D operator+(const Vector3D& le, const Vector3D& re) { return Vector3D(le.x + re.x, le.y + re.y, le.z + re.z); }

Vector3D operator-(const Vector3D& le, const Vector3D& re) { return Vector3D(le.x - re.x, le.y - re.y, le.z - re.z); }

Vector3D operator*(const Vector3D& le, const float re) { return Vector3D(le.x * re, le.y * re, le.z * re); }

Vector3D operator/(const Vector3D& le, const float re) { return Vector3D(le.x / re, le.y / re, le.z / re); }
