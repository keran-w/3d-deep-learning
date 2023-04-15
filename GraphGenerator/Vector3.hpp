#ifndef VECTOR3_HPP
#define VECTOR3_HPP

#include <cmath>
#include <vector>
#include <numeric>

namespace GraphGenerator
{
    struct Vector3
    {
        float x;
        float y;
        float z;

        float& operator[](int i)
        {
            if (i == 0)
            {
                return x;
            }
            else if (i == 1)
            {
                return y;
            }
            else
            {
                return z;
            }
        }

        float operator[](int i) const
        {
            if (i == 0)
            {
                return x;
            }
            else if (i == 1)
            {
                return y;
            }
            else
            {
                return z;
            }
        }

        constexpr float sqrLength() const
        {
            return x * x + y * y + z * z;
        }

        float length() const
        {
            return std::sqrt(x * x + y * y + z * z);
        }

        Vector3 normalized() const
        {
            float l = length();
            return { x / l, y / l, z / l };
        }

        friend constexpr float dot(Vector3 const& a, Vector3 const& b)
        {
            return a.x * b.x + a.y * b.y + a.z * b.z;
        }

        friend constexpr Vector3 cross(Vector3 const& a, Vector3 const& b)
        {
            return { a.y * b.z - a.z * b.y,
                     a.z * b.x - a.x * b.z,
                     a.x * b.y - a.y * b.x };
        }

        friend constexpr Vector3 operator+(Vector3 const& a, Vector3 const& b)
        {
            return { a.x + b.x, a.y + b.y, a.z + b.z };
        }

        friend constexpr Vector3 operator-(Vector3 const& a, Vector3 const& b)
        {
            return { a.x - b.x, a.y - b.y, a.z - b.z };
        }

        friend constexpr Vector3 operator*(Vector3 const& v, float a)
        {
            return { v.x * a, v.y * a, v.z * a };
        }

        friend constexpr Vector3 operator*(float a, Vector3 const& v)
        {
            return v * a;
        }

        friend constexpr Vector3 operator/(Vector3 const& v, float a)
        {
            return { v.x / a, v.y / a, v.z / a };
        }
    };

    Vector3 static inline mean(const std::vector<Vector3>& data)
    {
        Vector3 sum = std::accumulate(data.begin(), data.end(), Vector3{ 0.0f, 0.0f, 0.0f }, [](const Vector3& a, const Vector3& b)
            {
                return Vector3{ a.x + b.x, a.y + b.y, a.z + b.z };
            });
        return { sum.x / data.size(), sum.y / data.size(), sum.z / data.size() };
    }
}

#endif // !VECTOR3_HPP