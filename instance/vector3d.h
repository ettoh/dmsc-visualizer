#ifndef VECTOR_3D_H
#define VECTOR_3D_H

#include <cmath>

struct Vector3D {
    float x, y, z;

    Vector3D() : x(0.0f), y(0.0f), z(0.0f) {}
    Vector3D(const float x) : x(x), y(x), z(x) {}
    Vector3D(const float x, const float y, const float z) : x(x), y(y), z(z) {}
    float length() const;
    bool operator==(const float i) const { return x == i && y == i && z == i; }
    bool operator!=(const float i) const { return x != i || y != i || z != i; }
};

Vector3D operator+(const Vector3D& le, const Vector3D& re);
Vector3D operator-(const Vector3D& le, const Vector3D& re);
Vector3D operator*(const Vector3D& le, float re);
Vector3D operator/(const Vector3D& le, float re);
Vector3D normalize(const Vector3D& v);
float dot_product(const Vector3D& le, const Vector3D& re);
float length(const Vector3D& v);

#endif