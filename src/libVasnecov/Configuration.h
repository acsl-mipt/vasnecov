/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <ctime>
#include <QString>
#include "Types.h"

namespace Vasnecov
{
    const GLsizei cfg_displayWidthDefault = 320;
    const GLsizei cfg_displayHeightDefault = 320;

    const QString cfg_shareDir = "/usr/local/share/";
    const GLboolean cfg_showLoadingImage = 1; // Отображать табличку загрузки
    const GLint cfg_loadingImagePause = 150;

    const QString cfg_dirTextures = "stuff/textures/";
    const QString cfg_dirTexturesIPref = "i/";
    const QString cfg_dirTexturesDPref = "d/";
    const QString cfg_dirTexturesNPref = "n/";
    const QString cfg_dirMeshes = "stuff/meshes/";

    const QString cfg_textureFormat = "png";
    const QString cfg_meshFormat = "obj";
    const QString cfg_rawMeshFormat = "vmf";
    const GLboolean cfg_readFromMTL = 1; // Читать имя текстуры из мтл-библиотеки, указанной в обж
    const GLboolean cfg_sortTransparency = true;
    const GLuint cfg_elementMaxLevel = 16; // Количество максимальных уровней для ВЭлемента

    const GLuint cfg_lampsCountMax = 8;

    inline timespec timeDefault() // Типа, конструктор для timespec
    {
        timespec td;
        td.tv_sec = 0;
        td.tv_nsec = 0;
        return td;
    }
}
