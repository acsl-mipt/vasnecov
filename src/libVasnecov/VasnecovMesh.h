/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Класс описания трехмерных объектов для рендеринга
#pragma once

#include <vector>
#include <QVector2D>
#include <QVector3D>
#include "Configuration.h"
#include "VasnecovPipeline.h"

class VasnecovMesh
{
public:
    explicit VasnecovMesh(const QString& meshPath, const QString& name = QString());

    void setName(const QString& name); // Задать имя меша (необязательный параметр)
    VasnecovPipeline::ElementDrawingMethods type() const;
    GLboolean loadModel(GLboolean readFromMTL = Vasnecov::cfg_readFromMTL);
    GLboolean loadModel(const QString& path, GLboolean readFromMTL = Vasnecov::cfg_readFromMTL); // Загрузка модели (obj-файл)
    GLboolean loadRawModel();
    GLboolean loadRawModel(const QString& path);
    void drawModel(VasnecovPipeline* pipeline); // Отрисовка модели
    void drawBorderBox(VasnecovPipeline* pipeline); // Рисовать ограничивающий бокс
    const QVector3D& massCenter() const;

    GLboolean writeRawModel(const QString& path);

protected:
    void optimizeData();
    void calculateBox();
    GLboolean checkIndices();

    template<typename T>
    static T getPartOfArray(const char * &fromPos, const T &);
    template<typename T>
    static T getPartOfArray(const char * &fromPos);

private:
    VasnecovPipeline::
    ElementDrawingMethods   _type; // Тип отрисовки
    QString                 _name; // Имя меша (то, что пишется в мап мешей). Необязательный атрибут, для текстур и т.п.
    GLboolean               _isHidden; // Флаг на отрисовку
    QString                 _meshPath; // Адрес (относительно директории приложения) файла модели.
    GLboolean               _isLoaded;

    std::vector<GLuint>     _indices; // Индексы для отрисовки
    std::vector<QVector3D>  _vertices; // Координаты вершин
    std::vector<QVector3D>  _normals; // Координаты нормалей
    std::vector<QVector2D>  _textures; // Координаты текстур

    GLboolean               _hasTexture; // Флаг наличия внешней текстуры

    std::vector<QVector3D>  _borderBoxVertices; // Координаты ограничивающего бокса
    std::vector<GLuint>     _borderBoxIndices; // Индексы для ограничивающего бокса
    QVector3D               _massCenter; // Координата центра масс (по вершинам ограничивающей коробки)

    uint32_t                _magicNumber; // Always in BE (ex. 76 6d 66 01)

private:
    struct QuadsIndices
    {
        static const GLuint amount = 4;
        GLuint vertices[amount];
        GLuint normals[amount];
        GLuint textures[amount];

        QuadsIndices() :
            vertices{0},
            normals{0},
            textures{0}
        {}
    };
    struct TrianglesIndices
    {
        static const GLuint amount = 3;
        GLuint vertices[amount];
        GLuint normals[amount];
        GLuint textures[amount];

        TrianglesIndices() :
            vertices{0},
            normals{0},
            textures{0}
        {}
        TrianglesIndices& operator=(const QuadsIndices& value)
        {
            for(GLuint i = 0; i < amount; ++i)
            {
                vertices[i] = value.vertices[i];
                normals[i] = value.normals[i];
                textures[i] = value.textures[i];
            }

            return *this;
        }
    };
    struct LinesIndices
    {
        static const GLuint amount = 2;
        GLuint vertices[amount];
        GLuint textures[amount];

        LinesIndices() :
            vertices{0},
            textures{0}
        {}
        LinesIndices& operator=(const TrianglesIndices& value)
        {
            for(GLuint i = 0; i < amount; ++i)
            {
                vertices[i] = value.vertices[i];
                textures[i] = value.textures[i];
            }

            return *this;
        }
    };

private:
    Q_DISABLE_COPY(VasnecovMesh)

};

inline void VasnecovMesh :: setName(const QString& name)
{
    _name = name;
}

inline VasnecovPipeline::ElementDrawingMethods VasnecovMesh::type() const
{
    return _type;
}

inline const QVector3D& VasnecovMesh::massCenter() const
{
    return _massCenter;
}

template<typename T>
T VasnecovMesh::getPartOfArray(const char * &fromPos, const T &)
{
    const char *old = fromPos;
    fromPos += sizeof(T);
    return *reinterpret_cast<const T*>(old);
}

template<typename T>
T VasnecovMesh::getPartOfArray(const char * &fromPos)
{
    const char *old = fromPos;
    fromPos += sizeof(T);
    return *reinterpret_cast<const T*>(old);
}
