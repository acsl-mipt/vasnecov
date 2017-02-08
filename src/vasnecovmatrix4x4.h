// Класс матриц и прочей математики
// На данный момент, клон QMatrix4x4 из Qt 5.
// В Qt 4 этот класс основан на double, что нам не очень подходит.
// Когда у нас будет сборка системы с Qt 5, тогда просто перенаследуем оттуда QMatrix4x4
// (Хотя, учитывая, что массив данных private, успешное наследование маловероятно...)

/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef VASNECOVMATRIX4X4_H
#define VASNECOVMATRIX4X4_H

#pragma GCC diagnostic ignored "-Weffc++"
#include <QtGlobal>
#include <QVector3D>
#include <QVector4D>
#include <cmath>
#include <cstring>

namespace Vasnecov
{
	enum MatrixType
	{
		Identity		= 0x0000,
		Translation		= 0x0001,
		RotationX		= 0x0002,
		RotationY		= 0x0004,
		RotationZ		= 0x0008
	};
}

class VasnecovMatrix4x4
{
public:
	inline VasnecovMatrix4x4() { setToIdentity(); }
	explicit VasnecovMatrix4x4(const float *values, bool columnOrdered = true);
	inline VasnecovMatrix4x4(float m11, float m12, float m13, float m14,
					  float m21, float m22, float m23, float m24,
					  float m31, float m32, float m33, float m34,
					  float m41, float m42, float m43, float m44);

	VasnecovMatrix4x4(const float *values, int cols, int rows);

	inline const float& operator()(int row, int column) const;
	inline float& operator()(int row, int column);

#ifndef QT_NO_VECTOR4D
	inline QVector4D column(int index) const;
	inline void setColumn(int index, const QVector4D& value);

	inline QVector4D row(int index) const;
	inline void setRow(int index, const QVector4D& value);
#endif

	inline bool isIdentity() const;
	inline void setToIdentity();

	inline void fill(float value);

	double determinant() const;
	VasnecovMatrix4x4 inverted(bool *invertible = 0) const;
	VasnecovMatrix4x4 transposed() const;

	inline VasnecovMatrix4x4& operator+=(const VasnecovMatrix4x4& other);
	inline VasnecovMatrix4x4& operator-=(const VasnecovMatrix4x4& other);
	inline VasnecovMatrix4x4& operator*=(const VasnecovMatrix4x4& other);
	inline VasnecovMatrix4x4& operator*=(float factor);
	VasnecovMatrix4x4& operator/=(float divisor);
	inline bool operator==(const VasnecovMatrix4x4& other) const;
	inline bool operator!=(const VasnecovMatrix4x4& other) const;

	friend VasnecovMatrix4x4 operator+(const VasnecovMatrix4x4& m1, const VasnecovMatrix4x4& m2);
	friend VasnecovMatrix4x4 operator-(const VasnecovMatrix4x4& m1, const VasnecovMatrix4x4& m2);
	friend VasnecovMatrix4x4 operator*(const VasnecovMatrix4x4& m1, const VasnecovMatrix4x4& m2);
#ifndef QT_NO_VECTOR3D
	friend QVector3D operator*(const VasnecovMatrix4x4& matrix, const QVector3D& vector);
	friend QVector3D operator*(const QVector3D& vector, const VasnecovMatrix4x4& matrix);
#endif
#ifndef QT_NO_VECTOR4D
	friend QVector4D operator*(const QVector4D& vector, const VasnecovMatrix4x4& matrix);
	friend QVector4D operator*(const VasnecovMatrix4x4& matrix, const QVector4D& vector);
#endif

	friend VasnecovMatrix4x4 operator-(const VasnecovMatrix4x4& matrix);
	friend VasnecovMatrix4x4 operator*(float factor, const VasnecovMatrix4x4& matrix);
	friend VasnecovMatrix4x4 operator*(const VasnecovMatrix4x4& matrix, float factor);
	friend Q_GUI_EXPORT VasnecovMatrix4x4 operator/(const VasnecovMatrix4x4& matrix, float divisor);

	friend inline bool qFuzzyCompare(const VasnecovMatrix4x4& m1, const VasnecovMatrix4x4& m2);

#ifndef QT_NO_VECTOR3D
	void scale(const QVector3D& vector);
	void translate(const QVector3D& vector);
	void rotate(float angle, const QVector3D& vector);
#endif
	void scale(float x, float y);
	void scale(float x, float y, float z);
	void scale(float factor);
	void translate(float x, float y);
	void translate(float x, float y, float z);
	void rotate(float angle, float x, float y, float z = 0.0f);
#ifndef QT_NO_QUATERNION
	void rotate(const QQuaternion& quaternion);
#endif

	void ortho(float left, float right, float bottom, float top, float nearPlane, float farPlane);
	void frustum(float left, float right, float bottom, float top, float nearPlane, float farPlane);
	void perspective(float verticalAngle, float aspectRatio, float nearPlane, float farPlane);
#ifndef QT_NO_VECTOR3D
	void lookAt(const QVector3D& eye, const QVector3D& center, const QVector3D& up);
#endif

	void copyDataTo(float *values) const;

	inline float *data();
	inline const float *data() const { return *m; }
	inline const float *constData() const { return *m; }

	void optimize();

	/* Допилы для OpenGL.
	 * В QMatrix4x4 есть методы умножения заданной матрицы на матрицы перемещения и поворотов.
	 * Но нам не нужно умножение, нужно отдельно хранить каждую матрицу, чтобы потом применить общую трансформацию.
	 * Поэтому нужны функции приведения матрицы к этим типам.
	 */
	void setToTranslation(float vx, float vy, float vz);
	void setToRotation(float angle, Vasnecov::MatrixType type);
	void setToRotationX(float angle);
	void setToRotationY(float angle);
	void setToRotationZ(float angle);
#ifndef QT_NO_QUATERNION
	void setToRotation(const QQuaternion& quaternion);
#endif
	void setToScale(float sx, float sy, float sz);
	void setToOrtho(float left, float right, float bottom, float top, float nearPlane, float farPlane);
	void setToFrustum(float left, float right, float bottom, float top, float nearPlane, float farPlane);
	void setToPerspective(float verticalAngle, float aspectRatio, float nearPlane, float farPlane);
#ifndef QT_NO_VECTOR3D
	void setToLookAt(const QVector3D& eye, const QVector3D& center, const QVector3D& up);
#endif

private:
	float m[4][4];			// Column-major order to match OpenGL.
	int flagBits;           // Flag bits from the enum below.

	// When matrices are multiplied, the flag bits are or-ed together.
	enum {
		Identity        = 0x0000, // Identity matrix
		Translation     = 0x0001, // Contains a translation
		Scale           = 0x0002, // Contains a scale
		Rotation2D      = 0x0004, // Contains a rotation about the Z axis
		Rotation        = 0x0008, // Contains an arbitrary rotation
		Perspective     = 0x0010, // Last row is different from (0, 0, 0, 1)
		General         = 0x001f  // General matrix, unknown contents
	};

	// Construct without initializing identity matrix.
	explicit VasnecovMatrix4x4(int) { }

	VasnecovMatrix4x4 orthonormalInverse() const;
};

Q_DECLARE_TYPEINFO(VasnecovMatrix4x4, Q_MOVABLE_TYPE);

inline VasnecovMatrix4x4::VasnecovMatrix4x4
		(float m11, float m12, float m13, float m14,
		 float m21, float m22, float m23, float m24,
		 float m31, float m32, float m33, float m34,
		 float m41, float m42, float m43, float m44)
{
	m[0][0] = m11; m[0][1] = m21; m[0][2] = m31; m[0][3] = m41;
	m[1][0] = m12; m[1][1] = m22; m[1][2] = m32; m[1][3] = m42;
	m[2][0] = m13; m[2][1] = m23; m[2][2] = m33; m[2][3] = m43;
	m[3][0] = m14; m[3][1] = m24; m[3][2] = m34; m[3][3] = m44;
	flagBits = General;
}

inline const float& VasnecovMatrix4x4::operator()(int aRow, int aColumn) const
{
	Q_ASSERT(aRow >= 0 && aRow < 4 && aColumn >= 0 && aColumn < 4);
	return m[aColumn][aRow];
}

inline float& VasnecovMatrix4x4::operator()(int aRow, int aColumn)
{
	Q_ASSERT(aRow >= 0 && aRow < 4 && aColumn >= 0 && aColumn < 4);
	flagBits = General;
	return m[aColumn][aRow];
}

#ifndef QT_NO_VECTOR4D
inline QVector4D VasnecovMatrix4x4::column(int index) const
{
	Q_ASSERT(index >= 0 && index < 4);
	return QVector4D(m[index][0], m[index][1], m[index][2], m[index][3]);
}

inline void VasnecovMatrix4x4::setColumn(int index, const QVector4D& value)
{
	Q_ASSERT(index >= 0 && index < 4);
	m[index][0] = value.x();
	m[index][1] = value.y();
	m[index][2] = value.z();
	m[index][3] = value.w();
	flagBits = General;
}

inline QVector4D VasnecovMatrix4x4::row(int index) const
{
	Q_ASSERT(index >= 0 && index < 4);
	return QVector4D(m[0][index], m[1][index], m[2][index], m[3][index]);
}

inline void VasnecovMatrix4x4::setRow(int index, const QVector4D& value)
{
	Q_ASSERT(index >= 0 && index < 4);
	m[0][index] = value.x();
	m[1][index] = value.y();
	m[2][index] = value.z();
	m[3][index] = value.w();
	flagBits = General;
}
#endif

Q_GUI_EXPORT VasnecovMatrix4x4 operator/(const VasnecovMatrix4x4& matrix, float divisor);

inline bool VasnecovMatrix4x4::isIdentity() const
{
	if (flagBits == Identity)
		return true;
	if (m[0][0] != 1.0f || m[0][1] != 0.0f || m[0][2] != 0.0f)
		return false;
	if (m[0][3] != 0.0f || m[1][0] != 0.0f || m[1][1] != 1.0f)
		return false;
	if (m[1][2] != 0.0f || m[1][3] != 0.0f || m[2][0] != 0.0f)
		return false;
	if (m[2][1] != 0.0f || m[2][2] != 1.0f || m[2][3] != 0.0f)
		return false;
	if (m[3][0] != 0.0f || m[3][1] != 0.0f || m[3][2] != 0.0f)
		return false;
	return (m[3][3] == 1.0f);
}

inline void VasnecovMatrix4x4::setToIdentity()
{
	m[0][0] = 1.0f;
	m[0][1] = 0.0f;
	m[0][2] = 0.0f;
	m[0][3] = 0.0f;
	m[1][0] = 0.0f;
	m[1][1] = 1.0f;
	m[1][2] = 0.0f;
	m[1][3] = 0.0f;
	m[2][0] = 0.0f;
	m[2][1] = 0.0f;
	m[2][2] = 1.0f;
	m[2][3] = 0.0f;
	m[3][0] = 0.0f;
	m[3][1] = 0.0f;
	m[3][2] = 0.0f;
	m[3][3] = 1.0f;
	flagBits = Identity;
}

inline void VasnecovMatrix4x4::fill(float value)
{
	m[0][0] = value;
	m[0][1] = value;
	m[0][2] = value;
	m[0][3] = value;
	m[1][0] = value;
	m[1][1] = value;
	m[1][2] = value;
	m[1][3] = value;
	m[2][0] = value;
	m[2][1] = value;
	m[2][2] = value;
	m[2][3] = value;
	m[3][0] = value;
	m[3][1] = value;
	m[3][2] = value;
	m[3][3] = value;
	flagBits = General;
}

inline VasnecovMatrix4x4& VasnecovMatrix4x4::operator+=(const VasnecovMatrix4x4& other)
{
	m[0][0] += other.m[0][0];
	m[0][1] += other.m[0][1];
	m[0][2] += other.m[0][2];
	m[0][3] += other.m[0][3];
	m[1][0] += other.m[1][0];
	m[1][1] += other.m[1][1];
	m[1][2] += other.m[1][2];
	m[1][3] += other.m[1][3];
	m[2][0] += other.m[2][0];
	m[2][1] += other.m[2][1];
	m[2][2] += other.m[2][2];
	m[2][3] += other.m[2][3];
	m[3][0] += other.m[3][0];
	m[3][1] += other.m[3][1];
	m[3][2] += other.m[3][2];
	m[3][3] += other.m[3][3];
	flagBits = General;
	return *this;
}

inline VasnecovMatrix4x4& VasnecovMatrix4x4::operator-=(const VasnecovMatrix4x4& other)
{
	m[0][0] -= other.m[0][0];
	m[0][1] -= other.m[0][1];
	m[0][2] -= other.m[0][2];
	m[0][3] -= other.m[0][3];
	m[1][0] -= other.m[1][0];
	m[1][1] -= other.m[1][1];
	m[1][2] -= other.m[1][2];
	m[1][3] -= other.m[1][3];
	m[2][0] -= other.m[2][0];
	m[2][1] -= other.m[2][1];
	m[2][2] -= other.m[2][2];
	m[2][3] -= other.m[2][3];
	m[3][0] -= other.m[3][0];
	m[3][1] -= other.m[3][1];
	m[3][2] -= other.m[3][2];
	m[3][3] -= other.m[3][3];
	flagBits = General;
	return *this;
}

inline VasnecovMatrix4x4& VasnecovMatrix4x4::operator*=(const VasnecovMatrix4x4& other)
{
	flagBits |= other.flagBits;

	if (flagBits < Rotation2D) {
		m[3][0] += m[0][0] * other.m[3][0];
		m[3][1] += m[1][1] * other.m[3][1];
		m[3][2] += m[2][2] * other.m[3][2];

		m[0][0] *= other.m[0][0];
		m[1][1] *= other.m[1][1];
		m[2][2] *= other.m[2][2];
		return *this;
	}

	float m0, m1, m2;
	m0 = m[0][0] * other.m[0][0]
			+ m[1][0] * other.m[0][1]
			+ m[2][0] * other.m[0][2]
			+ m[3][0] * other.m[0][3];
	m1 = m[0][0] * other.m[1][0]
			+ m[1][0] * other.m[1][1]
			+ m[2][0] * other.m[1][2]
			+ m[3][0] * other.m[1][3];
	m2 = m[0][0] * other.m[2][0]
			+ m[1][0] * other.m[2][1]
			+ m[2][0] * other.m[2][2]
			+ m[3][0] * other.m[2][3];
	m[3][0] = m[0][0] * other.m[3][0]
			+ m[1][0] * other.m[3][1]
			+ m[2][0] * other.m[3][2]
			+ m[3][0] * other.m[3][3];
	m[0][0] = m0;
	m[1][0] = m1;
	m[2][0] = m2;

	m0 = m[0][1] * other.m[0][0]
			+ m[1][1] * other.m[0][1]
			+ m[2][1] * other.m[0][2]
			+ m[3][1] * other.m[0][3];
	m1 = m[0][1] * other.m[1][0]
			+ m[1][1] * other.m[1][1]
			+ m[2][1] * other.m[1][2]
			+ m[3][1] * other.m[1][3];
	m2 = m[0][1] * other.m[2][0]
			+ m[1][1] * other.m[2][1]
			+ m[2][1] * other.m[2][2]
			+ m[3][1] * other.m[2][3];
	m[3][1] = m[0][1] * other.m[3][0]
			+ m[1][1] * other.m[3][1]
			+ m[2][1] * other.m[3][2]
			+ m[3][1] * other.m[3][3];
	m[0][1] = m0;
	m[1][1] = m1;
	m[2][1] = m2;

	m0 = m[0][2] * other.m[0][0]
			+ m[1][2] * other.m[0][1]
			+ m[2][2] * other.m[0][2]
			+ m[3][2] * other.m[0][3];
	m1 = m[0][2] * other.m[1][0]
			+ m[1][2] * other.m[1][1]
			+ m[2][2] * other.m[1][2]
			+ m[3][2] * other.m[1][3];
	m2 = m[0][2] * other.m[2][0]
			+ m[1][2] * other.m[2][1]
			+ m[2][2] * other.m[2][2]
			+ m[3][2] * other.m[2][3];
	m[3][2] = m[0][2] * other.m[3][0]
			+ m[1][2] * other.m[3][1]
			+ m[2][2] * other.m[3][2]
			+ m[3][2] * other.m[3][3];
	m[0][2] = m0;
	m[1][2] = m1;
	m[2][2] = m2;

	m0 = m[0][3] * other.m[0][0]
			+ m[1][3] * other.m[0][1]
			+ m[2][3] * other.m[0][2]
			+ m[3][3] * other.m[0][3];
	m1 = m[0][3] * other.m[1][0]
			+ m[1][3] * other.m[1][1]
			+ m[2][3] * other.m[1][2]
			+ m[3][3] * other.m[1][3];
	m2 = m[0][3] * other.m[2][0]
			+ m[1][3] * other.m[2][1]
			+ m[2][3] * other.m[2][2]
			+ m[3][3] * other.m[2][3];
	m[3][3] = m[0][3] * other.m[3][0]
			+ m[1][3] * other.m[3][1]
			+ m[2][3] * other.m[3][2]
			+ m[3][3] * other.m[3][3];
	m[0][3] = m0;
	m[1][3] = m1;
	m[2][3] = m2;
	return *this;
}

inline VasnecovMatrix4x4& VasnecovMatrix4x4::operator*=(float factor)
{
	m[0][0] *= factor;
	m[0][1] *= factor;
	m[0][2] *= factor;
	m[0][3] *= factor;
	m[1][0] *= factor;
	m[1][1] *= factor;
	m[1][2] *= factor;
	m[1][3] *= factor;
	m[2][0] *= factor;
	m[2][1] *= factor;
	m[2][2] *= factor;
	m[2][3] *= factor;
	m[3][0] *= factor;
	m[3][1] *= factor;
	m[3][2] *= factor;
	m[3][3] *= factor;
	flagBits = General;
	return *this;
}

inline bool VasnecovMatrix4x4::operator==(const VasnecovMatrix4x4& other) const
{
	return m[0][0] == other.m[0][0] &&
		   m[0][1] == other.m[0][1] &&
		   m[0][2] == other.m[0][2] &&
		   m[0][3] == other.m[0][3] &&
		   m[1][0] == other.m[1][0] &&
		   m[1][1] == other.m[1][1] &&
		   m[1][2] == other.m[1][2] &&
		   m[1][3] == other.m[1][3] &&
		   m[2][0] == other.m[2][0] &&
		   m[2][1] == other.m[2][1] &&
		   m[2][2] == other.m[2][2] &&
		   m[2][3] == other.m[2][3] &&
		   m[3][0] == other.m[3][0] &&
		   m[3][1] == other.m[3][1] &&
		   m[3][2] == other.m[3][2] &&
		   m[3][3] == other.m[3][3];
}

inline bool VasnecovMatrix4x4::operator!=(const VasnecovMatrix4x4& other) const
{
	return m[0][0] != other.m[0][0] ||
		   m[0][1] != other.m[0][1] ||
		   m[0][2] != other.m[0][2] ||
		   m[0][3] != other.m[0][3] ||
		   m[1][0] != other.m[1][0] ||
		   m[1][1] != other.m[1][1] ||
		   m[1][2] != other.m[1][2] ||
		   m[1][3] != other.m[1][3] ||
		   m[2][0] != other.m[2][0] ||
		   m[2][1] != other.m[2][1] ||
		   m[2][2] != other.m[2][2] ||
		   m[2][3] != other.m[2][3] ||
		   m[3][0] != other.m[3][0] ||
		   m[3][1] != other.m[3][1] ||
		   m[3][2] != other.m[3][2] ||
		   m[3][3] != other.m[3][3];
}

inline VasnecovMatrix4x4 operator+(const VasnecovMatrix4x4& m1, const VasnecovMatrix4x4& m2)
{
	VasnecovMatrix4x4 m(1);
	m.m[0][0] = m1.m[0][0] + m2.m[0][0];
	m.m[0][1] = m1.m[0][1] + m2.m[0][1];
	m.m[0][2] = m1.m[0][2] + m2.m[0][2];
	m.m[0][3] = m1.m[0][3] + m2.m[0][3];
	m.m[1][0] = m1.m[1][0] + m2.m[1][0];
	m.m[1][1] = m1.m[1][1] + m2.m[1][1];
	m.m[1][2] = m1.m[1][2] + m2.m[1][2];
	m.m[1][3] = m1.m[1][3] + m2.m[1][3];
	m.m[2][0] = m1.m[2][0] + m2.m[2][0];
	m.m[2][1] = m1.m[2][1] + m2.m[2][1];
	m.m[2][2] = m1.m[2][2] + m2.m[2][2];
	m.m[2][3] = m1.m[2][3] + m2.m[2][3];
	m.m[3][0] = m1.m[3][0] + m2.m[3][0];
	m.m[3][1] = m1.m[3][1] + m2.m[3][1];
	m.m[3][2] = m1.m[3][2] + m2.m[3][2];
	m.m[3][3] = m1.m[3][3] + m2.m[3][3];
	m.flagBits = VasnecovMatrix4x4::General;
	return m;
}

inline VasnecovMatrix4x4 operator-(const VasnecovMatrix4x4& m1, const VasnecovMatrix4x4& m2)
{
	VasnecovMatrix4x4 m(1);
	m.m[0][0] = m1.m[0][0] - m2.m[0][0];
	m.m[0][1] = m1.m[0][1] - m2.m[0][1];
	m.m[0][2] = m1.m[0][2] - m2.m[0][2];
	m.m[0][3] = m1.m[0][3] - m2.m[0][3];
	m.m[1][0] = m1.m[1][0] - m2.m[1][0];
	m.m[1][1] = m1.m[1][1] - m2.m[1][1];
	m.m[1][2] = m1.m[1][2] - m2.m[1][2];
	m.m[1][3] = m1.m[1][3] - m2.m[1][3];
	m.m[2][0] = m1.m[2][0] - m2.m[2][0];
	m.m[2][1] = m1.m[2][1] - m2.m[2][1];
	m.m[2][2] = m1.m[2][2] - m2.m[2][2];
	m.m[2][3] = m1.m[2][3] - m2.m[2][3];
	m.m[3][0] = m1.m[3][0] - m2.m[3][0];
	m.m[3][1] = m1.m[3][1] - m2.m[3][1];
	m.m[3][2] = m1.m[3][2] - m2.m[3][2];
	m.m[3][3] = m1.m[3][3] - m2.m[3][3];
	m.flagBits = VasnecovMatrix4x4::General;
	return m;
}

inline VasnecovMatrix4x4 operator*(const VasnecovMatrix4x4& m1, const VasnecovMatrix4x4& m2)
{
	int flagBits = m1.flagBits | m2.flagBits;
	if (flagBits < VasnecovMatrix4x4::Rotation2D) {
		VasnecovMatrix4x4 m = m1;
		m.m[3][0] += m.m[0][0] * m2.m[3][0];
		m.m[3][1] += m.m[1][1] * m2.m[3][1];
		m.m[3][2] += m.m[2][2] * m2.m[3][2];

		m.m[0][0] *= m2.m[0][0];
		m.m[1][1] *= m2.m[1][1];
		m.m[2][2] *= m2.m[2][2];
		m.flagBits = flagBits;
		return m;
	}

	VasnecovMatrix4x4 m(1);
	m.m[0][0] = m1.m[0][0] * m2.m[0][0]
			  + m1.m[1][0] * m2.m[0][1]
			  + m1.m[2][0] * m2.m[0][2]
			  + m1.m[3][0] * m2.m[0][3];
	m.m[0][1] = m1.m[0][1] * m2.m[0][0]
			  + m1.m[1][1] * m2.m[0][1]
			  + m1.m[2][1] * m2.m[0][2]
			  + m1.m[3][1] * m2.m[0][3];
	m.m[0][2] = m1.m[0][2] * m2.m[0][0]
			  + m1.m[1][2] * m2.m[0][1]
			  + m1.m[2][2] * m2.m[0][2]
			  + m1.m[3][2] * m2.m[0][3];
	m.m[0][3] = m1.m[0][3] * m2.m[0][0]
			  + m1.m[1][3] * m2.m[0][1]
			  + m1.m[2][3] * m2.m[0][2]
			  + m1.m[3][3] * m2.m[0][3];

	m.m[1][0] = m1.m[0][0] * m2.m[1][0]
			  + m1.m[1][0] * m2.m[1][1]
			  + m1.m[2][0] * m2.m[1][2]
			  + m1.m[3][0] * m2.m[1][3];
	m.m[1][1] = m1.m[0][1] * m2.m[1][0]
			  + m1.m[1][1] * m2.m[1][1]
			  + m1.m[2][1] * m2.m[1][2]
			  + m1.m[3][1] * m2.m[1][3];
	m.m[1][2] = m1.m[0][2] * m2.m[1][0]
			  + m1.m[1][2] * m2.m[1][1]
			  + m1.m[2][2] * m2.m[1][2]
			  + m1.m[3][2] * m2.m[1][3];
	m.m[1][3] = m1.m[0][3] * m2.m[1][0]
			  + m1.m[1][3] * m2.m[1][1]
			  + m1.m[2][3] * m2.m[1][2]
			  + m1.m[3][3] * m2.m[1][3];

	m.m[2][0] = m1.m[0][0] * m2.m[2][0]
			  + m1.m[1][0] * m2.m[2][1]
			  + m1.m[2][0] * m2.m[2][2]
			  + m1.m[3][0] * m2.m[2][3];
	m.m[2][1] = m1.m[0][1] * m2.m[2][0]
			  + m1.m[1][1] * m2.m[2][1]
			  + m1.m[2][1] * m2.m[2][2]
			  + m1.m[3][1] * m2.m[2][3];
	m.m[2][2] = m1.m[0][2] * m2.m[2][0]
			  + m1.m[1][2] * m2.m[2][1]
			  + m1.m[2][2] * m2.m[2][2]
			  + m1.m[3][2] * m2.m[2][3];
	m.m[2][3] = m1.m[0][3] * m2.m[2][0]
			  + m1.m[1][3] * m2.m[2][1]
			  + m1.m[2][3] * m2.m[2][2]
			  + m1.m[3][3] * m2.m[2][3];

	m.m[3][0] = m1.m[0][0] * m2.m[3][0]
			  + m1.m[1][0] * m2.m[3][1]
			  + m1.m[2][0] * m2.m[3][2]
			  + m1.m[3][0] * m2.m[3][3];
	m.m[3][1] = m1.m[0][1] * m2.m[3][0]
			  + m1.m[1][1] * m2.m[3][1]
			  + m1.m[2][1] * m2.m[3][2]
			  + m1.m[3][1] * m2.m[3][3];
	m.m[3][2] = m1.m[0][2] * m2.m[3][0]
			  + m1.m[1][2] * m2.m[3][1]
			  + m1.m[2][2] * m2.m[3][2]
			  + m1.m[3][2] * m2.m[3][3];
	m.m[3][3] = m1.m[0][3] * m2.m[3][0]
			  + m1.m[1][3] * m2.m[3][1]
			  + m1.m[2][3] * m2.m[3][2]
			  + m1.m[3][3] * m2.m[3][3];
	m.flagBits = flagBits;
	return m;
}


#ifndef QT_NO_VECTOR3D

inline QVector3D operator*(const QVector3D& vector, const VasnecovMatrix4x4& matrix)
{
	float x, y, z, w;
	x = vector.x() * matrix.m[0][0] +
		vector.y() * matrix.m[0][1] +
		vector.z() * matrix.m[0][2] +
		matrix.m[0][3];
	y = vector.x() * matrix.m[1][0] +
		vector.y() * matrix.m[1][1] +
		vector.z() * matrix.m[1][2] +
		matrix.m[1][3];
	z = vector.x() * matrix.m[2][0] +
		vector.y() * matrix.m[2][1] +
		vector.z() * matrix.m[2][2] +
		matrix.m[2][3];
	w = vector.x() * matrix.m[3][0] +
		vector.y() * matrix.m[3][1] +
		vector.z() * matrix.m[3][2] +
		matrix.m[3][3];
	if (w == 1.0f)
		return QVector3D(x, y, z);
	else
		return QVector3D(x / w, y / w, z / w);
}

inline QVector3D operator*(const VasnecovMatrix4x4& matrix, const QVector3D& vector)
{
	if (matrix.flagBits == VasnecovMatrix4x4::Identity) {
		return vector;
	} else if (matrix.flagBits < VasnecovMatrix4x4::Rotation2D) {
		// Translation | Scale
		return QVector3D(vector.x() * matrix.m[0][0] + matrix.m[3][0],
						 vector.y() * matrix.m[1][1] + matrix.m[3][1],
						 vector.z() * matrix.m[2][2] + matrix.m[3][2]);
	} else if (matrix.flagBits < VasnecovMatrix4x4::Rotation) {
		// Translation | Scale | Rotation2D
		return QVector3D(vector.x() * matrix.m[0][0] + vector.y() * matrix.m[1][0] + matrix.m[3][0],
						 vector.x() * matrix.m[0][1] + vector.y() * matrix.m[1][1] + matrix.m[3][1],
						 vector.z() * matrix.m[2][2] + matrix.m[3][2]);
	} else {
		float x, y, z, w;
		x = vector.x() * matrix.m[0][0] +
			vector.y() * matrix.m[1][0] +
			vector.z() * matrix.m[2][0] +
			matrix.m[3][0];
		y = vector.x() * matrix.m[0][1] +
			vector.y() * matrix.m[1][1] +
			vector.z() * matrix.m[2][1] +
			matrix.m[3][1];
		z = vector.x() * matrix.m[0][2] +
			vector.y() * matrix.m[1][2] +
			vector.z() * matrix.m[2][2] +
			matrix.m[3][2];
		w = vector.x() * matrix.m[0][3] +
			vector.y() * matrix.m[1][3] +
			vector.z() * matrix.m[2][3] +
			matrix.m[3][3];
		if (w == 1.0f)
			return QVector3D(x, y, z);
		else
			return QVector3D(x / w, y / w, z / w);
	}
}

#endif

#ifndef QT_NO_VECTOR4D

inline QVector4D operator*(const QVector4D& vector, const VasnecovMatrix4x4& matrix)
{
	float x, y, z, w;
	x = vector.x() * matrix.m[0][0] +
		vector.y() * matrix.m[0][1] +
		vector.z() * matrix.m[0][2] +
		vector.w() * matrix.m[0][3];
	y = vector.x() * matrix.m[1][0] +
		vector.y() * matrix.m[1][1] +
		vector.z() * matrix.m[1][2] +
		vector.w() * matrix.m[1][3];
	z = vector.x() * matrix.m[2][0] +
		vector.y() * matrix.m[2][1] +
		vector.z() * matrix.m[2][2] +
		vector.w() * matrix.m[2][3];
	w = vector.x() * matrix.m[3][0] +
		vector.y() * matrix.m[3][1] +
		vector.z() * matrix.m[3][2] +
		vector.w() * matrix.m[3][3];
	return QVector4D(x, y, z, w);
}

inline QVector4D operator*(const VasnecovMatrix4x4& matrix, const QVector4D& vector)
{
	float x, y, z, w;
	x = vector.x() * matrix.m[0][0] +
		vector.y() * matrix.m[1][0] +
		vector.z() * matrix.m[2][0] +
		vector.w() * matrix.m[3][0];
	y = vector.x() * matrix.m[0][1] +
		vector.y() * matrix.m[1][1] +
		vector.z() * matrix.m[2][1] +
		vector.w() * matrix.m[3][1];
	z = vector.x() * matrix.m[0][2] +
		vector.y() * matrix.m[1][2] +
		vector.z() * matrix.m[2][2] +
		vector.w() * matrix.m[3][2];
	w = vector.x() * matrix.m[0][3] +
		vector.y() * matrix.m[1][3] +
		vector.z() * matrix.m[2][3] +
		vector.w() * matrix.m[3][3];
	return QVector4D(x, y, z, w);
}

#endif

inline VasnecovMatrix4x4 operator-(const VasnecovMatrix4x4& matrix)
{
	VasnecovMatrix4x4 m(1);
	m.m[0][0] = -matrix.m[0][0];
	m.m[0][1] = -matrix.m[0][1];
	m.m[0][2] = -matrix.m[0][2];
	m.m[0][3] = -matrix.m[0][3];
	m.m[1][0] = -matrix.m[1][0];
	m.m[1][1] = -matrix.m[1][1];
	m.m[1][2] = -matrix.m[1][2];
	m.m[1][3] = -matrix.m[1][3];
	m.m[2][0] = -matrix.m[2][0];
	m.m[2][1] = -matrix.m[2][1];
	m.m[2][2] = -matrix.m[2][2];
	m.m[2][3] = -matrix.m[2][3];
	m.m[3][0] = -matrix.m[3][0];
	m.m[3][1] = -matrix.m[3][1];
	m.m[3][2] = -matrix.m[3][2];
	m.m[3][3] = -matrix.m[3][3];
	m.flagBits = VasnecovMatrix4x4::General;
	return m;
}

inline VasnecovMatrix4x4 operator*(float factor, const VasnecovMatrix4x4& matrix)
{
	VasnecovMatrix4x4 m(1);
	m.m[0][0] = matrix.m[0][0] * factor;
	m.m[0][1] = matrix.m[0][1] * factor;
	m.m[0][2] = matrix.m[0][2] * factor;
	m.m[0][3] = matrix.m[0][3] * factor;
	m.m[1][0] = matrix.m[1][0] * factor;
	m.m[1][1] = matrix.m[1][1] * factor;
	m.m[1][2] = matrix.m[1][2] * factor;
	m.m[1][3] = matrix.m[1][3] * factor;
	m.m[2][0] = matrix.m[2][0] * factor;
	m.m[2][1] = matrix.m[2][1] * factor;
	m.m[2][2] = matrix.m[2][2] * factor;
	m.m[2][3] = matrix.m[2][3] * factor;
	m.m[3][0] = matrix.m[3][0] * factor;
	m.m[3][1] = matrix.m[3][1] * factor;
	m.m[3][2] = matrix.m[3][2] * factor;
	m.m[3][3] = matrix.m[3][3] * factor;
	m.flagBits = VasnecovMatrix4x4::General;
	return m;
}

inline VasnecovMatrix4x4 operator*(const VasnecovMatrix4x4& matrix, float factor)
{
	VasnecovMatrix4x4 m(1);
	m.m[0][0] = matrix.m[0][0] * factor;
	m.m[0][1] = matrix.m[0][1] * factor;
	m.m[0][2] = matrix.m[0][2] * factor;
	m.m[0][3] = matrix.m[0][3] * factor;
	m.m[1][0] = matrix.m[1][0] * factor;
	m.m[1][1] = matrix.m[1][1] * factor;
	m.m[1][2] = matrix.m[1][2] * factor;
	m.m[1][3] = matrix.m[1][3] * factor;
	m.m[2][0] = matrix.m[2][0] * factor;
	m.m[2][1] = matrix.m[2][1] * factor;
	m.m[2][2] = matrix.m[2][2] * factor;
	m.m[2][3] = matrix.m[2][3] * factor;
	m.m[3][0] = matrix.m[3][0] * factor;
	m.m[3][1] = matrix.m[3][1] * factor;
	m.m[3][2] = matrix.m[3][2] * factor;
	m.m[3][3] = matrix.m[3][3] * factor;
	m.flagBits = VasnecovMatrix4x4::General;
	return m;
}

inline bool qFuzzyCompare(const VasnecovMatrix4x4& m1, const VasnecovMatrix4x4& m2)
{
	return qFuzzyCompare(m1.m[0][0], m2.m[0][0]) &&
		   qFuzzyCompare(m1.m[0][1], m2.m[0][1]) &&
		   qFuzzyCompare(m1.m[0][2], m2.m[0][2]) &&
		   qFuzzyCompare(m1.m[0][3], m2.m[0][3]) &&
		   qFuzzyCompare(m1.m[1][0], m2.m[1][0]) &&
		   qFuzzyCompare(m1.m[1][1], m2.m[1][1]) &&
		   qFuzzyCompare(m1.m[1][2], m2.m[1][2]) &&
		   qFuzzyCompare(m1.m[1][3], m2.m[1][3]) &&
		   qFuzzyCompare(m1.m[2][0], m2.m[2][0]) &&
		   qFuzzyCompare(m1.m[2][1], m2.m[2][1]) &&
		   qFuzzyCompare(m1.m[2][2], m2.m[2][2]) &&
		   qFuzzyCompare(m1.m[2][3], m2.m[2][3]) &&
		   qFuzzyCompare(m1.m[3][0], m2.m[3][0]) &&
		   qFuzzyCompare(m1.m[3][1], m2.m[3][1]) &&
		   qFuzzyCompare(m1.m[3][2], m2.m[3][2]) &&
		   qFuzzyCompare(m1.m[3][3], m2.m[3][3]);
}

inline float *VasnecovMatrix4x4::data()
{
	// We have to assume that the caller will modify the matrix elements,
	// so we flip it over to "General" mode.
	flagBits = General;
	return *m;
}

inline void VasnecovMatrix4x4::setToTranslation(float vx, float vy, float vz)
{
	if(flagBits != Identity && flagBits != Translation)
	{
		setToIdentity();
	}

	m[3][0] = vx;
	m[3][1] = vy;
	m[3][2] = vz;

	flagBits = Translation;
}

inline void VasnecovMatrix4x4::setToRotationX(float angle)
{
	setToRotation(angle, Vasnecov::RotationX);
}

inline void VasnecovMatrix4x4::setToRotationY(float angle)
{
	setToRotation(angle, Vasnecov::RotationY);
}

inline void VasnecovMatrix4x4::setToRotationZ(float angle)
{
	setToRotation(angle, Vasnecov::RotationZ);
}

inline void VasnecovMatrix4x4::setToScale(float sx, float sy, float sz)
{
	if(flagBits != Identity && flagBits != Translation)
	{
		setToIdentity();
	}

	m[0][0] = sx;
	m[1][1] = sy;
	m[2][2] = sz;

	flagBits = Scale;
}


#pragma GCC diagnostic warning "-Weffc++"

#pragma GCC diagnostic ignored "-Weffc++"
#endif // VASNECOVMATRIX4X4_H
