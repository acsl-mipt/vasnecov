// Класс описания трехмерных объектов для рендеринга

#ifndef VASNECOVMESH_H
#define VASNECOVMESH_H

#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
#include <vector>
#include "configuration.h"
#include "vasnecovpipeline.h"
#ifndef _MSC_VER
    #pragma GCC diagnostic warning "-Weffc++"
#endif

class VasnecovMesh
{
public:
    VasnecovMesh(const GLstring &meshPath, VasnecovPipeline *pipeline, const GLstring &name = "");

    void setName(GLstring name); // Задать имя меша (необязательный параметр)
    VasnecovPipeline::ElementDrawingMethods type() const;
    GLboolean loadModel(GLboolean readFromMTL = Vasnecov::cfg_readFromMTL);
    GLboolean loadModel(const GLstring &path, GLboolean readFromMTL = Vasnecov::cfg_readFromMTL); // Загрузка модели (obj-файл)
    void drawModel(); // Отрисовка модели
    QVector3D cm() const;
    void drawBorderBox(); // Рисовать ограничивающий бокс

protected:
    void optimizeData();
    void calculateBox();

protected:
    VasnecovPipeline *const m_pipeline;
    VasnecovPipeline::ElementDrawingMethods m_type; // Тип отрисовки
    GLstring m_name; // Имя меша (то, что пишется в мап мешей). Необязательный атрибут, для текстур и т.п.
    GLboolean m_isHidden; // Флаг на отрисовку
    GLstring m_meshPath; // Адрес (относительно директории приложения) файла модели.
    GLboolean m_isLoaded;

    std::vector<GLuint> m_indices; // Индексы для отрисовки
    std::vector<QVector3D> m_vertices; // Координаты вершин
    std::vector<QVector3D> m_normals; // Координаты нормалей
    std::vector<QVector2D> m_textures; // Координаты текстур

    GLboolean m_hasTexture; // Флаг наличия внешней текстуры

    std::vector <QVector3D> m_borderBoxVertices; // Координаты ограничивающего бокса
    std::vector <GLuint> m_borderBoxIndices; // Индексы для ограничивающего бокса
    QVector3D m_cm; // Координата центра масс (по вершинам ограничивающей коробки)

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
        TrianglesIndices &operator=(const QuadsIndices &value)
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
        LinesIndices &operator=(const TrianglesIndices &value)
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

inline void VasnecovMesh :: setName(GLstring name)
{
    m_name = name;
}

inline VasnecovPipeline::ElementDrawingMethods VasnecovMesh::type() const
{
    return m_type;
}

inline QVector3D VasnecovMesh :: cm() const
{
    return m_cm;
}

#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
#endif // VASNECOVMESH_H
