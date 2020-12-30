#pragma once
#include "Vector.h"

class Matrix3x3
{
private:
	union
	{
		struct
		{
			float _11, _12, _13;
			float _21, _22, _23;
			float _31, _32, _33;
		};

		float m[3][3];

	};

	int row, col;
public:
	/* ������ */
	Matrix3x3()
	{
		this->_11 = 1, this->_12 = 0, this->_13 = 0;
		this->_21 = 0, this->_22 = 1, this->_23 = 0;
		this->_31 = 0, this->_32 = 0, this->_32 = 1;

		row = sizeof(this->m) / sizeof(this->m[0]); // �� ũ��
		col = sizeof(this->m[0]) / sizeof(int); // �� ũ��
	};

	Matrix3x3(float _11, float _12, float _13,
		float _21, float _22, float _23,
		float _31, float _32, float _33)
	{
		this->_11 = _11, this->_12 = _12, this->_13 = _13;
		this->_21 = _21, this->_22 = _22, this->_23 = _23;
		this->_31 = _31, this->_32 = _32, this->_33 = _33;
	}

	/* �Ҹ��� */
	~Matrix3x3() {};

	/* ����� ��� */
	bool operator==(const Matrix3x3& rhs)
	{
		if (sizeof(this->m) == sizeof(rhs.m)) // �ڷ��� ������ ��
		{
			if (this->row == rhs.row && this->col == rhs.col) // ��� ��� ������ ��
			{
				for (int i = 0; i < 3; i++)
				{
					for (int j = 0; j < 3; j++)
					{
						if (this->m[i][j] == rhs.m[i][j]) // ��� ���Ұ� ������ ��
							return true;
					}
				}
			}
		}
		return false;
	}

	/* ��� ���� */
	Matrix3x3 operator+(const Matrix3x3& rhs)
	{
		Matrix3x3 temp;

		if (this->row == rhs.row && this->col == rhs.col) // ���� ó��
		{
			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j < 3; j++)
				{
					temp.m[i][j] = 0; // �ʱ�ȭ
					temp.m[i][j] = this->m[i][j] + rhs.m[i][j];
				}
			}
		}
		return temp;
	}

	/* ��� ���� */
	Matrix3x3 operator-(const Matrix3x3& rhs)
	{
		Matrix3x3 temp;

		if (this->row == rhs.row && this->col == rhs.col) // ���� ó��
		{
			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j < 3; j++)
				{
					temp.m[i][j] = 0; // �ʱ�ȭ
					temp.m[i][j] = this->m[i][j] - rhs.m[i][j];
				}
			}
		}
		return temp;
	}

	/* ��� ���� */
	Matrix3x3 operator*(const Matrix3x3& rhs)
	{
		Matrix3x3 temp;

		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				temp.m[i][j] = 0; // �ʱ�ȭ
				for (int k = 0; k < 3; k++)
				{
					temp.m[i][j] += this->m[i][k] * rhs.m[k][j];
				}
			}
		}
		return temp;
	}

	/* ��� ��Į��� */
	Matrix3x3 operator*(float value)
	{
		Matrix3x3 temp;

		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				temp.m[i][j] = 0; // �ʱ�ȭ
				temp.m[i][j] = value * this->m[i][j];
			}
		}
		return temp;
	}

	/* 1x3(���) * 3x3(���) */
	Vec3D MatrixMultple1x3x3x3(Vec3D p_vec)
	{
		Matrix3x3 temp; // 1x3 * 3x3 ����� ������ ��� 
		Matrix3x3 vec; // 1x3 ���
		Vec3D result;

		/* 1x3 */
		vec = Matrix3x3
		(p_vec.Get_X(), p_vec.Get_Y(), p_vec.Get_Z()
			, 0.0f,          0.0f,          0.0f
			, 0.0f,          0.0f,          0.0f);

		for (int i = 0; i < 1; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				for (int k = 0; k < 3; k++)
				{
					temp.m[j][k] = 0; // �ʱ�ȭ
					temp.m[i][j] += vec.m[i][k] * this->m[k][j];
				}


			}
		}

		result.Set_pos(temp._11, temp._12, temp._13);
		return result;
	}
};

class Matrix4x4
{
private:
	union
	{
		struct
		{
			float _11, _12, _13, _14;
			float _21, _22, _23, _24;
			float _31, _32, _33, _34;
			float _41, _42, _43, _44;
		};

		float m[4][4];

	};
public:
	/* ������ */
	Matrix4x4()
	{
		this->_11 = 1, this->_12 = 0, this->_13 = 0, this->_14 = 0;
		this->_21 = 0, this->_22 = 1, this->_23 = 0, this->_24 = 0;
		this->_31 = 0, this->_32 = 0, this->_33 = 1, this->_34 = 0;
		this->_41 = 0, this->_42 = 0, this->_43 = 0, this->_44 = 1;
	};

	Matrix4x4(float _11, float _12, float _13, float _14,
		float _21, float _22, float _23, float _24,
		float _31, float _32, float _33, float _34,
		float _41, float _42, float _43, float _44)
	{
		this->_11 = _11, this->_12 = _12, this->_13 = _13, this->_14 = _14;
		this->_21 = _21, this->_22 = _22, this->_23 = _23, this->_24 = _24;
		this->_31 = _31, this->_32 = _32, this->_33 = _33, this->_34 = _34;
		this->_41 = _41, this->_42 = _42, this->_43 = _43, this->_44 = _44;
	}

	~Matrix4x4() {}; // �Ҹ���

	/* ����� ��� */
	bool operator==(const Matrix4x4& rhs)
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				if (this->m[i][j] == rhs.m[i][j]) // ��� ���Ұ� ������ ��
					return true;
			}
		}
		return false;
	}

	/* ��� ���� */
	Matrix4x4 operator+(const Matrix4x4& rhs)
	{
		Matrix4x4 temp;

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				temp.m[i][j] = 0; // �ʱ�ȭ
				temp.m[i][j] = this->m[i][j] + rhs.m[i][j];
			}
		}

		return temp;
	}

	/* ��� ���� */
	Matrix4x4 operator-(const Matrix4x4& rhs)
	{
		Matrix4x4 temp;
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				temp.m[i][j] = 0; // �ʱ�ȭ
				temp.m[i][j] = this->m[i][j] - rhs.m[i][j];
			}
		}

		return temp;
	}

	/* ��� ���� */
	Matrix4x4 operator*(const Matrix4x4& rhs)
	{
		Matrix4x4 temp;

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				temp.m[i][j] = 0; // �ʱ�ȭ
				for (int k = 0; k < 4; k++)
				{
					temp.m[i][j] += this->m[i][k] * rhs.m[k][j];
				}
			}
		}

		return temp;
	}

	/* ��� ��Į��� */
	Matrix4x4 operator*(float value)
	{
		Matrix4x4 temp;

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				temp.m[i][j] = 0; // �ʱ�ȭ
				temp.m[i][j] = value * this->m[i][j];
			}
		}
		return temp;
	}

	/* 1x4(�຤��) * 4x4(���) */
	Vec3D MatrixMultple1x4x4x4(Vec3D p_vec)
	{
		Matrix4x4 temp; // 1x4 * 4x4 = [1x4]
		Matrix4x4 vec; // 1x4 �຤��
		Vec3D result;

		/* 1x4 */
		vec = Matrix4x4
		(p_vec.Get_X(), p_vec.Get_Y(), p_vec.Get_Z(),    1.0f
	   , 0.0f,             0.0f,            0.0f,        0.0f
	   , 0.0f,             0.0f,            0.0f,        0.0f
	   , 0.0f,             0.0f,            0.0f,        0.0f);

		for (int i = 0; i < 1; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				for (int k = 0; k < 4; k++)
				{
					temp.m[j][k] = 0; // �ʱ�ȭ
					temp.m[i][j] += vec.m[i][k] * this->m[k][j];
				}
			}
		}

		result.Set_pos(temp._11, temp._12, temp._13);
		return result;
	}

};