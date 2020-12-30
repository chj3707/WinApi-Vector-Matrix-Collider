#pragma once
#include <cmath>

class Vec2D 
{
private:
	float x, y;
public:
	Vec2D() : x(0), y(0) // ������
	{
		
	}
	Vec2D(float _x, float _y)
	{
		Set_pos(_x, _y);
	}
	Vec2D(const Vec2D& ref) // ���� ������
	{
		this->x = ref.x;
		this->y = ref.y;
	}
	~Vec2D() {}// �Ҹ���

	void Set_pos(float p_x, float p_y)
	{
		x = p_x;
		y = p_y;
	}

	float Get_X()
	{
		return x;
	}
	float Get_Y()
	{
		return y;
	}


	/* ������ ��� */
	bool operator==(const Vec2D& rhs) 
	{
		if (x == rhs.x && y == rhs.y)
			return true;
		else
			return false;
	}

	/* ���� ���� */
	Vec2D operator+(const Vec2D& rhs) 
	{
		return Vec2D(x + rhs.x, y + rhs.y);
	}

	/* ���� ���� */
	Vec2D operator-(const Vec2D& rhs) 
	{
		return Vec2D(x - rhs.x, y - rhs.y);
	}

	/* ���� ��Į��� */
	Vec2D ScalaMultple(const float scala)
	{
		return Vec2D(scala * x, scala * y);
	}

	/* ������ ũ�� */
	float GetLength()
	{
		return sqrt(GetPowLength()); // sqrt(a) = ��Ʈ a
	}

	/* ������ ũ��^2 */
	float GetPowLength()
	{
		return pow(x, 2) + pow(y, 2); // pow(a,2) = a^2
	}

	/* ���� ����ȭ */
	Vec2D Normalization()
	{
		float length = GetLength();

		Vec2D NomalizeVec;
		NomalizeVec.x = x / length;
		NomalizeVec.y = y / length;

		return NomalizeVec;
	}

	/* ���� ���� */
	float Dot_Product(const Vec2D& rhs)
	{
		return (x * rhs.x) + (y * rhs.y);
	}

	/* �ڻ��� �� ���ϴ� �Լ� */
	float GetCosthetaVal(Vec2D& rhs)
	{
		return Dot_Product(rhs) / (this->GetLength() * rhs.GetLength()); // costheta = AB���� / Aũ�� * Bũ��
	}
};

class Vec3D
{
private:
	float m_x, m_y, m_z;
public:
	/* ������ */
	Vec3D() : m_x(0), m_y(0), m_z(0) {}
	Vec3D(float p_x, float p_y, float p_z) : m_x(p_x), m_y(p_y), m_z(p_z)	{}

	/* ���� ������ */
	Vec3D(const Vec3D& ref)
	{
		this->m_x = ref.m_x;
		this->m_y = ref.m_y;
		this->m_z = ref.m_z;
	}
	~Vec3D() {} // �Ҹ���

	void Set_pos(float p_x, float p_y, float p_z)
	{
		this->m_x = p_x;
		this->m_y = p_y;
		this->m_z = p_z;
	}
	float Get_X()
	{
		return m_x;
	}
	float Get_Y()
	{
		return m_y;
	}
	float Get_Z()
	{
		return m_z;
	}

	/* ������ ��� */
	bool operator==(const Vec3D& ref) 
	{
		if (m_x == ref.m_x
		 && m_y == ref.m_y 
		 && m_z == ref.m_z)
			return true;
		else
			return false;
	}

	/* ���� ���� */
	Vec3D operator+(const Vec3D& ref)
	{
		return Vec3D
		     (m_x + ref.m_x
			, m_y + ref.m_y
			, m_z + ref.m_z);
	}

	/* ���� ���� */
	Vec3D operator-(const Vec3D& ref)
	{
		return Vec3D
		    (m_x - ref.m_x
	       , m_y - ref.m_y
	       , m_z - ref.m_z);
	}

	/* ��Į�� �� */
	Vec3D ScalaMultple(const float scala)
	{
		return Vec3D
		     (scala * m_x
			, scala * m_y
			, scala * m_z);
	}

	/* ������ ũ�� */
	float GetLength()
	{
		return sqrt(GetPowLength()); // sqrt(a) = ��Ʈ a
	}

	/* ������ ũ��^2 */
	float GetPowLength()
	{
		return pow(m_x, 2) + pow(m_y, 2) + pow(m_z, 2); // pow(a,2) = a^2
	}

	/* ���� ����ȭ */
	Vec3D Normalization()
	{
		float length = GetLength();
		Vec3D NomalizeVec;

		NomalizeVec.m_x = m_x / length;
		NomalizeVec.m_y = m_y / length;
		NomalizeVec.m_z = m_z / length;

		return NomalizeVec;
	}

	/* ���� ���� */
	float Dot_Product(const Vec3D& ref)
	{
		return (m_x * ref.m_x) + (m_y * ref.m_y) + (m_z * ref.m_z);
	}

	/* �ڻ��� �� ���ϴ� �Լ� */
	float GetCosthetaVal(Vec3D& ref)
	{
		return Dot_Product(ref) / (this->GetLength() * ref.GetLength()); // costheta = AB���� / Aũ�� * Bũ��
	}
	
	/* ���� ���� */
	Vec3D Cross_Product(const Vec3D& ref)
	{
		return Vec3D((m_y * ref.m_z) - (m_z * ref.m_y)
			       , (m_z * ref.m_x) - (m_x * ref.m_z)
			       , (m_x * ref.m_y) - (m_y * ref.m_x));

		/* ���� A(x1,y1,z1), ���� B(x2,y2,z2) �϶�
		 * (y1 * z2) - (z1 * y2), (z1 * x2) - (x1 * z2), (x1 * y2) - (y1 * x2) 
		 * ������ ���� */
	}
};