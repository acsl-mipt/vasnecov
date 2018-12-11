/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "vasnecovuniverse.h"
#include <QFile>
#include <QDir>
#include <QDirIterator>
#ifdef _MSC_VER
    #include <windows.h>
#endif
#include <GL/glu.h>
#include "vasnecovmesh.h"
#include <bmcl/Logging.h>
#ifndef _MSC_VER
    #pragma GCC diagnostic warning "-Weffc++"
#endif

/*!
   \class VasnecovUniverse
   \brief Класс Вселенной. Обеспечивает добавление и удаление объектов, контролирует их состояния.

   Основной класс библиотеки. С помощью него создаются миры и элементы. Им уже удаляются. Так же
   позволяет задавать некоторые параметры отрисовки. Ну, и конечно же, загружать внешние ресурсы. Все
   методы, создающие объекты, возвращают указатель на этот объект в случае успеха, и ноль - в случае
   провала. Методы удаления элементов возвращают true в случае успеха.
 */

/*!
 \brief Конструктор Вселенной.
*/
VasnecovUniverse::VasnecovUniverse(const QGLContext *context) :
    m_context(context),
    m_backgroundColor(0, 0, 0, 255),

    m_width(Vasnecov::cfg_displayWidthDefault),
    m_height(Vasnecov::cfg_displayHeightDefault),

    m_loading(false),
    m_loadingImageTimer(Vasnecov::timeDefault()),
    m_lampsCountMax(Vasnecov::cfg_lampsCountMax)
{
    Q_INIT_RESOURCE(vasnecov);

    if(!m_loadingImage0.load(":/share/loading0.png") ||
       !m_loadingImage1.load(":/share/loading1.png"))
    {
        Vasnecov::problem("Не загрузилась иконка предзагрузки");
    }
    if(!m_loadingImage0.isNull())
    {
        m_loadingImage0 = m_loadingImage0.mirrored(0, 1);
    }
    if(!m_loadingImage1.isNull())
    {
        m_loadingImage1 = m_loadingImage1.mirrored(0, 1);
    }
#ifndef _MSC_VER
    if(clock_gettime(CLOCK_MONOTONIC, &m_loadingImageTimer) < 0)
    {
        Vasnecov::problem("Проблемы с таймером картинки загрузки");
    }
#endif
}

/*!
 \brief

 \fn VasnecovUniverse::~VasnecovUniverse
*/
VasnecovUniverse::~VasnecovUniverse()
{
    Q_CLEANUP_RESOURCE(vasnecov);
}

/*!
 \brief Добавление мира во Вселенную.

 Метод добавляет новый мир. Дальнейшее управление осуществляется через указатель на этот мир.

 \param posX горизонтальная позиция левой нижней точки окна мира
 \param posY вертикальная позиция левой нижней точки окна мира
 \param width ширина окна
 \param height высота окна
 \return VasnecovWorld указатель на созданный мир
*/
VasnecovWorld *VasnecovUniverse::addWorld(GLint posX, GLint posY, GLsizei width, GLsizei height)
{
    if(width > Vasnecov::cfg_worldWidthMin && width < Vasnecov::cfg_worldWidthMax &&
       height > Vasnecov::cfg_worldHeightMin && height < Vasnecov::cfg_worldHeightMax)
    {
        VasnecovWorld *newWorld = new VasnecovWorld(&m_pipeline, posX, posY, width, height);

        m_elements.addElement(newWorld);

        return newWorld;
    }

    Vasnecov::problem("Неверные размеры мира");
    return nullptr;
}

/*!
 \brief Добавление источника света во Вселенную.

 \param name наименование источника (уникальность не требуется)
 \param world мир, в котором размещается источник
 \param type тип источника света
 \return VasnecovLamp указатель на созданный источник
*/
VasnecovLamp *VasnecovUniverse::addLamp(const std::string &name, VasnecovWorld *world, VasnecovLamp::LampTypes type)
{
    if(!world)
    {
        Vasnecov::problem("Мир не задан");
        return nullptr;
    }

    GLuint index(GL_LIGHT0);

    // Поиск мира в списке
    if(!m_elements.findRawElement(world))
    {
        Vasnecov::problem("Мир задан неверно");
        return nullptr;
    }
    size_t count = m_elements.rawLampsCount();
    if(count < m_lampsCountMax)
    {
        // Индексы задаются тупо прибавлением. Если какие-то фонари удалялись, то индекс будет неверным.
        // Но это не страшно, т.к. при обновлении данных в updateData индексы будут переназначены
        index += count;
    }

    VasnecovLamp *lamp = new VasnecovLamp(&m_pipeline, name, type, index);
    if(m_elements.addElement(lamp))
    {
        world->designerAddElement(lamp);
        return lamp;
    }
    else
    {
        delete lamp;
        lamp = nullptr;

        Vasnecov::problem("Неверный фонарь либо дублирование данных");
        return nullptr;
    }
}

/*!
 \brief Создание ссылки на источник света в другом мире.

 Если источник отсутствует во Вселенной или уже есть в этом мире, то метод возвращает ноль.
 При этом создаётся именно ссылка, а не копия. Благодаря чему источник изменяется извне единожды, но
 отображается в обоих мира.

  Подобное поведение удобно для создания двух миров, которые описывают одно и то же пространство, но
  отображают его с разных положений камеры, каждый в своём окне.

 \param lamp указатель дублируемого источника
 \param world мир. в который дублируется источник
 \return VasnecovLamp указатель на сам источник в случае успеха, ноль - в случае провала
*/
VasnecovLamp *VasnecovUniverse::referLampToWorld(VasnecovLamp *lamp, VasnecovWorld *world)
{
    if(!lamp || !world)
    {
        Vasnecov::problem("Фонарь или мир не заданы");
        return nullptr;
    }

    // Поиск фонаря в общем списке
    if(!m_elements.findRawElement(lamp))
    {
        Vasnecov::problem("Заданный фонарь не найден");
        return nullptr;
    }
    // Поиск мира в списке
    if(!m_elements.findRawElement(world))
    {
        Vasnecov::problem("Мир задан неверно");
        return nullptr;
    }

    if(world->designerAddElement(lamp, true))
    {
        return lamp;
    }
    else
    {
        Vasnecov::problem("Неверный фонарь либо дублирование данных");
        return nullptr;
    }
}

/*!
 \brief

 \param name
 \param world
 \param parent
 \return VasnecovProduct
*/
VasnecovProduct *VasnecovUniverse::addAssembly(const std::string &name, VasnecovWorld *world, VasnecovProduct *parent)
{
    if(!world)
    {
        Vasnecov::problem("Мир не задан");
        return nullptr;
    }

    VasnecovProduct *assembly(nullptr);
    GLuint level(0);

    // Поиск мира в списке
    if(!m_elements.findRawElement(world))
    {
        Vasnecov::problem("Мир задан неверно");
        return nullptr;
    }

    if(parent)
    {
        if(world->designerFindElement(parent))
        {
            level = parent->designerLevel() + 1;
            if(level > Vasnecov::cfg_elementMaxLevel)
            {
                Vasnecov::problem("Превышен максимальный уровень вложенности изделия");
                return nullptr;
            }
        }
        else
        {
            Vasnecov::problem("Родительский узел не найден");
            return nullptr;
        }
    }

    // world && (parent exists)
    assembly = new VasnecovProduct(&m_pipeline, name, VasnecovProduct::ProductTypeAssembly, parent, level);

    if(parent)
    {
        assembly->designerSetMatrixM1(parent->designerMatrixMs());
        parent->designerAddChild(assembly);
    }
    m_elements.addElement(assembly);
    world->designerAddElement(assembly); // Здесь не требуется проверка на дубликаты, т.к. указатель assembly девственно чист

    return assembly;
}

/*!
 \brief

 \param name
 \param world
 \param meshName
 \param parent
 \return VasnecovProduct
*/
VasnecovProduct *VasnecovUniverse::addPart(const std::string &name, VasnecovWorld *world, const std::string &meshName, VasnecovProduct *parent)
{
    return addPart(name, world, meshName, nullptr, parent);
}

/*!
 \brief

 \param name
 \param world
 \param meshName
 \param material
 \param parent
 \return VasnecovProduct
*/
VasnecovProduct *VasnecovUniverse::addPart(const std::string &name, VasnecovWorld *world, const std::string &meshName, VasnecovMaterial *material, VasnecovProduct *parent)
{
    if(!world)
    {
        Vasnecov::problem("Мир не задан");
        return nullptr;
    }

    VasnecovProduct *part(nullptr);
    VasnecovMesh *mesh(nullptr);
    GLuint level(0);

    // Проверка на наличие меша и его догрузка при необходимости
    if(!meshName.empty())
    {
        std::string corMeshName = correctFileId(meshName, Vasnecov::cfg_meshFormat);
        // Поиск меша в списке
        mesh = designerFindMesh(corMeshName);

        if(!mesh)
        {
            // Попытка загрузить насильно
            // Метод загрузки сам управляет мьютексом
            if(!loadMeshFile(corMeshName))
            {
                Vasnecov::problem("Не найден заданный меш");
                return nullptr;
            }

            mesh = designerFindMesh(corMeshName);

            if(!mesh)
            {
                // Условие невозможное после попытки загрузки, но для надёжности оставим :)
                Vasnecov::problem("Не найден заданный меш");
                return nullptr;
            }
        }
    }
    else
    {
        Vasnecov::problem("Не указан меш");
        return nullptr;
    }

    // Поиск мира в списке
    if(!m_elements.findRawElement(world))
    {
        Vasnecov::problem("Мир задан не верно");
        return nullptr;
    }
    // Поиск материала
    if(material)
    {
        if(!m_elements.findRawElement(material))
        {
            Vasnecov::problem("Не найден заданный материал");
            return nullptr;
        }
    }
    // Указан родитель
    if(parent)
    {
        if(world->designerFindElement(parent))
        {
            level = parent->designerLevel() + 1;
            if(level > Vasnecov::cfg_elementMaxLevel)
            {
                Vasnecov::problem("Превышен максимальный уровень вложенности изделия");
                return nullptr;
            }
        }
        else
        {
            Vasnecov::problem("Родительский узел не найден");
            return nullptr;
        }
    }

    // world && mesh && (parent exists)
    part = new VasnecovProduct(&m_pipeline, name, mesh, material, parent, level);

    if(parent)
    {
        part->designerSetMatrixM1(parent->designerMatrixMs());
        parent->designerAddChild(part);
    }
    m_elements.addElement(part);
    world->designerAddElement(part); // Здесь не требуется проверка на дубликаты, т.к. указатель part девственно чист

    return part;
}

/*!
 \brief

 \param name
 \param world
 \param meshName
 \param textureName
 \param parent
 \return VasnecovProduct
*/
VasnecovProduct *VasnecovUniverse::addPart(const std::string &name, VasnecovWorld *world, const std::string &meshName, const std::string &textureName, VasnecovProduct *parent)
{
    if(!textureName.empty())
    {
        return addPart(name, world, meshName, addMaterial(raw_data.dirTexturesDPref + textureName), parent);
    }
    else
    {
        return addPart(name, world, meshName, nullptr, parent);
    }
}

/*!
 \brief

 \param product
 \param world
 \return VasnecovProduct
*/
VasnecovProduct *VasnecovUniverse::referProductToWorld(VasnecovProduct *product, VasnecovWorld *world)
{
    if(!product || !world)
    {
        Vasnecov::problem("Элемент или мир не заданы");
        return nullptr;
    }

    // Поиск изделия в общем списке
    if(!m_elements.findRawElement(product))
    {
        Vasnecov::problem("Заданное изделие не найдено");
        return nullptr;
    }
    // Поиск мира в списке
    if(!m_elements.findRawElement(world))
    {
        Vasnecov::problem("Мир задан неверно");
        return nullptr;
    }

    if(world->designerAddElement(product, true))
    {
        return product;
    }
    else
    {
        Vasnecov::problem("Неверное изделие либо дублирование данных");
        return nullptr;
    }
}

/*!
 \brief Удаление продукта и всех дочерних продуктов.

 Удаляет продукт и все дочерние продукты из всех списков и аннулирует все внутренние ссылки на него.

 \note В зависимости от количества элементов и сложности структуры взаимных связей между элементами
 выполнение этого метода может быть ресурсоёмким. Если конечное приложение содержит больше одного управляющего
 потока, то остальные управляющие потоки будут находится в ожидании, т.к. данный метод блокирует мьютекс
 на всё время выполнения.

 \param product укзатель на удаляемый продукт.
 \return GLboolean true в случае успешного удаления.
*/
GLboolean VasnecovUniverse::removeProduct(VasnecovProduct *product)
{
    if(!product)
        return false;

    /* Тут необходимо:
     * 1. Проверить используется ли материал продукта в других продуктах.
     * Если нет - удалить материал тоже (при этом текстуру не удалять)
     * 2. Удалить из списка родителя.
     * 3. Удалить всех детей, если продукт является узлом.
     * 4. Убить все чужие матрицы, которые были от этих (продукт + дети + внуки) продуктов.
     */

    if(m_elements.findRawElement(product))
    {
        // Удаление продуктов - долгая операция с заблокированным мьютексом
        std::vector<VasnecovProduct *> delProd(product->designerAllChildren());
        delProd.push_back(product);

        m_elements.removeElements(delProd);

        // Удаление материалов
        std::vector<VasnecovMaterial *> delMat;
        for(VasnecovProduct* dit: delProd)
        {
            if(dit->designerMaterial())
            {
                delMat.push_back(dit->designerMaterial());
            }
        }
        // Если материал встречается где-то в других продуктах, то из списка претендентов он вычеркивается
        for(const auto& pit: m_elements.rawProducts())
        {
            auto i = std::find_if(delMat.begin(), delMat.end(), [&](const auto& mit) {return pit->designerMaterial() == mit; });
            if (i != delMat.end())
                delMat.erase(i);
        }
        // Непосредственное удаление больше не нужных материалов
        m_elements.removeElements(delMat);

        // Прочие удаления
        for(VasnecovProduct* dit: delProd)
        {
            // Удаление из мира
            for(auto& wit: m_elements.rawWorlds().raw())
            {
                wit->designerRemoveElement(dit);
            }

            // Удаление из списка родителей
            if(dit->designerParent())
            {
                dit->designerParent()->designerRemoveChild(dit);
            }

            // Удаление чужих матриц
            const QMatrix4x4 *matrix = dit->designerExportingMatrix();
            designerRemoveThisAlienMatrix(matrix);
        }

        return true;
    }

    return false;
}

VasnecovFigure *VasnecovUniverse::addFigure(const std::string &name, VasnecovWorld *world)
{
    if(!world)
    {
        Vasnecov::problem("Мир не задан");
        return nullptr;
    }

    VasnecovFigure *figure(nullptr);

    // Поиск мира в списке
    if(!m_elements.findRawElement(world))
    {
        Vasnecov::problem("Мир задан не верно");
        return nullptr;
    }

    figure = new VasnecovFigure(&m_pipeline, name);

    if(m_elements.addElement(figure))
    {
        world->designerAddElement(figure);
        return figure;
    }
    else
    {
        delete figure;
        figure = nullptr;

        Vasnecov::problem("Неверная фигура либо дублирование данных");
        return nullptr;
    }
}

GLboolean VasnecovUniverse::removeFigure(VasnecovFigure *figure)
{
    if(!figure)
        return false;

    if(m_elements.findRawElement(figure))
    {
        m_elements.removeElement(figure);

        // Прочие удаления
        // Удаление из мира
        for(auto& wit: m_elements.rawWorlds().raw())
        {
            wit->designerRemoveElement(figure);
        }

        // Удаление чужих матриц
        designerRemoveThisAlienMatrix(figure->designerExportingMatrix());

        return true;
    }

    return false;
}
/*!
   \brief Добавление новой метки

   Поведение аналогично \a VasnecovUniverse::addLamp.

   \param name наименование
   \param world мир, в котором создаётся метка
   \param width ширина метки
   \param height высота метки
   \return VasnecovLabel указатель на успешно созданную метку.
 */
VasnecovLabel *VasnecovUniverse::addLabel(const std::string &name, VasnecovWorld *world, GLfloat width, GLfloat height)
{
    return addLabel(name, world, width, height, "");
}
/*!
   \brief Перегруженная версия \a VasnecovUniverse::addLabel

   Добавляет интерфейсную текстуру, заданную по имени.

   \param textureName имя интерфейсной текстуры. См. \a VasnecovUniverse::loadTextures.
 */
VasnecovLabel *VasnecovUniverse::addLabel(const std::string &name, VasnecovWorld *world, GLfloat width, GLfloat height, const std::string &textureName)
{
    if(!world)
    {
        Vasnecov::problem("Мир не задан");
        return nullptr;
    }

    VasnecovLabel *label(nullptr);
    VasnecovTexture *texture(nullptr);

    // Проверка на наличие текстуры и её догрузка при необходимости
    if(!textureName.empty()) // Иначе нулевая текстура
    {
        std::string corTextureName = correctFileId(textureName, Vasnecov::cfg_textureFormat);

        texture = designerFindTexture(raw_data.dirTexturesIPref + corTextureName);

        if(!texture)
        {
            // Попытка загрузить насильно
            // Метод загрузки сам управляет мьютексом
            if(!loadTextureFile(raw_data.dirTexturesIPref + corTextureName))
            {
                Vasnecov::problem("Не найдена заданная текстура");
                return nullptr;
            }

            texture = designerFindTexture(raw_data.dirTexturesIPref + corTextureName);
            if(!texture)
            {
                // Условие невозможное после попытки загрузки, но для надёжности оставим :)
                Vasnecov::problem("Не найдена заданная текстура");
                return nullptr;
            }
        }
    }

    // Поиск мира в списке
    if(!m_elements.findRawElement(world))
    {
        Vasnecov::problem("Мир задан не верно");
        return nullptr;
    }

    label = new VasnecovLabel(&m_pipeline, name, QVector2D(width, height), texture);

    if(m_elements.addElement(label))
    {
        world->designerAddElement(label);
        return label;
    }
    else
    {
        delete label;
        label = nullptr;

        Vasnecov::problem("Неверная метка либо дублирование данных");
        return nullptr;
    }
}

VasnecovLabel *VasnecovUniverse::referLabelToWorld(VasnecovLabel *label, VasnecovWorld *world)
{
    if(!label || !world)
    {
        Vasnecov::problem("Элемент или мир не заданы");
        return nullptr;
    }

    // Поиск в общем списке
    if(!m_elements.findRawElement(label))
    {
        Vasnecov::problem("Заданная метка не найдена");
        return nullptr;
    }
    // Поиск мира в списке
    if(!m_elements.findRawElement(world))
    {
        Vasnecov::problem("Мир задан неверно");
        return nullptr;
    }

    if(world->designerAddElement(label, true))
    {
        return label;
    }
    else
    {
        Vasnecov::problem("Неверная метка либо дублирование данных");
        return nullptr;
    }
}
/*!
   \brief Удаление метки.

   Удаляет метку из всех списков и аннулирует все внутренние ссылки на неё.

   \param label - указатель на метку.
   \return true в случае успеха.
 */
GLboolean VasnecovUniverse::removeLabel(VasnecovLabel *label)
{
    if(!label)
        return false;

    if(m_elements.findRawElement(label))
    {
        m_elements.removeElement(label);

        // Прочие удаления
        // Удаление из мира
        for(auto& wit: m_elements.rawWorlds().raw())
        {
            wit->designerRemoveElement(label);
        }

        // Удаление чужих матриц
        designerRemoveThisAlienMatrix(label->designerExportingMatrix());

        return true;
    }

    return false;
}

/*!
 \brief

 \param textureName
 \return VasnecovMaterial
*/
VasnecovMaterial *VasnecovUniverse::addMaterial(const std::string &textureName)
{
    VasnecovTexture *texture(nullptr);

    // Проверка на наличие текстуры и её догрузка при необходимости
    if(!textureName.empty()) // Иначе нулевая текстура
    {
        // Поиск текстуры в списке
        std::string corTextureName = correctFileId(textureName, Vasnecov::cfg_textureFormat);

        texture = designerFindTexture(corTextureName);

        if(!texture)
        {
            // Попытка загрузить насильно
            // Метод загрузки сам управляет мьютексом
            if(!loadTextureFile(corTextureName))
            {
                Vasnecov::problem("Заданная текстура не найдена");
                return nullptr;
            }

            texture = designerFindTexture(corTextureName);

            if(!texture)
            {
                // Условие невозможное после попытки загрузки, но для надёжности оставим :)
                Vasnecov::problem("Заданная текстура не найдена");
                return nullptr;
            }
        }
    }

    VasnecovMaterial *material = new VasnecovMaterial(&m_pipeline, texture);
    if(m_elements.addElement(material))
    {
        return material;
    }
    else
    {
        delete material;
        material = nullptr;

        Vasnecov::problem("Неверный материал либо дублирование данных");
        return nullptr;
    }
}

/*!
 \brief

 \return VasnecovMaterial
*/
VasnecovMaterial *VasnecovUniverse::addMaterial()
{
    VasnecovMaterial *material = new VasnecovMaterial(&m_pipeline);
    if(m_elements.addElement(material))
    {
        return material;
    }
    else
    {
        delete material;
        material = nullptr;

        Vasnecov::problem("Неверный материал либо дублирование данных");
        return nullptr;
    }
}

/*!
 \brief

 \param textureName
 \param type
 \return VasnecovTexture
*/
VasnecovTexture *VasnecovUniverse::textureByName(const std::string &textureName, Vasnecov::TextureTypes type)
{
    VasnecovTexture *texture(nullptr);
    std::string newName;

    switch(type)
    {
        case Vasnecov::TextureTypeDiffuse:
            newName = raw_data.dirTexturesDPref + textureName;
            break;
        case Vasnecov::TextureTypeInterface:
            newName = raw_data.dirTexturesIPref + textureName;
            break;
        case Vasnecov::TextureTypeNormal:
            newName = raw_data.dirTexturesNPref + textureName;
            break;
        default: // Использовать путь без префиксов
            newName = textureName;
    }

    texture = designerFindTexture(newName);

    return texture;
}

/*!
 \brief

 \param color
*/
void VasnecovUniverse::setBackgroundColor(const QColor &color)
{
    m_backgroundColor = color;
}
/*!
 \brief

 \param rgb
*/
void VasnecovUniverse::setBackgroundColor(QRgb rgb)
{
    QColor color(rgb);
    setBackgroundColor(color);
}
/*!
 \brief

 \param dir
 \return GLboolean
*/
GLboolean VasnecovUniverse::setTexturesDir(const std::string &dir)
{
    return setDirectory(dir, raw_data.dirTextures);
}
/*!
 \brief

 \param dir
 \return GLboolean
*/
GLboolean VasnecovUniverse::setMeshesDir(const std::string &dir)
{
    return setDirectory(dir, raw_data.dirMeshes);
}

/*!
 \brief Загрузка всех ресурсов из всех соответствующих директорий.

 \note В зависимости от объема данных метод может выполняться длительное время. Поэтому имеет смысл
 вызывать его только на этапах начальной загрузки приложения.

*/
void VasnecovUniverse::loadAll()
{
    loadMeshes();
    loadTextures();
}

/*!
 \brief

 \param fileName
 \return GLboolean
*/
GLboolean VasnecovUniverse::loadMesh(const std::string &fileName)
{
    if(fileName.empty())
        return false;
    return loadMeshFile(fileName);
}
/*!
 \brief

 \param dirName
 \param withSub
 \return GLuint
*/
GLuint VasnecovUniverse::loadMeshes(const std::string &dirName, GLboolean withSub)
{
    LoadingStatus lStatus(&m_loading);
    GLuint res(0);

    res = handleFilesInDir(raw_data.dirMeshes, dirName, Vasnecov::cfg_meshFormat, &VasnecovUniverse::loadMeshFile, withSub);

    return res;
}

/*!
 \brief

 \param fileName
 \return GLboolean
*/
GLboolean VasnecovUniverse::loadTexture(const std::string &fileName)
{
    if(fileName.empty())
        return false;
    return loadTextureFile(fileName);
}
/*!
 \brief

 \param dirName
 \param withSub
 \return GLuint
*/
GLuint VasnecovUniverse::loadTextures(const std::string &dirName, GLboolean withSub)
{
    LoadingStatus lStatus(&m_loading);
    GLuint res(0);

    res  = handleFilesInDir(raw_data.dirTextures, raw_data.dirTexturesDPref + dirName, Vasnecov::cfg_textureFormat, &VasnecovUniverse::loadTextureFile, withSub);
    res += handleFilesInDir(raw_data.dirTextures, raw_data.dirTexturesIPref + dirName, Vasnecov::cfg_textureFormat, &VasnecovUniverse::loadTextureFile, withSub);
    res += handleFilesInDir(raw_data.dirTextures, raw_data.dirTexturesNPref + dirName, Vasnecov::cfg_textureFormat, &VasnecovUniverse::loadTextureFile, withSub);

    return res;
}

QString VasnecovUniverse::info(GLuint type)
{
    QString res;

    switch (type)
    {
        case GL_VERSION:
            res = m_techVersion;
            break;
        case GL_RENDERER:
            res = m_techRenderer;
            break;
#ifndef _MSC_VER
        case GL_SHADING_LANGUAGE_VERSION:
            m_techSL.update();
            res = m_techSL;
            break;
#endif
        case GL_EXTENSIONS:
            res = m_techExtensions;
            break;
        default:
            res = "OpenGL " + m_techVersion +
                  " at " + m_techRenderer +
                  " with " + m_techSL +
                  " and \n" + m_techExtensions;
            break;
    }

    return res;
}

/*!
 \brief

*/
void VasnecovUniverse::renderInitialize()
{
    // Инициализация состояний
    m_pipeline.initialize();

    glGetIntegerv(GL_MAX_LIGHTS, reinterpret_cast<GLint *>(&m_lampsCountMax));

    // Заполнение общих данных
    QString exts(reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS)));
    exts = exts.replace(" ", "\n");

    BMCL_INFO() << "OpenGL " << reinterpret_cast<const char *>(glGetString(GL_VERSION));
    m_techRenderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));
    m_techVersion = reinterpret_cast<const char *>(glGetString(GL_VERSION));
#ifndef _MSC_VER
    m_techSL.set(reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION)));
#endif
    m_techExtensions = exts;
}


VasnecovMesh *VasnecovUniverse::designerFindMesh(const std::string &name)
{
    if(raw_data.meshes.count(name))
    {
        return raw_data.meshes[name];
    }
    else
    {
        return nullptr;
    }
}

/*!
 \brief

 \param name
 \return VasnecovTexture
*/
VasnecovTexture *VasnecovUniverse::designerFindTexture(const std::string &name)
{
    if(raw_data.textures.count(name))
    {
        return raw_data.textures[name];
    }
    else
    {
        return nullptr;
    }
}

GLboolean VasnecovUniverse::designerRemoveThisAlienMatrix(const QMatrix4x4 *alienMs)
{
    GLboolean res = false;

    for(std::vector<VasnecovLamp *>::const_iterator lit = m_elements.rawLamps().begin();
        lit != m_elements.rawLamps().end(); ++lit)
    {
        res |= (*lit)->designerRemoveThisAlienMatrix(alienMs);
    }
    for(std::vector<VasnecovProduct *>::const_iterator pit = m_elements.rawProducts().begin();
        pit != m_elements.rawProducts().end(); ++pit)
    {
        res |= (*pit)->designerRemoveThisAlienMatrix(alienMs);
    }
    for(std::vector<VasnecovFigure *>::const_iterator lit = m_elements.rawFigures().begin();
        lit != m_elements.rawFigures().end(); ++lit)
    {
        res |= (*lit)->designerRemoveThisAlienMatrix(alienMs);
    }
    for(std::vector<VasnecovLabel *>::const_iterator lit = m_elements.rawLabels().begin();
        lit != m_elements.rawLabels().end(); ++lit)
    {
        res |= (*lit)->designerRemoveThisAlienMatrix(alienMs);
    }
    return res;
}

/*!
 \brief

 \param newDir
 \param oldDir
 \return GLboolean
*/
GLboolean VasnecovUniverse::setDirectory(const std::string &newDir, std::string &oldDir) const
{
    if(!newDir.empty())
    {
        QDir qdir(QString::fromStdString(newDir));
        if(qdir.exists())
        {
            std::string forNewDir = qdir.path().toStdString();
            forNewDir += "/"; // Для дальнейшей подстановки в адреса
            if(forNewDir != oldDir)
            {
                oldDir = forNewDir;
                return true;
            }
        }
    }
    return false;
}

/*!
 \brief

 \param dirPref
 \param targetDir
 \param format
 \param workFun
 \param withSub
 \return GLuint
*/
GLuint VasnecovUniverse::handleFilesInDir(const std::string &dirPref, const std::string &targetDir, const std::string &format, GLboolean (VasnecovUniverse::*workFun)(const std::string &), GLboolean withSub)
{
    GLuint res(0);

    QString qdirPref = QString::fromStdString(dirPref);
    QString qtargetDir = QString::fromStdString(targetDir);
    QString qformat = "." + QString::fromStdString(format);

    QDir dir(qdirPref + qtargetDir);

    QDirIterator::IteratorFlag flag(QDirIterator::NoIteratorFlags);
    if(withSub)
    {
        flag = QDirIterator::Subdirectories;
    }

    QDirIterator iterator(dir.path(), flag);

    while(iterator.hasNext())
    {
        iterator.next();
        if (!iterator.fileInfo().isDir())
        {
            QString fileName = iterator.fileName();

            if(fileName.endsWith(qformat))
            {
                QString fullFileName = iterator.filePath();
                fullFileName.remove(0, qdirPref.size());

                res += (this->*workFun)(fullFileName.toStdString());
            }
        }
    }
    return res;
}

/*!
 \brief

 \param fileName
 \return GLboolean
*/
GLboolean VasnecovUniverse::loadMeshFile(const std::string &fileName)
{
    std::string path = raw_data.dirMeshes + fileName; // Путь файла с расширением
    std::string fileId = fileName;

    if(correctPath(path, fileId, Vasnecov::cfg_meshFormat))
    {
        if(!raw_data.meshes.count(fileId))
        {
            VasnecovMesh *mesh = new VasnecovMesh(path, &m_pipeline, fileId);
            if(mesh->loadModel())
            {
                if(addMesh(mesh, fileId))
                {
                    return true;
                }
            }
            delete mesh;
            mesh = nullptr;
        }
    }
    return false;
}

/*!

 \brief

 \param fileName
 \return GLboolean
*/
GLboolean VasnecovUniverse::loadTextureFile(const std::string &fileName)
{
    // Поиск префикса типа текстуры в адресе
    Vasnecov::TextureTypes type(Vasnecov::TextureTypeUndefined);

    if(fileName.find(raw_data.dirTexturesDPref) == 0) // Первое вхождение, номер символа - ноль
    {
        type = Vasnecov::TextureTypeDiffuse;
    }
    else if(fileName.find(raw_data.dirTexturesIPref) == 0)
    {
        type = Vasnecov::TextureTypeInterface;
    }
    else if(fileName.find(raw_data.dirTexturesNPref) == 0)
    {
        type = Vasnecov::TextureTypeNormal;
    }

    std::string path = raw_data.dirTextures + fileName; // Путь файла с расширением

    std::string fileId = fileName;

    if(correctPath(path, fileId, Vasnecov::cfg_textureFormat))
    {
        if(!raw_data.textures.count(fileId)) // Данные
        {
            QImage image(QString::fromStdString(path));

            if(!image.isNull())
            {
                // Проверка на соотношение сторон (чудо-алгоритм от Мастана)
                if((image.width() & (image.width() - 1)) == 0 && (image.height() & (image.height() - 1)) == 0)
                {
                    VasnecovTexture *texture(nullptr);

                    switch(type)
                    {
                        case Vasnecov::TextureTypeDiffuse:
                            texture = new VasnecovTextureDiffuse(image);
                            break;
                        case Vasnecov::TextureTypeInterface:
                            texture = new VasnecovTextureInterface(image);
                            break;
                        case Vasnecov::TextureTypeNormal:
                            texture = new VasnecovTextureNormal(image);
                            break;
                        default:
                            Vasnecov::problem("Тип текстуры указан неверно: ", path);
                            return false;
                    }

                    if(addTexture(texture, fileId))
                    {
                        return true;
                    }

                    delete texture;
                    texture = nullptr;
                }
                else
                {
                    Vasnecov::problem("Текстура неверного размера: ", path);
                }
            }
        }
    }
    return false;
}

/*!
 \brief

 \param path
 \param fileId
 \param format
 \return GLboolean
*/
GLboolean VasnecovUniverse::correctPath(std::string &path, std::string &fileId, const std::string &format) const
{
    if(!format.empty())
    {
        size_t posR;
        posR = fileId.rfind("." + format);

        // Только файлы нужного расширения
        if(posR != std::string::npos &&
           (posR + 1 + format.size()) == fileId.size()) // Позиция + размер точки + размер расширения
        {
            // Отрезать расширение
            fileId = fileId.substr(0, posR);
        }
        else
        {
            path = path + "." + format;
        }
    }

    QFile file(QString::fromStdString(path));
    if(file.exists())
    {
        return true;
    }

    return false;
}

std::string VasnecovUniverse::correctFileId(const std::string &fileId, const std::string &format) const
{
    std::string res(fileId);
    if (format.empty())
        return res;

    size_t posR;
    posR = fileId.rfind("." + format);

    // Только файлы нужного расширения
    if(posR != std::string::npos && (posR + 1 + format.size()) == fileId.size()) // Позиция + размер точки + размер расширения
    {
        // Отрезать расширение
        res = fileId.substr(0, posR);
    }
    return res;
}

GLboolean VasnecovUniverse::addTexture(VasnecovTexture *texture, const std::string &fileId)
{
    if(texture)
    {
        GLboolean added = true;

        if(raw_data.textures.count(fileId))
        {
            added = false;
        }
        if(added)
        {
            raw_data.textures[fileId] = texture;
            raw_data.texturesForLoading.push_back(texture);
            raw_data.setUpdateFlag(Textures);
        }

        return added;
    }
    Vasnecov::problem("Неверная текстура либо дублирование данных");
    return false;
}

/*!
 \brief

 \param mesh
 \param fileId
 \return GLboolean
*/
GLboolean VasnecovUniverse::addMesh(VasnecovMesh *mesh, const std::string &fileId)
{
    if(mesh)
    {
        GLboolean added = true;

        if(raw_data.meshes.count(fileId))
        {
            added = false;
        }
        if(added)
        {
            raw_data.meshes[fileId] = mesh;
//			raw_data.meshesForLoading.push_back(texture);
//			raw_data.setUpdateFlag(Meshes);
        }

        return added;
    }
    Vasnecov::problem("Неверный меш либо дублирование данных");
    return false;
}


/*!
 \brief

 \return GLenum
*/
GLenum VasnecovUniverse::renderUpdateData()
{
    GLenum wasUpdated(0);

    // Обновление настроек
    if(raw_data.wasUpdated)
    {
        m_pipeline.setContext(m_context);
    }

    // Обновление содержимого списков
    wasUpdated |= m_elements.synchronizeAll();

    if(wasUpdated)
    {
        // Обновление индексов фонарей
        GLuint index = GL_LIGHT0;
        int i = 0;
        for(const auto& lit: m_elements.pureLamps())
        {
            lit->renderSetIndex(i++);
        }
    }

    if(raw_data.wasUpdated)
    {
        // Загрузка (догрузка) ресурсов
        // Всё это делается с захваченным мьютексом, поэтому при больших загрузках стоять будет всё
        if(raw_data.isUpdateFlag(Meshes))
        {
            if(!raw_data.meshesForLoading.empty())
            {
//				for(auto VasnecovMesh* mit: raw_data.meshesForLoading)
//				{

//				}
            }
        }
        if(raw_data.isUpdateFlag(Textures))
        {
            if(!raw_data.texturesForLoading.empty())
            {
                for(VasnecovTexture* tit: raw_data.texturesForLoading)
                {
                    if(!tit->loadImage())
                    {
                        // TODO: remove wrong textures from raw_data.textures and all objects
                    }
                }
                raw_data.texturesForLoading.clear();
                glBindTexture(GL_TEXTURE_2D, m_pipeline.m_texture2D); // Возврат текущей текстуры
            }
        }

        wasUpdated = true;
    }

    // Обновление данных элементов
    m_elements.forEachPureWorld(renderUpdateElementData<VasnecovWorld>);
    m_elements.forEachPureMaterial(renderUpdateElementData<VasnecovMaterial>);

    m_elements.forEachPureLamp(renderUpdateElementData<VasnecovLamp>);
    m_elements.forEachPureProduct(renderUpdateElementData<VasnecovProduct>);
    m_elements.forEachPureFigure(renderUpdateElementData<VasnecovFigure>);
    m_elements.forEachPureLabel(renderUpdateElementData<VasnecovLabel>);


    raw_data.wasUpdated = 0;

    if(m_pipeline.wasSomethingUpdated())
    {
        wasUpdated = true;
        m_pipeline.clearSomethingUpdates();
    }

    return wasUpdated;
}

/*!
 \brief

*/
void VasnecovUniverse::renderDrawLoadingImage()
{
    // Выводить сообщение о процессе загрузки
    if(Vasnecov::cfg_showLoadingImage)
    {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(0, m_width, 0, m_height); // Ортогональная проекция
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        QImage *image(0);

        timespec tm;
        GLint diff(0); // миллисекунды

#ifndef _MSC_VER
        clock_gettime(CLOCK_MONOTONIC, &tm);

        diff = (tm.tv_sec - m_loadingImageTimer.tv_sec)*1e3 + (tm.tv_nsec - m_loadingImageTimer.tv_nsec)*1e-6;
        if(diff > Vasnecov::cfg_loadingImagePause * 2)
        {
            clock_gettime(CLOCK_MONOTONIC, &m_loadingImageTimer);
            image = &m_loadingImage0;
        }
        else if(diff > Vasnecov::cfg_loadingImagePause)
        {
            image = &m_loadingImage1;
        }
        else
        {
            image = &m_loadingImage0;
        }
#endif
        if(image && !image->isNull())
        {
            glRasterPos2i(0.5f*m_width - image->width()*0.5f,
                          0.5f*m_height - image->height()*0.5f);
            glDrawPixels(image->width(), image->height(), GL_BGRA_EXT, GL_UNSIGNED_BYTE, image->bits());
        }
    }
}
/*!
 \brief

 \param width
 \param height
*/
void VasnecovUniverse::renderDrawAll(GLsizei width, GLsizei height)
{
    // Обновление данных
    renderUpdateData();

    {
        m_width = width;
        m_height = height;

        // Очистка экрана и т.п.
        m_pipeline.setBackgroundColor(m_backgroundColor);
        m_pipeline.clearAll();

        // Включение параметров
        m_pipeline.enableBlending(true);
        m_pipeline.enableDepth(true);


        // Прогонка миров по списку
        m_elements.forEachPureWorld(VasnecovWorld::renderDrawElement<VasnecovWorld>);

        // Возврат для отрисовки интерфейса
        m_pipeline.setDrawingType(Vasnecov::PolygonDrawingTypeNormal);
        m_pipeline.disableDepth(true); // Иначе Qt рисует коряво, почему-то
        m_pipeline.enableBackFaces(true);
        m_pipeline.enableTexture2D(0, true);
        m_pipeline.clearZBuffer();

        m_pipeline.setViewport(0, 0, m_width, m_height);
    }

    // Индикатор факта загрузки
    if(m_loading)
    {
        renderDrawLoadingImage();
    }
}

//--------------------------------------------------------------------------------------------------

Vasnecov::UniverseAttributes::~UniverseAttributes()
{
    for (auto& rit : meshes)
    {
        delete (rit.second);
        rit.second = nullptr;
    }
    for (auto& rit : textures)
    {
        delete (rit.second);
        rit.second = nullptr;
    }
}


#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
