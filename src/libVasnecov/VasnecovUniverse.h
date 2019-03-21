/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Класс описания всех элементов, которые можно отобразить.
// Очень условно, аналог Контроллера в модели MVC (ну, или как - расширение Модели).
// Содержит основные массивы данных для отрисовки (меши, списки текстур, миры).
#pragma once

#include <QString>
#include <QImage>
#include <bmcl/Rc.h>
#include <map>
#include "Configuration.h"
#include "VasnecovMaterial.h"
#include "VasnecovWorld.h"
#include "ElementList.h"

class VasnecovFigure;
class VasnecovLamp;
class VasnecovProduct;
class VasnecovResourceManager;
class VasnecovTerrain;
class VasnecovLabel;

class VasnecovUniverse
{
    // Управление индикатором загрузки
    class LoadingStatus
    {
    public:
        LoadingStatus(Vasnecov::MutualData<GLboolean>* loading) :
            m_loading(loading)
        {
            m_loading->set(true);
        }
        ~LoadingStatus()
        {
            m_loading->set(false);
        }
    private:
        Vasnecov::MutualData<GLboolean>* m_loading;

        Q_DISABLE_COPY(LoadingStatus)
    };

    // Шаблон контейнера списков с возможностью удаления
    template <typename T>
    class ElementFullBox : public Vasnecov::ElementBox<T>
    {
    public:
        ElementFullBox();
        ~ElementFullBox();

        virtual GLboolean synchronize();
        GLboolean removeElement(T* element);
        const std::vector<T*>& deleting() const;

    protected:
        std::vector<T*> m_deleting;
    };

    // Список контейнеров списков, расширенный
    class UniverseElementList : public Vasnecov::ElementList<ElementFullBox>
    {
    public:
        UniverseElementList();

        VasnecovWorld* findRawElement(VasnecovWorld* world) const {return _worlds.findElement(world);}
        VasnecovMaterial* findRawElement(VasnecovMaterial* material) const {return _materials.findElement(material);}
        using Vasnecov::ElementList<ElementFullBox>::findRawElement;

        GLboolean addElement(VasnecovWorld* world, GLboolean check = false) {return _worlds.addElement(world, check);}
        GLboolean addElement(VasnecovMaterial* material, GLboolean check = false) {return _materials.addElement(material, check);}
        using Vasnecov::ElementList<ElementFullBox>::addElement;

        GLboolean removeElement(VasnecovWorld* world) {return _worlds.removeElement(world);}
        GLboolean removeElement(VasnecovMaterial* material) {return _materials.removeElement(material);}
        using Vasnecov::ElementList<ElementFullBox>::removeElement;

        GLuint removeElements(const std::vector<VasnecovWorld*>& deletingList) {return _worlds.removeElements(deletingList);}
        GLuint removeElements(const std::vector<VasnecovMaterial*>& deletingList) {return _materials.removeElements(deletingList);}
        using Vasnecov::ElementList<ElementFullBox>::removeElements;

        GLboolean synchronizeWorlds() {return _worlds.synchronize();}
        GLboolean synchronizeMaterials() {return _materials.synchronize();}

        const std::vector<VasnecovWorld*>& rawWorlds() const {return _worlds.raw();}
        const std::vector<VasnecovMaterial*>& rawMaterials() const {return _materials.raw();}

        const std::vector<VasnecovWorld*>& pureWorlds() const {return _worlds.pure();}
        const std::vector<VasnecovMaterial*>& pureMaterials() const {return _materials.pure();}

        GLboolean hasPureWorlds() const {return _worlds.hasPure();}
        GLboolean hasPureMaterials() const {return _materials.hasPure();}

        GLuint rawWorldsCount() const {return _worlds.rawCount();}
        GLuint rawMaterialsCount() const {return _materials.rawCount();}

        template <typename F>
        void forEachPureWorld(F fun) const {_worlds.forEachPure(fun);}
        template <typename F>
        void forEachPureMaterial(F fun) const {_materials.forEachPure(fun);}

        virtual GLboolean synchronizeAll()
        {
            GLboolean res(false);

            res |= _worlds.synchronize();
            res |= _materials.synchronize();
            res |= Vasnecov::ElementList<ElementFullBox>::synchronizeAll();

            return res;
        }

        // Работа со списками удаления
        const std::vector<VasnecovWorld*>& deletingWorlds() const       {return _worlds.deleting();}
        const std::vector<VasnecovMaterial*>& deletingMaterials() const {return _materials.deleting();}
        const std::vector<VasnecovLamp*>& deletingLamps() const         {return _lamps.deleting();}
        const std::vector<VasnecovProduct*>& deletingProduct() const    {return _products.deleting();}
        const std::vector<VasnecovFigure*>& deletingFigure() const      {return _figures.deleting();}
        const std::vector<VasnecovTerrain*>& deletingTerrain() const    {return _terrains.deleting();}
        const std::vector<VasnecovLabel*>& deletingLabel() const        {return _labels.deleting();}

    protected:
        ElementFullBox<VasnecovWorld> _worlds;
        ElementFullBox<VasnecovMaterial> _materials;
    };


public:
    explicit VasnecovUniverse(VasnecovResourceManager* resourceManager, const QGLContext* context = nullptr);
    explicit VasnecovUniverse(const QGLContext* context = nullptr);
    ~VasnecovUniverse();

public:
    // Методы добавления блокируют мьютекс на все время исполнения
    // Но это не критично, т.к. поток отрисовки использует tryLock и просто не обновляет данные, рисуя старые
    VasnecovWorld* addWorld(GLint posX, GLint posY, GLsizei width, GLsizei height);

    VasnecovLamp* addLamp(const QString& name,
                          VasnecovWorld* world,
                          Vasnecov::LampTypes type = Vasnecov::LampTypeCelestial);
    VasnecovLamp* referLampToWorld(VasnecovLamp* lamp, VasnecovWorld* world);
    GLboolean removeLamp(VasnecovLamp* lamp);

    // Добавление новых продуктов
    VasnecovProduct* addAssembly(const QString& name,
                                  VasnecovWorld* world,
                                  VasnecovProduct* parent = nullptr);

    VasnecovProduct* addPart(const QString& name,
                              VasnecovWorld* world,
                              const QString& meshName,
                              VasnecovProduct* parent = nullptr); // Материал по умолчанию

    VasnecovProduct* addPart(const QString& name,
                              VasnecovWorld* world,
                              const QString& meshName,
                              VasnecovMaterial* material,
                              VasnecovProduct* parent = nullptr);

    VasnecovProduct* addPart(const QString& name,
                              VasnecovWorld* world,
                              const QString& meshName,
                              const QString& textureName,
                              VasnecovProduct* parent = nullptr); // Материал по умолчанию с указанной текстурой
    VasnecovProduct* referProductToWorld(VasnecovProduct* product, VasnecovWorld* world); // Сделать дубликат изделия в заданный мир
    GLboolean removeProduct(VasnecovProduct* product);

    VasnecovFigure* addFigure(const QString& name,
                              VasnecovWorld* world);
    GLboolean removeFigure(VasnecovFigure* figure);

    VasnecovTerrain* addTerrain(const QString& name,
                                VasnecovWorld* world);
    GLboolean removeTerrain(VasnecovTerrain* terrain);

    VasnecovLabel* addLabel(const QString& name,
                            VasnecovWorld* world,
                            GLfloat width,
                            GLfloat height);
    VasnecovLabel* addLabel(const QString& name,
                            VasnecovWorld* world,
                            GLfloat width,
                            GLfloat height,
                            const QString& textureName);
    VasnecovLabel* referLabelToWorld(VasnecovLabel* label, VasnecovWorld* world);
    GLboolean removeLabel(VasnecovLabel* label);

    // Добавление материала
    VasnecovMaterial* addMaterial(const QString& textureName);
    VasnecovMaterial* addMaterial();
    VasnecovTexture* textureByName(const QString& textureName, Vasnecov::TextureTypes type = Vasnecov::TextureTypeDiffuse);


    // Настройки рендеринга
    void setContext(const QGLContext* context);
    void setBackgroundColor(const QColor& color);
    void setBackgroundColor(QRgb rgb);

    // Загрузка ресурсов
    /*
     * Загрузка ресурсов происходит из директории ресурсов,
     * [pathToApp]/stuff/ - по умолчанию.
     * Имена ресурсов соответствуют адресу файла из этой директории.
     */
    // TODO: Unloading resources with full cleaning Worlds's content
    GLboolean setTexturesDir(const QString& dir);
    GLboolean setMeshesDir(const QString& dir);

    void loadAll(); // Загрузка всех ресурсов из своих директорий

    GLboolean loadMesh(const QString& filePath); // Загрузка конкретного меша
    GLuint loadMeshes(const QString& dirName = "", GLboolean withSub = true); // Загрузка всех мешей
    GLboolean loadTexture(const QString& filePath, Vasnecov::TextureTypes type = Vasnecov::TextureTypeDiffuse);
    GLuint loadTextures(const QString& dirName = "", GLboolean withSub = true); // Загрузка всех текстур

    QString info(GLuint type = 0);

private:
    // Методы, вызываемые из внешних потоков (работают с сырыми данными)
    GLboolean designerRemoveThisAlienMatrix(const QMatrix4x4* alienMs);
    template <typename T>
    GLboolean designerRemoveSimpleElement(T* element);

private:
    GLenum renderUpdateData(); // Единственный метод, который лочит мьютекс из основного потока (потока отрисовки)

private:
    // Базовая инициализация и циклическая отрисовка
    void renderInitialize();
    void renderDrawAll(GLsizei width, GLsizei height);
    void renderDrawLoadingImage();

    template <typename T>
    static void renderUpdateElementData(T* element)
    {
        if(element != nullptr)
            element->renderUpdateData();
    }

private:
    VasnecovPipeline                        _pipeline;
    Vasnecov::MutualData<const QGLContext*> _context;
    Vasnecov::MutualData<QColor>            _backgroundColor;
    // Размеры окна вывода
    GLsizei                                 _width;
    GLsizei                                 _height;
    // Картинка для индикации загрузки
    Vasnecov::MutualData<GLboolean>         _loading;
    QImage                                  _loadingImage0, _loadingImage1;
    timespec                                _loadingImageTimer;

    GLuint                                  _lampsCountMax;

    // Списки миров
    // Списки общих (между мирами) данных
    Vasnecov::Attributes                    raw_data;
    bmcl::Rc<VasnecovResourceManager>       _resourceManager;
    UniverseElementList                     _elements;

    enum Updated
    {
        Worlds			= 0x0000004,
        Materials		= 0x0000008,
        Products		= 0x0000010,
        Figures			= 0x0000020,
        Labels			= 0x0000040,
        Lamps			= 0x0000080,
        Terrains		= 0x0000100,
        Flags			= 0x0001000,
        Loading			= 0x0002000,
        BackColor		= 0x0004000,

        Context			= 0x0080000,
        Tech01			= 0x0100000,
        Tech02			= 0x0200000,
        Tech03			= 0x0400000,
        Tech04			= 0x0800000
    };

    // Технологическая информация (raw - из потока ренедеринга, pure - во внешнем)
    Vasnecov::MutualData<QString>           _techRenderer;
    Vasnecov::MutualData<QString>           _techVersion;
    Vasnecov::MutualData<QString>           _techSL;
    Vasnecov::MutualData<QString>           _techExtensions;

    friend class VasnecovScene;
    friend class VasnecovWidget;

private:
    Q_DISABLE_COPY(VasnecovUniverse)
};

inline void VasnecovUniverse::setContext(const QGLContext *context)
{
    if(!context)
        return;

    _context.set(context);
}

template <typename T>
VasnecovUniverse::ElementFullBox<T>::ElementFullBox() :
    m_deleting()
{}
template <typename T>
VasnecovUniverse::ElementFullBox<T>::~ElementFullBox()
{
    for(typename std::vector<T *>::iterator eit = this->_raw.begin();
        eit != this->_raw.end(); ++eit)
    {
        delete (*eit);
        (*eit) = 0;
    }

    for(typename std::vector<T *>::iterator eit = this->m_deleting.begin();
        eit != this->m_deleting.end(); ++eit)
    {
        delete (*eit);
        (*eit) = 0;
    }
}
template <typename T>
GLboolean VasnecovUniverse::ElementFullBox<T>::synchronize()
{
    if(this->_wasUpdated)
    {
        this->_pure.swap(this->_buffer);
        this->_wasUpdated = false;

        if(!m_deleting.empty())
        {
            for(typename std::vector<T *>::iterator eit = m_deleting.begin();
                eit != m_deleting.end(); ++eit)
            {
                delete (*eit);
                (*eit) = nullptr;
            }
            m_deleting.clear();
        }
        return true;
    }
    return false;
}

template <typename T>
GLboolean VasnecovUniverse::ElementFullBox<T>::removeElement(T *element)
{
    if(element)
    {
        for(typename std::vector<T *>::iterator eit = this->_raw.begin();
            eit != this->_raw.end(); ++eit)
        {
            if((*eit) == element)
            {
                m_deleting.push_back(*eit);

                this->_raw.erase(eit);
                this->_buffer = this->_raw;

                this->_wasUpdated = true;
                return true;
            }
        }
    }

    return false;
}

template <typename T>
const std::vector<T *> &VasnecovUniverse::ElementFullBox<T>::deleting() const
{
    return m_deleting;
}

template<typename T>
GLboolean VasnecovUniverse::designerRemoveSimpleElement(T* element)
{
    if(!element)
        return false;

    if(_elements.findRawElement(element))
    {
        _elements.removeElement(element);

        // Прочие удаления
        // Удаление из мира
        for(const auto world : _elements.rawWorlds())
            world->designerRemoveElement(element);

        // Удаление чужих матриц
        designerRemoveThisAlienMatrix(element->designerExportingMatrix());

        return true;
    }

    return false;
}
