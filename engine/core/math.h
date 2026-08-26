#pragma once

#include <cmath>
#include <cstddef>

namespace se {

inline constexpr float kPi = 3.14159265358979323846f;
inline constexpr float kTwoPi = 2.0f * kPi;
inline constexpr float kHalfPi = 0.5f * kPi;

struct Vec2 {
  float x, y;

  constexpr Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
  constexpr Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
  constexpr Vec2 operator*(float s) const { return {x * s, y * s}; }
  constexpr Vec2 operator/(float s) const { return {x / s, y / s}; }
  Vec2& operator+=(const Vec2& o) {
    x += o.x;
    y += o.y;
    return *this;
  }
  Vec2& operator/=(float s) {
    x /= s;
    y /= s;
    return *this;
  }

  constexpr float Dot(const Vec2& o) const { return x * o.x + y * o.y; }
  constexpr float LengthSquared() const { return x * x + y * y; }
  float Length() const { return std::sqrt(LengthSquared()); }

  // Safe: a ~zero vector normalizes to zero rather than NaN.
  Vec2 Normalized() const {
    float len = Length();
    return len > 0 ? Vec2{x / len, y / len} : Vec2{0, 0};
  }

  Vec2 Rotated(float radians) const {
    float s = std::sin(radians);
    float c = std::cos(radians);
    return {x * c - y * s, x * s + y * c};
  }

  // Unit vector at `radians` (CCW from +X). Angle 0 points along +X, the
  // convention ship outlines and facing share.
  static Vec2 FromAngle(float radians) {
    return {std::cos(radians), std::sin(radians)};
  }
};

constexpr float DistSquared(const Vec2& a, const Vec2& b) {
  return (a - b).LengthSquared();
}

// The play field: a rectangle centered at the origin with the given
// half-extents
struct Bounds {
  float half_w, half_h;

  constexpr Vec2 Wrap(Vec2 p) const {
    if (p.x > half_w)
      p.x -= 2.0f * half_w;
    else if (p.x < -half_w)
      p.x += 2.0f * half_w;
    if (p.y > half_h)
      p.y -= 2.0f * half_h;
    else if (p.y < -half_h)
      p.y += 2.0f * half_h;
    return p;
  }

  constexpr Vec2 Clamp(Vec2 p) const {
    if (p.x > half_w)
      p.x = half_w;
    else if (p.x < -half_w)
      p.x = -half_w;
    if (p.y > half_h)
      p.y = half_h;
    else if (p.y < -half_h)
      p.y = -half_h;
    return p;
  }

  // Repulsion force from walls
  constexpr Vec2 RepulsionAt(Vec2 p, float margin) const {
    return {AxisPush(p.x, half_w, margin), AxisPush(p.y, half_h, margin)};
  }

 private:
  static constexpr float AxisPush(float v, float half, float margin) {
    const float depth = margin - (half - (v > 0.0f ? v : -v));
    if (depth <= 0.0f) return 0.0f;
    const float res = depth < margin ? depth / margin : 1.0f;
    return v > 0.0f ? -res : res;
  }
};

struct Vec3 {
  float x, y, z;

  constexpr Vec3 operator+(const Vec3& o) const {
    return Vec3{x + o.x, y + o.y, z + o.z};
  }

  constexpr Vec3 operator*(float s) const { return Vec3{x * s, y * s, z * s}; }
};

struct Color {
  float r, g, b, a;
};

struct Mat3 {
  Vec3 cols[3];

  static Mat3 Translate(float tx, float ty) {
    return Mat3{Vec3{1, 0, 0}, Vec3{0, 1, 0}, Vec3{tx, ty, 1}};
  }

  static Mat3 Rotate(float radians) {
    float sin = std::sin(radians);
    float cos = std::cos(radians);
    return Mat3{Vec3{cos, sin, 0}, Vec3{-sin, cos, 0}, Vec3{0, 0, 1}};
  }

  static Mat3 Scale(float sx, float sy) {
    return Mat3{Vec3{sx, 0, 0}, Vec3{0, sy, 0}, Vec3{0, 0, 1}};
  }

  static Mat3 Ortho(float l, float r, float b, float t) {
    float sx = 2 / (r - l);
    float sy = 2 / (t - b);
    float tx = -(r + l) / (r - l);
    float ty = -(t + b) / (t - b);
    return Mat3{Vec3{sx, 0, 0}, Vec3{0, sy, 0}, Vec3{tx, ty, 1}};
  }

  Mat3 operator*(const Mat3& rhs) const {
    Mat3 res;
    for (std::size_t i = 0; i < 3; i++) {
      res.cols[i] = cols[0] * rhs.cols[i].x + cols[1] * rhs.cols[i].y +
                    cols[2] * rhs.cols[i].z;
    }
    return res;
  }
};

}  // namespace se
