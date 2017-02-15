#include "vasnecovmesh.h"
#include <QVector2D>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "vasnecovpipeline.h"
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
VasnecovMesh::VasnecovMesh(const GLstring &meshPath, VasnecovPipeline *pipeline, const GLstring &name) :
    m_pipeline(pipeline),
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
    if(!m_meshPath.empty())
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
GLboolean VasnecovMesh::loadModel(const GLstring &path, GLboolean readFromMTL)
{
    m_meshPath = path;

    // Списки для данных в грубом виде
    std::vector <TrianglesIndices> rawIndices; // Набор индексов для всего подряд
    std::vector <QVector3D> rawVertices; // Координаты вершин
    std::vector <QVector3D> rawNormals; // Координаты нормалей
    std::vector <QVector2D> rawTextures; // Координаты текстур

    std::ifstream objFile(m_meshPath.c_str());
    if(!objFile)
    {
        Vasnecov::problem("Не удалось открыть файл модели: " + m_meshPath);
        return 0;
    }

    char ch;
    GLstring textLine;

    // Возможно, полезным будет добавить reserve() в вектора точек. Но для этого нужно дважды парсить файл для определения размера списков.
    while(objFile.get(ch))
    {
        std::stringstream tlStream; // Поток строки
        QVector3D cPoint; // -/- для точки в 3Д
        QVector2D cTPos; // -/- для точки в 2Д

        // Прогон на определение типа строки
        switch(ch)
        {
            case '#': // Комментарий
                getline(objFile, textLine);
                break;
            case 'v': // Вершины
                objFile.get(ch);
                GLfloat v;

                switch(ch)
                {
                    case ' ': // Вершины "v"
                    case '\t':
                        cPoint.setX(0);
                        cPoint.setY(0);
                        cPoint.setZ(0);

                        getline(objFile, textLine);
                        tlStream << textLine;

                        tlStream >> v;
                        cPoint.setX(v);
                        tlStream >> v;
                        cPoint.setY(v);
                        tlStream >> v;
                        cPoint.setZ(v);

                        rawVertices.push_back(cPoint);
                        break;
                    case 't': // Текстуры "vt", поддержка только плоских (двухмерных) текстурных координат
                        cTPos.setX(0);
                        cTPos.setY(0);

                        getline(objFile, textLine);
                        tlStream << textLine;

                        tlStream >> v;
                        cTPos.setX(v);
                        tlStream >> v;
                        cTPos.setY(-v); // из-за того, что текстура читается кверху ногами. На досуге разобраться!

                        rawTextures.push_back(cTPos);
                        break;
                    case 'n': // Нормали "vn"
                        cPoint.setX(0);
                        cPoint.setY(0);
                        cPoint.setZ(0);

                        getline(objFile, textLine);
                        tlStream << textLine;

                        tlStream >> v;
                        cPoint.setX(v);
                        tlStream >> v;
                        cPoint.setY(v);
                        tlStream >> v;
                        cPoint.setZ(v);

                        rawNormals.push_back(cPoint);
                        break;
                    default:
                        getline(objFile, textLine);
                        break;
                }
                break;
            case 'f': // Полигоны (поддерживаются только треугольники, остальное не читается; отрицательные индексы не учитываются)
                objFile.get(ch);
                if(ch == ' ' || ch == '\t') // Алгоритм, наверно, идиотский! Наличие кучи разной удобности плюшек STL оказало своё черное влияние :) В смысле, через сканф это всё делается куда проще...
                {
                    TrianglesIndices cIndex;

                    getline(objFile, textLine);

                    // Разбивается на 3 строки, анализируется по количеству слешей, заменяются слеши на пробелы, потом в поток, а из потока в уинты.
                    tlStream << textLine;

                    GLuint singleSlash, doubleSlash; // Количество слешей и двойных слешей
                    size_t res; // Поисковая позиция

                    // Перебор узлов треугольника
                    for(GLuint i = 0; i < 3; ++i)
                    {
                        GLstring fLine;
                        std::stringstream fStream;

                        tlStream >> fLine;

                        if(!fLine.empty())
                        {
                            for(singleSlash = 0, res = 0; (res=fLine.find("/", res+1)) != GLstring::npos; ++singleSlash);
                            for(doubleSlash = 0, res = 0; (res=fLine.find("//", res+1)) != GLstring::npos; ++doubleSlash);

                            if(singleSlash == 0 && doubleSlash == 0) // "v"
                            {
                                fStream << fLine;
                                fStream >> cIndex.vertices[i];

                                cIndex.vertices[i]--;
                            }
                            else if(singleSlash == 1 && doubleSlash == 0) // "v/t"
                            {
                                std::replace(fLine.begin(), fLine.end(), '/', ' ');
                                fStream << fLine;
                                fStream >> cIndex.vertices[i];
                                fStream >> cIndex.textures[i];

                                cIndex.vertices[i]--;
                                cIndex.textures[i]--;
                            }
                            else if(singleSlash == 2 && doubleSlash == 1) // "v//n"
                            {
                                std::replace(fLine.begin(), fLine.end(), '/', ' ');
                                fStream << fLine;
                                fStream >> cIndex.vertices[i];
                                fStream >> cIndex.normals[i];

                                cIndex.vertices[i]--;
                                cIndex.normals[i]--;
                            }
                            else if(singleSlash == 2 && doubleSlash == 0) // "v/t/n"
                            {
                                std::replace(fLine.begin(), fLine.end(), '/', ' ');
                                fStream << fLine;
                                fStream >> cIndex.vertices[i];
                                fStream >> cIndex.textures[i];
                                fStream >> cIndex.normals[i];

                                cIndex.vertices[i]--;
                                cIndex.textures[i]--;
                                cIndex.normals[i]--;
                            }
                        }
                    }

                    rawIndices.push_back(cIndex);
                }
                break;
            case 'm': // Библиотека материалов (mtllib)
                getline(objFile, textLine);
                break;
            case 'u': // Указатель на материал (usemtl)
                {
                    getline(objFile, textLine);
                    if(readFromMTL)
                    {
                        size_t pos_prob;
                        pos_prob = textLine.find_first_of(" "); // Отделение текста по пробелу
                        if(pos_prob != GLstring::npos)
                        {
                            size_t pos_null;
                            pos_null = textLine.find("(null)"); // исключение материала с именем (null) - где-то используется для обозначения отсутствующих материалов.
                            if(pos_null == GLstring::npos)
                            {
//								m_textureD = stroka.substr(pos_prob+1, GLstring::npos);
                                m_hasTexture = 1;
                            }
                        }
                    }
                    else if(m_name != "")
                    {
//						m_textureD = m_name;
                        m_hasTexture = 1;
                    }
                }
                break;
            case 'g': // Группа
                getline(objFile, textLine);
                break;
            case 's': // Группирование по сглаживанию
                getline(objFile, textLine);
                break;
            default: // Всё остальное в мусор
                getline(objFile, textLine);
                break;
        }
    }

    if(!objFile.eof())
    {
        Vasnecov::problem("Ошибка чтения файла модели: " + m_meshPath);
        objFile.close();
        return 0;
    }
    objFile.close();

    // Проверка на наличие индексов
    GLuint vm = rawVertices.size();
    GLuint nm = rawNormals.size();
    GLuint tm = rawTextures.size();
    GLuint im = rawIndices.size();

    if(vm == 0)
    {
        Vasnecov::problem("Модель не содержит точек: " + m_meshPath);

        m_isLoaded = false;
        return m_isLoaded;
    }

    int fails(0);

    for(GLuint i = 0; i < im; ++i)
    {
        for(GLuint j = 0; j < 3; ++j)
        {
            GLuint vi = rawIndices[i].vertices[j];
            GLuint ni = rawIndices[i].normals[j];
            GLuint ti = rawIndices[i].textures[j];

            if(vi >= vm && vm != 0)
            {
                fails++;
            }
            if(ni >= nm && nm != 0)
            {
                fails++;
            }
            if(ti >= tm && tm != 0)
            {
                fails++;
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
    m_indices.reserve(rawIndices.size());

    for(GLuint i = 0; i < rawIndices.size(); ++i)
    {
        for(GLuint j = 0; j < 3; ++j)
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

        m_pipeline->drawElements(VasnecovPipeline::Triangles,
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

    m_cm.setX((m_borderBoxVertices[0].x() + m_borderBoxVertices[6].x())*0.5);
    m_cm.setY((m_borderBoxVertices[0].y() + m_borderBoxVertices[6].y())*0.5);
    m_cm.setZ((m_borderBoxVertices[0].z() + m_borderBoxVertices[6].z())*0.5);


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
