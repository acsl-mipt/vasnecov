/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "VasnecovUniverse.h"
#ifdef _MSC_VER
    #include <windows.h>
#endif
#include <GL/glu.h>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <bmcl/Logging.h>

#include "VasnecovFigure.h"
#include "VasnecovLabel.h"
#include "VasnecovLamp.h"
#include "VasnecovMesh.h"
#include "VasnecovProduct.h"
#include "VasnecovResourceManager.h"
#include "VasnecovTerrain.h"
#include "VasnecovTerrain.h"

VasnecovUniverse::VasnecovUniverse(VasnecovResourceManager* resourceManager, const QGLContext *context) :
    _pipeline(),
    _context(raw_data.wasUpdated, Context, context),
    _backgroundColor(raw_data.wasUpdated, BackColor, QColor(0, 0, 0, 255)),

    _width(Vasnecov::cfg_displayWidthDefault),
    _height(Vasnecov::cfg_displayHeightDefault),

    _loading(raw_data.wasUpdated, Loading, false),
    _loadingImage0(),
    _loadingImage1(),
    _loadingImageTimer(Vasnecov::timeDefault()),
    _lampsCountMax(Vasnecov::cfg_lampsCountMax),

    raw_data(),
    _resourceManager(resourceManager),
    _elements(),

    _techRenderer(raw_data.wasUpdated, Tech01),
    _techVersion(raw_data.wasUpdated, Tech02),
    _techSL(raw_data.wasUpdated, Tech03),
    _techExtensions(raw_data.wasUpdated, Tech04)
{
    Q_INIT_RESOURCE(vasnecov);

    if(!_loadingImage0.load(":/share/loading0.png") ||
       !_loadingImage1.load(":/share/loading1.png"))
    {
        Vasnecov::problem("Can't load loading image");
    }
    if(!_loadingImage0.isNull())
    {
        _loadingImage0 = _loadingImage0.mirrored(0, 1);
    }
    if(!_loadingImage1.isNull())
    {
        _loadingImage1 = _loadingImage1.mirrored(0, 1);
    }
#ifndef _MSC_VER
    if(clock_gettime(CLOCK_MONOTONIC, &_loadingImageTimer) < 0)
    {
        Vasnecov::problem("Problems with loading image timer");
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
        VasnecovWorld *newWorld = new VasnecovWorld(&_pipeline, posX, posY, width, height);

        _elements.addElement(newWorld);

        return newWorld;
    }

    Vasnecov::problem("Incorrect world size");
    return nullptr;
}
VasnecovLamp *VasnecovUniverse::addLamp(const QString& name, VasnecovWorld *world, Vasnecov::LampTypes type)
{
    if(!world)
    {
        Vasnecov::problem("World is not set");
        return nullptr;
    }

    GLuint index(GL_LIGHT0);

    // Поиск мира в списке
    if(!_elements.findRawElement(world))
    {
        Vasnecov::problem("World is set incorrectly");
        return nullptr;
    }
    size_t count = _elements.rawLampsCount();
    if(count < _lampsCountMax)
    {
        // Индексы задаются тупо прибавлением. Если какие-то фонари удалялись, то индекс будет неверным.
        // Но это не страшно, т.к. при обновлении данных в updateData индексы будут переназначены
        index += count;
    }

    VasnecovLamp *lamp = new VasnecovLamp(&_pipeline, name, type, index);
    if(_elements.addElement(lamp))
    {
        world->designerAddElement(lamp);
        return lamp;
    }
    else
    {
        delete lamp;
        lamp = nullptr;

        Vasnecov::problem("Incorrect lamp or data duplication");
        return nullptr;
    }
}
VasnecovLamp *VasnecovUniverse::referLampToWorld(VasnecovLamp *lamp, VasnecovWorld *world)
{
    if(!lamp || !world)
    {
        Vasnecov::problem("Lamp or world is incorrect");
        return nullptr;
    }

    // Поиск фонаря в общем списке
    if(!_elements.findRawElement(lamp))
    {
        Vasnecov::problem("Lamp is not found");
        return nullptr;
    }
    // Поиск мира в списке
    if(!_elements.findRawElement(world))
    {
        Vasnecov::problem("Wrong world");
        return nullptr;
    }

    if(world->designerAddElement(lamp, true))
    {
        return lamp;
    }
    else
    {
        Vasnecov::problem("Incorrect lamp or data duplicating");
        return nullptr;
    }
}

GLboolean VasnecovUniverse::removeLamp(VasnecovLamp* lamp)
{
    return designerRemoveSimpleElement(lamp);
}
VasnecovProduct *VasnecovUniverse::addAssembly(const QString& name, VasnecovWorld *world, VasnecovProduct *parent)
{
    if(!world)
    {
        Vasnecov::problem("World is not set");
        return nullptr;
    }

    VasnecovProduct *assembly(nullptr);
    GLuint level(0);

    // Поиск мира в списке
    if(!_elements.findRawElement(world))
    {
        Vasnecov::problem("Wrong world");
        return nullptr;
    }

    if(parent)
    {
        if(world->designerFindElement(parent))
        {
            level = parent->designerLevel() + 1;
            if(level > Vasnecov::cfg_elementMaxLevel)
            {
                Vasnecov::problem("Maximum nesting level is exceeded");
                return nullptr;
            }
        }
        else
        {
            Vasnecov::problem("Parent node is not found");
            return nullptr;
        }
    }

    // world && (parent exists)
    assembly = new VasnecovProduct(&_pipeline, name, VasnecovProduct::ProductTypeAssembly, parent, level);

    if(parent)
    {
        assembly->designerSetMatrixM1(parent->designerMatrixMs());
        parent->designerAddChild(assembly);
    }
    _elements.addElement(assembly);
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
        Vasnecov::problem("World is not set");
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
        mesh = _resourceManager->designerFindMesh(corMeshName);

        if(mesh == nullptr)
            mesh = _resourceManager->designerFindMesh(meshName); // Full path

        if(mesh == nullptr)
        {
            // Попытка загрузить насильно
            // Метод загрузки сам управляет мьютексом
            if(!_resourceManager->loadMeshFile(corMeshName))
            {
                Vasnecov::problem("Mesh can't be loaded: ", corMeshName);
                return nullptr;
            }

            mesh = _resourceManager->designerFindMesh(corMeshName);

            if(!mesh)
            {
                // Условие невозможное после попытки загрузки, но для надёжности оставим :)
                Vasnecov::problem("Mesh is not found: ", corMeshName);
                return nullptr;
            }
        }
    }
    else
    {
        Vasnecov::problem("Mesh is not set");
        return nullptr;
    }

    // Поиск мира в списке
    if(!_elements.findRawElement(world))
    {
        Vasnecov::problem("Wrong world");
        return nullptr;
    }
    // Поиск материала
    if(material)
    {
        if(!_elements.findRawElement(material))
        {
            Vasnecov::problem("Material is not found");
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
                Vasnecov::problem("Maximum nesting level is exceeded");
                return nullptr;
            }
        }
        else
        {
            Vasnecov::problem("Parent node is not found");
            return nullptr;
        }
    }

    // world && mesh && (parent exists)
    part = new VasnecovProduct(&_pipeline, name, mesh, material, parent, level);

    if(parent)
    {
        part->designerSetMatrixM1(parent->designerMatrixMs());
        parent->designerAddChild(part);
    }
    _elements.addElement(part);
    world->designerAddElement(part); // Здесь не требуется проверка на дубликаты, т.к. указатель part девственно чист

    return part;
}
VasnecovProduct *VasnecovUniverse::addPart(const QString& name, VasnecovWorld *world, const QString& meshName, const QString& textureName, VasnecovProduct *parent)
{
    if(!textureName.isEmpty())
    {
        return addPart(name, world, meshName, addMaterial(textureName), parent);
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
        Vasnecov::problem("Element or world is not found");
        return nullptr;
    }

    // Поиск изделия в общем списке
    if(!_elements.findRawElement(product))
    {
        Vasnecov::problem("Product is not found");
        return nullptr;
    }
    // Поиск мира в списке
    if(!_elements.findRawElement(world))
    {
        Vasnecov::problem("Wrong world");
        return nullptr;
    }

    if(world->designerAddElement(product, true))
    {
        return product;
    }
    else
    {
        Vasnecov::problem("Incorrect product or data duplicating");
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

    if(_elements.findRawElement(product))
    {
        // Удаление продуктов - долгая операция с заблокированным мьютексом
        std::vector<VasnecovProduct *> delProd(product->designerAllChildren());
        delProd.push_back(product);

        _elements.removeElements(delProd);

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
        for(std::vector<VasnecovProduct *>::const_iterator pit = _elements.rawProducts().begin();
            pit != _elements.rawProducts().end(); ++pit)
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
        _elements.removeElements(delMat);

        // Прочие удаления
        for(std::vector<VasnecovProduct *>::iterator dit = delProd.begin();
            dit != delProd.end(); ++dit)
        {
            // Удаление из мира
            for(std::vector<VasnecovWorld *>::const_iterator wit = _elements.rawWorlds().begin();
                wit != _elements.rawWorlds().end(); ++wit)
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
        Vasnecov::problem("World is not set");
        return nullptr;
    }

    VasnecovFigure *figure(nullptr);

    // Поиск мира в списке
    if(!_elements.findRawElement(world))
    {
        Vasnecov::problem("Incorrect world");
        return nullptr;
    }

    figure = new VasnecovFigure(&_pipeline, name);

    if(_elements.addElement(figure))
    {
        world->designerAddElement(figure);
        return figure;
    }
    else
    {
        delete figure;
        figure = nullptr;

        Vasnecov::problem("Incorrect figure or data duplicating");
        return nullptr;
    }
}

GLboolean VasnecovUniverse::removeFigure(VasnecovFigure *figure)
{
    return designerRemoveSimpleElement(figure);
}

VasnecovTerrain*VasnecovUniverse::addTerrain(const QString& name, VasnecovWorld* world)
{
    if(world == nullptr)
    {
        Vasnecov::problem("World is not set");
        return nullptr;
    }

    // Поиск мира в списке
    if(!_elements.findRawElement(world))
    {
        Vasnecov::problem("Incorrect world");
        return nullptr;
    }

    VasnecovTerrain *terrain = new VasnecovTerrain(&_pipeline, name);

    if(_elements.addElement(terrain))
    {
        world->designerAddElement(terrain);
        return terrain;
    }
    else
    {
        delete terrain;
        Vasnecov::problem("Incorrect or duplicated terrain");
    }
    return nullptr;
}

GLboolean VasnecovUniverse::removeTerrain(VasnecovTerrain* terrain)
{
    return designerRemoveSimpleElement(terrain);
}

VasnecovLabel *VasnecovUniverse::addLabel(const QString& name, VasnecovWorld *world, GLfloat width, GLfloat height)
{
    return addLabel(name, world, width, height, QString());
}
VasnecovLabel *VasnecovUniverse::addLabel(const QString& name, VasnecovWorld *world, GLfloat width, GLfloat height, const QString& textureName)
{
    if(!world)
    {
        Vasnecov::problem("World is not set");
        return nullptr;
    }

    VasnecovLabel *label(nullptr);
    VasnecovTexture *texture(nullptr);

    // Проверка на наличие текстуры и её догрузка при необходимости
    if(!textureName.isEmpty()) // Иначе нулевая текстура
    {
        QString corTextureName = VasnecovResourceManager::correctFileId(_resourceManager->texturesIPref() + textureName, Vasnecov::cfg_textureFormat);

        texture = _resourceManager->designerFindTexture(corTextureName);

        if(texture == nullptr)
            texture = _resourceManager->designerFindTexture(textureName);

        if(!texture)
        {
            // Попытка загрузить насильно
            // Метод загрузки сам управляет мьютексом
            if(!_resourceManager->loadTextureFile(corTextureName))
            {
                Vasnecov::problem("Label texture is not found: ", corTextureName);
                return nullptr;
            }

            texture = _resourceManager->designerFindTexture(corTextureName);
            if(!texture)
            {
                // Условие невозможное после попытки загрузки, но для надёжности оставим :)
                Vasnecov::problem("Label texture is not found yet: ", textureName);
                return nullptr;
            }
        }
    }

    // Поиск мира в списке
    if(!_elements.findRawElement(world))
    {
        Vasnecov::problem("Wrong world");
        return nullptr;
    }

    label = new VasnecovLabel(&_pipeline, name, QVector2D(width, height), texture);

    if(_elements.addElement(label))
    {
        world->designerAddElement(label);
        return label;
    }
    else
    {
        delete label;
        label = nullptr;

        Vasnecov::problem("Incorrect label or data duplicating");
        return nullptr;
    }
}
VasnecovLabel *VasnecovUniverse::referLabelToWorld(VasnecovLabel *label, VasnecovWorld *world)
{
    if(!label || !world)
    {
        Vasnecov::problem("Element or world is not set");
        return nullptr;
    }

    // Поиск в общем списке
    if(!_elements.findRawElement(label))
    {
        Vasnecov::problem("Label is not found");
        return nullptr;
    }
    // Поиск мира в списке
    if(!_elements.findRawElement(world))
    {
        Vasnecov::problem("Wrong world");
        return nullptr;
    }

    if(world->designerAddElement(label, true))
    {
        return label;
    }
    else
    {
        Vasnecov::problem("Incorrect label or data duplicating");
        return nullptr;
    }
}
GLboolean VasnecovUniverse::removeLabel(VasnecovLabel *label)
{
    return designerRemoveSimpleElement(label);
}
VasnecovMaterial *VasnecovUniverse::addMaterial(const QString& textureName)
{
    VasnecovTexture *texture(nullptr);

    // Проверка на наличие текстуры и её догрузка при необходимости
    if(!textureName.isEmpty()) // Иначе нулевая текстура
    {
        // Поиск текстуры в списке
        QString corTextureName = VasnecovResourceManager::correctFileId(_resourceManager->texturesDPref() + textureName, Vasnecov::cfg_textureFormat);

        texture = _resourceManager->designerFindTexture(corTextureName);

        if(texture == nullptr)
            texture = _resourceManager->designerFindTexture(textureName);

        if(!texture)
        {
            // Попытка загрузить насильно
            // Метод загрузки сам управляет мьютексом
            if(!_resourceManager->loadTextureFile(corTextureName))
            {
                Vasnecov::problem("Material texture is not found: ", textureName);
                return nullptr;
            }

            texture = _resourceManager->designerFindTexture(corTextureName);

            if(!texture)
            {
                // Условие невозможное после попытки загрузки, но для надёжности оставим :)
                Vasnecov::problem("Material texture is not found yet: ", textureName);
                return nullptr;
            }
        }
    }

    VasnecovMaterial *material = new VasnecovMaterial(&_pipeline, texture);
    if(_elements.addElement(material))
    {
        return material;
    }
    else
    {
        delete material;
        material = nullptr;

        Vasnecov::problem("Incorrect material or data duplicating");
        return nullptr;
    }
}
VasnecovMaterial *VasnecovUniverse::addMaterial()
{
    VasnecovMaterial *material = new VasnecovMaterial(&_pipeline);
    if(_elements.addElement(material))
    {
        return material;
    }
    else
    {
        delete material;
        material = nullptr;

        Vasnecov::problem("Incorrect material or data duplicating");
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
            newName = _resourceManager->texturesDPref() + textureName;
            break;
        case Vasnecov::TextureTypeInterface:
            newName = _resourceManager->texturesIPref() + textureName;
            break;
        case Vasnecov::TextureTypeNormal:
            newName = _resourceManager->texturesNPref() + textureName;
            break;
        default: // Использовать путь без префиксов
            newName = textureName;
    }

    texture = _resourceManager->designerFindTexture(newName);

    return texture;
}
void VasnecovUniverse::setBackgroundColor(const QColor &color)
{
    _backgroundColor.set(color);
}
void VasnecovUniverse::setBackgroundColor(QRgb rgb)
{
    QColor color(rgb);
    setBackgroundColor(color);
}
GLboolean VasnecovUniverse::setTexturesDir(const QString& dir)
{
    return _resourceManager->setTexturesDir(dir);
}
GLboolean VasnecovUniverse::setMeshesDir(const QString& dir)
{
    return _resourceManager->setMeshesDir(dir);
}
void VasnecovUniverse::loadAll()
{
    loadMeshes();
    loadTextures();
}
GLboolean VasnecovUniverse::loadMesh(const QString& filePath)
{
    if(filePath.isEmpty())
        return false;

    LoadingStatus lStatus(&_loading);
    GLboolean res(false);

    res = _resourceManager->loadMeshFileByPath(filePath);

    return res;
}
GLuint VasnecovUniverse::loadMeshes(const QString& dirName, GLboolean withSub)
{
    LoadingStatus lStatus(&_loading);
    GLuint res(0);

    res = _resourceManager->handleMeshesDir(dirName, withSub);

    return res;
}
GLboolean VasnecovUniverse::loadTexture(const QString& filePath, Vasnecov::TextureTypes type)
{
    if(filePath.isEmpty())
        return false;

    LoadingStatus lStatus(&_loading);
    GLboolean res(false);

    res = _resourceManager->loadTextureFileByPath(filePath, type);

    return res;
}
GLuint VasnecovUniverse::loadTextures(const QString& dirName, GLboolean withSub)
{
    LoadingStatus lStatus(&_loading);
    GLuint res(0);

    res = _resourceManager->handleTexturesDir(dirName, withSub);

    return res;
}

QString VasnecovUniverse::info(GLuint type)
{
    QString res;

    switch (type)
    {
        case GL_VERSION:
            _techVersion.update();
            res = _techVersion.pure();
            break;
        case GL_RENDERER:
            _techRenderer.update();
            res = _techRenderer.pure();
            break;
#ifndef _MSC_VER
        case GL_SHADING_LANGUAGE_VERSION:
            _techSL.update();
            res = _techSL.pure();
            break;
#endif
        case GL_EXTENSIONS:
            _techExtensions.update();
            res = _techExtensions.pure();
            break;
        default:
            _techVersion.update();
            _techRenderer.update();
            _techSL.update();
            _techExtensions.update();

            res = "OpenGL " + _techVersion.pure() +
                  " at " + _techRenderer.pure() +
                  " with " + _techSL.pure() +
                  " and \n" + _techExtensions.pure();
            break;
    }

    return res;
}
void VasnecovUniverse::renderInitialize()
{
    // Инициализация состояний
    _pipeline.initialize();

    glGetIntegerv(GL_MAX_LIGHTS, reinterpret_cast<GLint *>(&_lampsCountMax));

    // Заполнение общих данных
    QString exts(reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS)));
    exts = exts.replace(" ", "\n");

    BMCL_INFO() << "OpenGL " << reinterpret_cast<const char *>(glGetString(GL_VERSION));
    _techRenderer.set(reinterpret_cast<const char *>(glGetString(GL_RENDERER)));
    _techVersion.set(reinterpret_cast<const char *>(glGetString(GL_VERSION)));
#ifndef _MSC_VER
    _techSL.set(reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION)));
#endif
    _techExtensions.set(exts);
}

GLboolean VasnecovUniverse::designerRemoveThisAlienMatrix(const QMatrix4x4 *alienMs)
{
    GLboolean res(false);

    for(const auto element : _elements.rawLamps())
        res |= element->designerRemoveThisAlienMatrix(alienMs);

    for(const auto element : _elements.rawProducts())
        res |= element->designerRemoveThisAlienMatrix(alienMs);

    for(const auto element : _elements.rawFigures())
        res |= element->designerRemoveThisAlienMatrix(alienMs);

    for(const auto element : _elements.rawTerrains())
        res |= element->designerRemoveThisAlienMatrix(alienMs);

    for(const auto element : _elements.rawLabels())
        res |= element->designerRemoveThisAlienMatrix(alienMs);

    return res;
}

GLenum VasnecovUniverse::renderUpdateData()
{
    GLenum wasUpdated(0);

    // Обновление настроек
    if(raw_data.wasUpdated)
    {
        if(_context.update())
        {
            _pipeline.setContext(_context.pure());
        }

        _loading.update();
        _backgroundColor.update();
    }

    // Обновление содержимого списков
    wasUpdated |= _elements.synchronizeAll();

    if(wasUpdated)
    {
        // Обновление индексов фонарей
        GLuint index(GL_LIGHT0);
        for(std::vector<VasnecovLamp *>::const_iterator lit = _elements.pureLamps().begin();
            lit != _elements.pureLamps().end(); ++lit, ++index)
        {
            (*lit)->renderSetIndex(index);
        }
    }

    if(_resourceManager->renderUpdate())
    {
        glBindTexture(GL_TEXTURE_2D, _pipeline.m_texture2D); // Возврат текущей текстуры
        wasUpdated = true;
    }

    // Обновление данных элементов
    _elements.forEachPureWorld(renderUpdateElementData<VasnecovWorld>);
    _elements.forEachPureMaterial(renderUpdateElementData<VasnecovMaterial>);

    _elements.forEachPureLamp(renderUpdateElementData<VasnecovLamp>);
    _elements.forEachPureProduct(renderUpdateElementData<VasnecovProduct>);
    _elements.forEachPureFigure(renderUpdateElementData<VasnecovFigure>);
    _elements.forEachPureTerrain(renderUpdateElementData<VasnecovTerrain>);
    _elements.forEachPureLabel(renderUpdateElementData<VasnecovLabel>);


    raw_data.wasUpdated = 0;

    if(_pipeline.wasSomethingUpdated())
    {
        wasUpdated = true;
        _pipeline.clearSomethingUpdates();
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
        gluOrtho2D(0, _width, 0, _height); // Ортогональная проекция
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        QImage *image(0);

        timespec tm;
        GLint diff(0); // миллисекунды

#ifndef _MSC_VER
        clock_gettime(CLOCK_MONOTONIC, &tm);

        diff = (tm.tv_sec - _loadingImageTimer.tv_sec)*1e3 + (tm.tv_nsec - _loadingImageTimer.tv_nsec)*1e-6;
        if(diff > Vasnecov::cfg_loadingImagePause * 2)
        {
            clock_gettime(CLOCK_MONOTONIC, &_loadingImageTimer);
            image = &_loadingImage0;
        }
        else if(diff > Vasnecov::cfg_loadingImagePause)
        {
            image = &_loadingImage1;
        }
        else
        {
            image = &_loadingImage0;
        }
#endif
        if(image && !image->isNull())
        {
            glRasterPos2i(0.5f*_width - image->width()*0.5f,
                          0.5f*_height - image->height()*0.5f);
            glDrawPixels(image->width(), image->height(), GL_BGRA_EXT, GL_UNSIGNED_BYTE, image->bits());
        }
    }
}
void VasnecovUniverse::renderDrawAll(GLsizei width, GLsizei height)
{
    // Обновление данных
    renderUpdateData();

    {
        _width = width;
        _height = height;

        // Очистка экрана и т.п.
        _pipeline.setBackgroundColor(_backgroundColor.pure());
        _pipeline.clearAll();

        // Включение параметров
        _pipeline.enableBlending(true);
        _pipeline.enableDepth(true);


        // Прогонка миров по списку
        _elements.forEachPureWorld(VasnecovWorld::renderDrawElement<VasnecovWorld>);

        // Возврат для отрисовки интерфейса
        _pipeline.setDrawingType(Vasnecov::PolygonDrawingTypeNormal);
        _pipeline.disableDepth(true); // Иначе Qt рисует коряво, почему-то
        _pipeline.enableBackFaces(true);
        _pipeline.enableTexture2D(0, true);
        _pipeline.clearZBuffer();

        _pipeline.setViewport(0, 0, _width, _height);
    }

    // Индикатор факта загрузки
    if(_loading.pure())
    {
        renderDrawLoadingImage();
    }
}

VasnecovUniverse::UniverseElementList::UniverseElementList() :
    Vasnecov::ElementList<VasnecovUniverse::ElementFullBox>(),
    _worlds(),
    _materials()
{}
