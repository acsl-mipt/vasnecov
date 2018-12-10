/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "vasnecovelement.h"
#include "technologist.h"

/*!
  \class VasnecovAbstractElement
   \brief Абстрактный класс трехмерных элементов, задействованных в сцене.

   Класс обеспечивает обозначение, позиционирование (в т.ч. числе ссылочное) и управление видимостью
   любого объекта. От него наследуются все классы, которые описывают трёхмерный объекты.

   \sa VasnecovElement
 */

//--------------------------------------------------------------------------------------------------

/*!
 \brief Конструктор абстрактного класса

 \param pipeline указатель на конвейер OpenGL
 \param name имя объекта

 \sa VasnecovPipeline
*/
VasnecovAbstractElement::VasnecovAbstractElement(VasnecovPipeline *pipeline, const QString& name) :
    Vasnecov::CoreObject(pipeline, name),
    m_alienMs(nullptr)
{
    raw_qX = raw_qX.fromAxisAndAngle(1.0, 0.0, 0.0, raw_angles.x());
    raw_qY = raw_qY.fromAxisAndAngle(0.0, 1.0, 0.0, raw_angles.y());
    raw_qZ = raw_qZ.fromAxisAndAngle(0.0, 0.0, 1.0, raw_angles.z());
}

/*!
 \brief

 \fn VasnecovAbstractElement::setCoordinates
 \param coordinates
*/
void VasnecovAbstractElement::setCoordinates(const QVector3D &coordinates)
{
    if(raw_coordinates != coordinates)
    {
        raw_coordinates = coordinates;
        designerUpdateMatrixMs();
    }
}
/*!
 \brief

 \fn VasnecovAbstractElement::setCoordinates
 \param x
 \param y
 \param z
*/
void VasnecovAbstractElement::setCoordinates(GLfloat x, GLfloat y, GLfloat z)
{
    setCoordinates(QVector3D(x, y, z));
}
/*!
 \brief

 \fn VasnecovAbstractElement::incrementCoordinates
 \param increment
*/
void VasnecovAbstractElement::incrementCoordinates(const QVector3D &increment)
{
    if(increment.x() != 0.0f || increment.y() != 0.0f || increment.z() != 0.0f)
    {
        raw_coordinates += increment;
        designerUpdateMatrixMs();
    }
}
/*!
 \brief

 \fn VasnecovAbstractElement::incrementCoordinates
 \param x
 \param y
 \param z
*/
void VasnecovAbstractElement::incrementCoordinates(GLfloat x, GLfloat y, GLfloat z)
{
    incrementCoordinates(QVector3D(x, y, z));
}
/*!
 \brief

 \fn VasnecovAbstractElement::coordinates
 \return QVector3D
*/
const QVector3D& VasnecovAbstractElement::coordinates() const
{
    return raw_coordinates;
}

/*!
 \brief

 \fn VasnecovAbstractElement::setAngles
 \param angles
*/
void VasnecovAbstractElement::setAngles(const QVector3D &angles)
{
    if (raw_angles == angles)
        return;

    GLenum rotate = 0;

    if(raw_angles.x() != angles.x())
    {
        raw_angles.setX(Vasnecov::trimAngle(angles.x()));
        raw_qX = raw_qX.fromAxisAndAngle(1.0, 0.0, 0.0, raw_angles.x());
        rotate |= Vasnecov::RotationX;
    }
    if(raw_angles.y() != angles.y())
    {
        raw_angles.setY(Vasnecov::trimAngle(angles.y()));
        raw_qY = raw_qY.fromAxisAndAngle(0.0, 1.0, 0.0, raw_angles.y());
        rotate |= Vasnecov::RotationY;
    }
    if(raw_angles.z() != angles.z())
    {
        raw_angles.setZ(Vasnecov::trimAngle(angles.z()));
        raw_qZ = raw_qZ.fromAxisAndAngle(0.0, 0.0, 1.0, raw_angles.z());
        rotate |= Vasnecov::RotationZ;
    }

    if(rotate)
    {
        designerUpdateMatrixMs();
    }
}
/*!
 \brief

 \fn VasnecovAbstractElement::setAngles
 \param x
 \param y
 \param z
*/
void VasnecovAbstractElement::setAngles(GLfloat x, GLfloat y, GLfloat z)
{
    setAngles(QVector3D(x, y, z));
}
/*!
 \brief

 \fn VasnecovAbstractElement::incrementAngles
 \param increment
*/
void VasnecovAbstractElement::incrementAngles(const QVector3D &increment)
{
    if (increment.x() == 0.0f && increment.y() == 0.0f && increment.z() == 0.0f)
        return;

    GLenum rotate = 0;

    if(increment.x() != 0.0)
    {
        raw_angles.setX(Vasnecov::trimAngle(raw_angles.x() + increment.x()));
        raw_qX = raw_qX.fromAxisAndAngle(1.0, 0.0, 0.0, raw_angles.x());
        rotate |= Vasnecov::RotationX;
    }
    if(increment.y() != 0.0)
    {
        raw_angles.setY(Vasnecov::trimAngle(raw_angles.y() + increment.y()));
        raw_qY = raw_qY.fromAxisAndAngle(0.0, 1.0, 0.0, raw_angles.y());
        rotate |= Vasnecov::RotationY;
    }
    if(increment.z() != 0.0)
    {
        raw_angles.setZ(Vasnecov::trimAngle(raw_angles.z() + increment.z()));
        raw_qZ = raw_qZ.fromAxisAndAngle(0.0, 0.0, 1.0, raw_angles.z());
        rotate |= Vasnecov::RotationZ;
    }

    if(rotate)
    {
        designerUpdateMatrixMs();
    }
}
/*!
 \brief

 \fn VasnecovAbstractElement::incrementAngles
 \param x
 \param y
 \param z
*/
void VasnecovAbstractElement::incrementAngles(GLfloat x, GLfloat y, GLfloat z)
{
    incrementAngles(QVector3D(x, y, z));
}
/*!
 \brief

 \fn VasnecovAbstractElement::setAnglesRad
 \param angles
*/
void VasnecovAbstractElement::setAnglesRad(const QVector3D &angles)
{
    setAngles(angles.x() * c_radToDeg, angles.y() * c_radToDeg, angles.z() * c_radToDeg);
}
/*!
 \brief

 \fn VasnecovAbstractElement::setAnglesRad
 \param x
 \param y
 \param z
*/
void VasnecovAbstractElement::setAnglesRad(GLfloat x, GLfloat y, GLfloat z)
{
    setAngles(x * c_radToDeg, y * c_radToDeg, z * c_radToDeg);
}
/*!
 \brief

 \fn VasnecovAbstractElement::incrementAnglesRad
 \param increment
*/
void VasnecovAbstractElement::incrementAnglesRad(const QVector3D &increment)
{
    incrementAngles(increment.x() * c_radToDeg, increment.y() * c_radToDeg, increment.z() * c_radToDeg);
}
/*!
 \brief

 \fn VasnecovAbstractElement::incrementAnglesRad
 \param x
 \param y
 \param z
*/
void VasnecovAbstractElement::incrementAnglesRad(GLfloat x, GLfloat y, GLfloat z)
{
    incrementAngles(x * c_radToDeg, y * c_radToDeg, z * c_radToDeg);
}
/*!
 \brief

 \fn VasnecovAbstractElement::angles
 \return QVector3D
*/
const QVector3D& VasnecovAbstractElement::angles() const
{
    return raw_angles;
}

void VasnecovAbstractElement::setPositionFromElement(const VasnecovAbstractElement *element)
{
    if(element)
    {
        // NOTE: After this element's coordinates, angles & quaternions will be not actual
        m_Ms = element->designerMatrixMs();
    }
}
/*!
 \brief Удаляет чужую матрицу преобразований.

*/
void VasnecovAbstractElement::detachFromOtherElement()
{
    m_alienMs = nullptr;
}

void VasnecovAbstractElement::attachToElement(const VasnecovAbstractElement *element)
{
    if(element)
    {
        m_alienMs = element->designerExportingMatrix();
    }
}

void VasnecovAbstractElement::designerUpdateMatrixMs()
{
    // Сначала вращение по оси Z, далее - X-Y.
    QMatrix4x4 newMatrix;
    newMatrix.translate(raw_coordinates);

    QQuaternion qRot;
    qRot = raw_qZ * raw_qX * raw_qY;

    newMatrix.rotate(qRot);
    m_Ms = newMatrix;
}

/*!
 \brief

 \fn VasnecovAbstractElement::renderUpdateData
 \return GLenum
*/
GLenum VasnecovAbstractElement::renderUpdateData()
{
    GLenum updated = raw_wasUpdated;

    if(raw_wasUpdated)
    {
        pure_pipeline->setSomethingWasUpdated();
        Vasnecov::CoreObject::renderUpdateData();
    }

    return updated;
}

/*!
 \brief

 \fn VasnecovElement::VasnecovElement
 \param pipeline
 \param name
*/
VasnecovElement::VasnecovElement(VasnecovPipeline *pipeline, const QString& name) :
    VasnecovAbstractElement(pipeline, name),
    m_color(QColor(255, 255, 255, 255)),
    m_scale(1.0f),
    m_isTransparency(false),

    pure_distance(0.0f)
{
}

/*!
 \brief

 \fn VasnecovElement::setColor
 \param color
*/
void VasnecovElement::setColor(const QColor &color)
{
    m_color = color;
}

/*!
 \brief

 \fn VasnecovElement::setColor
 \param r
 \param g
 \param b
 \param a
*/
void VasnecovElement::setColor(GLint r, GLint g, GLint b, GLint a)
{
    QColor newColor;
    newColor.setRgb(r, g, b, a);
    setColor(newColor);
}

/*!
 \brief

 \fn VasnecovElement::setColorF
 \param r
 \param g
 \param b
 \param a
*/
void VasnecovElement::setColorF(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    QColor newColor;
    newColor.setRgbF(r, g, b, a);
    setColor(newColor);
}

/*!
 \brief

 \fn VasnecovElement::setColor
 \param rgb
*/
void VasnecovElement::setColor(QRgb rgb)
{
    QColor newColor;
    newColor.setRgb(rgb);
    setColor(newColor);
}

/*!
 \brief

 \fn VasnecovElement::setColorWithAlpha
 \param rgba
*/
void VasnecovElement::setColorWithAlpha(QRgb rgba)
{
    QColor newColor;
    newColor.setRgba(rgba);
    setColor(newColor);
}

/*!
 \brief

 \fn VasnecovElement::setColorAlpha
 \param alpha
*/
void VasnecovElement::setColorAlpha(GLint alpha)
{
    QColor newColor = color();
    newColor.setAlpha(alpha);
    setColor(newColor);
}

/*!
 \brief

 \fn VasnecovElement::setColorAlphaF
 \param alpha
*/
void VasnecovElement::setColorAlphaF(GLfloat alpha)
{
    QColor newColor = color();
    newColor.setAlphaF(alpha);
    setColor(newColor);
}

/*!
 \brief

 \fn VasnecovElement::color
 \return QColor
*/
QColor VasnecovElement::color() const
{
    return m_color;
}

/*!
 \brief

 \fn VasnecovElement::setScale
 \param scale
*/
void VasnecovElement::setScale(GLfloat scale)
{
    if (m_scale == scale)
        return;
    m_scale = scale;
    designerUpdateMatrixMs();
}
/*!
 \brief

 \fn VasnecovElement::scale
 \return GLfloat
*/
GLfloat VasnecovElement::scale() const
{
    return m_scale;
}
/*!
 \brief

 \fn VasnecovElement::isTransparency
 \return GLboolean
*/
GLboolean VasnecovElement::isTransparency() const
{
    return m_isTransparency;
}

bool VasnecovElement::renderCompareByReverseDistance(VasnecovElement *first, VasnecovElement *second)
{
    if(first && second && first != second)
    {
        if(first->renderDistance() > second->renderDistance())
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    return false;
}
bool VasnecovElement::renderCompareByDirectDistance(VasnecovElement *first, VasnecovElement *second)
{
    if(first && second && first != second)
    {
        if(first->renderDistance() > second->renderDistance())
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    return false;
}

void VasnecovElement::designerUpdateMatrixMs()
{
    QMatrix4x4 newMatrix;
    newMatrix.translate(raw_coordinates);

    QQuaternion qRot;
    qRot = raw_qZ * raw_qX * raw_qY;

    newMatrix.rotate(qRot);

    if(m_scale != 1.0f)
    {
        newMatrix.scale(m_scale, m_scale, m_scale);
    }

    m_Ms = newMatrix;
}
/*!
 \brief

 \fn VasnecovElement::renderUpdateData
 \return GLenum
*/
GLenum VasnecovElement::renderUpdateData()
{
    GLenum updated = raw_wasUpdated;

    if(raw_wasUpdated)
    {
        pure_pipeline->setSomethingWasUpdated();
        VasnecovAbstractElement::renderUpdateData();
    }

    return updated;
}

GLfloat VasnecovElement::renderCalculateDistanceToPlane(const QVector3D &planePoint, const QVector3D &normal)
{
    QVector3D centerPoint;

    if(m_alienMs)
    {
        centerPoint = (*m_alienMs) * m_Ms * centerPoint;
    }
    else
    {
        centerPoint = m_Ms * centerPoint;
    }

    pure_distance = centerPoint.distanceToPlane(planePoint, normal);

    return pure_distance;
}
