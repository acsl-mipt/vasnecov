#ifndef VASNECOV_CONFIGURATION_H
#define VASNECOV_CONFIGURATION_H

#pragma GCC diagnostic ignored "-Weffc++"
#include <ctime>
#include "types.h"
#pragma GCC diagnostic warning "-Weffc++"

namespace Vasnecov
{
	const GLsizei cfg_displayWidthDefault = 320;
	const GLsizei cfg_displayHeightDefault = 320;

	const GLstring cfg_shareDir = "/usr/local/share/";
	const GLstring cfg_libraryName = "libVasnecov";
	const GLboolean cfg_showLoadingImage = 1; // Отображать табличку загрузки
	const GLstring cfg_loadingImageFile = "loading.png";
	const int cfg_loadingImagePause = 150;

	const GLstring cfg_dirTextures = "stuff/textures/";
	const GLstring cfg_dirTexturesIPref = "i/";
	const GLstring cfg_dirTexturesDPref = "d/";
	const GLstring cfg_dirTexturesNPref = "n/";
	const GLstring cfg_dirMeshes = "stuff/meshes/";

	const GLstring cfg_textureFormat = "png";
	const GLstring cfg_meshFormat = "obj";
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

#pragma GCC diagnostic ignored "-Weffc++"
#endif // CONFIGURATION_H
