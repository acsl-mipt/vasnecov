/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "vasnecovmesh.h"
#include <QVector2D>
#include <QFile>
#include <QVector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "technologist.h"
#ifndef _MSC_VER
    #pragma GCC diagnostic warning "-Weffc++"
#endif

/*!
 \brief

 \fn VasnecovMesh::VasnecovMesh
 \param meshPath
 \param name
*/
VasnecovMesh::VasnecovMesh(const QString &meshPath, VasnecovPipeline *pipeline, const QString &name) :
    m_pipeline(pipeline),
    m_type(VasnecovPipeline::Points),
    m_name(name),
    m_isHidden(true),
    m_meshPath(meshPath),
    m_isLoaded(false),

    m_indices(),
    m_vertices(),
    m_normals(),
    m_textures(),

    m_hasTexture(false),
    m_borderBoxVertices(8),
    m_borderBoxIndices(24),
    m_cm()
{
}

/*!
 \brief

 \fn VasnecovMesh::loadModel
 \param readFromMTL
 \return GLboolean
*/
GLboolean VasnecovMesh::loadModel(GLboolean readFromMTL)
{
    if(!m_meshPath.isEmpty())
    {
        return loadModel(m_meshPath, readFromMTL);
    }
    else
    {
        return false;
    }
}
/*!
 \brief

 \fn VasnecovMesh::loadModel
 \param path
 \param readFromMTL
 \return GLboolean
*/

GLboolean VasnecovMesh::loadModel(const QString &path, GLboolean readFromMTL)
{
    m_meshPath = path;
    m_type = VasnecovPipeline::Points;

    // Списки для данных в грубом виде
    std::vector <TrianglesIndices> rawIndices; // Набор индексов для всего подряд
    std::vector <LinesIndices> rawLinesIndices; // Набор индексов для отрисовки линий
    std::vector <QVector3D> rawVertices; // Координаты вершин
    std::vector <QVector3D> rawNormals; // Координаты нормалей
    std::vector <QVector2D> rawTextures; // Координаты текстур

    QFile objFile(path);
    if(!objFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        Vasnecov::problem("Не удалось открыть файл модели: " + m_meshPath);
        return 0;
    }

    // Возможно, полезным будет добавить reserve() в вектора точек. Но для этого нужно дважды парсить файл для определения размера списков.
    const qint64 maxLineSize = 512;

    while(!objFile.atEnd())
    {
        // "Object files can be in ASCII format (.obj)" - на это и рассчитываем
        QByteArray line = objFile.readLine(maxLineSize);

        // Добавление для переноса строк
        while(line.endsWith('\\'))
        {
            line.chop(1);
            line.append(objFile.readLine(maxLineSize));
        }

        line = line.simplified();

        if(!line.isEmpty())
        {
            QString textLine;
            QVector<QStringRef> parts;

            // Прогон на определение типа строки
            switch(line.at(0))
            {
                case '#': // Комментарий
                    break;
                case 'v': // Вершины: v, vt, vn, vp
                    if(line.size() > 2) // 'v' + ' ' + data
                    {
                        switch(line.at(1))
                        {
                            case ' ': // Вершины "v"
                                textLine = QString::fromLatin1(line.constData() + 2, line.size() - 2);
                                parts = textLine.splitRef(' ');

                                if(parts.size() >= 3)
                                {
                                    rawVertices.push_back(QVector3D(parts.at(0).toFloat(),
                                                                    parts.at(1).toFloat(),
                                                                    parts.at(2).toFloat()));
                                }
                                break;
                            case 't': // Текстуры "vt", поддержка только плоских (двухмерных) текстурных координат
                                textLine = QString::fromLatin1(line.constData() + 3, line.size() - 3);
                                parts = textLine.splitRef(' ');

                                if(parts.size() >= 2)
                                {
                                    rawTextures.push_back(QVector2D(parts.at(0).toFloat(),
                                                                   -parts.at(1).toFloat())); // из-за того, что текстура читается кверху ногами. На досуге разобраться!
                                }
                                break;
                            case 'n': // Нормали "vn"
                                textLine = QString::fromLatin1(line.constData() + 3, line.size() - 3);
                                parts = textLine.splitRef(' ');

                                if(parts.size() >= 3)
                                {
                                    rawNormals.push_back(QVector3D(parts.at(0).toFloat(),
                                                                   parts.at(1).toFloat(),
                                                                   parts.at(2).toFloat()));
                                }
                                break;
                            case 'p':
                            default:
                                break;
                        }
                    }
                    break;
                case 'f': // Полигоны (поддерживаются только треугольники, остальное не читается; отрицательные индексы не учитываются)
                    if(line.size() > 2 && line.at(1) == ' ')
                    {
                        // Разбивается на 3 строки, анализируется по количеству слешей
                        textLine = QString::fromLatin1(line.constData() + 2, line.size() - 2);
                        parts = textLine.splitRef(' ');

                        if(static_cast<GLuint>(parts.size()) == TrianglesIndices::amount)
                        {
                            TrianglesIndices cIndex;
                            bool correct(true);

                            // Перебор узлов треугольника
                            for(uint i = 0; i < TrianglesIndices::amount; ++i)
                            {
                                QVector<QStringRef> blocks = parts.at(i).split('/');

                                if(blocks.size() == 3) // "v/t/n" or "v//n"
                                {
                                    // Индексы obj-файла начинаются с единицы, поэтому вычитаем
                                    cIndex.vertices[i] = blocks.at(0).toUInt() - 1;

                                    if(!blocks.at(1).isEmpty())
                                        cIndex.textures[i] = blocks.at(1).toUInt() - 1;

                                    cIndex.normals[i]  = blocks.at(2).toUInt() - 1;
                                }
                                else if(blocks.size() == 2) // "v/t"
                                {
                                    cIndex.vertices[i] = blocks.at(0).toUInt() - 1;
                                    cIndex.textures[i] = blocks.at(1).toUInt() - 1;
                                }
                                else if(blocks.size() == 1) // "v"
                                {
                                    cIndex.vertices[i] = blocks.at(0).toUInt() - 1;
                                }
                                else
                                {
                                    correct = false;
                                    break;
                                }
                            }

                            if(correct)
                            {
                                rawIndices.push_back(cIndex);
                                if(m_type != VasnecovPipeline::Triangles)
                                {
                                    m_type = VasnecovPipeline::Triangles;
                                }
                            }
                        }
                    }
                    break;
                case 'l': // Отрисовка линиями, если не заданы полигоны
                    if(line.size() > 2 && line.at(1) == ' ')
                    {
                        if(m_type != VasnecovPipeline::Triangles)
                        {
                            textLine = QString::fromLatin1(line.constData() + 2, line.size() - 2);
                            parts = textLine.splitRef(' ');

                            GLuint actSize = static_cast<GLuint>(parts.size());
                            // Линия может состоять из 2 точек (Blender)
                            // А может из нескольких. Тогда приводим одну линию к нескольким, состоящим из 2 точек.
                            if(actSize >= LinesIndices::amount)
                            {
                                LinesIndices cIndex;
                                bool correct(true);

                                // Перебор узлов треугольника
                                for(uint i = 0, li = 0; i < actSize; ++i)
                                {
                                    QVector<QStringRef> blocks = parts.at(i).split('/');

                                    if(blocks.size() == 2) // "v/t"
                                    {
                                        cIndex.vertices[li] = blocks.at(0).toUInt() - 1;
                                        cIndex.textures[li] = blocks.at(1).toUInt() - 1;
                                    }
                                    else if(blocks.size() == 1) // "v"
                                    {
                                        cIndex.vertices[li] = blocks.at(0).toUInt() - 1;
                                    }
                                    else
                                    {
                                        correct = false;
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
                                        rawLinesIndices.push_back(cIndex);

                                        cIndex.vertices[0] = cIndex.vertices[li];
                                        cIndex.textures[0] = cIndex.textures[li];
                                    }
                                }

                                if(correct)
                                {
                                    if(m_type != VasnecovPipeline::Lines)
                                    {
                                        m_type = VasnecovPipeline::Lines;
                                    }
                                }
                            }
                        }
                    }
                    break;
                case 'm': // Библиотека материалов (mtllib), группы (mg)
                    break;
                case 'u': // Указатель на материал (usemtl)
                    {
                        if(readFromMTL)
                        {
                            textLine = QString::fromLatin1(line.constData(), line.size());
                            parts = textLine.splitRef(' ');

                            if(parts.size() == 2)
                            {
                                if(parts.at(0) == "usemtl")
                                {
                                    // исключение материала с именем (null) - где-то используется для обозначения отсутствующих материалов.
                                    if(parts.at(1) != "(null)")
                                    {
//        								m_textureD = parts.at(1);
                                        m_hasTexture = 1;
                                    }
                                }
                                else if(m_name != "")
                                {
//            						m_textureD = m_name;
                                    m_hasTexture = 1;
                                }
                            }
                        }
                    }
                    break;
                case 'g': // Группа
                    break;
                case 's': // Группирование по сглаживанию
                    break;
                default: // Всё остальное в мусор
                    break;
            }
        }
    }

    objFile.close();

    // Проверка на наличие индексов
    GLuint vm = rawVertices.size();
    GLuint nm = rawNormals.size();
    GLuint tm = rawTextures.size();
    GLuint indCount = rawIndices.size();
    if(m_type == VasnecovPipeline::Lines)
        indCount = rawLinesIndices.size();

    if(vm == 0)
    {
        Vasnecov::problem("Модель не содержит точек: " + m_meshPath);

        m_isLoaded = false;
        return m_isLoaded;
    }

    GLint fails(0);

    if(m_type == VasnecovPipeline::Lines)
    {
        for(GLuint i = 0; i < indCount; ++i)
        {
            for(GLuint j = 0; j < LinesIndices::amount; ++j)
            {
                GLuint vi = rawLinesIndices[i].vertices[j];
                GLuint ti = rawLinesIndices[i].textures[j];

                if(vi >= vm && vm != 0)
                {
                    ++fails;
                }
                if(ti >= tm && tm != 0)
                {
                    ++fails;
                }
            }
        }
    }
    else
    {
        for(GLuint i = 0; i < indCount; ++i)
        {
            for(GLuint j = 0; j < TrianglesIndices::amount; ++j)
            {
                GLuint vi = rawIndices[i].vertices[j];
                GLuint ni = rawIndices[i].normals[j];
                GLuint ti = rawIndices[i].textures[j];

                if(vi >= vm && vm != 0)
                {
                    ++fails;
                }
                if(ni >= nm && nm != 0)
                {
                    ++fails;
                }
                if(ti >= tm && tm != 0)
                {
                    ++fails;
                }
            }
        }
    }

    if(fails > 0)
    {
        Vasnecov::problem("Некорректные данные модели: " + m_meshPath + ", битых индексов: ", fails);

        m_isLoaded = false;
        return m_isLoaded;
    }

    // Приведение данных к нормальному виду (пригодному для отрисовки по общему индексу)
    m_indices.reserve(indCount);

    if(m_type == VasnecovPipeline::Lines)
    {
        for(GLuint i = 0; i < indCount; ++i)
        {
            for(GLuint j = 0; j < LinesIndices::amount; ++j)
            {
                GLuint vi = rawLinesIndices[i].vertices[j];
                GLuint ti = rawLinesIndices[i].textures[j];

                m_vertices.push_back(rawVertices[vi]);

                if(tm)
                {
                    m_textures.push_back(rawTextures[ti]);
                }

                m_indices.push_back(m_vertices.size()-1);
            }
        }
    }
    else
    {
        for(GLuint i = 0; i < indCount; ++i)
        {
            for(GLuint j = 0; j < TrianglesIndices::amount; ++j)
            {
                GLuint vi = rawIndices[i].vertices[j];
                GLuint ni = rawIndices[i].normals[j];
                GLuint ti = rawIndices[i].textures[j];

                m_vertices.push_back(rawVertices[vi]);

                if(nm)
                {
                    m_normals.push_back(rawNormals[ni]);
                }
                if(tm)
                {
                    m_textures.push_back(rawTextures[ti]);
                }

                m_indices.push_back(m_vertices.size()-1);
            }
        }
    }

    optimizeData();
    calculateBox();

    // Выставление флагов
    m_isLoaded = true;
    m_isHidden = false;

    return m_isLoaded;
}

/*!
 \brief

 \fn VasnecovMesh::drawModel
 \param scale
*/
void VasnecovMesh::drawModel()
{
    if(!m_isHidden && m_isLoaded)
    {
        std::vector<QVector3D> *norms(0);
        std::vector<QVector2D> *texts(0);

        if(!m_normals.empty())
        {
            norms = &m_normals;
        }
        if(!m_textures.empty())
        {
            texts = &m_textures;
        }

        m_pipeline->drawElements(m_type,
                                 &m_indices,
                                 &m_vertices,
                                 norms,
                                 texts);
    }
}

/*!
 \brief

 \fn VasnecovMesh::drawBorderBox
*/
void VasnecovMesh::drawBorderBox()
{
    if(!m_isHidden && m_isLoaded)
    {
        m_pipeline->drawElements(VasnecovPipeline::Lines, &m_borderBoxIndices, &m_borderBoxVertices);
    }
}

void VasnecovMesh::optimizeData()
{
    // Оптимизация массивов
    // Проверка на наличие индексов
    std::vector<GLuint> rawIndices(m_indices);
    m_indices.clear();

    std::vector<QVector3D> rawVertices(m_vertices);
    m_vertices.clear();

    std::vector<QVector3D> rawNormals(m_normals);
    m_normals.clear();

    std::vector<QVector2D> rawTextures(m_textures);
    m_textures.clear();

    // Массив oldIndices - индексы просто по порядку
    for(GLuint i = 0; i < rawIndices.size(); ++i)
    {
        QVector3D ver(rawVertices[i]);
        GLboolean found(false);
        GLuint fIndex(0);

        // Поиск точки в списке точек
        for(fIndex = 0; fIndex < m_vertices.size(); ++fIndex)
        {
            if(m_vertices[fIndex] == ver)
            {
                found = true;
                if(!rawNormals.empty())
                {
                    // Есть нормали и их индексы не совпадают - переходим к следующей точке
                    if(m_normals[fIndex] != rawNormals[i])
                    {
                        found = false;
                        continue;
                    }
                }
                if(!rawTextures.empty())
                {
                    if(m_textures[fIndex] != rawTextures[i])
                    {
                        found = false;
                        continue;
                    }
                }
                break;
            }
        }

        if(found) // Точка найдена, пишем индекс дубля
        {
            m_indices.push_back(fIndex);
        }
        else // Точка не найдена, заносим новые данные
        {
            m_vertices.push_back(ver);
            if(!rawNormals.empty())
            {
                m_normals.push_back(rawNormals[i]);
            }
            if(!rawTextures.empty())
            {
                m_textures.push_back(rawTextures[i]);
            }

            m_indices.push_back(m_vertices.size() - 1); // Добавляем правильный индекс
        }
    }
}

void VasnecovMesh::calculateBox()
{
    GLuint vm = m_vertices.size();

    // Определение ограничивающих боксов и центра "масс"
    if(vm > 0)
    {
        m_borderBoxVertices[0].setX(m_vertices[0].x());
        m_borderBoxVertices[0].setY(m_vertices[0].y());
        m_borderBoxVertices[0].setZ(m_vertices[0].z());

        m_borderBoxVertices[6].setX(m_vertices[0].x());
        m_borderBoxVertices[6].setY(m_vertices[0].y());
        m_borderBoxVertices[6].setZ(m_vertices[0].z());
    }


    for(GLuint i = 0; i < vm; ++i)
    {
        // Минимальная точка
        if(m_vertices[i].x() < m_borderBoxVertices[0].x())
        {
            m_borderBoxVertices[0].setX(m_vertices[i].x());
        }
        if(m_vertices[i].y() < m_borderBoxVertices[0].y())
        {
            m_borderBoxVertices[0].setY(m_vertices[i].y());
        }
        if(m_vertices[i].z() < m_borderBoxVertices[0].z())
        {
            m_borderBoxVertices[0].setZ(m_vertices[i].z());
        }

        // Максимальная точка
        if(m_vertices[i].x() > m_borderBoxVertices[6].x())
        {
            m_borderBoxVertices[6].setX(m_vertices[i].x());
        }
        if(m_vertices[i].y() > m_borderBoxVertices[6].y())
        {
            m_borderBoxVertices[6].setY(m_vertices[i].y());
        }
        if(m_vertices[i].z() > m_borderBoxVertices[6].z())
        {
            m_borderBoxVertices[6].setZ(m_vertices[i].z());
        }
    }

    m_borderBoxVertices[1].setX(m_borderBoxVertices[0].x()); m_borderBoxVertices[1].setY(m_borderBoxVertices[6].y()); m_borderBoxVertices[1].setZ(m_borderBoxVertices[0].z());
    m_borderBoxVertices[2].setX(m_borderBoxVertices[6].x()); m_borderBoxVertices[2].setY(m_borderBoxVertices[6].y()); m_borderBoxVertices[2].setZ(m_borderBoxVertices[0].z());
    m_borderBoxVertices[3].setX(m_borderBoxVertices[6].x()); m_borderBoxVertices[3].setY(m_borderBoxVertices[0].y()); m_borderBoxVertices[3].setZ(m_borderBoxVertices[0].z());

    m_borderBoxVertices[4].setX(m_borderBoxVertices[0].x()); m_borderBoxVertices[4].setY(m_borderBoxVertices[0].y()); m_borderBoxVertices[4].setZ(m_borderBoxVertices[6].z());
    m_borderBoxVertices[5].setX(m_borderBoxVertices[0].x()); m_borderBoxVertices[5].setY(m_borderBoxVertices[6].y()); m_borderBoxVertices[5].setZ(m_borderBoxVertices[6].z());
    m_borderBoxVertices[7].setX(m_borderBoxVertices[6].x()); m_borderBoxVertices[7].setY(m_borderBoxVertices[0].y()); m_borderBoxVertices[7].setZ(m_borderBoxVertices[6].z());

    m_cm.setX((m_borderBoxVertices[0].x() + m_borderBoxVertices[6].x())*0.5f);
    m_cm.setY((m_borderBoxVertices[0].y() + m_borderBoxVertices[6].y())*0.5f);
    m_cm.setZ((m_borderBoxVertices[0].z() + m_borderBoxVertices[6].z())*0.5f);


    // Индексы для бокса
    m_borderBoxIndices[0] = 0;
    m_borderBoxIndices[1] = 1;
    m_borderBoxIndices[2] = 1;
    m_borderBoxIndices[3] = 2;
    m_borderBoxIndices[4] = 2;
    m_borderBoxIndices[5] = 3;
    m_borderBoxIndices[6] = 3;
    m_borderBoxIndices[7] = 0;
    m_borderBoxIndices[8] = 4;
    m_borderBoxIndices[9] = 5;
    m_borderBoxIndices[10] = 5;
    m_borderBoxIndices[11] = 6;
    m_borderBoxIndices[12] = 6;
    m_borderBoxIndices[13] = 7;
    m_borderBoxIndices[14] = 7;
    m_borderBoxIndices[15] = 4;
    m_borderBoxIndices[16] = 0;
    m_borderBoxIndices[17] = 4;
    m_borderBoxIndices[18] = 1;
    m_borderBoxIndices[19] = 5;
    m_borderBoxIndices[20] = 2;
    m_borderBoxIndices[21] = 6;
    m_borderBoxIndices[22] = 3;
    m_borderBoxIndices[23] = 7;
}


#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
