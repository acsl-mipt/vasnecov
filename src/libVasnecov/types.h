/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef VASNECOV_TYPES_H
#define VASNECOV_TYPES_H

#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
#ifdef _MSC_VER
    #include <windows.h>
    #define _USE_MATH_DEFINES
    #include <math.h>
#endif
#include <GL/gl.h>
#include <string>
#include <cmath>
#include <QtGlobal>
#include <QVector3D>
#ifndef _MSC_VER
    #pragma GCC diagnostic warning "-Weffc++"
#endif

const GLfloat M_2PI = static_cast<GLfloat>(M_PI * 2.0);

const GLfloat c_radToDeg = static_cast<GLfloat>(180.0 / M_PI); // Радианы в градусы
const GLfloat c_degToRad = static_cast<GLfloat>(M_PI / 180.0); // Градусы в радианы

class VasnecovElement;
class VasnecovMaterial;
class VasnecovMesh;
class VasnecovProduct;
class VasnecovFigure;

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

    enum WorldTypes
    {
        WorldTypePerspective = 1, // Перспективный тип проекции
        WorldTypeOrthographic  = 2 // Ортогональный тип проекции
    };

    enum PolygonDrawingTypes
    {
        PolygonDrawingTypeNormal = GL_FILL,
        PolygonDrawingTypeLines = GL_LINE,
        PolygonDrawingTypePoints = GL_POINT
    };
    // Характеристики вида
    struct WorldParameters
    {
        Vasnecov::WorldTypes projection; // Тип проекции (орто/перспектива)
        GLint x, y; // координаты мира (окна просмотра) в плоскости экрана
        GLsizei width, height;	// ширина, высота окна просмотра
        Vasnecov::PolygonDrawingTypes drawingType; // GL_FILL, GL_LINE, GL_POINT
        GLboolean depth; // Тест глубины
        GLboolean light;

        WorldParameters() :
            projection(WorldTypePerspective),
            x(0), y(0),
            width(320), height(280),
            drawingType(Vasnecov::PolygonDrawingTypeNormal),
            depth(true),
            light(true)
        {
        }
        bool operator!=(const WorldParameters& other) const
        {
            return projection != other.projection ||
                   x != other.x ||
                   y != other.y ||
                   width != other.width ||
                   height != other.height ||
                   drawingType != other.drawingType ||
                   depth != other.depth ||
                   light != other.light;
        }
        bool operator==(const WorldParameters& other) const
        {
            return projection == other.projection &&
                   x == other.x &&
                   y == other.y &&
                   width == other.width &&
                   height == other.height &&
                   drawingType == other.drawingType &&
                   depth == other.depth &&
                   light == other.light;
        }
    };
    struct Perspective
    {
        GLfloat angle; // Угол раствора
        GLfloat ratio; // Соотношение сторон
        GLfloat frontBorder; // Передняя граница
        GLfloat backBorder; // Задняя граница

        Perspective() :
            angle(35.0f),
            ratio(4/3),
            frontBorder(0.1f),
            backBorder(1000.0f)
        {
        }
        bool operator!=(const Perspective& other) const
        {
            return !qFuzzyCompare(*this, other);
        }
        bool operator==(const Perspective& other) const
        {
            return qFuzzyCompare(*this, other);
        }
        bool qFuzzyCompare(const Perspective& first, const Perspective& second) const
        {
            return ::qFuzzyCompare(first.angle, second.angle) &&
                   ::qFuzzyCompare(first.ratio, second.ratio) &&
                   ::qFuzzyCompare(first.frontBorder, second.frontBorder) &&
                   ::qFuzzyCompare(first.backBorder, second.backBorder);
        }
    };
    struct Ortho
    {
        GLfloat left, right;
        GLfloat bottom, top;
        GLfloat front, back;

        Ortho() :
            left(-1.0f), right(1.0f),
            bottom(-1.0f), top(1.0f),
            front(-1.0f), back(1.0f)
        {}
        bool operator!=(const Ortho& other) const
        {
            return !qFuzzyCompare(*this, other);
        }
        bool operator==(const Ortho& other) const
        {
            return qFuzzyCompare(*this, other);
        }
        bool qFuzzyCompare(const Ortho& first, const Ortho& second) const
        {
            return ::qFuzzyCompare(first.left, second.left) &&
                   ::qFuzzyCompare(first.right, second.right) &&
                   ::qFuzzyCompare(first.bottom, second.bottom) &&
                   ::qFuzzyCompare(first.top, second.top) &&
                   ::qFuzzyCompare(first.front, second.front) &&
                   ::qFuzzyCompare(first.back, second.back);
        }
    };
    enum TextureTypes
    {
        TextureTypeUndefined = 0,
        TextureTypeInterface = 1,
        TextureTypeDiffuse = 2,
        TextureTypeNormal = 3
    };

    struct Attributes
    {
        GLenum wasUpdated;

        Attributes() : wasUpdated(false)
        {
        }
        virtual ~Attributes(){}

        void setUpdateFlag(GLenum flag)
        {
            wasUpdated |= flag;
        }
        void clearUpdateFlag()
        {
            wasUpdated = 0;
        }
        GLenum updateFlag() const
        {
            return wasUpdated;
        }
        GLboolean isUpdateFlag(GLenum flag) const
        {
            return (wasUpdated & flag) != 0;
        }
    };

    // Камера
    struct Camera
    {
        QVector3D position; // Позиция камеры в пространстве
        QVector3D target; // Позиция точки, на которую сфокусирована камера
        GLfloat roll;

        Camera() : position(0.0f, 0.0f, 1.85f), roll(0.0f) { }
        bool operator!=(const Camera& other) const
        {
            return position != other.position || target != other.target || roll != other.roll;
        }
        bool operator==(const Camera& other) const
        {
            return position == other.position && target == other.target && roll == other.roll;
        }
    };

    class Line
    {
    public:
        Line() {}
        Line(const QVector3D &p1, const QVector3D &p2) : m_p1(p1), m_p2(p2) {}

        bool isNull() const {return m_p1 == m_p2;}
        bool isEmpty() const {return m_p1.isNull() && m_p2.isNull();}
        QVector3D p1() const {return m_p1;}
        QVector3D p2() const {return m_p2;}

        GLfloat x1() const {return m_p1.x();}
        GLfloat y1() const {return m_p1.y();}
        GLfloat z1() const {return m_p1.z();}

        GLfloat x2() const {return m_p1.x();}
        GLfloat y2() const {return m_p1.y();}
        GLfloat z2() const {return m_p1.z();}

        GLfloat length() const {return m_p1.distanceToPoint(m_p2);}
        QVector3D direction() const {return QVector3D(m_p2 - m_p1).normalized();}

        void setP1(const QVector3D &p1) {m_p1 = p1;}
        void setP2(const QVector3D &p2) {m_p2 = p2;}
        void setPoints(const QVector3D &p1, const QVector3D &p2) {m_p1 = p1; m_p2 = p2;}

        bool operator!=(const Line& other) const
        {
            return m_p1 != other.m_p1 || m_p2 != other.m_p2;
        }
        bool operator==(const Line& other) const
        {
            return m_p1 == other.m_p1 && m_p2 == other.m_p2;
        }

    private:
        QVector3D m_p1;
        QVector3D m_p2;
    };
}
#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
#endif // VASNECOV_TYPES_H
