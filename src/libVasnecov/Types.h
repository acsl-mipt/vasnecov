/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#ifdef _MSC_VER
    #include <windows.h>
    #define _USE_MATH_DEFINES
    #include <math.h>
#endif
#include <QtOpenGL>
#include <QString>
#include <QVector3D>
#include <QtGlobal>
#include <cmath>

const GLfloat M_2PI = static_cast<GLfloat>(M_PI * 2.0);

const GLfloat c_radToDeg = static_cast<GLfloat>(180.0 / M_PI); // Радианы в градусы
const GLfloat c_degToRad = static_cast<GLfloat>(M_PI / 180.0); // Градусы в радианы

class VasnecovElement;
class VasnecovFigure;
class VasnecovMaterial;
class VasnecovMesh;
class VasnecovProduct;
class VasnecovTexture;

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
    class WorldParameters
    {
    public:
        explicit WorldParameters() :
            m_projection(WorldTypePerspective),
            m_x(0), m_y(0),
            m_width(320), m_height(280),
            m_drawingType(Vasnecov::PolygonDrawingTypeNormal),
            m_depth(true),
            m_light(true)
        {
        }
        bool operator!=(const WorldParameters& other) const
        {
            return m_projection != other.m_projection ||
                   m_x != other.m_x ||
                   m_y != other.m_y ||
                   m_width != other.m_width ||
                   m_height != other.m_height ||
                   m_drawingType != other.m_drawingType ||
                   m_depth != other.m_depth ||
                   m_light != other.m_light;
        }
        bool operator==(const WorldParameters& other) const
        {
            return m_projection == other.m_projection &&
                   m_x == other.m_x &&
                   m_y == other.m_y &&
                   m_width == other.m_width &&
                   m_height == other.m_height &&
                   m_drawingType == other.m_drawingType &&
                   m_depth == other.m_depth &&
                   m_light == other.m_light;
        }

        Vasnecov::WorldTypes projection() const;
        void setProjection(const Vasnecov::WorldTypes& projection);

        GLint x() const;
        void setX(const GLint& x);

        GLint y() const;
        void setY(const GLint& y);

        GLsizei width() const;
        void setWidth(const GLsizei& width);

        GLsizei height() const;
        void setHeight(const GLsizei& height);

        Vasnecov::PolygonDrawingTypes drawingType() const;
        void setDrawingType(const Vasnecov::PolygonDrawingTypes& drawingType);

        GLboolean depth() const;
        void setDepth(const GLboolean& depth);

        GLboolean light() const;
        void setLight(const GLboolean& light);

    private:
        Vasnecov::WorldTypes            m_projection; // Тип проекции (орто/перспектива)
        GLint                           m_x, m_y; // координаты мира (окна просмотра) в плоскости экрана
        GLsizei                         m_width, m_height;	// ширина, высота окна просмотра
        Vasnecov::PolygonDrawingTypes   m_drawingType; // GL_FILL, GL_LINE, GL_POINT
        GLboolean                       m_depth; // Тест глубины
        GLboolean                       m_light;
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

        Attributes() :
            wasUpdated(false)
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
    class Camera
    {
    public:
        explicit Camera() :
            m_position(0.0f, 0.0f, 1.85f),
            m_target(),
            m_roll(0.0f)
        {}
        bool operator!=(const Camera& other) const
        {
            return m_position != other.m_position ||
                   m_target != other.m_target ||
                   !qFuzzyCompare(m_roll, other.m_roll);
        }
        bool operator==(const Camera& other) const
        {
            return m_position == other.m_position &&
                   m_target == other.m_target &&
                   qFuzzyCompare(m_roll, other.m_roll);
        }
        const QVector3D& position() const {return m_position;}
        const QVector3D& target() const {return m_target;}
        GLfloat roll() const {return m_roll;}

        void setTarget(const QVector3D& target) {m_target = target;}
        void setPosition(const QVector3D& position) {m_position = position;}
        void setTarget(float x, float y, float z) {m_target = QVector3D(x, y, z);}
        void setPosition(float x, float y, float z) {m_position = QVector3D(x, y, z);}
        void setRoll(GLfloat roll) {m_roll = roll;}

    private:
        QVector3D m_position; // Позиция камеры в пространстве
        QVector3D m_target; // Позиция точки, на которую сфокусирована камера
        GLfloat   m_roll;
    };

    class Line
    {
    public:
        explicit Line()
            : m_p1(), m_p2()
        {}
        Line(const QVector3D &p1, const QVector3D &p2)
            : m_p1(p1), m_p2(p2)
        {}

        bool isNull() const {return m_p1 == m_p2;}
        bool isEmpty() const {return m_p1.isNull() && m_p2.isNull();}
        const QVector3D& p1() const {return m_p1;}
        const QVector3D& p2() const {return m_p2;}

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
            return m_p1 != other.m_p1 ||
                   m_p2 != other.m_p2;
        }
        bool operator==(const Line& other) const
        {
            return m_p1 == other.m_p1 &&
                   m_p2 == other.m_p2;
        }

    private:
        QVector3D m_p1, m_p2;
    };

inline Vasnecov::WorldTypes WorldParameters::projection() const
{
    return m_projection;
}

inline void WorldParameters::setProjection(const Vasnecov::WorldTypes& projection)
{
    m_projection = projection;
}

inline GLint WorldParameters::x() const
{
    return m_x;
}

inline void WorldParameters::setX(const GLint& x)
{
    m_x = x;
}

inline GLint WorldParameters::y() const
{
    return m_y;
}

inline void WorldParameters::setY(const GLint& y)
{
    m_y = y;
}

inline GLsizei WorldParameters::width() const
{
    return m_width;
}

inline void WorldParameters::setWidth(const GLsizei& width)
{
    m_width = width;
}

inline GLsizei WorldParameters::height() const
{
    return m_height;
}

inline void WorldParameters::setHeight(const GLsizei& height)
{
    m_height = height;
}

inline Vasnecov::PolygonDrawingTypes WorldParameters::drawingType() const
{
    return m_drawingType;
}

inline void WorldParameters::setDrawingType(const Vasnecov::PolygonDrawingTypes& drawingType)
{
    m_drawingType = drawingType;
}

inline GLboolean WorldParameters::depth() const
{
    return m_depth;
}

inline void WorldParameters::setDepth(const GLboolean& depth)
{
    m_depth = depth;
}

inline GLboolean WorldParameters::light() const
{
    return m_light;
}

inline void WorldParameters::setLight(const GLboolean& light)
{
    m_light = light;
}

}
