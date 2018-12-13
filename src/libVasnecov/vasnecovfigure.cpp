/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "vasnecovfigure.h"
#include "technologist.h"
#include <QFile>
#include <QSize>
#ifndef _MSC_VER
    #pragma GCC diagnostic warning "-Weffc++"
#endif

/*!
 \brief

 \fn VasnecovFigure::VasnecovFigure
 \param pipeline
 \param imya_
*/
VasnecovFigure::VasnecovFigure(VasnecovPipeline *pipeline, const std::string &name) :
    VasnecovElement(pipeline, name),
    m_type(VasnecovPipeline::LoopLine),
    m_points(raw_wasUpdated, Points, true),
    m_thickness(1.0f),
    m_lighting(false),
    m_depth(true)
{
}
/*!
 \brief

 \fn VasnecovFigure::~VasnecovFigure
*/
VasnecovFigure::~VasnecovFigure()
{
}

std::vector<QVector3D> VasnecovFigure::readPointsFromObj(const std::string &fileName)
{
    std::vector<QVector3D> points;

    QFile objFile(QString::fromStdString(fileName));

    if(objFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        std::vector<QVector3D> verticies;
        std::vector<QSize> indicies;
        const qint64 maxLineSize = 512;

        while(!objFile.atEnd())
        {
            QByteArray line = objFile.readLine(maxLineSize);

            // Добавление для переноса строк
            while(line.endsWith('\\'))
            {
                line.chop(1);
                line.append(objFile.readLine(maxLineSize));
            }

            line = line.simplified();

            if(line.size() >= 2)
            {
                QString textLine;
                QVector<QStringRef> parts;

                if(line.at(0) == 'v' && line.at(1) == ' ')
                {
                    textLine = QString::fromLatin1(line.constData() + 2, line.size() - 2);
                    parts = textLine.splitRef(' ');

                    if(parts.size() >= 3)
                    {
                        verticies.push_back(QVector3D(parts.at(0).toFloat(),
                                                      parts.at(1).toFloat(),
                                                      parts.at(2).toFloat()));
                    }
                }
                else if(line.at(0) == 'l' && line.at(1) == ' ')
                {
                    textLine = QString::fromLatin1(line.constData() + 2, line.size() - 2);
                    parts = textLine.splitRef(' ');

                    GLuint actSize = static_cast<GLuint>(parts.size());
                    // Линия может состоять из 2 точек (Blender)
                    // А может из нескольких. Тогда приводим одну линию к нескольким, состоящим из 2 точек.
                    if(actSize >= 2)
                    {
                        GLuint vertices[2];

                        // Перебор узлов треугольника
                        for(uint i = 0, li = 0; i < actSize; ++i)
                        {
                            QVector<QStringRef> blocks = parts.at(i).split('/');

                            if(blocks.size() == 1) // "v"
                            {
                                vertices[li] = blocks.at(0).toUInt() - 1;
                            }
                            else
                            {
                                break;
                            }

                            // После первого прохода для всех остальных
                            if(li != 1)
                            {
                                li = 1;
                            }
                            // Для всех последующих точек
                            if(i > 0)
                            {
                                indicies.push_back(QSize(vertices[0], vertices[1]));

                                vertices[0] = vertices[li];
                            }
                        }
                    }
                }
            }
        }

        GLint fails(0);

        points.reserve(indicies.size() * 2);
        for(uint i = 0; i < indicies.size(); ++i)
        {
            GLuint first = static_cast<GLuint>(indicies.at(i).width());
            GLuint last  = static_cast<GLuint>(indicies.at(i).height());

            if(first < verticies.size() && last < verticies.size())
            {
                points.push_back(verticies.at(first));
                points.push_back(verticies.at(last));
            }
            else
            {
                ++fails;
            }
        }

        if(fails > 0)
        {
            Vasnecov::problem("Некорректные данные геометрии: " + fileName + ", битых индексов: ", fails);
        }
    }
    else
    {
        Vasnecov::problem("Не удалось открыть файл геометрии: " + fileName);
    }

    return points;
}

/*!
 \brief

 \fn VasnecovFigure::ZadatTochki
 \param toch
*/
void VasnecovFigure::setPoints(std::vector <QVector3D>&& points)
{
    m_points.set(std::move(points));
}
/*!
 \brief

 \fn VasnecovFigure::ZadatTochki
 \param toch
*/
void VasnecovFigure::setPoints(const std::vector <QVector3D>& points)
{
    m_points.set(points);
}

/*!
 \brief

 \fn VasnecovFigure::DobavitTochku
 \param toch
*/
void VasnecovFigure::addLastPoint(const QVector3D &point)
{
    m_points.addLast(point);
}

void VasnecovFigure::removeLastPoint()
{
    m_points.removeLast();
}

void VasnecovFigure::replaceLastPoint(const QVector3D &point)
{
    m_points.replaceLast(point);
}
/*!
 \brief

 \fn VasnecovFigure::OchistitTochki
*/
void VasnecovFigure::clearPoints()
{
    m_points.clear();
}

GLuint VasnecovFigure::pointsAmount() const
{
    return m_points.rawVerticesSize();
}

void VasnecovFigure::addFirstPoint(const QVector3D &point)
{
    m_points.addFirst(point);
}
void VasnecovFigure::removeFirstPoint()
{
    m_points.removeFirst();
}

void VasnecovFigure::replaceFirstPoint(const QVector3D &point)
{
    m_points.replaceFirst(point);
}
/*!
 \brief

 \fn VasnecovFigure::ZadatTip
 \param tip_
 \return GLboolean
*/
GLboolean VasnecovFigure::setType(VasnecovFigure::Types type)
{
    return designerSetType(type);
}

VasnecovFigure::Types VasnecovFigure::type() const
{
    switch(m_type)
    {
        case VasnecovPipeline::PolyLine:
            return TypePolyline;
        case VasnecovPipeline::LoopLine:
            return TypePolylineLoop;
        case VasnecovPipeline::FanTriangle:
            return TypePolygons;
        case VasnecovPipeline::Lines:
            return TypeLines;
        case VasnecovPipeline::Points:
            return TypePoints;
        case VasnecovPipeline::Triangles:
            return TypeTriangles;
        default:
            return TypeUnknown;
    }
}
/*!
 \brief

 \fn VasnecovFigure::ZadatTolschinu
 \param tol_
 \return GLint
*/
GLint VasnecovFigure::setThickness(GLfloat thick)
{
    if(thick >= 1.0f && thick <= 16.0f)
    {
        m_thickness = thick;
        return 1;
    }
    else
    {
        return 0;
    }
}

GLfloat VasnecovFigure::thickness() const
{
    return m_thickness;
}

void VasnecovFigure::setOptimization(GLboolean optimize)
{
    m_points.setOptimization(optimize);
}
GLboolean VasnecovFigure::optimization() const
{
    return m_points.optimization();
}

void VasnecovFigure::createLine(GLfloat length, const QColor &color)
{
    if (length <= 0.0f)
        return;

    designerSetType(VasnecovFigure::TypeLines);
    if(color.isValid())
        m_color = color;
    m_points.set(std::vector<QVector3D>{QVector3D(0.0, 0.0, 0.0), QVector3D(length, 0.0, 0.0)});
}

void VasnecovFigure::createLine(const QVector3D &first, const QVector3D &second, const QColor &color)
{
    if (first == second)
        return;
    designerSetType(VasnecovFigure::TypeLines);
    if(color.isValid())
        m_color = color;
    m_points.set(std::vector<QVector3D>{first, second});
}

void VasnecovFigure::createLine(const Vasnecov::Line &line, const QColor &color)
{
    if(!line.isNull())
        createLine(line.p1(), line.p2(), color);
}

void VasnecovFigure::createCircle(GLfloat r, const QColor &color, GLuint factor)
{
    if (r <= 0.0f || factor <= 0)
        return;

    designerSetType(VasnecovFigure::TypePolylineLoop);
    if(color.isValid())
        m_color = color;

    std::vector<QVector3D> circ;
    circ.reserve(factor);

    QVector3D kt;
    for(GLuint i = 0; i < factor; ++i)
    {
        GLfloat angle(M_2PI*i/factor);
        kt.setX(r * cos(angle));
        kt.setY(r * sin(angle));
        circ.push_back(kt);
    }
    m_points.set(std::move(circ));
}

void VasnecovFigure::createArc(GLfloat r, GLfloat startAngle, GLfloat spanAngle, const QColor &color, GLuint factor)
{
    if (r <= 0.0f || spanAngle == 0.0f || factor <= 0)
        return;

    designerSetType(VasnecovFigure::TypePolyline);
    if(color.isValid())
        m_color = color;

    startAngle *= c_degToRad;
    spanAngle *= c_degToRad;

    std::vector<QVector3D> circ;
    GLfloat endAngle = startAngle + spanAngle;
    GLuint steps(std::abs(static_cast<float>(factor) * spanAngle / M_2PI));
    if(steps == 0)// Получается слишком маленький отрезок, поэтому он просто рисуется по двум точкам
        steps = 1;

    GLfloat pointAngle(0.0);
    circ.reserve(steps + 1);

    QVector3D kt;
    for(GLuint i = 0; i < steps; ++i)
    {
        if(spanAngle > 0)
        {
            pointAngle = M_2PI*i/factor;
        }
        else
        {
            pointAngle = -M_2PI*i/factor;
        }
        kt.setX(r*cos(pointAngle));
        kt.setY(r*sin(pointAngle));
        circ.push_back(kt);
    }

    kt.setX(r*cos(endAngle));
    kt.setY(r*sin(endAngle));
    circ.push_back(kt);

    m_points.set(std::move(circ));
}

void VasnecovFigure::createPie(GLfloat r, GLfloat startAngle, GLfloat spanAngle, const QColor &color, GLuint factor)
{
    if (r <= 0.0 || spanAngle == 0.0 || factor <= 0)
        return;

    designerSetType(VasnecovFigure::TypePolygons);
    if(color.isValid())
        m_color = color;

    startAngle *= c_degToRad;
    spanAngle *= c_degToRad;

    std::vector<QVector3D> circ;
    GLfloat endAngle = startAngle + spanAngle;
    GLuint steps = std::abs(static_cast<float>(factor) * spanAngle / (2.0f*M_PI));
    if (steps == 0) // Получается слишком маленький отрезок, поэтому он просто рисуется по двум точкам
        steps = 1;

    GLfloat pointAngle(0.0);

    circ.reserve(1 + steps + 1);
    circ.push_back(QVector3D(0, 0, 0)); // Начальная позиция в центре круга

    QVector3D kt;
    for(GLuint i = 0; i < steps; ++i)
    {
        if(spanAngle > 0)
        {
            pointAngle = M_2PI*i/factor;
        }
        else
        {
            pointAngle = -M_2PI*i/factor;
        }
        kt.setX(r * cos(pointAngle));
        kt.setY(r * sin(pointAngle));
        circ.push_back(kt);
    }

    kt.setX(r * cos(endAngle));
    kt.setY(r * sin(endAngle));
    circ.push_back(kt);

    m_points.set(std::move(circ));
}

void VasnecovFigure::createSquareGrid(GLfloat width, GLfloat height, const QColor &color, GLuint horizontals, GLuint verticals)
{
    if (!horizontals && !verticals)
        return;

    designerSetType(VasnecovFigure::TypeLines);

    if(color.isValid())
        m_color = color;

    std::vector<QVector3D> points;
    points.reserve((horizontals + verticals) * 2);

    if(horizontals)
    {
        GLfloat step = (horizontals > 1) ? height/(horizontals - 1) : 0.0f;

        QVector3D p1(- width / 2.0f,
                        (horizontals > 1) ? - height / 2.0f : 0.0f,
                        0.0f);
        QVector3D p2( width / 2.0f, p1.y(), 0.0f);

        for(GLuint i = 0; i < horizontals; ++i)
        {
            points.push_back(p1);
            points.push_back(p2);

            p1.setY(p1.y() + step);
            p2.setY(p1.y());
        }
    }

    if(verticals)
    {
        GLfloat step((verticals > 1) ? width/(verticals - 1) : 0.0f);

        QVector3D p1((verticals > 1) ? - width / 2.0f : 0.0f,
                        - height / 2.0f,
                        0.0f);
        QVector3D p2(p1.x(), height / 2.0f, 0.0f);

        for(GLuint i = 0; i < verticals; ++i)
        {
            points.push_back(p1);
            points.push_back(p2);

            p1.setX(p1.x() + step);
            p2.setX(p1.x());
        }
    }

    m_points.set(std::move(points));
}

void VasnecovFigure::createMeshFromFile(const std::string &fileName, const QColor &color)
{
    designerSetType(VasnecovFigure::TypeLines);
    if(color.isValid())
        m_color = color;
    m_points.set(readPointsFromObj(fileName));
}

void VasnecovFigure::createMeshFromPoints(const std::vector<QVector3D> &points, const QColor &color)
{
    designerSetType(VasnecovFigure::TypeLines);
    if(color.isValid())
        m_color = color;
    m_points.set(points);
}

GLboolean VasnecovFigure::designerSetType(VasnecovFigure::Types type)
{
    switch(type)
    {
        case TypePolyline:
            m_type = VasnecovPipeline::PolyLine;
            return true;
        case TypePolylineLoop:
            m_type = VasnecovPipeline::LoopLine;
            return true;
        case TypePolygons:
            m_type = VasnecovPipeline::FanTriangle;
            m_lighting = true;
            return true;
        case TypeLines:
            m_type = VasnecovPipeline::Lines;
            return true;
        case TypePoints:
            m_type = VasnecovPipeline::Points;
            return true;
        case TypeTriangles:
            m_type = VasnecovPipeline::Triangles;
            m_lighting = true;
            return true;
        default:
            return false;
    }
}

GLenum VasnecovFigure::renderUpdateData()
{
    // Проверка прозрачности
    GLboolean transp = false;

    if(m_color.alphaF() < 1.0)
    {
        transp = true;
    }
    m_isTransparency = transp;

    // Далее, как обычно
    GLenum updated = raw_wasUpdated;

    if(raw_wasUpdated)
    {
        if((raw_wasUpdated & Points) != Points) // FIXME: correct updating
        {
            pure_pipeline->setSomethingWasUpdated();
        }

        // Копирование сырых данных в основные
        m_points.update();
        VasnecovElement::renderUpdateData();
    }

    return updated;
}

/*!
 \brief

 \fn VasnecovFigure::RisovatFiguru
*/
void VasnecovFigure::renderDraw()
{
    if (m_isHidden)
        return;

    renderApplyTranslation();

    pure_pipeline->activateLamps(m_lighting);
    pure_pipeline->activateDepth(m_depth);

    pure_pipeline->setColor(m_color);
    pure_pipeline->setLineWidth(m_thickness);
    pure_pipeline->setPointSize(m_thickness);

    pure_pipeline->drawElements(m_type,
                                &m_points.pureIndices(),
                                &m_points.pureVertices());
}

GLfloat VasnecovFigure::renderCalculateDistanceToPlane(const QVector3D &planePoint, const QVector3D &normal)
{
    QVector3D centerPoint = m_points.cm();

    if(m_alienMs)
        centerPoint = (*m_alienMs) * m_Ms * centerPoint;
    else
        centerPoint = m_Ms * centerPoint;

    pure_distance = centerPoint.distanceToPlane(planePoint, normal);
    return pure_distance;
}

void VasnecovFigure::VertexManager::set(const std::vector<QVector3D>& points)
{
    auto ps = points;
    set(std::move(ps));
}

void VasnecovFigure::VertexManager::set(std::vector<QVector3D>&& points)
{   // Заливка в сырые данные с удалением дубликатов точек
    raw_vertices.clear();
    raw_indices.clear();

    if (points.empty())
    {
        prepareUpdate();
        return;
    }

    if (m_optimize)
    {
        raw_indices.reserve(points.size());
        for (GLuint i = 0; i < points.size(); ++i)
        {
            GLuint fIndex = 0;
            if (optimizedIndex(points[i], fIndex))
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
        raw_indices.resize(points.size());
        for (GLuint i = 0; i < points.size(); ++i)
        {
            raw_indices[i] = i;
        }
        raw_vertices = std::move(points);
    }
    prepareUpdate();
}
void VasnecovFigure::VertexManager::clear()
{
    raw_vertices.clear();
    raw_indices.clear();
    prepareUpdate();
}

void VasnecovFigure::VertexManager::addLast(const QVector3D& point)
{
    // Т.к. рендер только читает чистые данные, то можно их прочитать и из другого потока
    if (pure_indices.empty())
    {
        raw_vertices.push_back(point);
        raw_indices.push_back(static_cast<GLuint>(raw_vertices.size()) - 1);
        prepareUpdate();
        return;
    }

    raw_vertices = pure_vertices;
    raw_indices = pure_indices;

    if (raw_vertices[raw_indices.back()] != point)
    {
        if (m_optimize)
        {
            GLuint fIndex(0);
            if (optimizedIndex(point, fIndex))
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

        prepareUpdate();
    }
}
void VasnecovFigure::VertexManager::removeLast()
{
    if (pure_indices.empty())
        return;
    raw_vertices = pure_vertices;
    raw_indices = pure_indices;
    removeByIndexIterator(raw_indices.end() - 1);
    prepareUpdate();
}
void VasnecovFigure::VertexManager::replaceLast(const QVector3D& point)
{
    if (pure_indices.empty())
        return;
    raw_vertices = pure_vertices;
    raw_indices = pure_indices;
    if (raw_vertices[raw_indices.back()] == point)
        return;
    // TODO: add replace with optimization removing
    raw_vertices[raw_indices.back()] = point;
    prepareUpdate();
}

void VasnecovFigure::VertexManager::addFirst(const QVector3D& point)
{
    if (pure_indices.empty())
    {
        raw_vertices.push_back(point);
        raw_indices.insert(raw_indices.begin(), static_cast<GLuint>(raw_vertices.size()) - 1);
        prepareUpdate();
        return;
    }
    raw_vertices = pure_vertices;
    raw_indices = pure_indices;

    if (raw_vertices[raw_indices.front()] == point)
        return;

    if (m_optimize)
    {
        GLuint fIndex(0);
        if (optimizedIndex(point, fIndex))
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

    prepareUpdate();
}
void VasnecovFigure::VertexManager::removeFirst()
{
    if (pure_indices.empty())
        return;
    raw_vertices = pure_vertices;
    raw_indices = pure_indices;
    removeByIndexIterator(raw_indices.begin());
    prepareUpdate();
}
void VasnecovFigure::VertexManager::replaceFirst(const QVector3D& point)
{
    if (pure_indices.empty())
        return;
    raw_vertices = pure_vertices;
    raw_indices = pure_indices;
    if (raw_vertices[raw_indices.front()] != point)
    {
        raw_vertices[raw_indices.front()] = point;
        prepareUpdate();
    }
}

GLenum VasnecovFigure::VertexManager::update()
{
    if ((m_wasUpdated & m_flag) == 0)
        return 0;
    pure_vertices.swap(raw_vertices);
    pure_indices.swap(raw_indices);
    pure_cm = raw_cm;
    m_wasUpdated = m_wasUpdated & ~m_flag; // Удаление своего флага из общего
    return m_flag;
}

GLboolean VasnecovFigure::VertexManager::optimizedIndex(const QVector3D& vert, GLuint& fIndex) const
{
    for (fIndex = 0; fIndex < raw_vertices.size(); ++fIndex)
    {
        if (vert == raw_vertices[fIndex])
        {
            return true;
        }
    }
    return false;
}

void VasnecovFigure::VertexManager::removeByIndexIterator(const std::vector<GLuint>::iterator& needed)
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

    if (found1 == needed && found2 == raw_indices.end()) // Не найден
    {
        // Удаляем вершину
        raw_vertices.erase(raw_vertices.begin() + index);

        // Изменяем индексы вершин после удалённой (сдвигаем на один)
        for (std::vector<GLuint>::iterator iit = next; // итератор на индекс, следующий за удаляемым
            iit != raw_indices.end(); ++iit)
        {
            --(*iit);
        }
    }
}
void VasnecovFigure::VertexManager::prepareUpdate() // FIXME: add checking comparity
{
    m_wasUpdated |= m_flag;

    raw_cm = QVector3D(); // Нулевой по умолчанию
    if (!raw_vertices.empty())
        return;

    for (const auto& i : raw_vertices)
    {
        raw_cm += i;
    }
    raw_cm /= raw_vertices.size();
}


#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
