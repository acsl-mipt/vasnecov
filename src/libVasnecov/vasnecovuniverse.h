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
#include "configuration.h"
#include "vasnecovmaterial.h"
#include "vasnecovfigure.h"
#include "vasnecovworld.h"
#include "vasnecovproduct.h"
#include "vasnecovlabel.h"
#include "elementlist.h"

class VasnecovMaterial;
class VasnecovResourceManager;

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

        VasnecovWorld* findRawElement(VasnecovWorld* world) const {return m_worlds.findElement(world);}
        VasnecovMaterial* findRawElement(VasnecovMaterial* material) const {return m_materials.findElement(material);}
        using Vasnecov::ElementList<ElementFullBox>::findRawElement;

        GLboolean addElement(VasnecovWorld* world, GLboolean check = false) {return m_worlds.addElement(world, check);}
        GLboolean addElement(VasnecovMaterial* material, GLboolean check = false) {return m_materials.addElement(material, check);}
        using Vasnecov::ElementList<ElementFullBox>::addElement;

        GLboolean removeElement(VasnecovWorld* world) {return m_worlds.removeElement(world);}
        GLboolean removeElement(VasnecovMaterial* material) {return m_materials.removeElement(material);}
        using Vasnecov::ElementList<ElementFullBox>::removeElement;

        GLuint removeElements(const std::vector<VasnecovWorld*>& deletingList) {return m_worlds.removeElements(deletingList);}
        GLuint removeElements(const std::vector<VasnecovMaterial*>& deletingList) {return m_materials.removeElements(deletingList);}
        using Vasnecov::ElementList<ElementFullBox>::removeElements;

        GLboolean synchronizeWorlds() {return m_worlds.synchronize();}
        GLboolean synchronizeMaterials() {return m_materials.synchronize();}

        const std::vector<VasnecovWorld*>& rawWorlds() const {return m_worlds.raw();}
        const std::vector<VasnecovMaterial*>& rawMaterials() const {return m_materials.raw();}

        const std::vector<VasnecovWorld*>& pureWorlds() const {return m_worlds.pure();}
        const std::vector<VasnecovMaterial*>& pureMaterials() const {return m_materials.pure();}

        GLboolean hasPureWorlds() const {return m_worlds.hasPure();}
        GLboolean hasPureMaterials() const {return m_materials.hasPure();}

        GLuint rawWorldsCount() const {return m_worlds.rawCount();}
        GLuint rawMaterialsCount() const {return m_materials.rawCount();}

        template <typename F>
        void forEachPureWorld(F fun) const {m_worlds.forEachPure(fun);}
        template <typename F>
        void forEachPureMaterial(F fun) const {m_materials.forEachPure(fun);}

        virtual GLboolean synchronizeAll()
        {
            GLboolean res(false);

            res |= m_worlds.synchronize();
            res |= m_materials.synchronize();
            res |= Vasnecov::ElementList<ElementFullBox>::synchronizeAll();

            return res;
        }

        // Работа со списками удаления
        const std::vector<VasnecovWorld*>& deletingWorlds() const {return m_worlds.deleting();}
        const std::vector<VasnecovMaterial*>& deletingMaterials() const {return m_materials.deleting();}
        const std::vector<VasnecovLamp*>& deletingLamps() const {return m_lamps.deleting();}
        const std::vector<VasnecovProduct*>& deletingProduct() const {return m_products.deleting();}
        const std::vector<VasnecovFigure*>& deletingFigure() const {return m_figures.deleting();}
        const std::vector<VasnecovLabel*>& deletingLabel() const {return m_labels.deleting();}

    protected:
        ElementFullBox<VasnecovWorld> m_worlds;
        ElementFullBox<VasnecovMaterial> m_materials;
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
                          VasnecovLamp::LampTypes type = VasnecovLamp::LampTypeCelestial);
    VasnecovLamp* referLampToWorld(VasnecovLamp* lamp, VasnecovWorld* world);

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

    GLboolean loadMesh(const QString& fileName); // Загрузка конкретного меша
    GLuint loadMeshes(const QString& dirName = "", GLboolean withSub = true); // Загрузка всех мешей
    GLboolean loadTexture(const QString& fileName);
    GLuint loadTextures(const QString& dirName = "", GLboolean withSub = true); // Загрузка всех текстур

    QString info(GLuint type = 0);

protected:
    // Методы, вызываемые из внешних потоков (работают с сырыми данными)
    GLboolean designerRemoveThisAlienMatrix(const QMatrix4x4* alienMs);

protected:
    GLenum renderUpdateData(); // Единственный метод, который лочит мьютекс из основного потока (потока отрисовки)

protected:
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
    VasnecovPipeline m_pipeline;
    Vasnecov::MutualData<const QGLContext*> m_context;
    Vasnecov::MutualData<QColor> m_backgroundColor;

    GLsizei m_width, m_height; // Размеры окна вывода

    // Картинка для индикации загрузки
    Vasnecov::MutualData<GLboolean> m_loading;
    QImage m_loadingImage0, m_loadingImage1;
    timespec m_loadingImageTimer;

    GLuint m_lampsCountMax;

    // Списки миров
    // Списки общих (между мирами) данных
    Vasnecov::Attributes                raw_data;
    bmcl::Rc<VasnecovResourceManager>   m_resourceManager;
    UniverseElementList                 m_elements;

    enum Updated
    {
        Worlds			= 0x0000004,
        Materials		= 0x0000008,
        Products		= 0x0000010,
        Figures			= 0x0000020,
        Labels			= 0x0000040,
        Lamps			= 0x0000080,
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
    Vasnecov::MutualData<QString> m_techRenderer;
    Vasnecov::MutualData<QString> m_techVersion;
    Vasnecov::MutualData<QString> m_techSL;
    Vasnecov::MutualData<QString> m_techExtensions;

    friend class VasnecovScene;
    friend class VasnecovWidget;

private:
    Q_DISABLE_COPY(VasnecovUniverse)
};

inline void VasnecovUniverse::setContext(const QGLContext *context)
{
    if(!context)
        return;

    m_context.set(context);
}

//--------------------------------------------------------------------------------------------------
template <typename T>
VasnecovUniverse::ElementFullBox<T>::ElementFullBox() :
    m_deleting()
{}
template <typename T>
VasnecovUniverse::ElementFullBox<T>::~ElementFullBox()
{
    for(typename std::vector<T *>::iterator eit = this->m_raw.begin();
        eit != this->m_raw.end(); ++eit)
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
    if(this->m_wasUpdated)
    {
        this->m_pure.swap(this->m_buffer);
        this->m_wasUpdated = false;

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
        for(typename std::vector<T *>::iterator eit = this->m_raw.begin();
            eit != this->m_raw.end(); ++eit)
        {
            if((*eit) == element)
            {
                m_deleting.push_back(*eit);

                this->m_raw.erase(eit);
                this->m_buffer = this->m_raw;

                this->m_wasUpdated = true;
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
