/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "VasnecovElement.h"
#include "Technologist.h"

VasnecovAbstractElement::VasnecovAbstractElement(VasnecovPipeline *pipeline, const QString& name) :
    Vasnecov::CoreObject(pipeline, name),
    raw_coordinates(),
    raw_angles(),
    raw_qX(), raw_qY(), raw_qZ(),

    m_Ms(raw_wasUpdated, MatrixMs),
    m_alienMs(raw_wasUpdated, AlienMatrix, nullptr)
{
    raw_qX = raw_qX.fromAxisAndAngle(1.0, 0.0, 0.0, raw_angles.x());
    raw_qY = raw_qY.fromAxisAndAngle(0.0, 1.0, 0.0, raw_angles.y());
    raw_qZ = raw_qZ.fromAxisAndAngle(0.0, 0.0, 1.0, raw_angles.z());
}

void VasnecovAbstractElement::setCoordinates(const QVector3D &coordinates)
{
    if(raw_coordinates != coordinates)
    {
        raw_coordinates = coordinates;
        designerUpdateMatrixMs();
    }
}
void VasnecovAbstractElement::setCoordinates(GLfloat x, GLfloat y, GLfloat z)
{
    QVector3D coordinates(x, y, z);

    setCoordinates(coordinates);
}
void VasnecovAbstractElement::incrementCoordinates(const QVector3D &increment)
{
    if(increment.x() != 0.0f || increment.y() != 0.0f || increment.z() != 0.0f)
    {
        raw_coordinates += increment;
        designerUpdateMatrixMs();
    }
}
void VasnecovAbstractElement::incrementCoordinates(GLfloat x, GLfloat y, GLfloat z)
{
    QVector3D increment(x, y, z);

    incrementCoordinates(increment);
}
QVector3D VasnecovAbstractElement::coordinates() const
{
    QVector3D coordinates(raw_coordinates);
    return coordinates;
}
void VasnecovAbstractElement::setAngles(const QVector3D &angles)
{
    if(raw_angles != angles)
    {
        GLenum rotate(0);

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
}
void VasnecovAbstractElement::setAngles(GLfloat x, GLfloat y, GLfloat z)
{
    QVector3D angles(x, y, z);

    setAngles(angles);
}
void VasnecovAbstractElement::incrementAngles(const QVector3D &increment)
{
    if(increment.x() != 0.0f || increment.y() != 0.0f || increment.z() != 0.0f)
    {
        GLenum rotate(0);

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
}
void VasnecovAbstractElement::incrementAngles(GLfloat x, GLfloat y, GLfloat z)
{
    QVector3D increment(x, y, z);

    incrementAngles(increment);
}
void VasnecovAbstractElement::setAnglesRad(const QVector3D &angles)
{
    setAngles(angles.x() * c_radToDeg, angles.y() * c_radToDeg, angles.z() * c_radToDeg);
}
void VasnecovAbstractElement::setAnglesRad(GLfloat x, GLfloat y, GLfloat z)
{
    setAngles(x * c_radToDeg, y * c_radToDeg, z * c_radToDeg);
}
void VasnecovAbstractElement::incrementAnglesRad(const QVector3D &increment)
{
    incrementAngles(increment.x() * c_radToDeg, increment.y() * c_radToDeg, increment.z() * c_radToDeg);
}
void VasnecovAbstractElement::incrementAnglesRad(GLfloat x, GLfloat y, GLfloat z)
{
    incrementAngles(x * c_radToDeg, y * c_radToDeg, z * c_radToDeg);
}
QVector3D VasnecovAbstractElement::angles() const
{
    QVector3D angles(raw_angles);
    return angles;
}

void VasnecovAbstractElement::setPositionFromElement(const VasnecovAbstractElement *element)
{
    if(element)
    {
        // NOTE: After this element's coordinates, angles & quaternions will be not actual
        m_Ms.set(element->designerMatrixMs());
    }
}
void VasnecovAbstractElement::detachFromOtherElement()
{
    m_alienMs.set(nullptr);
}

void VasnecovAbstractElement::attachToElement(const VasnecovAbstractElement *element)
{
    if(element)
    {
        m_alienMs.set(element->designerExportingMatrix());
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
    m_Ms.set(newMatrix);
}

GLenum VasnecovAbstractElement::renderUpdateData()
{
    GLenum updated(raw_wasUpdated);

    if(raw_wasUpdated)
    {
        pure_pipeline->setSomethingWasUpdated();

        // Копирование сырых данных в основные
        m_Ms.update();
        m_alienMs.update();

        Vasnecov::CoreObject::renderUpdateData();
    }

    return updated;
}

VasnecovElement::VasnecovElement(VasnecovPipeline *pipeline, const QString& name) :
    VasnecovAbstractElement(pipeline, name),
    m_color(raw_wasUpdated, Color, QColor(255, 255, 255, 255)),
    m_scale(raw_wasUpdated, Scale, 1.0f),
    m_isTransparency(raw_wasUpdated, Transparency, false),

    pure_distance(0.0f)
{
}
void VasnecovElement::setColor(const QColor &color)
{
    m_color.set(color);
}
void VasnecovElement::setColor(GLint r, GLint g, GLint b, GLint a)
{
    QColor newColor;
    newColor.setRgb(r, g, b, a);
    setColor(newColor);
}
void VasnecovElement::setColorF(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    QColor newColor;
    newColor.setRgbF(r, g, b, a);
    setColor(newColor);
}
void VasnecovElement::setColor(QRgb rgb)
{
    QColor newColor;
    newColor.setRgb(rgb);
    setColor(newColor);
}
void VasnecovElement::setColorWithAlpha(QRgb rgba)
{
    QColor newColor;
    newColor.setRgba(rgba);
    setColor(newColor);
}
void VasnecovElement::setColorAlpha(GLint alpha)
{
    QColor newColor(color());
    newColor.setAlpha(alpha);
    setColor(newColor);
}
void VasnecovElement::setColorAlphaF(GLfloat alpha)
{
    QColor newColor(color());
    newColor.setAlphaF(alpha);
    setColor(newColor);
}
QColor VasnecovElement::color() const
{
    QColor color(m_color.raw());
    return color;
}
void VasnecovElement::setScale(GLfloat scale)
{
    if(m_scale.set(scale))
    {
        designerUpdateMatrixMs();
    }
}
GLfloat VasnecovElement::scale() const
{
    GLfloat scale(m_scale.raw());
    return scale;
}
GLboolean VasnecovElement::isTransparency() const
{
    GLboolean transparency(m_isTransparency.raw());
    return transparency;
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

    if(m_scale.raw() != 1.0f)
    {
        newMatrix.scale(m_scale.raw(), m_scale.raw(), m_scale.raw());
    }

    m_Ms.set(newMatrix);
}
GLenum VasnecovElement::renderUpdateData()
{
    GLenum updated(raw_wasUpdated);

    if(raw_wasUpdated)
    {
        pure_pipeline->setSomethingWasUpdated();

        // Копирование сырых данных в основные
        m_color.update();
        m_scale.update();
        m_isTransparency.update();

        VasnecovAbstractElement::renderUpdateData();
    }

    return updated;
}

GLfloat VasnecovElement::renderCalculateDistanceToPlane(const QVector3D &planePoint, const QVector3D &normal)
{
    QVector3D centerPoint;

    if(m_alienMs.pure())
    {
        centerPoint = (*m_alienMs.pure()) * m_Ms.pure() * centerPoint;
    }
    else
    {
        centerPoint = m_Ms.pure() * centerPoint;
    }

    pure_distance = centerPoint.distanceToPlane(planePoint, normal);

    return pure_distance;
}
