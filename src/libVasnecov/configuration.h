/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef VASNECOV_CONFIGURATION_H
#define VASNECOV_CONFIGURATION_H

#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
#include <ctime>
#include "types.h"
#ifndef _MSC_VER
    #pragma GCC diagnostic warning "-Weffc++"
#endif

namespace Vasnecov
{
    const GLsizei cfg_displayWidthDefault = 320;
    const GLsizei cfg_displayHeightDefault = 320;

    const std::string cfg_shareDir = "/usr/local/share/";
    const GLboolean cfg_showLoadingImage = 1; // Отображать табличку загрузки
    const GLint cfg_loadingImagePause = 150;

    const std::string cfg_dirTextures = "stuff/textures/";
    const std::string cfg_dirTexturesIPref = "i/";
    const std::string cfg_dirTexturesDPref = "d/";
    const std::string cfg_dirTexturesNPref = "n/";
    const std::string cfg_dirMeshes = "stuff/meshes/";

    const std::string cfg_textureFormat = "png";
    const std::string cfg_meshFormat = "obj";
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

#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
#endif // CONFIGURATION_H
