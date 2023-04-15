#ifndef MATRIX3_HPP
#define MATRIX3_HPP

#include <vector>
#include <numeric>
#include <algorithm>
#include <cmath>

#include "Vector3.hpp"

namespace GraphGenerator
{
	class Matrix3
	{
	public:
		float data[3][3];

		Matrix3()
		{
			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j < 3; j++)
				{
					data[i][j] = 0.0f;
				}
			}
		}

		void computeCovariance(const std::vector<Vector3>& centeredData)
		{
			for (const auto& point : centeredData)
			{
				for (int i = 0; i < 3; i++)
				{
					for (int j = 0; j < 3; j++)
					{
						data[i][j] += point[i] * point[j];
					}
				}
			}
			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j < 3; j++)
				{
					data[i][j] /= centeredData.size();
				}
			}
		}

		friend constexpr Vector3 operator*(const Matrix3& m, const Vector3& v)
		{
			Vector3 result{ 0.0f, 0.0f, 0.0f };
			for (int i = 0; i < 3; ++i)
			{
				result[i] = 0;
				for (int j = 0; j < 3; ++j)
				{
					result[i] += m.data[i][j] * v[j];
				}
			}
			return result;
		}
	};
}

#endif // !MATRIX3_HPP