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
#ifndef VASNECOV_UNIVERSE_H
#define VASNECOV_UNIVERSE_H

#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
#include <QString>
#include <QImage>
#include <map>
#include "configuration.h"
#include "vasnecovmaterial.h"
#include "vasnecovfigure.h"
#include "vasnecovworld.h"
#include "vasnecovproduct.h"
#include "vasnecovlabel.h"
#include "elementlist.h"
#ifndef _MSC_VER
    #pragma GCC diagnostic warning "-Weffc++"
#endif

class VasnecovMaterial;

namespace Vasnecov
{
    struct UniverseAttributes : public Attributes
    {
        // Данные, используемые только в потоке управления
        std::map<std::string, VasnecovMesh*> meshes;
        std::map<std::string, VasnecovTexture*> textures;

        std::string dirMeshes; // Основная директория мешей
        std::string dirTextures; // Основная директория текстур
        std::string dirTexturesDPref;
        std::string dirTexturesNPref;
        std::string dirTexturesIPref;

        // Списки для загрузки
        // Поскольку используется только один OpenGL контекст (в основном потоке), приходится использовать списки действий.
        std::vector<VasnecovMesh*> meshesForLoading;
        std::vector<VasnecovTexture*> texturesForLoading;

        UniverseAttributes() :
            dirMeshes(Vasnecov::cfg_dirMeshes),
            dirTextures(Vasnecov::cfg_dirTextures),
            dirTexturesDPref(Vasnecov::cfg_dirTexturesDPref),
            dirTexturesNPref(Vasnecov::cfg_dirTexturesNPref),
            dirTexturesIPref(Vasnecov::cfg_dirTexturesIPref)
        {
        }
        ~UniverseAttributes() override;
    };
}
class VasnecovUniverse
{
    // Управление индикатором загрузки
    class LoadingStatus
    {
    public:
        explicit LoadingStatus(GLboolean* loading) : m_loading(loading)
        {
            *m_loading = true;
        }
        ~LoadingStatus()
        {
            *m_loading = false;
        }
    private:
        GLboolean* m_loading;

        Q_DISABLE_COPY(LoadingStatus)
    };

    // Шаблон контейнера списков с возможностью удаления
    template <typename T>
    class ElementFullBox : public Vasnecov::ElementBox<T>
    {
    public:
        ElementFullBox();
        ~ElementFullBox() override;

        GLboolean synchronize() override;
        GLboolean removeElement(const T* element) override;
        const std::vector<T*>& deleting() const;

    protected:
        std::vector<T*> m_deleting;
    };

    // Список контейнеров списков, расширенный
    class UniverseElementList : public Vasnecov::ElementList<ElementFullBox>
    {
    public:
        VasnecovWorld* findRawElement(VasnecovWorld* world) const {return m_worlds.findElement(world);}
        VasnecovMaterial* findRawElement(VasnecovMaterial* material) const {return m_materials.findElement(material);}
        using Vasnecov::ElementList<ElementFullBox>::findRawElement;

        GLboolean addElement(VasnecovWorld* world, GLboolean check = false) {return m_worlds.addElement(world, check);}
        GLboolean addElement(VasnecovMaterial* material, GLboolean check = false) {return m_materials.addElement(material, check);}
        using Vasnecov::ElementList<ElementFullBox>::addElement;

        GLboolean removeElement(const VasnecovWorld* world) {return m_worlds.removeElement(world);}
        GLboolean removeElement(const VasnecovMaterial* material) {return m_materials.removeElement(material);}
        using Vasnecov::ElementList<ElementFullBox>::removeElement;

        GLuint removeElements(const std::vector<VasnecovWorld*>& deletingList) {return m_worlds.removeElements(deletingList);}
        GLuint removeElements(const std::vector<VasnecovMaterial*>& deletingList) {return m_materials.removeElements(deletingList);}
        using Vasnecov::ElementList<ElementFullBox>::removeElements;

        GLboolean synchronizeWorlds() {return m_worlds.synchronize();}
        GLboolean synchronizeMaterials() {return m_materials.synchronize();}

        const ElementFullBox<VasnecovWorld>& rawWorlds() const {return m_worlds;}
        const ElementFullBox<VasnecovMaterial>& rawMaterials() const {return m_materials;}

        const ElementFullBox<VasnecovWorld>& pureWorlds() const {return m_worlds;}
        const ElementFullBox<VasnecovMaterial>& pureMaterials() const {return m_materials;}

        GLboolean hasPureWorlds() const {return m_worlds.hasPure();}
        GLboolean hasPureMaterials() const {return m_materials.hasPure();}

        GLuint rawWorldsCount() const {return m_worlds.rawCount();}
        GLuint rawMaterialsCount() const {return m_materials.rawCount();}

        template <typename F>
        void forEachPureWorld(F fun) const {m_worlds.forEachPure(fun);}
        template <typename F>
        void forEachPureMaterial(F fun) const {m_materials.forEachPure(fun);}

        GLboolean synchronizeAll() override
        {
            GLboolean res = false;

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
    explicit VasnecovUniverse(const QGLContext* context = nullptr);
    ~VasnecovUniverse();

public:
    // Методы добавления блокируют мьютекс на все время исполнения
    // Но это не критично, т.к. поток отрисовки использует tryLock и просто не обновляет данные, рисуя старые
    VasnecovWorld* addWorld(GLint posX, GLint posY, GLsizei width, GLsizei height);

    VasnecovLamp* addLamp(const std::string& name,
                          VasnecovWorld* world,
                          VasnecovLamp::LampTypes type = VasnecovLamp::LampTypeCelestial);
    VasnecovLamp* referLampToWorld(VasnecovLamp* lamp, VasnecovWorld* world);

    // Добавление новых продуктов
    VasnecovProduct* addAssembly(const std::string& name,
                                  VasnecovWorld* world,
                                  VasnecovProduct* parent = nullptr);

    VasnecovProduct* addPart(const std::string& name,
                              VasnecovWorld* world,
                              const std::string& meshName,
                              VasnecovProduct* parent = nullptr); // Материал по умолчанию

    VasnecovProduct* addPart(const std::string& name,
                              VasnecovWorld* world,
                              const std::string& meshName,
                              VasnecovMaterial* material,
                              VasnecovProduct* parent = nullptr);

    VasnecovProduct* addPart(const std::string& name,
                              VasnecovWorld* world,
                              const std::string& meshName,
                              const std::string& textureName,
                              VasnecovProduct* parent = nullptr); // Материал по умолчанию с указанной текстурой
    VasnecovProduct* referProductToWorld(VasnecovProduct* product, VasnecovWorld* world); // Сделать дубликат изделия в заданный мир
    GLboolean removeProduct(VasnecovProduct* product);

    VasnecovFigure* addFigure(const std::string& name, VasnecovWorld* world);
    VasnecovFigure* addFigure(std::string&& name, VasnecovWorld* world);
    GLboolean removeFigure(const VasnecovFigure* figure);

    VasnecovLabel* addLabel(const std::string& name,
                            VasnecovWorld* world,
                            GLfloat width,
                            GLfloat height);
    VasnecovLabel* addLabel(const std::string& name,
                            VasnecovWorld* world,
                            GLfloat width,
                            GLfloat height,
                            const std::string& textureName);
    VasnecovLabel* referLabelToWorld(VasnecovLabel* label, VasnecovWorld* world);
    GLboolean removeLabel(VasnecovLabel* label);

    // Добавление материала
    VasnecovMaterial* addMaterial(const std::string& textureName);
    VasnecovMaterial* addMaterial();
    VasnecovTexture* textureByName(const std::string& textureName, Vasnecov::TextureTypes type = Vasnecov::TextureTypeDiffuse);


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
    GLboolean setTexturesDir(const std::string& dir);
    GLboolean setMeshesDir(const std::string& dir);

    void loadAll(); // Загрузка всех ресурсов из своих директорий

    GLboolean loadMesh(const std::string& fileName); // Загрузка конкретного меша
    GLuint loadMeshes(const std::string& dirName = "", GLboolean withSub = true); // Загрузка всех мешей
    GLboolean loadTexture(const std::string& fileName);
    GLuint loadTextures(const std::string& dirName = "", GLboolean withSub = true); // Загрузка всех текстур

    QString info(GLuint type = 0);

protected:
    // Блокирует мьютекс, но вызывается из других методов
    // TODO: make abstract class Resource for textures, meshes, may be shaders. And use with template like an Element
    GLboolean addTexture(VasnecovTexture* texture, const std::string& fileId);
    GLboolean addMesh(VasnecovMesh* mesh, const std::string& fileId);

    // Работа с файлами ресурсов
    GLuint handleFilesInDir(const std::string& dirPref,
                            const std::string& targetDir,
                            const std::string& format,
                            GLboolean (VasnecovUniverse::*workFun)(const std::string&),
                            GLboolean withSub = true); // Поиск файлов в директории и выполнение с ними метода
    GLboolean loadMeshFile(const std::string& fileName);
    GLboolean loadTextureFile(const std::string& fileName);

protected:
    // Методы, вызываемые из внешних потоков (работают с сырыми данными)
    VasnecovMesh* designerFindMesh(const std::string& name);
    VasnecovTexture* designerFindTexture(const std::string& name);

    GLboolean designerRemoveThisAlienMatrix(const QMatrix4x4* alienMs);

protected:
    // Вспомогательные (не привязаны к внутренним данным)
    GLboolean setDirectory(const std::string& newDir, std::string& oldDir) const;
    GLboolean correctPath(std::string& path, std::string& fileId, const std::string& format) const; // Добавляет расширение в путь, удаляет его из fileId, проверяет наличие файла
    static std::string correctFileId(const std::string& fileId, const std::string& format); // Удаляет формат из имени

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
        if (!element)
            return;
        element->renderUpdateData();
    }

private:
    VasnecovPipeline m_pipeline;
    const QGLContext* m_context;
    QColor m_backgroundColor;

    GLsizei m_width, m_height; // Размеры окна вывода

    // Картинка для индикации загрузки
    GLboolean m_loading;
    QImage m_loadingImage0;
    QImage m_loadingImage1;
    timespec m_loadingImageTimer;

    GLuint m_lampsCountMax;

    // Списки миров
    // Списки общих (между мирами) данных
    Vasnecov::UniverseAttributes raw_data;
    UniverseElementList m_elements;

    enum Updated
    {
        Meshes			= 0x0000001,
        Textures		= 0x0000002,
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
    QString m_techRenderer;
    QString m_techVersion;
    QString m_techSL;
    QString m_techExtensions;

    friend class VasnecovScene;
    friend class VasnecovWidget;

private:
    Q_DISABLE_COPY(VasnecovUniverse)
};

inline void VasnecovUniverse::setContext(const QGLContext *context)
{
    if(!context)
        return;
    m_context = context;
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
    if(!this->m_wasUpdated)
        return false;

    this->m_pure = this->m_raw;
    this->m_wasUpdated = false;

    if(!m_deleting.empty())
    {
        for(typename std::vector<T *>::iterator eit = m_deleting.begin(); eit != m_deleting.end(); ++eit)
        {
            delete (*eit);
            (*eit) = nullptr;
        }
        m_deleting.clear();
    }
    return true;
}

template <typename T>
GLboolean VasnecovUniverse::ElementFullBox<T>::removeElement(const T *element)
{
    if (!element)
        return false;

    for(typename std::vector<T *>::iterator eit = this->m_raw.begin(); eit != this->m_raw.end(); ++eit)
    {
        if((*eit) == element)
        {
            m_deleting.push_back(*eit);

            this->m_raw.erase(eit);
            this->m_wasUpdated = true;
            return true;
        }
    }

    return false;
}

template <typename T>
const std::vector<T *> &VasnecovUniverse::ElementFullBox<T>::deleting() const
{
    return m_deleting;
}

#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
#endif // VASNECOV_UNIVERSE_H
