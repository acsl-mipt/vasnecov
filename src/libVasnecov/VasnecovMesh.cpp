/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "VasnecovMesh.h"
#include <QVector2D>
#include <QtEndian>
#include <QFile>
#include <QVector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "Technologist.h"

VasnecovMesh::VasnecovMesh(const QString& meshPath, const QString& name)
    : _type(VasnecovPipeline::Points)
    , _name(name)
    , _isHidden(true)
    , _meshPath(meshPath)
    , _isLoaded(false)

    , _indices()
    , _vertices()
    , _normals()
    , _textures()

    , _hasTexture(false)
    , _borderBoxVertices(8)
    , _borderBoxIndices(24)
    , _massCenter()
    , _magicNumber(qToBigEndian(0x766d6601))
{
}
GLboolean VasnecovMesh::loadModel(GLboolean readFromMTL)
{
    if(!_meshPath.isEmpty())
    {
        return loadModel(_meshPath, readFromMTL);
    }
    else
    {
        return false;
    }
}
GLboolean VasnecovMesh::loadModel(const QString &path, GLboolean readFromMTL)
{
    _meshPath = path;
    _type = VasnecovPipeline::Points;

    // Списки для данных в грубом виде
    std::vector <TrianglesIndices> rawIndices; // Набор индексов для всего подряд
    std::vector <LinesIndices> rawLinesIndices; // Набор индексов для отрисовки линий
    std::vector <QVector3D> rawVertices; // Координаты вершин
    std::vector <QVector3D> rawNormals; // Координаты нормалей
    std::vector <QVector2D> rawTextures; // Координаты текстур

    QFile objFile(path);
    if(!objFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        Vasnecov::problem("Can't open model file: " + _meshPath);
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
                                if(_type != VasnecovPipeline::Triangles)
                                {
                                    _type = VasnecovPipeline::Triangles;
                                }
                            }
                        }
                    }
                    break;
                case 'l': // Отрисовка линиями, если не заданы полигоны
                    if(line.size() > 2 && line.at(1) == ' ')
                    {
                        if(_type != VasnecovPipeline::Triangles)
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
                                    if(_type != VasnecovPipeline::Lines)
                                    {
                                        _type = VasnecovPipeline::Lines;
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
                                        _hasTexture = 1;
                                    }
                                }
                                else if(_name != "")
                                {
//            						m_textureD = m_name;
                                    _hasTexture = 1;
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
    if(_type == VasnecovPipeline::Lines)
        indCount = rawLinesIndices.size();

    if(vm == 0)
    {
        Vasnecov::problem("Model does not have points: " + _meshPath);

        _isLoaded = false;
        return _isLoaded;
    }

    GLint fails(0);

    if(_type == VasnecovPipeline::Lines)
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
        Vasnecov::problem("Incorrect geometry data: " + _meshPath + ", wrong indices: ", fails);

        _isLoaded = false;
        return _isLoaded;
    }

    // Приведение данных к нормальному виду (пригодному для отрисовки по общему индексу)
    _indices.reserve(indCount);

    if(_type == VasnecovPipeline::Lines)
    {
        for(GLuint i = 0; i < indCount; ++i)
        {
            for(GLuint j = 0; j < LinesIndices::amount; ++j)
            {
                GLuint vi = rawLinesIndices[i].vertices[j];
                GLuint ti = rawLinesIndices[i].textures[j];

                _vertices.push_back(rawVertices[vi]);

                if(tm)
                {
                    _textures.push_back(rawTextures[ti]);
                }

                _indices.push_back(_vertices.size()-1);
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

                _vertices.push_back(rawVertices[vi]);

                if(nm)
                {
                    _normals.push_back(rawNormals[ni]);
                }
                if(tm)
                {
                    _textures.push_back(rawTextures[ti]);
                }

                _indices.push_back(_vertices.size()-1);
            }
        }
    }

    optimizeData();
    calculateBox();

    // Выставление флагов
    _isLoaded = true;
    _isHidden = false;

    return _isLoaded;
}

GLboolean VasnecovMesh::loadRawModel()
{
    return loadRawModel(_meshPath);
}

GLboolean VasnecovMesh::loadRawModel(const QString& path)
{
    _meshPath = path;
    QFile rawFile(path);
    if(!rawFile.open(QIODevice::ReadOnly))
    {
        Vasnecov::problem("Can't open raw-model file: " + _meshPath);
        return false;
    }

    QByteArray data;

    data = rawFile.read(sizeof (_magicNumber) + sizeof (uint16_t) * 2);  // MN & block size & type

    const char *dataPos = data.constData();
    uint32_t magic = getPartOfArray(dataPos, magic);
    if(magic != _magicNumber)
    {
        Vasnecov::problem("Model is not VMF-file: " + _meshPath);
        return false;
    }

    uint16_t blocksAmount = getPartOfArray(dataPos, blocksAmount);
    uint16_t type = getPartOfArray(dataPos, type);

    // Sizes
    data.append(rawFile.read(sizeof (uint32_t) * 4));
    dataPos = data.constData() + 8;

    uint32_t indicesSize = getPartOfArray(dataPos, indicesSize);
    uint32_t verticesSize = getPartOfArray(dataPos, verticesSize);
    uint32_t normalsSize = getPartOfArray(dataPos, normalsSize);
    uint32_t texturesSize = getPartOfArray(dataPos, texturesSize);

    // Indices
    data = rawFile.read(indicesSize * sizeof (uint32_t));
    dataPos = data.constData();
    _indices.reserve(indicesSize);
    for(size_t i = 0; i < indicesSize; ++i)
    {
        _indices.push_back(getPartOfArray<uint32_t>(dataPos));
    }

    // Vertices
    data = rawFile.read(verticesSize * sizeof (float) * 3);
    dataPos = data.constData();
    _vertices.reserve(verticesSize);
    for(size_t i = 0; i < verticesSize; ++i)
    {
        float x = getPartOfArray<float>(dataPos);
        float y = getPartOfArray<float>(dataPos);
        float z = getPartOfArray<float>(dataPos);

        _vertices.emplace_back(x, y, z);
    }

    // Normals
    data = rawFile.read(normalsSize * sizeof (float) * 3);
    dataPos = data.constData();
    _normals.reserve(normalsSize);
    for(size_t i = 0; i < normalsSize; ++i)
    {
        float x = getPartOfArray<float>(dataPos);
        float y = getPartOfArray<float>(dataPos);
        float z = getPartOfArray<float>(dataPos);

        _normals.emplace_back(x, y, z);
    }

    // Textures
    data = rawFile.read(texturesSize * sizeof (float) * 3);
    dataPos = data.constData();
    _textures.reserve(texturesSize);
    for(size_t i = 0; i < texturesSize; ++i)
    {
        float x = getPartOfArray<float>(dataPos);
        float y = getPartOfArray<float>(dataPos);

        _textures.emplace_back(x, y);
    }

    _type = static_cast<VasnecovPipeline::ElementDrawingMethods>(type);

    calculateBox();

    _isLoaded = true;
    _isHidden = false;

    // FIXME: check indices!

    return true;
}
void VasnecovMesh::drawModel(VasnecovPipeline* pipeline)
{
    if(pipeline == nullptr)
        return;

    if(!_isHidden && _isLoaded)
    {
        std::vector<QVector3D> *norms(nullptr);
        std::vector<QVector2D> *texts(nullptr);

        if(!_normals.empty())
        {
            norms = &_normals;
        }
        if(!_textures.empty())
        {
            texts = &_textures;
        }

        pipeline->drawElements(_type,
                               &_indices,
                               &_vertices,
                               norms,
                               texts);
    }
}
void VasnecovMesh::drawBorderBox(VasnecovPipeline* pipeline)
{
    if(pipeline == nullptr)
        return;

    if(!_isHidden && _isLoaded)
    {
        pipeline->drawElements(VasnecovPipeline::Lines, &_borderBoxIndices, &_borderBoxVertices);
    }
}

/**
  File structure (Little-endian):

  --- Main header block [4+2+2]
  [4] - magic number (BE: 0x766d6601 - vmf1, 1 - version number)
  [2] - uint16_t - blocks amount 'a' (default - 4: indices, vertices, normals, textures)
  [2] - uint16_t - type of drawing

  --- Sizes [4 blocks example]
  [4 * a] - uint32_t - sizes of indices, vertices, normals, textures ('i', 'v', 'n', 't')

  --- Values [4 blocks example]
  [4 * i] - uint32_t - Index
  [4 * 3 * v] - float * 3 - Vertex 3D-vector
  [4 * 3 * n] - float * 3 - Normal 3D-vector
  [4 * 2 * t] - float * 2 - Texture 2D-vector

 */


GLboolean VasnecovMesh::writeRawModel(const QString& path)
{
    if(!_isLoaded)
    {
        Vasnecov::problem("Writing model was not loaded: ", _meshPath);
        return false;
    }

    QFile rawFile(path);
    if(!rawFile.open(QIODevice::WriteOnly))
    {
        Vasnecov::problem("Can't open writing raw-model file: " + _meshPath);
        return false;
    }
    else
    {
        rawFile.write(reinterpret_cast<const char*>(&_magicNumber), sizeof(_magicNumber));

        uint16_t blocksAmount(4); // Indices, vertices, normals, textures
        rawFile.write(reinterpret_cast<const char*>(&blocksAmount), sizeof(blocksAmount));
        uint16_t type(_type);
        rawFile.write(reinterpret_cast<const char*>(&type), sizeof(type));

        uint32_t blockSize(0);
        blockSize = _indices.size();
        rawFile.write(reinterpret_cast<const char*>(&blockSize), sizeof(blockSize));

        blockSize = _vertices.size();
        rawFile.write(reinterpret_cast<const char*>(&blockSize), sizeof(blockSize));

        blockSize = _normals.size();
        rawFile.write(reinterpret_cast<const char*>(&blockSize), sizeof(blockSize));

        blockSize = _textures.size();
        rawFile.write(reinterpret_cast<const char*>(&blockSize), sizeof(blockSize));

        // data
        for(auto ind : _indices)
        {
            uint32_t i = ind;
            rawFile.write(reinterpret_cast<const char*>(&i), sizeof(i));
        }

        for(auto value : _vertices)
        {
            float c;
            c = value.x();
            rawFile.write(reinterpret_cast<const char*>(&c), sizeof(c));
            c = value.y();
            rawFile.write(reinterpret_cast<const char*>(&c), sizeof(c));
            c = value.z();
            rawFile.write(reinterpret_cast<const char*>(&c), sizeof(c));
        }

        for(auto value : _normals)
        {
            float c;
            c = value.x();
            rawFile.write(reinterpret_cast<const char*>(&c), sizeof(c));
            c = value.y();
            rawFile.write(reinterpret_cast<const char*>(&c), sizeof(c));
            c = value.z();
            rawFile.write(reinterpret_cast<const char*>(&c), sizeof(c));
        }

        for(auto value : _textures)
        {
            float c;
            c = value.x();
            rawFile.write(reinterpret_cast<const char*>(&c), sizeof(c));
            c = value.y();
            rawFile.write(reinterpret_cast<const char*>(&c), sizeof(c));
        }

    }

    return true;
}

void VasnecovMesh::optimizeData()
{
    // Оптимизация массивов
    // Проверка на наличие индексов
    std::vector<GLuint> rawIndices(_indices);
    _indices.clear();

    std::vector<QVector3D> rawVertices(_vertices);
    _vertices.clear();

    std::vector<QVector3D> rawNormals(_normals);
    _normals.clear();

    std::vector<QVector2D> rawTextures(_textures);
    _textures.clear();

    // Массив oldIndices - индексы просто по порядку
    for(GLuint i = 0; i < rawIndices.size(); ++i)
    {
        QVector3D ver(rawVertices[i]);
        GLboolean found(false);
        GLuint fIndex(0);

        // Поиск точки в списке точек
        for(fIndex = 0; fIndex < _vertices.size(); ++fIndex)
        {
            if(_vertices[fIndex] == ver)
            {
                found = true;
                if(!rawNormals.empty())
                {
                    // Есть нормали и их индексы не совпадают - переходим к следующей точке
                    if(_normals[fIndex] != rawNormals[i])
                    {
                        found = false;
                        continue;
                    }
                }
                if(!rawTextures.empty())
                {
                    if(_textures[fIndex] != rawTextures[i])
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
            _indices.push_back(fIndex);
        }
        else // Точка не найдена, заносим новые данные
        {
            _vertices.push_back(ver);
            if(!rawNormals.empty())
            {
                _normals.push_back(rawNormals[i]);
            }
            if(!rawTextures.empty())
            {
                _textures.push_back(rawTextures[i]);
            }

            _indices.push_back(_vertices.size() - 1); // Добавляем правильный индекс
        }
    }
}

void VasnecovMesh::calculateBox()
{
    GLuint vm = _vertices.size();

    // Определение ограничивающих боксов и центра "масс"
    if(vm > 0)
    {
        _borderBoxVertices[0].setX(_vertices[0].x());
        _borderBoxVertices[0].setY(_vertices[0].y());
        _borderBoxVertices[0].setZ(_vertices[0].z());

        _borderBoxVertices[6].setX(_vertices[0].x());
        _borderBoxVertices[6].setY(_vertices[0].y());
        _borderBoxVertices[6].setZ(_vertices[0].z());
    }


    for(GLuint i = 0; i < vm; ++i)
    {
        // Минимальная точка
        if(_vertices[i].x() < _borderBoxVertices[0].x())
        {
            _borderBoxVertices[0].setX(_vertices[i].x());
        }
        if(_vertices[i].y() < _borderBoxVertices[0].y())
        {
            _borderBoxVertices[0].setY(_vertices[i].y());
        }
        if(_vertices[i].z() < _borderBoxVertices[0].z())
        {
            _borderBoxVertices[0].setZ(_vertices[i].z());
        }

        // Максимальная точка
        if(_vertices[i].x() > _borderBoxVertices[6].x())
        {
            _borderBoxVertices[6].setX(_vertices[i].x());
        }
        if(_vertices[i].y() > _borderBoxVertices[6].y())
        {
            _borderBoxVertices[6].setY(_vertices[i].y());
        }
        if(_vertices[i].z() > _borderBoxVertices[6].z())
        {
            _borderBoxVertices[6].setZ(_vertices[i].z());
        }
    }

    _borderBoxVertices[1].setX(_borderBoxVertices[0].x()); _borderBoxVertices[1].setY(_borderBoxVertices[6].y()); _borderBoxVertices[1].setZ(_borderBoxVertices[0].z());
    _borderBoxVertices[2].setX(_borderBoxVertices[6].x()); _borderBoxVertices[2].setY(_borderBoxVertices[6].y()); _borderBoxVertices[2].setZ(_borderBoxVertices[0].z());
    _borderBoxVertices[3].setX(_borderBoxVertices[6].x()); _borderBoxVertices[3].setY(_borderBoxVertices[0].y()); _borderBoxVertices[3].setZ(_borderBoxVertices[0].z());

    _borderBoxVertices[4].setX(_borderBoxVertices[0].x()); _borderBoxVertices[4].setY(_borderBoxVertices[0].y()); _borderBoxVertices[4].setZ(_borderBoxVertices[6].z());
    _borderBoxVertices[5].setX(_borderBoxVertices[0].x()); _borderBoxVertices[5].setY(_borderBoxVertices[6].y()); _borderBoxVertices[5].setZ(_borderBoxVertices[6].z());
    _borderBoxVertices[7].setX(_borderBoxVertices[6].x()); _borderBoxVertices[7].setY(_borderBoxVertices[0].y()); _borderBoxVertices[7].setZ(_borderBoxVertices[6].z());

    _massCenter.setX((_borderBoxVertices[0].x() + _borderBoxVertices[6].x())*0.5f);
    _massCenter.setY((_borderBoxVertices[0].y() + _borderBoxVertices[6].y())*0.5f);
    _massCenter.setZ((_borderBoxVertices[0].z() + _borderBoxVertices[6].z())*0.5f);


    // Индексы для бокса
    _borderBoxIndices[0] = 0;
    _borderBoxIndices[1] = 1;
    _borderBoxIndices[2] = 1;
    _borderBoxIndices[3] = 2;
    _borderBoxIndices[4] = 2;
    _borderBoxIndices[5] = 3;
    _borderBoxIndices[6] = 3;
    _borderBoxIndices[7] = 0;
    _borderBoxIndices[8] = 4;
    _borderBoxIndices[9] = 5;
    _borderBoxIndices[10] = 5;
    _borderBoxIndices[11] = 6;
    _borderBoxIndices[12] = 6;
    _borderBoxIndices[13] = 7;
    _borderBoxIndices[14] = 7;
    _borderBoxIndices[15] = 4;
    _borderBoxIndices[16] = 0;
    _borderBoxIndices[17] = 4;
    _borderBoxIndices[18] = 1;
    _borderBoxIndices[19] = 5;
    _borderBoxIndices[20] = 2;
    _borderBoxIndices[21] = 6;
    _borderBoxIndices[22] = 3;
    _borderBoxIndices[23] = 7;
}

GLboolean VasnecovMesh::checkIndices()
{
    return false;
}
