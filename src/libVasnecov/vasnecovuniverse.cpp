/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "vasnecovuniverse.h"
#ifdef _MSC_VER
    #include <windows.h>
#endif
#include <GL/glu.h>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <bmcl/Logging.h>

#include "vasnecovfigure.h"
#include "vasnecovlabel.h"
#include "vasnecovlamp.h"
#include "vasnecovmesh.h"
#include "vasnecovproduct.h"
#include "vasnecovresourcemanager.h"
#include "vasnecovterrain.h"

VasnecovUniverse::VasnecovUniverse(VasnecovResourceManager* resourceManager, const QGLContext *context) :
    m_pipeline(),
    m_context(raw_data.wasUpdated, Context, context),
    m_backgroundColor(raw_data.wasUpdated, BackColor, QColor(0, 0, 0, 255)),

    m_width(Vasnecov::cfg_displayWidthDefault),
    m_height(Vasnecov::cfg_displayHeightDefault),

    m_loading(raw_data.wasUpdated, Loading, false),
    m_loadingImage0(),
    m_loadingImage1(),
    m_loadingImageTimer(Vasnecov::timeDefault()),
    m_lampsCountMax(Vasnecov::cfg_lampsCountMax),

    raw_data(),
    m_resourceManager(resourceManager),
    m_elements(),

    m_techRenderer(raw_data.wasUpdated, Tech01),
    m_techVersion(raw_data.wasUpdated, Tech02),
    m_techSL(raw_data.wasUpdated, Tech03),
    m_techExtensions(raw_data.wasUpdated, Tech04)
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

VasnecovUniverse::VasnecovUniverse(const QGLContext* context)
    : VasnecovUniverse(new VasnecovResourceManager(), context)
{}
VasnecovUniverse::~VasnecovUniverse()
{
    Q_CLEANUP_RESOURCE(vasnecov);
}
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
VasnecovLamp *VasnecovUniverse::addLamp(const QString& name, VasnecovWorld *world, Vasnecov::LampTypes type)
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
VasnecovProduct *VasnecovUniverse::addAssembly(const QString& name, VasnecovWorld *world, VasnecovProduct *parent)
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
VasnecovProduct *VasnecovUniverse::addPart(const QString& name, VasnecovWorld *world, const QString& meshName, VasnecovProduct *parent)
{
    return addPart(name, world, meshName, nullptr, parent);
}
VasnecovProduct *VasnecovUniverse::addPart(const QString& name, VasnecovWorld *world, const QString& meshName, VasnecovMaterial *material, VasnecovProduct *parent)
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
    if(!meshName.isEmpty())
    {
        QString corMeshName = VasnecovResourceManager::correctFileId(meshName, Vasnecov::cfg_meshFormat);
        // Поиск меша в списке
        mesh = m_resourceManager->designerFindMesh(corMeshName);

        if(!mesh)
        {
            // Попытка загрузить насильно
            // Метод загрузки сам управляет мьютексом
            if(!m_resourceManager->loadMeshFile(corMeshName))
            {
                Vasnecov::problem("Не найден заданный меш");
                return nullptr;
            }

            mesh = m_resourceManager->designerFindMesh(corMeshName);

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
VasnecovProduct *VasnecovUniverse::addPart(const QString& name, VasnecovWorld *world, const QString& meshName, const QString& textureName, VasnecovProduct *parent)
{
    if(!textureName.isEmpty())
    {
        return addPart(name, world, meshName, addMaterial(m_resourceManager->texturesDPref() + textureName), parent);
    }
    else
    {
        return addPart(name, world, meshName, nullptr, parent);
    }
}
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
        for(std::vector<VasnecovProduct *>::iterator dit = delProd.begin();
            dit != delProd.end(); ++dit)
        {
            if((*dit)->designerMaterial())
            {
                delMat.push_back((*dit)->designerMaterial());
            }
        }
        // Если материал встречается где-то в других продуктах, то из списка претендентов он вычеркивается
        for(std::vector<VasnecovProduct *>::const_iterator pit = m_elements.rawProducts().begin();
            pit != m_elements.rawProducts().end(); ++pit)
        {
            for(std::vector<VasnecovMaterial *>::iterator mit = delMat.begin();
                mit != delMat.end(); ++mit)
            {
                if((*pit)->designerMaterial() == (*mit))
                {
                    delMat.erase(mit);
                    break;
                }
            }
        }
        // Непосредственное удаление больше не нужных материалов
        m_elements.removeElements(delMat);

        // Прочие удаления
        for(std::vector<VasnecovProduct *>::iterator dit = delProd.begin();
            dit != delProd.end(); ++dit)
        {
            // Удаление из мира
            for(std::vector<VasnecovWorld *>::const_iterator wit = m_elements.rawWorlds().begin();
                wit != m_elements.rawWorlds().end(); ++wit)
            {
                (*wit)->designerRemoveElement((*dit));
            }

            // Удаление из списка родителей
            if((*dit)->designerParent())
            {
                (*dit)->designerParent()->designerRemoveChild(*dit);
            }

            // Удаление чужих матриц
            const QMatrix4x4 *matrix = (*dit)->designerExportingMatrix();
            designerRemoveThisAlienMatrix(matrix);
        }

        return true;
    }

    return false;
}

VasnecovFigure *VasnecovUniverse::addFigure(const QString& name, VasnecovWorld *world)
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
        for(std::vector<VasnecovWorld *>::const_iterator wit = m_elements.rawWorlds().begin();
            wit != m_elements.rawWorlds().end(); ++wit)
        {
            (*wit)->designerRemoveElement(figure);
        }

        // Удаление чужих матриц
        designerRemoveThisAlienMatrix(figure->designerExportingMatrix());

        return true;
    }

    return false;
}

VasnecovTerrain*VasnecovUniverse::addTerrain(const QString& name, VasnecovWorld* world)
{
    return nullptr;
}

GLboolean VasnecovUniverse::removeTerrain(VasnecovTerrain* terrain)
{
    return false;
}

VasnecovLabel *VasnecovUniverse::addLabel(const QString& name, VasnecovWorld *world, GLfloat width, GLfloat height)
{
    return addLabel(name, world, width, height, "");
}
VasnecovLabel *VasnecovUniverse::addLabel(const QString& name, VasnecovWorld *world, GLfloat width, GLfloat height, const QString& textureName)
{
    if(!world)
    {
        Vasnecov::problem("Мир не задан");
        return nullptr;
    }

    VasnecovLabel *label(nullptr);
    VasnecovTexture *texture(nullptr);

    // Проверка на наличие текстуры и её догрузка при необходимости
    if(!textureName.isEmpty()) // Иначе нулевая текстура
    {
        QString corTextureName = VasnecovResourceManager::correctFileId(textureName, Vasnecov::cfg_textureFormat);

        texture = m_resourceManager->designerFindTexture(m_resourceManager->texturesIPref() + corTextureName);

        if(!texture)
        {
            // Попытка загрузить насильно
            // Метод загрузки сам управляет мьютексом
            if(!m_resourceManager->loadTextureFile(m_resourceManager->texturesIPref() + corTextureName))
            {
                Vasnecov::problem("Не найдена заданная текстура");
                return nullptr;
            }

            texture = m_resourceManager->designerFindTexture(m_resourceManager->texturesIPref() + corTextureName);
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
GLboolean VasnecovUniverse::removeLabel(VasnecovLabel *label)
{
    if(!label)
        return false;

    if(m_elements.findRawElement(label))
    {
        m_elements.removeElement(label);

        // Прочие удаления
        // Удаление из мира
        for(std::vector<VasnecovWorld *>::const_iterator wit = m_elements.rawWorlds().begin();
            wit != m_elements.rawWorlds().end(); ++wit)
        {
            (*wit)->designerRemoveElement(label);
        }

        // Удаление чужих матриц
        designerRemoveThisAlienMatrix(label->designerExportingMatrix());

        return true;
    }

    return false;
}
VasnecovMaterial *VasnecovUniverse::addMaterial(const QString& textureName)
{
    VasnecovTexture *texture(nullptr);

    // Проверка на наличие текстуры и её догрузка при необходимости
    if(!textureName.isEmpty()) // Иначе нулевая текстура
    {
        // Поиск текстуры в списке
        QString corTextureName = VasnecovResourceManager::correctFileId(textureName, Vasnecov::cfg_textureFormat);

        texture = m_resourceManager->designerFindTexture(corTextureName);

        if(!texture)
        {
            // Попытка загрузить насильно
            // Метод загрузки сам управляет мьютексом
            if(!m_resourceManager->loadTextureFile(corTextureName))
            {
                Vasnecov::problem("Заданная текстура не найдена");
                return nullptr;
            }

            texture = m_resourceManager->designerFindTexture(corTextureName);

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
VasnecovTexture *VasnecovUniverse::textureByName(const QString& textureName, Vasnecov::TextureTypes type)
{
    VasnecovTexture *texture(nullptr);
    QString newName;

    switch(type)
    {
        case Vasnecov::TextureTypeDiffuse:
            newName = m_resourceManager->texturesDPref() + textureName;
            break;
        case Vasnecov::TextureTypeInterface:
            newName = m_resourceManager->texturesIPref() + textureName;
            break;
        case Vasnecov::TextureTypeNormal:
            newName = m_resourceManager->texturesNPref() + textureName;
            break;
        default: // Использовать путь без префиксов
            newName = textureName;
    }

    texture = m_resourceManager->designerFindTexture(newName);

    return texture;
}
void VasnecovUniverse::setBackgroundColor(const QColor &color)
{
    m_backgroundColor.set(color);
}
void VasnecovUniverse::setBackgroundColor(QRgb rgb)
{
    QColor color(rgb);
    setBackgroundColor(color);
}
GLboolean VasnecovUniverse::setTexturesDir(const QString& dir)
{
    return m_resourceManager->setTexturesDir(dir);
}
GLboolean VasnecovUniverse::setMeshesDir(const QString& dir)
{
    return m_resourceManager->setMeshesDir(dir);
}
void VasnecovUniverse::loadAll()
{
    loadMeshes();
    loadTextures();
}
GLboolean VasnecovUniverse::loadMesh(const QString& fileName)
{
    if(fileName.isEmpty())
        return false;

    LoadingStatus lStatus(&m_loading);
    GLboolean res(false);

    res = m_resourceManager->loadMeshFile(fileName);

    return res;
}
GLuint VasnecovUniverse::loadMeshes(const QString& dirName, GLboolean withSub)
{
    LoadingStatus lStatus(&m_loading);
    GLuint res(0);

    res = m_resourceManager->handleMeshesDir(dirName, withSub);

    return res;
}
GLboolean VasnecovUniverse::loadTexture(const QString& fileName)
{
    if(fileName.isEmpty())
        return false;

    LoadingStatus lStatus(&m_loading);
    GLboolean res(false);

    res = m_resourceManager->loadTextureFile(fileName);

    return res;
}
GLuint VasnecovUniverse::loadTextures(const QString& dirName, GLboolean withSub)
{
    LoadingStatus lStatus(&m_loading);
    GLuint res(0);

    res = m_resourceManager->handleTexturesDir(dirName, withSub);

    return res;
}

QString VasnecovUniverse::info(GLuint type)
{
    QString res;

    switch (type)
    {
        case GL_VERSION:
            m_techVersion.update();
            res = m_techVersion.pure();
            break;
        case GL_RENDERER:
            m_techRenderer.update();
            res = m_techRenderer.pure();
            break;
#ifndef _MSC_VER
        case GL_SHADING_LANGUAGE_VERSION:
            m_techSL.update();
            res = m_techSL.pure();
            break;
#endif
        case GL_EXTENSIONS:
            m_techExtensions.update();
            res = m_techExtensions.pure();
            break;
        default:
            m_techVersion.update();
            m_techRenderer.update();
            m_techSL.update();
            m_techExtensions.update();

            res = "OpenGL " + m_techVersion.pure() +
                  " at " + m_techRenderer.pure() +
                  " with " + m_techSL.pure() +
                  " and \n" + m_techExtensions.pure();
            break;
    }

    return res;
}
void VasnecovUniverse::renderInitialize()
{
    // Инициализация состояний
    m_pipeline.initialize();

    glGetIntegerv(GL_MAX_LIGHTS, reinterpret_cast<GLint *>(&m_lampsCountMax));

    // Заполнение общих данных
    QString exts(reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS)));
    exts = exts.replace(" ", "\n");

    BMCL_INFO() << "OpenGL " << reinterpret_cast<const char *>(glGetString(GL_VERSION));
    m_techRenderer.set(reinterpret_cast<const char *>(glGetString(GL_RENDERER)));
    m_techVersion.set(reinterpret_cast<const char *>(glGetString(GL_VERSION)));
#ifndef _MSC_VER
    m_techSL.set(reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION)));
#endif
    m_techExtensions.set(exts);
}

GLboolean VasnecovUniverse::designerRemoveThisAlienMatrix(const QMatrix4x4 *alienMs)
{
    GLboolean res(false);

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

GLenum VasnecovUniverse::renderUpdateData()
{
    GLenum wasUpdated(0);

    // Обновление настроек
    if(raw_data.wasUpdated)
    {
        if(m_context.update())
        {
            m_pipeline.setContext(m_context.pure());
        }

        m_loading.update();
        m_backgroundColor.update();
    }

    // Обновление содержимого списков
    wasUpdated |= m_elements.synchronizeAll();

    if(wasUpdated)
    {
        // Обновление индексов фонарей
        GLuint index(GL_LIGHT0);
        for(std::vector<VasnecovLamp *>::const_iterator lit = m_elements.pureLamps().begin();
            lit != m_elements.pureLamps().end(); ++lit, ++index)
        {
            (*lit)->renderSetIndex(index);
        }
    }

    if(m_resourceManager->renderUpdate())
    {
        glBindTexture(GL_TEXTURE_2D, m_pipeline.m_texture2D); // Возврат текущей текстуры
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
void VasnecovUniverse::renderDrawAll(GLsizei width, GLsizei height)
{
    // Обновление данных
    renderUpdateData();

    {
        m_width = width;
        m_height = height;

        // Очистка экрана и т.п.
        m_pipeline.setBackgroundColor(m_backgroundColor.pure());
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
    if(m_loading.pure())
    {
        renderDrawLoadingImage();
    }
}

VasnecovUniverse::UniverseElementList::UniverseElementList() :
    Vasnecov::ElementList<VasnecovUniverse::ElementFullBox>(),
    m_worlds(),
    m_materials()
{}
