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

#include "vasnecovmatrix4x4.h"
#include <QQuaternion>
#ifdef _MSC_VER
    #include <windows.h>
    #define _USE_MATH_DEFINES
    #include <math.h>
#endif
#ifndef _MSC_VER
    #pragma GCC diagnostic warning "-Weffc++"
#endif

QT_BEGIN_NAMESPACE

//static const float inv_dist_to_plane = 1.0f / 1024.0f;

/*!
 \brief

 \fn VasnecovMatrix4x4::VasnecovMatrix4x4
 \param values
 \param columnOrdered
*/
VasnecovMatrix4x4::VasnecovMatrix4x4(const float *values, bool columnOrdered) :
    flagBits(General)
{
    if(columnOrdered)
    {
        for (int col = 0; col < 4; ++col)
            for (int row = 0; row < 4; ++row)
                m[col][row] = values[col * 4 + row];
    }
    else
    {
        for (int row = 0; row < 4; ++row)
            for (int col = 0; col < 4; ++col)
                m[col][row] = values[row * 4 + col];
    }
    flagBits = General;
}
/*!
 \brief

 \fn VasnecovMatrix4x4::VasnecovMatrix4x4
 \param values
 \param cols
 \param rows
*/
VasnecovMatrix4x4::VasnecovMatrix4x4(const float *values, int cols, int rows) :
    flagBits(General)
{
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            if (col < cols && row < rows)
                m[col][row] = values[col * rows + row];
            else if (col == row)
                m[col][row] = 1.0f;
            else
                m[col][row] = 0.0f;
        }
    }
    flagBits = General;
}

/*!
 \brief

 \fn matrixDet2
 \param m[][]
 \param col0
 \param col1
 \param row0
 \param row1
 \return double
*/
static inline double matrixDet2(const double m[4][4], int col0, int col1, int row0, int row1)
{
    return m[col0][row0] * m[col1][row1] - m[col0][row1] * m[col1][row0];
}


// The 4x4 matrix inverse algorithm is based on that described at:
// http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q24
// Some optimization has been done to avoid making copies of 3x3
// sub-matrices and to unroll the loops.

// Calculate the determinant of a 3x3 sub-matrix.
//     | A B C |
// M = | D E F |   det(M) = A * (EI - HF) - B * (DI - GF) + C * (DH - GE)
//     | G H I |
/*!
 \brief

 \fn matrixDet3
 \param m[][]
 \param col0
 \param col1
 \param col2
 \param row0
 \param row1
 \param row2
 \return double
*/
static inline double matrixDet3
    (const double m[4][4], int col0, int col1, int col2,
     int row0, int row1, int row2)
{
    return m[col0][row0] * matrixDet2(m, col1, col2, row1, row2)
            - m[col1][row0] * matrixDet2(m, col0, col2, row1, row2)
            + m[col2][row0] * matrixDet2(m, col0, col1, row1, row2);
}

// Calculate the determinant of a 4x4 matrix.
/*!
 \brief

 \fn matrixDet4
 \param m[][]
 \return double
*/
static inline double matrixDet4(const double m[4][4])
{
    double det;
    det  = m[0][0] * matrixDet3(m, 1, 2, 3, 1, 2, 3);
    det -= m[1][0] * matrixDet3(m, 0, 2, 3, 1, 2, 3);
    det += m[2][0] * matrixDet3(m, 0, 1, 3, 1, 2, 3);
    det -= m[3][0] * matrixDet3(m, 0, 1, 2, 1, 2, 3);
    return det;
}

/*!
 \brief

 \fn copyToDoubles
 \param m[][]
 \param mm[][]
*/
static inline void copyToDoubles(const float m[4][4], double mm[4][4])
{
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            mm[i][j] = double(m[i][j]);
}

/*!
 \brief

 \fn VasnecovMatrix4x4::determinant
 \return double
*/
double VasnecovMatrix4x4::determinant() const
{
    if ((flagBits & ~(Translation | Rotation2D | Rotation)) == Identity)
        return 1.0;

    double mm[4][4];
    copyToDoubles(m, mm);
    if (flagBits < Rotation2D)
        return mm[0][0] * mm[1][1] * mm[2][2]; // Translation | Scale
    if (flagBits < Perspective)
        return matrixDet3(mm, 0, 1, 2, 0, 1, 2);
    return matrixDet4(mm);
}

/*!
 \brief

 \fn VasnecovMatrix4x4::inverted
 \param invertible
 \return VasnecovMatrix4x4
*/
VasnecovMatrix4x4 VasnecovMatrix4x4::inverted(bool *invertible) const
{
    // Handle some of the easy cases first.
    if (flagBits == Identity) {
        if (invertible)
            *invertible = true;
        return VasnecovMatrix4x4();
    } else if (flagBits == Translation) {
        VasnecovMatrix4x4 inv;
        inv.m[3][0] = -m[3][0];
        inv.m[3][1] = -m[3][1];
        inv.m[3][2] = -m[3][2];
        inv.flagBits = Translation;
        if (invertible)
            *invertible = true;
        return inv;
    } else if (flagBits < Rotation2D) {
        // Translation | Scale
        if (m[0][0] == 0 || m[1][1] == 0 || m[2][2] == 0) {
            if (invertible)
                *invertible = false;
            return VasnecovMatrix4x4();
        }
        VasnecovMatrix4x4 inv;
        inv.m[0][0] = 1.0f / m[0][0];
        inv.m[1][1] = 1.0f / m[1][1];
        inv.m[2][2] = 1.0f / m[2][2];
        inv.m[3][0] = -m[3][0] * inv.m[0][0];
        inv.m[3][1] = -m[3][1] * inv.m[1][1];
        inv.m[3][2] = -m[3][2] * inv.m[2][2];
        inv.flagBits = flagBits;

        if (invertible)
            *invertible = true;
        return inv;
    } else if ((flagBits & ~(Translation | Rotation2D | Rotation)) == Identity) {
        if (invertible)
            *invertible = true;
        return orthonormalInverse();
    } else if (flagBits < Perspective) {
        VasnecovMatrix4x4 inv(1); // The "1" says to not load the identity.

        double mm[4][4];
        copyToDoubles(m, mm);

        double det = matrixDet3(mm, 0, 1, 2, 0, 1, 2);
        if (det == 0.0f) {
            if (invertible)
                *invertible = false;
            return VasnecovMatrix4x4();
        }
        det = 1.0f / det;

        inv.m[0][0] =  matrixDet2(mm, 1, 2, 1, 2) * det;
        inv.m[0][1] = -matrixDet2(mm, 0, 2, 1, 2) * det;
        inv.m[0][2] =  matrixDet2(mm, 0, 1, 1, 2) * det;
        inv.m[0][3] = 0;
        inv.m[1][0] = -matrixDet2(mm, 1, 2, 0, 2) * det;
        inv.m[1][1] =  matrixDet2(mm, 0, 2, 0, 2) * det;
        inv.m[1][2] = -matrixDet2(mm, 0, 1, 0, 2) * det;
        inv.m[1][3] = 0;
        inv.m[2][0] =  matrixDet2(mm, 1, 2, 0, 1) * det;
        inv.m[2][1] = -matrixDet2(mm, 0, 2, 0, 1) * det;
        inv.m[2][2] =  matrixDet2(mm, 0, 1, 0, 1) * det;
        inv.m[2][3] = 0;
        inv.m[3][0] = -inv.m[0][0] * m[3][0] - inv.m[1][0] * m[3][1] - inv.m[2][0] * m[3][2];
        inv.m[3][1] = -inv.m[0][1] * m[3][0] - inv.m[1][1] * m[3][1] - inv.m[2][1] * m[3][2];
        inv.m[3][2] = -inv.m[0][2] * m[3][0] - inv.m[1][2] * m[3][1] - inv.m[2][2] * m[3][2];
        inv.m[3][3] = 1;
        inv.flagBits = flagBits;

        if (invertible)
            *invertible = true;
        return inv;
    }

    VasnecovMatrix4x4 inv(1); // The "1" says to not load the identity.

    double mm[4][4];
    copyToDoubles(m, mm);

    double det = matrixDet4(mm);
    if (det == 0.0f) {
        if (invertible)
            *invertible = false;
        return VasnecovMatrix4x4();
    }
    det = 1.0f / det;

    inv.m[0][0] =  matrixDet3(mm, 1, 2, 3, 1, 2, 3) * det;
    inv.m[0][1] = -matrixDet3(mm, 0, 2, 3, 1, 2, 3) * det;
    inv.m[0][2] =  matrixDet3(mm, 0, 1, 3, 1, 2, 3) * det;
    inv.m[0][3] = -matrixDet3(mm, 0, 1, 2, 1, 2, 3) * det;
    inv.m[1][0] = -matrixDet3(mm, 1, 2, 3, 0, 2, 3) * det;
    inv.m[1][1] =  matrixDet3(mm, 0, 2, 3, 0, 2, 3) * det;
    inv.m[1][2] = -matrixDet3(mm, 0, 1, 3, 0, 2, 3) * det;
    inv.m[1][3] =  matrixDet3(mm, 0, 1, 2, 0, 2, 3) * det;
    inv.m[2][0] =  matrixDet3(mm, 1, 2, 3, 0, 1, 3) * det;
    inv.m[2][1] = -matrixDet3(mm, 0, 2, 3, 0, 1, 3) * det;
    inv.m[2][2] =  matrixDet3(mm, 0, 1, 3, 0, 1, 3) * det;
    inv.m[2][3] = -matrixDet3(mm, 0, 1, 2, 0, 1, 3) * det;
    inv.m[3][0] = -matrixDet3(mm, 1, 2, 3, 0, 1, 2) * det;
    inv.m[3][1] =  matrixDet3(mm, 0, 2, 3, 0, 1, 2) * det;
    inv.m[3][2] = -matrixDet3(mm, 0, 1, 3, 0, 1, 2) * det;
    inv.m[3][3] =  matrixDet3(mm, 0, 1, 2, 0, 1, 2) * det;
    inv.flagBits = flagBits;

    if (invertible)
        *invertible = true;
    return inv;
}

/*!
 \brief

 \fn VasnecovMatrix4x4::transposed
 \return VasnecovMatrix4x4
*/
VasnecovMatrix4x4 VasnecovMatrix4x4::transposed() const
{
    VasnecovMatrix4x4 result(1); // The "1" says to not load the identity.
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            result.m[col][row] = m[row][col];
        }
    }
    // When a translation is transposed, it becomes a perspective transformation.
    result.flagBits = (flagBits & Translation ? General : flagBits);
    return result;
}

/*!
 \brief

 \fn VasnecovMatrix4x4::operator /=
 \param divisor
 \return VasnecovMatrix4x4 &VasnecovMatrix4x4::operator
*/
VasnecovMatrix4x4& VasnecovMatrix4x4::operator/=(float divisor)
{
    m[0][0] /= divisor;
    m[0][1] /= divisor;
    m[0][2] /= divisor;
    m[0][3] /= divisor;
    m[1][0] /= divisor;
    m[1][1] /= divisor;
    m[1][2] /= divisor;
    m[1][3] /= divisor;
    m[2][0] /= divisor;
    m[2][1] /= divisor;
    m[2][2] /= divisor;
    m[2][3] /= divisor;
    m[3][0] /= divisor;
    m[3][1] /= divisor;
    m[3][2] /= divisor;
    m[3][3] /= divisor;
    flagBits = General;
    return *this;
}

/*!
 \brief

 \fn operator /
 \param matrix
 \param divisor
 \return VasnecovMatrix4x4 operator
*/
VasnecovMatrix4x4 operator/(const VasnecovMatrix4x4& matrix, float divisor)
{
    VasnecovMatrix4x4 m(1); // The "1" says to not load the identity.
    m.m[0][0] = matrix.m[0][0] / divisor;
    m.m[0][1] = matrix.m[0][1] / divisor;
    m.m[0][2] = matrix.m[0][2] / divisor;
    m.m[0][3] = matrix.m[0][3] / divisor;
    m.m[1][0] = matrix.m[1][0] / divisor;
    m.m[1][1] = matrix.m[1][1] / divisor;
    m.m[1][2] = matrix.m[1][2] / divisor;
    m.m[1][3] = matrix.m[1][3] / divisor;
    m.m[2][0] = matrix.m[2][0] / divisor;
    m.m[2][1] = matrix.m[2][1] / divisor;
    m.m[2][2] = matrix.m[2][2] / divisor;
    m.m[2][3] = matrix.m[2][3] / divisor;
    m.m[3][0] = matrix.m[3][0] / divisor;
    m.m[3][1] = matrix.m[3][1] / divisor;
    m.m[3][2] = matrix.m[3][2] / divisor;
    m.m[3][3] = matrix.m[3][3] / divisor;
    m.flagBits = VasnecovMatrix4x4::General;
    return m;
}

#ifndef QT_NO_VECTOR3D

/*!
    Multiplies this matrix by another that scales coordinates by
    the components of \a vector.

    \sa translate(), rotate()
*/
void VasnecovMatrix4x4::scale(const QVector3D& vector)
{
    float vx = vector.x();
    float vy = vector.y();
    float vz = vector.z();
    if (flagBits < Scale) {
        m[0][0] = vx;
        m[1][1] = vy;
        m[2][2] = vz;
    } else if (flagBits < Rotation2D) {
        m[0][0] *= vx;
        m[1][1] *= vy;
        m[2][2] *= vz;
    } else if (flagBits < Rotation) {
        m[0][0] *= vx;
        m[0][1] *= vx;
        m[1][0] *= vy;
        m[1][1] *= vy;
        m[2][2] *= vz;
    } else {
        m[0][0] *= vx;
        m[0][1] *= vx;
        m[0][2] *= vx;
        m[0][3] *= vx;
        m[1][0] *= vy;
        m[1][1] *= vy;
        m[1][2] *= vy;
        m[1][3] *= vy;
        m[2][0] *= vz;
        m[2][1] *= vz;
        m[2][2] *= vz;
        m[2][3] *= vz;
    }
    flagBits |= Scale;
}

#endif

/*!
    \overload

    Multiplies this matrix by another that scales coordinates by the
    components \a x, and \a y.

    \sa translate(), rotate()
*/
void VasnecovMatrix4x4::scale(float x, float y)
{
    if (flagBits < Scale) {
        m[0][0] = x;
        m[1][1] = y;
    } else if (flagBits < Rotation2D) {
        m[0][0] *= x;
        m[1][1] *= y;
    } else if (flagBits < Rotation) {
        m[0][0] *= x;
        m[0][1] *= x;
        m[1][0] *= y;
        m[1][1] *= y;
    } else {
        m[0][0] *= x;
        m[0][1] *= x;
        m[0][2] *= x;
        m[0][3] *= x;
        m[1][0] *= y;
        m[1][1] *= y;
        m[1][2] *= y;
        m[1][3] *= y;
    }
    flagBits |= Scale;
}

/*!
    \overload

    Multiplies this matrix by another that scales coordinates by the
    components \a x, \a y, and \a z.

    \sa translate(), rotate()
*/
void VasnecovMatrix4x4::scale(float x, float y, float z)
{
    if (flagBits < Scale) {
        m[0][0] = x;
        m[1][1] = y;
        m[2][2] = z;
    } else if (flagBits < Rotation2D) {
        m[0][0] *= x;
        m[1][1] *= y;
        m[2][2] *= z;
    } else if (flagBits < Rotation) {
        m[0][0] *= x;
        m[0][1] *= x;
        m[1][0] *= y;
        m[1][1] *= y;
        m[2][2] *= z;
    } else {
        m[0][0] *= x;
        m[0][1] *= x;
        m[0][2] *= x;
        m[0][3] *= x;
        m[1][0] *= y;
        m[1][1] *= y;
        m[1][2] *= y;
        m[1][3] *= y;
        m[2][0] *= z;
        m[2][1] *= z;
        m[2][2] *= z;
        m[2][3] *= z;
    }
    flagBits |= Scale;
}

/*!
    \overload

    Multiplies this matrix by another that scales coordinates by the
    given \a factor.

    \sa translate(), rotate()
*/
void VasnecovMatrix4x4::scale(float factor)
{
    if (flagBits < Scale) {
        m[0][0] = factor;
        m[1][1] = factor;
        m[2][2] = factor;
    } else if (flagBits < Rotation2D) {
        m[0][0] *= factor;
        m[1][1] *= factor;
        m[2][2] *= factor;
    } else if (flagBits < Rotation) {
        m[0][0] *= factor;
        m[0][1] *= factor;
        m[1][0] *= factor;
        m[1][1] *= factor;
        m[2][2] *= factor;
    } else {
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
    }
    flagBits |= Scale;
}

#ifndef QT_NO_VECTOR3D
/*!
    Multiplies this matrix by another that translates coordinates by
    the components of \a vector.

    \sa scale(), rotate()
*/

void VasnecovMatrix4x4::translate(const QVector3D& vector)
{
    float vx = vector.x();
    float vy = vector.y();
    float vz = vector.z();
    if (flagBits == Identity) {
        m[3][0] = vx;
        m[3][1] = vy;
        m[3][2] = vz;
    } else if (flagBits == Translation) {
        m[3][0] += vx;
        m[3][1] += vy;
        m[3][2] += vz;
    } else if (flagBits == Scale) {
        m[3][0] = m[0][0] * vx;
        m[3][1] = m[1][1] * vy;
        m[3][2] = m[2][2] * vz;
    } else if (flagBits == (Translation | Scale)) {
        m[3][0] += m[0][0] * vx;
        m[3][1] += m[1][1] * vy;
        m[3][2] += m[2][2] * vz;
    } else if (flagBits < Rotation) {
        m[3][0] += m[0][0] * vx + m[1][0] * vy;
        m[3][1] += m[0][1] * vx + m[1][1] * vy;
        m[3][2] += m[2][2] * vz;
    } else {
        m[3][0] += m[0][0] * vx + m[1][0] * vy + m[2][0] * vz;
        m[3][1] += m[0][1] * vx + m[1][1] * vy + m[2][1] * vz;
        m[3][2] += m[0][2] * vx + m[1][2] * vy + m[2][2] * vz;
        m[3][3] += m[0][3] * vx + m[1][3] * vy + m[2][3] * vz;
    }
    flagBits |= Translation;
}
#endif

/*!
    \overload

    Multiplies this matrix by another that translates coordinates
    by the components \a x, and \a y.

    \sa scale(), rotate()
*/
void VasnecovMatrix4x4::translate(float x, float y)
{
    if (flagBits == Identity) {
        m[3][0] = x;
        m[3][1] = y;
    } else if (flagBits == Translation) {
        m[3][0] += x;
        m[3][1] += y;
    } else if (flagBits == Scale) {
        m[3][0] = m[0][0] * x;
        m[3][1] = m[1][1] * y;
    } else if (flagBits == (Translation | Scale)) {
        m[3][0] += m[0][0] * x;
        m[3][1] += m[1][1] * y;
    } else if (flagBits < Rotation) {
        m[3][0] += m[0][0] * x + m[1][0] * y;
        m[3][1] += m[0][1] * x + m[1][1] * y;
    } else {
        m[3][0] += m[0][0] * x + m[1][0] * y;
        m[3][1] += m[0][1] * x + m[1][1] * y;
        m[3][2] += m[0][2] * x + m[1][2] * y;
        m[3][3] += m[0][3] * x + m[1][3] * y;
    }
    flagBits |= Translation;
}

/*!
    \overload

    Multiplies this matrix by another that translates coordinates
    by the components \a x, \a y, and \a z.

    \sa scale(), rotate()
*/
void VasnecovMatrix4x4::translate(float x, float y, float z)
{
    if (flagBits == Identity) {
        m[3][0] = x;
        m[3][1] = y;
        m[3][2] = z;
    } else if (flagBits == Translation) {
        m[3][0] += x;
        m[3][1] += y;
        m[3][2] += z;
    } else if (flagBits == Scale) {
        m[3][0] = m[0][0] * x;
        m[3][1] = m[1][1] * y;
        m[3][2] = m[2][2] * z;
    } else if (flagBits == (Translation | Scale)) {
        m[3][0] += m[0][0] * x;
        m[3][1] += m[1][1] * y;
        m[3][2] += m[2][2] * z;
    } else if (flagBits < Rotation) {
        m[3][0] += m[0][0] * x + m[1][0] * y;
        m[3][1] += m[0][1] * x + m[1][1] * y;
        m[3][2] += m[2][2] * z;
    } else {
        m[3][0] += m[0][0] * x + m[1][0] * y + m[2][0] * z;
        m[3][1] += m[0][1] * x + m[1][1] * y + m[2][1] * z;
        m[3][2] += m[0][2] * x + m[1][2] * y + m[2][2] * z;
        m[3][3] += m[0][3] * x + m[1][3] * y + m[2][3] * z;
    }
    flagBits |= Translation;
}

#ifndef QT_NO_VECTOR3D
/*!
    Multiples this matrix by another that rotates coordinates through
    \a angle degrees about \a vector.

    \sa scale(), translate()
*/

void VasnecovMatrix4x4::rotate(float angle, const QVector3D& vector)
{
    rotate(angle, vector.x(), vector.y(), vector.z());
}

#endif

/*!
    \overload

    Multiplies this matrix by another that rotates coordinates through
    \a angle degrees about the vector (\a x, \a y, \a z).

    \sa scale(), translate()
*/
void VasnecovMatrix4x4::rotate(float angle, float x, float y, float z)
{
    if (angle == 0.0f)
        return;
    float c, s;
    if (angle == 90.0f || angle == -270.0f) {
        s = 1.0f;
        c = 0.0f;
    } else if (angle == -90.0f || angle == 270.0f) {
        s = -1.0f;
        c = 0.0f;
    } else if (angle == 180.0f || angle == -180.0f) {
        s = 0.0f;
        c = -1.0f;
    } else {
        float a = angle * M_PI / 180.0f;
        c = cosf(a);
        s = sinf(a);
    }
    if (x == 0.0f) {
        if (y == 0.0f) {
            if (z != 0.0f) {
                // Rotate around the Z axis.
                if (z < 0)
                    s = -s;
                float tmp;
                m[0][0] = (tmp = m[0][0]) * c + m[1][0] * s;
                m[1][0] = m[1][0] * c - tmp * s;
                m[0][1] = (tmp = m[0][1]) * c + m[1][1] * s;
                m[1][1] = m[1][1] * c - tmp * s;
                m[0][2] = (tmp = m[0][2]) * c + m[1][2] * s;
                m[1][2] = m[1][2] * c - tmp * s;
                m[0][3] = (tmp = m[0][3]) * c + m[1][3] * s;
                m[1][3] = m[1][3] * c - tmp * s;

                flagBits |= Rotation2D;
                return;
            }
        } else if (z == 0.0f) {
            // Rotate around the Y axis.
            if (y < 0)
                s = -s;
            float tmp;
            m[2][0] = (tmp = m[2][0]) * c + m[0][0] * s;
            m[0][0] = m[0][0] * c - tmp * s;
            m[2][1] = (tmp = m[2][1]) * c + m[0][1] * s;
            m[0][1] = m[0][1] * c - tmp * s;
            m[2][2] = (tmp = m[2][2]) * c + m[0][2] * s;
            m[0][2] = m[0][2] * c - tmp * s;
            m[2][3] = (tmp = m[2][3]) * c + m[0][3] * s;
            m[0][3] = m[0][3] * c - tmp * s;

            flagBits |= Rotation;
            return;
        }
    } else if (y == 0.0f && z == 0.0f) {
        // Rotate around the X axis.
        if (x < 0)
            s = -s;
        float tmp;
        m[1][0] = (tmp = m[1][0]) * c + m[2][0] * s;
        m[2][0] = m[2][0] * c - tmp * s;
        m[1][1] = (tmp = m[1][1]) * c + m[2][1] * s;
        m[2][1] = m[2][1] * c - tmp * s;
        m[1][2] = (tmp = m[1][2]) * c + m[2][2] * s;
        m[2][2] = m[2][2] * c - tmp * s;
        m[1][3] = (tmp = m[1][3]) * c + m[2][3] * s;
        m[2][3] = m[2][3] * c - tmp * s;

        flagBits |= Rotation;
        return;
    }

    double len = double(x) * double(x) +
                 double(y) * double(y) +
                 double(z) * double(z);
    if (!qFuzzyCompare(len, 1.0) && !qFuzzyIsNull(len)) {
        len = sqrt(len);
        x = float(double(x) / len);
        y = float(double(y) / len);
        z = float(double(z) / len);
    }
    float ic = 1.0f - c;
    VasnecovMatrix4x4 rot(1); // The "1" says to not load the identity.
    rot.m[0][0] = x * x * ic + c;
    rot.m[1][0] = x * y * ic - z * s;
    rot.m[2][0] = x * z * ic + y * s;
    rot.m[3][0] = 0.0f;
    rot.m[0][1] = y * x * ic + z * s;
    rot.m[1][1] = y * y * ic + c;
    rot.m[2][1] = y * z * ic - x * s;
    rot.m[3][1] = 0.0f;
    rot.m[0][2] = x * z * ic - y * s;
    rot.m[1][2] = y * z * ic + x * s;
    rot.m[2][2] = z * z * ic + c;
    rot.m[3][2] = 0.0f;
    rot.m[0][3] = 0.0f;
    rot.m[1][3] = 0.0f;
    rot.m[2][3] = 0.0f;
    rot.m[3][3] = 1.0f;
    rot.flagBits = Rotation;
    *this *= rot;
}

#ifndef QT_NO_QUATERNION

void VasnecovMatrix4x4::rotate(const QQuaternion& quaternion)
{
    // Algorithm from:
    // http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q54
    VasnecovMatrix4x4 m(1);
    float xx = quaternion.x() * quaternion.x();
    float xy = quaternion.x() * quaternion.y();
    float xz = quaternion.x() * quaternion.z();
    float xw = quaternion.x() * quaternion.scalar();
    float yy = quaternion.y() * quaternion.y();
    float yz = quaternion.y() * quaternion.z();
    float yw = quaternion.y() * quaternion.scalar();
    float zz = quaternion.z() * quaternion.z();
    float zw = quaternion.z() * quaternion.scalar();
    m.m[0][0] = 1.0f - 2 * (yy + zz);
    m.m[1][0] =        2 * (xy - zw);
    m.m[2][0] =        2 * (xz + yw);
    m.m[3][0] = 0.0f;
    m.m[0][1] =        2 * (xy + zw);
    m.m[1][1] = 1.0f - 2 * (xx + zz);
    m.m[2][1] =        2 * (yz - xw);
    m.m[3][1] = 0.0f;
    m.m[0][2] =        2 * (xz - yw);
    m.m[1][2] =        2 * (yz + xw);
    m.m[2][2] = 1.0f - 2 * (xx + yy);
    m.m[3][2] = 0.0f;
    m.m[0][3] = 0.0f;
    m.m[1][3] = 0.0f;
    m.m[2][3] = 0.0f;
    m.m[3][3] = 1.0f;
    m.flagBits = Rotation;
    *this *= m;
}

#endif

void VasnecovMatrix4x4::ortho(float left, float right, float bottom, float top, float nearPlane, float farPlane)
{
    // Bail out if the projection volume is zero-sized.
    if (left == right || bottom == top || nearPlane == farPlane)
        return;

    // Construct the projection.
    float width = right - left;
    float invheight = top - bottom;
    float clip = farPlane - nearPlane;
    VasnecovMatrix4x4 m(1);
    m.m[0][0] = 2.0f / width;
    m.m[1][0] = 0.0f;
    m.m[2][0] = 0.0f;
    m.m[3][0] = -(left + right) / width;
    m.m[0][1] = 0.0f;
    m.m[1][1] = 2.0f / invheight;
    m.m[2][1] = 0.0f;
    m.m[3][1] = -(top + bottom) / invheight;
    m.m[0][2] = 0.0f;
    m.m[1][2] = 0.0f;
    m.m[2][2] = -2.0f / clip;
    m.m[3][2] = -(nearPlane + farPlane) / clip;
    m.m[0][3] = 0.0f;
    m.m[1][3] = 0.0f;
    m.m[2][3] = 0.0f;
    m.m[3][3] = 1.0f;
    m.flagBits = Translation | Scale;

    // Apply the projection.
    *this *= m;
}

void VasnecovMatrix4x4::frustum(float left, float right, float bottom, float top, float nearPlane, float farPlane)
{
    // Bail out if the projection volume is zero-sized.
    if (left == right || bottom == top || nearPlane == farPlane)
        return;

    // Construct the projection.
    VasnecovMatrix4x4 m(1);
    float width = right - left;
    float invheight = top - bottom;
    float clip = farPlane - nearPlane;
    m.m[0][0] = 2.0f * nearPlane / width;
    m.m[1][0] = 0.0f;
    m.m[2][0] = (left + right) / width;
    m.m[3][0] = 0.0f;
    m.m[0][1] = 0.0f;
    m.m[1][1] = 2.0f * nearPlane / invheight;
    m.m[2][1] = (top + bottom) / invheight;
    m.m[3][1] = 0.0f;
    m.m[0][2] = 0.0f;
    m.m[1][2] = 0.0f;
    m.m[2][2] = -(nearPlane + farPlane) / clip;
    m.m[3][2] = -2.0f * nearPlane * farPlane / clip;
    m.m[0][3] = 0.0f;
    m.m[1][3] = 0.0f;
    m.m[2][3] = -1.0f;
    m.m[3][3] = 0.0f;
    m.flagBits = General;

    // Apply the projection.
    *this *= m;
}

void VasnecovMatrix4x4::perspective(float verticalAngle, float aspectRatio, float nearPlane, float farPlane)
{
    // Bail out if the projection volume is zero-sized.
    if (nearPlane == farPlane || aspectRatio == 0.0f)
        return;

    // Construct the projection.
    VasnecovMatrix4x4 m(1);
    float radians = (verticalAngle / 2.0f) * M_PI / 180.0f;
    float sine = sinf(radians);
    if (sine == 0.0f)
        return;
    float cotan = cosf(radians) / sine;
    float clip = farPlane - nearPlane;
    m.m[0][0] = cotan / aspectRatio;
    m.m[1][0] = 0.0f;
    m.m[2][0] = 0.0f;
    m.m[3][0] = 0.0f;
    m.m[0][1] = 0.0f;
    m.m[1][1] = cotan;
    m.m[2][1] = 0.0f;
    m.m[3][1] = 0.0f;
    m.m[0][2] = 0.0f;
    m.m[1][2] = 0.0f;
    m.m[2][2] = -(nearPlane + farPlane) / clip;
    m.m[3][2] = -(2.0f * nearPlane * farPlane) / clip;
    m.m[0][3] = 0.0f;
    m.m[1][3] = 0.0f;
    m.m[2][3] = -1.0f;
    m.m[3][3] = 0.0f;
    m.flagBits = General;

    // Apply the projection.
    *this *= m;
}

#ifndef QT_NO_VECTOR3D

void VasnecovMatrix4x4::lookAt(const QVector3D& eye, const QVector3D& center, const QVector3D& up)
{
    QVector3D forward = (center - eye).normalized();
    QVector3D side = QVector3D::crossProduct(forward, up).normalized();
    QVector3D upVector = QVector3D::crossProduct(side, forward);

    VasnecovMatrix4x4 m(1);
    m.m[0][0] = side.x();
    m.m[1][0] = side.y();
    m.m[2][0] = side.z();
    m.m[3][0] = 0.0f;
    m.m[0][1] = upVector.x();
    m.m[1][1] = upVector.y();
    m.m[2][1] = upVector.z();
    m.m[3][1] = 0.0f;
    m.m[0][2] = -forward.x();
    m.m[1][2] = -forward.y();
    m.m[2][2] = -forward.z();
    m.m[3][2] = 0.0f;
    m.m[0][3] = 0.0f;
    m.m[1][3] = 0.0f;
    m.m[2][3] = 0.0f;
    m.m[3][3] = 1.0f;
    m.flagBits = Rotation;

    *this *= m;
    translate(-eye);
}

#endif

/*!
 \brief

 \fn VasnecovMatrix4x4::copyDataTo
 \param values
*/
void VasnecovMatrix4x4::copyDataTo(float *values) const
{
    for (int row = 0; row < 4; ++row)
        for (int col = 0; col < 4; ++col)
            values[row * 4 + col] = float(m[col][row]);
}

// Helper routine for inverting orthonormal matrices that consist
// of just rotations and translations.
/*!
 \brief

 \fn VasnecovMatrix4x4::orthonormalInverse
 \return VasnecovMatrix4x4
*/
VasnecovMatrix4x4 VasnecovMatrix4x4::orthonormalInverse() const
{
    VasnecovMatrix4x4 result(1);  // The '1' says not to load identity

    result.m[0][0] = m[0][0];
    result.m[1][0] = m[0][1];
    result.m[2][0] = m[0][2];

    result.m[0][1] = m[1][0];
    result.m[1][1] = m[1][1];
    result.m[2][1] = m[1][2];

    result.m[0][2] = m[2][0];
    result.m[1][2] = m[2][1];
    result.m[2][2] = m[2][2];

    result.m[0][3] = 0.0f;
    result.m[1][3] = 0.0f;
    result.m[2][3] = 0.0f;

    result.m[3][0] = -(result.m[0][0] * m[3][0] + result.m[1][0] * m[3][1] + result.m[2][0] * m[3][2]);
    result.m[3][1] = -(result.m[0][1] * m[3][0] + result.m[1][1] * m[3][1] + result.m[2][1] * m[3][2]);
    result.m[3][2] = -(result.m[0][2] * m[3][0] + result.m[1][2] * m[3][1] + result.m[2][2] * m[3][2]);
    result.m[3][3] = 1.0f;

    result.flagBits = flagBits;

    return result;
}

/*!
 \brief

 \fn VasnecovMatrix4x4::optimize
*/
void VasnecovMatrix4x4::optimize()
{
    // If the last row is not (0, 0, 0, 1), the matrix is not a special type.
    flagBits = General;
    if (m[0][3] != 0 || m[1][3] != 0 || m[2][3] != 0 || m[3][3] != 1)
        return;

    flagBits &= ~Perspective;

    // If the last column is (0, 0, 0, 1), then there is no translation.
    if (m[3][0] == 0 && m[3][1] == 0 && m[3][2] == 0)
        flagBits &= ~Translation;

    // If the two first elements of row 3 and column 3 are 0, then any rotation must be about Z.
    if (!m[0][2] && !m[1][2] && !m[2][0] && !m[2][1]) {
        flagBits &= ~Rotation;
        // If the six non-diagonal elements in the top left 3x3 matrix are 0, there is no rotation.
        if (!m[0][1] && !m[1][0]) {
            flagBits &= ~Rotation2D;
            // Check for identity.
            if (m[0][0] == 1 && m[1][1] == 1 && m[2][2] == 1)
                flagBits &= ~Scale;
        } else {
            // If the columns are orthonormal and form a right-handed system, then there is no scale.
            double mm[4][4];
            copyToDoubles(m, mm);
            double det = matrixDet2(mm, 0, 1, 0, 1);
            double lenX = mm[0][0] * mm[0][0] + mm[0][1] * mm[0][1];
            double lenY = mm[1][0] * mm[1][0] + mm[1][1] * mm[1][1];
            double lenZ = mm[2][2];
            if (qFuzzyCompare(det, 1.0) && qFuzzyCompare(lenX, 1.0)
                    && qFuzzyCompare(lenY, 1.0) && qFuzzyCompare(lenZ, 1.0))
            {
                flagBits &= ~Scale;
            }
        }
    } else {
        // If the columns are orthonormal and form a right-handed system, then there is no scale.
        double mm[4][4];
        copyToDoubles(m, mm);
        double det = matrixDet3(mm, 0, 1, 2, 0, 1, 2);
        double lenX = mm[0][0] * mm[0][0] + mm[0][1] * mm[0][1] + mm[0][2] * mm[0][2];
        double lenY = mm[1][0] * mm[1][0] + mm[1][1] * mm[1][1] + mm[1][2] * mm[1][2];
        double lenZ = mm[2][0] * mm[2][0] + mm[2][1] * mm[2][1] + mm[2][2] * mm[2][2];
        if (qFuzzyCompare(det, 1.0) && qFuzzyCompare(lenX, 1.0)
                && qFuzzyCompare(lenY, 1.0) && qFuzzyCompare(lenZ, 1.0))
        {
            flagBits &= ~Scale;
        }
    }
}

/*!
 \brief

 \fn VasnecovMatrix4x4::setToRotation
 \param angle
 \param type
*/
void VasnecovMatrix4x4::setToRotation(float angle, Vasnecov::MatrixType type)
{
    if(type != Vasnecov::RotationX &&
       type != Vasnecov::RotationY &&
       type != Vasnecov::RotationZ)
    {
        return;
    }

    memset(m, 0, sizeof(m)); //	memset должен быть шустрее setToIdentity()

    if(angle == 0.0f)
    {
        m[0][0] = 1.0f;
        m[1][1] = 1.0f;
        m[2][2] = 1.0f;
        m[3][3] = 1.0f;

        flagBits = Identity;

        return;
    }
    float c, s;
    if (angle == 90.0f || angle == -270.0f)
    {
        s = 1.0f;
        c = 0.0f;
    }
    else if (angle == -90.0f || angle == 270.0f)
    {
        s = -1.0f;
        c = 0.0f;
    }
    else if (angle == 180.0f || angle == -180.0f)
    {
        s = 0.0f;
        c = -1.0f;
    }
    else
    {
        float a = angle * M_PI / 180.0f;
        c = cos(a);
        s = sin(a);
    }

    if(type == Vasnecov::RotationX)
    {
        m[0][0] = 1.0f;
        m[1][1] = c;
        m[1][2] = s;
        m[2][1] = -s;
        m[2][2] = c;
        m[3][3] = 1.0f;
    }
    else if(type == Vasnecov::RotationY)
    {
        m[0][0] = c;
        m[0][2] = -s;
        m[1][1] = 1.0f;
        m[2][0] = s;
        m[2][2] = c;
        m[3][3] = 1.0f;
    }
    else if(type == Vasnecov::RotationZ)
    {
        m[0][0] = c;
        m[0][1] = s;
        m[1][0] = -s;
        m[1][1] = c;
        m[2][2] = 1.0f;
        m[3][3] = 1.0f;
    }
    flagBits = Rotation;
}

#ifndef QT_NO_QUATERNION

void VasnecovMatrix4x4::setToRotation(const QQuaternion& quaternion)
{
    // Algorithm from:
    // http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q54
    float xx = quaternion.x() * quaternion.x();
    float xy = quaternion.x() * quaternion.y();
    float xz = quaternion.x() * quaternion.z();
    float xw = quaternion.x() * quaternion.scalar();
    float yy = quaternion.y() * quaternion.y();
    float yz = quaternion.y() * quaternion.z();
    float yw = quaternion.y() * quaternion.scalar();
    float zz = quaternion.z() * quaternion.z();
    float zw = quaternion.z() * quaternion.scalar();
    m[0][0] = 1.0f - 2 * (yy + zz);
    m[1][0] =        2 * (xy - zw);
    m[2][0] =        2 * (xz + yw);
    m[3][0] = 0.0f;
    m[0][1] =        2 * (xy + zw);
    m[1][1] = 1.0f - 2 * (xx + zz);
    m[2][1] =        2 * (yz - xw);
    m[3][1] = 0.0f;
    m[0][2] =        2 * (xz - yw);
    m[1][2] =        2 * (yz + xw);
    m[2][2] = 1.0f - 2 * (xx + yy);
    m[3][2] = 0.0f;
    m[0][3] = 0.0f;
    m[1][3] = 0.0f;
    m[2][3] = 0.0f;
    m[3][3] = 1.0f;
    flagBits = Rotation;
}

#endif

void VasnecovMatrix4x4::setToOrtho(float left, float right, float bottom, float top, float nearPlane, float farPlane)
{
    // Bail out if the projection volume is zero-sized.
    if (left == right || bottom == top || nearPlane == farPlane)
        return;

    // Construct the projection.
    float width = right - left;
    float invheight = top - bottom;
    float clip = farPlane - nearPlane;

    m[0][0] = 2.0f / width;
    m[1][0] = 0.0f;
    m[2][0] = 0.0f;
    m[3][0] = -(left + right) / width;
    m[0][1] = 0.0f;
    m[1][1] = 2.0f / invheight;
    m[2][1] = 0.0f;
    m[3][1] = -(top + bottom) / invheight;
    m[0][2] = 0.0f;
    m[1][2] = 0.0f;
    m[2][2] = -2.0f / clip;
    m[3][2] = -(nearPlane + farPlane) / clip;
    m[0][3] = 0.0f;
    m[1][3] = 0.0f;
    m[2][3] = 0.0f;
    m[3][3] = 1.0f;
    flagBits = Translation | Scale;
}

void VasnecovMatrix4x4::setToFrustum(float left, float right, float bottom, float top, float nearPlane, float farPlane)
{
    // Bail out if the projection volume is zero-sized.
    if (left == right || bottom == top || nearPlane == farPlane)
        return;

    // Construct the projection.
    float width = right - left;
    float invheight = top - bottom;
    float clip = farPlane - nearPlane;
    m[0][0] = 2.0f * nearPlane / width;
    m[1][0] = 0.0f;
    m[2][0] = (left + right) / width;
    m[3][0] = 0.0f;
    m[0][1] = 0.0f;
    m[1][1] = 2.0f * nearPlane / invheight;
    m[2][1] = (top + bottom) / invheight;
    m[3][1] = 0.0f;
    m[0][2] = 0.0f;
    m[1][2] = 0.0f;
    m[2][2] = -(nearPlane + farPlane) / clip;
    m[3][2] = -2.0f * nearPlane * farPlane / clip;
    m[0][3] = 0.0f;
    m[1][3] = 0.0f;
    m[2][3] = -1.0f;
    m[3][3] = 0.0f;
    flagBits = General;
}

void VasnecovMatrix4x4::setToPerspective(float verticalAngle, float aspectRatio, float nearPlane, float farPlane)
{
    // Bail out if the projection volume is zero-sized.
    if (nearPlane == farPlane || aspectRatio == 0.0f)
        return;

    // Construct the projection.
    float radians = (verticalAngle / 2.0f) * M_PI / 180.0f;
    float sine = sinf(radians);
    if (sine == 0.0f)
        return;
    float cotan = cosf(radians) / sine;
    float clip = farPlane - nearPlane;
    m[0][0] = cotan / aspectRatio;
    m[1][0] = 0.0f;
    m[2][0] = 0.0f;
    m[3][0] = 0.0f;
    m[0][1] = 0.0f;
    m[1][1] = cotan;
    m[2][1] = 0.0f;
    m[3][1] = 0.0f;
    m[0][2] = 0.0f;
    m[1][2] = 0.0f;
    m[2][2] = -(nearPlane + farPlane) / clip;
    m[3][2] = -(2.0f * nearPlane * farPlane) / clip;
    m[0][3] = 0.0f;
    m[1][3] = 0.0f;
    m[2][3] = -1.0f;
    m[3][3] = 0.0f;
    flagBits = General;
}

#ifndef QT_NO_VECTOR3D

void VasnecovMatrix4x4::setToLookAt(const QVector3D& eye, const QVector3D& center, const QVector3D& up)
{
    QVector3D forward = (center - eye).normalized();
    QVector3D side = QVector3D::crossProduct(forward, up).normalized();
    QVector3D upVector = QVector3D::crossProduct(side, forward);

    m[0][0] = side.x();
    m[1][0] = side.y();
    m[2][0] = side.z();
    m[3][0] = 0.0f;
    m[0][1] = upVector.x();
    m[1][1] = upVector.y();
    m[2][1] = upVector.z();
    m[3][1] = 0.0f;
    m[0][2] = -forward.x();
    m[1][2] = -forward.y();
    m[2][2] = -forward.z();
    m[3][2] = 0.0f;
    m[0][3] = 0.0f;
    m[1][3] = 0.0f;
    m[2][3] = 0.0f;
    m[3][3] = 1.0f;
    flagBits = Rotation;

    translate(-eye);
}

#endif


QT_END_NAMESPACE

#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
