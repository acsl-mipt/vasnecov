/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Класс фигур для мира
#pragma once

#include "VasnecovElement.h"

// Класс фигур (плоских, по сути)
class VasnecovFigure : public VasnecovElement
{
public:
    enum Types
    {
        TypeUnknown = 0,
        TypePolyline  = 1, // Тип фигуры (ломаная линия) GL_LINE_STRIP
        TypePolylineLoop = 2, // Замкнутая линия GL_LINE_LOOP
        TypePolygons  = 3, // Сплошная заливка GL_TRIANGLE_FAN
        TypeLines  = 4, // Линии (отрезки) GL_LINES
        TypePoints  = 5, // Точки GL_POINTS
        TypeTriangles  = 6 // Треугольники GL_TRIANGLES
    };
    enum LineStyles
    {
        StyleSolid = 0,
        StyleDashed,
        StyleDotDash,
        StyleDotDotDash,
        StyleDotted,
    };

    VasnecovFigure(VasnecovPipeline* pipeline, const QString& name = QString());
    ~VasnecovFigure();

    static std::vector<QVector3D> readPointsFromObj(const QString& fileName);

    void setPoints(const std::vector<QVector3D>& points);
    void clearPoints();
    GLuint pointsAmount() const;

    void addFirstPoint(const QVector3D& point);
    void removeFirstPoint();
    void replaceFirstPoint(const QVector3D& point);

    void addLastPoint(const QVector3D& point);
    void removeLastPoint();
    void replaceLastPoint(const QVector3D& point);

    GLboolean setType(VasnecovFigure::Types type);
    VasnecovFigure::Types type() const;

    GLboolean setLineStyle(VasnecovFigure::LineStyles style);
    VasnecovFigure::LineStyles style() const;

    void enableLighting();
    void disableLighting();
    GLboolean lighting() const;

    void setDepth(bool depth);
    void enableDepth();
    void disableDepth();
    GLboolean depth() const;

    GLint setThickness(GLfloat thick);
    GLfloat thickness() const;

    void setOptimization(GLboolean optimize);
    GLboolean optimization() const;

    // Making some simple figures
    void createLine(GLfloat length, const QColor& color = QColor());
    void createLine(const QVector3D& first, const QVector3D& second, const QColor& color = QColor());
    void createLine(const Vasnecov::Line& line, const QColor& color = QColor());
    void createCircle(GLfloat r, const QColor& color = QColor(), GLuint factor = 64); // Circle at horizontal plane
    void createArc(GLfloat r, GLfloat startAngle, GLfloat spanAngle, const QColor& color = QColor(), GLuint factor = 128);
    void createPie(GLfloat r, GLfloat startAngle, GLfloat spanAngle, const QColor& color = QColor(), GLuint factor = 128);
    void createSquareGrid(GLfloat width, GLfloat height, const QColor& color = QColor(), GLuint horizontals = 2, GLuint verticals = 2);
    void createMeshFromFile(const QString& fileName, const QColor& color = QColor());
    void createMeshFromPoints(const std::vector<QVector3D>& points, const QColor& color = QColor());

    const std::vector<QVector3D>& vertices() const {return m_points.vertices();}
    QVector3D firstVertex() const;
    QVector3D lastVertex() const;
    QVector3D vertex(size_t index) const;
    size_t verticesAmount() const;

protected:
    GLboolean designerSetType(VasnecovFigure::Types type);

    GLenum renderUpdateData();
    void renderDraw();

    GLfloat renderCalculateDistanceToPlane(const QVector3D& planePoint, const QVector3D& normal);

    GLenum renderType() const;
    GLushort renderLineStyle() const;
    QVector3D renderCm() const;
    GLboolean renderLighting() const;
    GLboolean renderHasDepth() const;

private:
    // Класс для управления массивами вершин и индексов
    class VertexManager
    {
    public:
        VertexManager(GLenum& wasUpdated, const GLenum flag, GLboolean optimize = false) :
            m_flag(flag),
            m_wasUpdated(wasUpdated),
            m_optimize(optimize),
            raw_vertices(),
            raw_indices(),
#ifdef SINGLE_THREAD_REALIZATION
            pure_vertices(raw_vertices),
            pure_indices(raw_indices),
#else
            pure_vertices(),
            pure_indices(),
#endif
            raw_cm(),
            pure_cm()
        {}
        void setOptimization(GLboolean optimize)
        {
            m_optimize = optimize;
        }
        GLboolean optimization() const
        {
            return m_optimize;
        }

        GLuint rawVerticesSize() const
        {
            return static_cast<GLuint>(raw_vertices.size());
        }
        const std::vector<QVector3D>& vertices() const {return raw_vertices;}

        void set(const std::vector<QVector3D>& points)
        {
            // Заливка в сырые данные с удалением дубликатов точек
            raw_vertices.clear();
            raw_indices.clear();

            if(!points.empty())
            {
                raw_indices.reserve(points.size());
                // Резервирование для точек неактуально, т.к. их итоговое количество неизвестно

                if(m_optimize)
                {
                    for(GLuint i = 0; i < points.size(); ++i)
                    {
                        GLuint fIndex(0);
                        if(optimizedIndex(points[i], fIndex))
                        {
                            raw_indices.push_back(fIndex);
                        }
                        else
                        {
                            raw_vertices.push_back(points[i]);
                            raw_indices.push_back((GLuint)raw_vertices.size() - 1);
                        }
                    }
                }
                else
                {
                    raw_vertices = points;
                    for(GLuint i = 0; i < points.size(); ++i)
                    {
                        raw_indices.push_back(i);
                    }
                }
            }
            prepareUpdate();
        }
        void clear()
        {
            raw_vertices.clear();
            raw_indices.clear();

            prepareUpdate();
        }

        void addLast(const QVector3D& point)
        {
            if(raw_indices.empty())
            {
                raw_vertices.push_back(point);
                raw_indices.push_back(static_cast<GLuint>(raw_vertices.size()) - 1);
            }
            else
            {
                if(m_optimize)
                {
                    GLuint fIndex(0);
                    if(optimizedIndex(point, fIndex))
                    {
                        raw_indices.push_back(fIndex);
                    }
                    else
                    {
                        raw_vertices.push_back(point);
                        raw_indices.push_back(static_cast<GLuint>(raw_vertices.size()) - 1);
                    }
                }
                else
                {
                    raw_vertices.push_back(point);
                    raw_indices.push_back(static_cast<GLuint>(raw_vertices.size()) - 1);
                }
            }
            prepareUpdate();
        }
        void removeLast()
        {
            if(raw_indices.empty())
                return;

            removeByIndexIterator(raw_indices.end() - 1);
            prepareUpdate();
        }
        void replaceLast(const QVector3D& point)
        {
            if(raw_indices.empty())
            {
                addLast(point);
                return;
            }

            if(raw_vertices[raw_indices.back()] == point)
                return;

            // TODO: add replace with optimization removing
            raw_vertices[raw_indices.back()] = point;
            prepareUpdate();
        }

        void addFirst(const QVector3D& point)
        {
            if(raw_indices.empty())
            {
                raw_vertices.push_back(point);
                raw_indices.push_back(0);
            }
            else
            {
                if(m_optimize)
                {
                    GLuint fIndex(0);
                    if(optimizedIndex(point, fIndex))
                    {
                        raw_indices.insert(raw_indices.begin(), fIndex);
                    }
                    else
                    {
                        raw_vertices.push_back(point);
                        raw_indices.insert(raw_indices.begin(), static_cast<GLuint>(raw_vertices.size()) - 1);
                    }
                }
                else
                {
                    raw_vertices.push_back(point);
                    raw_indices.insert(raw_indices.begin(), static_cast<GLuint>(raw_vertices.size()) - 1);
                }
            }

            prepareUpdate();
        }
        void removeFirst()
        {
            if(raw_indices.empty())
                return;

            removeByIndexIterator(raw_indices.begin());
            prepareUpdate();
        }
        void replaceFirst(const QVector3D& point)
        {
            if(raw_indices.empty())
            {
                addFirst(point);
                return;
            }

            if(raw_vertices[raw_indices.front()] == point)
                return;

            raw_vertices[raw_indices.front()] = point;
            prepareUpdate();
        }

        GLenum update()
        {
            if((m_wasUpdated & m_flag) != 0)
            {
#ifndef SINGLE_THREAD_REALIZATION
//                pure_vertices.swap(raw_vertices);
//                pure_indices.swap(raw_indices);

                pure_vertices = raw_vertices;
                pure_indices  = raw_indices;
#endif
                pure_cm = raw_cm;

                m_wasUpdated = m_wasUpdated &~ m_flag; // Удаление своего флага из общего
                return m_flag;
            }
            return 0;
        }

        const std::vector<QVector3D>* pureVertices() const
        {
            return &pure_vertices;
        }
        const std::vector<GLuint>* pureIndices() const
        {
            return &pure_indices;
        }
        QVector3D cm() const
        {
            return pure_cm;
        }

    private:
        GLboolean optimizedIndex(const QVector3D& vert, GLuint& fIndex) const
        {
            for(fIndex = 0; fIndex < raw_vertices.size(); ++fIndex)
            {
                if(vert == raw_vertices[fIndex])
                {
                    return true;
                }
            }
            return false;
        }
        void removeByIndexIterator(const std::vector<GLuint>::iterator& needed)
        {
            GLuint index = *needed;

            // Поиск такого же индекса в списке индексов
            // Если он есть, значит точка используется еще где-то, поэтому список точек не трогаем
            std::vector<GLuint>::iterator found1, found2;
            // Ищем в списке до самого итератора и после
            found1 = find(raw_indices.begin(), needed, index);
            found2 = find(needed + 1, raw_indices.end(), index);

            // Удаление самого индекса
            std::vector<GLuint>::iterator next = raw_indices.erase(needed);

            if(found1 == needed && found2 == raw_indices.end()) // Не найден
            {
                // Удаляем вершину
                raw_vertices.erase(raw_vertices.begin() + index);

                // Изменяем индексы вершин после удалённой (сдвигаем на один)
                for(std::vector<GLuint>::iterator iit = next; // итератор на индекс, следующий за удаляемым
                    iit != raw_indices.end(); ++iit)
                {
                    --(*iit);
                }
            }
        }
        void prepareUpdate() // FIXME: add checking comparity
        {
            m_wasUpdated |= m_flag;

            raw_cm = QVector3D(); // Нулевой по умолчанию
            if(!raw_vertices.empty())
            {
                for(GLuint i = 0; i < raw_vertices.size(); ++i)
                {
                    raw_cm += raw_vertices[i];
                }
                raw_cm /= raw_vertices.size();
            }
        }

    private:
        const GLenum m_flag; // Флаг. Идентификатор, выдаваемый результатом синхронизации update()
        GLenum& m_wasUpdated; // Ссылка на общий флаг обновлений
        GLboolean m_optimize;

        std::vector<QVector3D> raw_vertices; // Точки сырых данных
        std::vector<GLuint> raw_indices;
#ifdef SINGLE_THREAD_REALIZATION
        std::vector<QVector3D>& pure_vertices;
        std::vector<GLuint>& pure_indices;
#else
        std::vector<QVector3D> pure_vertices;
        std::vector<GLuint> pure_indices;
#endif
        QVector3D raw_cm;
        QVector3D pure_cm;
    };

    Vasnecov::MutualData<VasnecovPipeline::ElementDrawingMethods> m_type; // Тип отрисовки
    Vasnecov::MutualData<GLushort> m_lineStyle;
    VertexManager m_points;

    Vasnecov::MutualData<GLfloat> m_thickness; // Толщина линий
    Vasnecov::MutualData<GLboolean> m_lighting; // Освещение фигуры
    Vasnecov::MutualData<GLboolean> m_depth; // Тест глубины

    enum Updated
    {
        Type		= 0x0200,
        Points		= 0x0400,
        Thickness	= 0x0800,
        Lighting	= 0x1000,
        Depth		= 0x2000,
        LineStyle   = 0x4000,
    };

    friend class VasnecovUniverse;
    friend class VasnecovWorld;

private:
    Q_DISABLE_COPY(VasnecovFigure)
};

inline void VasnecovFigure::enableLighting()
{
    m_lighting.set(true);
}
inline void VasnecovFigure::disableLighting()
{
    m_lighting.set(false);
}
inline GLboolean VasnecovFigure::lighting() const
{
    return m_lighting.raw();
}

inline void VasnecovFigure::setDepth(bool depth)
{
    m_depth.set(depth);
}
inline void VasnecovFigure::enableDepth()
{
    m_depth.set(true);
}
inline void VasnecovFigure::disableDepth()
{
    m_depth.set(false);
}
inline GLboolean VasnecovFigure::depth() const
{
    return m_depth.raw();
}
inline GLenum VasnecovFigure::renderType() const
{
    return m_type.pure();
}
inline GLushort VasnecovFigure::renderLineStyle() const
{
    return m_lineStyle.pure();
}
inline QVector3D VasnecovFigure::renderCm() const
{
    return m_points.cm();
}

inline GLboolean VasnecovFigure::renderLighting() const
{
    return m_lighting.pure();
}

inline GLboolean VasnecovFigure::renderHasDepth() const
{
    return m_depth.pure();
}
