/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "vasnecovmaterial.h"
#include "vasnecovpipeline.h"
#ifndef _MSC_VER
    #pragma GCC diagnostic warning "-Weffc++"
#endif

/*!
 \brief

 \fn VasnecovMaterial::VasnecovMaterial
 \param pipeline
*/
VasnecovMaterial::VasnecovMaterial(VasnecovPipeline *pipeline, const std::string &name) :
    Vasnecov::CoreObject(pipeline, name),
    m_textureD(nullptr),
    m_textureN(nullptr),

    m_ambientColor(51, 51, 51, 255),
    m_diffuseColor(204, 204, 204, 255),
    m_specularColor(0, 0, 0, 255),
    m_emissionColor(0, 0, 0, 255),
    m_shininess(0)
{
//	QColor c;

//	c.setRgbF(0.2f, 0.2f, 0.2f, 1.0f);
//	m_ambientColor.set(c);

//	c.setRgbF(0.8f, 0.8f, 0.8f, 1.0f);
//	m_diffuseColor.set(c);
}

/*!
 \brief

 \fn VasnecovMaterial::VasnecovMaterial
 \param pipeline
 \param textureD
 \param textureN
*/
VasnecovMaterial::VasnecovMaterial(VasnecovPipeline *pipeline,
                                   VasnecovTexture *textureD,
                                   VasnecovTexture *textureN,
                                   const std::string &name) :
    Vasnecov::CoreObject(pipeline, name),
    m_textureD(textureD),
    m_textureN(textureN),

    m_ambientColor(51, 51, 51, 255),
    m_diffuseColor(204, 204, 204, 255),
    m_specularColor(0, 0, 0, 255),
    m_emissionColor(0, 0, 0, 255),
    m_shininess(0)
{
}

/*!
 \brief

 \fn VasnecovMaterial::designerSetAmbientAndDiffuseColor
 \param color
*/
void VasnecovMaterial::designerSetAmbientAndDiffuseColor(const QColor &color)
{
    m_ambientColor = color;
    m_diffuseColor = color;
}

/*!
 \brief

 \fn VasnecovMaterial::renderUpdateData
 \return GLenum
*/
GLenum VasnecovMaterial::renderUpdateData()
{
    GLenum updated = raw_wasUpdated;

    if(raw_wasUpdated)
    {
        pure_pipeline->setSomethingWasUpdated();
        Vasnecov::CoreObject::renderUpdateData();
    }

    return updated;
}

/*!
 \brief

 \fn VasnecovMaterial::renderDraw
*/
void VasnecovMaterial::renderDraw()
{
    pure_pipeline->setMaterialColors(m_ambientColor,
                                     m_diffuseColor,
                                     m_specularColor,
                                     m_emissionColor,
                                     m_shininess);

    if(m_textureD)
    {
        pure_pipeline->enableTexture2D(m_textureD->id());
    }
    else
    {
        pure_pipeline->disableTexture2D();
    }
}

#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
