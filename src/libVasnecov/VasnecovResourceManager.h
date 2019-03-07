/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <bmcl/ThreadSafeRefCountable.h>
#include "Configuration.h"
#include "Types.h"

class VasnecovResourceManager : public bmcl::ThreadSafeRefCountable<std::size_t>
{
public:
    explicit VasnecovResourceManager();
    ~VasnecovResourceManager();

    GLboolean setTexturesDir(const QString& dir);
    GLboolean setMeshesDir(const QString& dir);

    GLboolean loadMeshFile(const QString& fileName);
    GLboolean loadMeshFileByPath(const QString& filePath);
    GLboolean loadTextureFile(const QString& fileName);

    size_t meshesAmount() const;

    static GLboolean setDirectory(const QString& newDir, QString& oldDir);
    static GLboolean correctPath(QString& path, QString& fileId, const QString& format); // Добавляет расширение в путь, удаляет его из fileId, проверяет наличие файла
    static QString correctFileId(const QString& fileId, const QString& format); // Удаляет формат из имени

private:
    GLboolean addTexture(VasnecovTexture* texture, const QString& fileId);
    GLboolean addMesh(VasnecovMesh* mesh, const QString& fileId);

    VasnecovMesh* designerFindMesh(const QString& name);
    VasnecovTexture* designerFindTexture(const QString& name);

    bool handleMeshesDir(const QString& dirName, GLboolean withSub);
    bool handleTexturesDir(const QString& dirName, GLboolean withSub);

    // Работа с файлами ресурсов
    GLuint handleFilesInDir(const QString& dirPref,
                            const QString& targetDir,
                            const QString& format,
                            GLboolean (VasnecovResourceManager::*workFun)(const QString&),
                            GLboolean withSub = true); // Поиск файлов в директории и выполнение с ними метода

    bool renderUpdate();

    const QString& texturesDPref() const {return dirTexturesDPref;}
    const QString& texturesNPref() const {return dirTexturesNPref;}
    const QString& texturesIPref() const {return dirTexturesIPref;}

private:
    enum Updated
    {
        Meshes			= 0x0000001,
        Textures		= 0x0000002,
    };

    Vasnecov::Attributes raw_data;

    // Данные, используемые только в потоке управления
    std::map<QString, VasnecovMesh*>    meshes;
    std::map<QString, VasnecovTexture*> textures;

    QString dirMeshes; // Основная директория мешей
    QString dirTextures; // Основная директория текстур
    QString dirTexturesDPref;
    QString dirTexturesNPref;
    QString dirTexturesIPref;

    // Списки для загрузки
    // Поскольку используется только один OpenGL контекст (в основном потоке), приходится использовать списки действий.
    std::vector<VasnecovMesh*>      meshesForLoading;
    std::vector<VasnecovTexture*>   texturesForLoading;

    friend class VasnecovUniverse;

    Q_DISABLE_COPY(VasnecovResourceManager)
};


