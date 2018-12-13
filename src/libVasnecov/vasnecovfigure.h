/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Класс фигур для мира
#ifndef VASNECOVFIGURE_H
#define VASNECOVFIGURE_H

#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
#include "vasnecovelement.h"
#ifndef _MSC_VER
    #pragma GCC diagnostic warning "-Weffc++"
#endif

// Класс фигур (плоских, по сути)
class VasnecovFigure : public VasnecovElement
{
    // Класс для управления массивами вершин и индексов
    class VertexManager
    {
    public:
        VertexManager(GLenum& wasUpdated, const GLenum flag, GLboolean optimize = true) :
            m_flag(flag),
            m_wasUpdated(wasUpdated),
            m_optimize(optimize)
        {}
        void setOptimization(GLboolean optimize) { m_optimize = optimize; }
        GLboolean optimization() const { return m_optimize; }
        GLuint rawVerticesSize() const { return static_cast<GLuint>(raw_vertices.size()); }

        void set(std::vector<QVector3D>&& points);
        void set(const std::vector<QVector3D>& points);
        void clear();
        void addLast(const QVector3D& point);
        void removeLast();
        void replaceLast(const QVector3D& point);
        void addFirst(const QVector3D& point);
        void removeFirst();
        void replaceFirst(const QVector3D& point);
        GLenum update();

        const std::vector<QVector3D>& pureVertices() const { return pure_vertices; }
        const std::vector<GLuint>& pureIndices() const { return pure_indices; }
        const QVector3D& cm() const { return pure_cm; }

    private:
        GLboolean optimizedIndex(const QVector3D& vert, GLuint& fIndex) const;
        void removeByIndexIterator(const std::vector<GLuint>::iterator& needed);
        void prepareUpdate();
    private:
        const GLenum m_flag; // Флаг. Идентификатор, выдаваемый результатом синхронизации update()
        GLenum& m_wasUpdated; // Ссылка на общий флаг обновлений
        GLboolean m_optimize;

        std::vector<QVector3D> raw_vertices; // Точки сырых данных
        std::vector<GLuint> raw_indices;

        std::vector<QVector3D> pure_vertices; // Уникальные вершины (без дубликатов) для отрисовки
        std::vector<GLuint> pure_indices; // Индексы отрисовки

        QVector3D raw_cm;
        QVector3D pure_cm;
    };

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

public:
    VasnecovFigure(VasnecovPipeline* pipeline, const std::string& name = std::string());
    ~VasnecovFigure();

    static std::vector<QVector3D> readPointsFromObj(const std::string& fileName);

    void setPoints(std::vector<QVector3D>&& points);
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

    void enableLighting() { m_lighting = true; }
    void disableLighting() { m_lighting = false; }
    GLboolean lighting() const { return m_lighting; }

    void enableDepth() { m_depth = true; }
    void disableDepth() { m_depth = false; }
    GLboolean depth() const { return m_depth; }

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
    void createMeshFromFile(const std::string& fileName, const QColor& color = QColor());
    void createMeshFromPoints(const std::vector<QVector3D>& points, const QColor& color = QColor());

protected:
    GLboolean designerSetType(VasnecovFigure::Types type);

    GLenum renderUpdateData() override;
    void renderDraw() override;

    GLfloat renderCalculateDistanceToPlane(const QVector3D& planePoint, const QVector3D& normal) override;

    GLenum renderType() const { return m_type; }
    const QVector3D& renderCm() const { return m_points.cm(); }
    GLboolean renderLighting() const { return m_lighting; }

protected:
    VasnecovPipeline::ElementDrawingMethods m_type; // Тип отрисовки
    VertexManager m_points;

    GLfloat   m_thickness; // Толщина линий
    GLboolean m_lighting; // Освещение фигуры
    GLboolean m_depth; // Тест глубины

    enum Updated
    {
        Type		= 0x0200,
        Points		= 0x0400,
        Thickness	= 0x0800,
        Lighting	= 0x1000,
        Depth		= 0x2000
    };

    friend class VasnecovUniverse;
    friend class VasnecovWorld;

private:
    Q_DISABLE_COPY(VasnecovFigure)
};


#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
#endif // VASNECOVFIGURE_H
