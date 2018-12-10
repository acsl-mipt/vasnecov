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

/*!
 \brief

 \fn VasnecovMaterial::VasnecovMaterial
 \param pipeline
*/
VasnecovMaterial::VasnecovMaterial(VasnecovPipeline *pipeline,
                                   const QString &name) :
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
                                   const QString &name) :
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

 \fn VasnecovMaterial::setTextureD
 \param textureD
*/
void VasnecovMaterial::setTextureD(VasnecovTexture *textureD)
{
    m_textureD = textureD;
}

/*!
 \brief

 \fn VasnecovMaterial::textureD
 \return VasnecovTexture
*/
VasnecovTexture *VasnecovMaterial::textureD() const
{
    VasnecovTexture *texture(m_textureD);
    return texture;
}

/*!
 \brief

 \fn VasnecovMaterial::setTextureN
 \param textureN
*/
void VasnecovMaterial::setTextureN(VasnecovTexture *textureN)
{
    m_textureN = textureN;
}

/*!
 \brief

 \fn VasnecovMaterial::textureN
 \return VasnecovTexture
*/
VasnecovTexture *VasnecovMaterial::textureN() const
{
    VasnecovTexture *texture(m_textureN);
    return texture;
}

/*!
 \brief

 \fn VasnecovMaterial::setAmbientColor
 \param color
*/
void VasnecovMaterial::setAmbientColor(const QColor &color)
{
    m_ambientColor = color;
}

/*!
 \brief

 \fn VasnecovMaterial::setDiffuseColor
 \param color
*/
void VasnecovMaterial::setDiffuseColor(const QColor &color)
{
    m_diffuseColor = color;
}

/*!
 \brief

 \fn VasnecovMaterial::setSpecularColor
 \param color
*/
void VasnecovMaterial::setSpecularColor(const QColor &color)
{
    m_specularColor = color;
}

/*!
 \brief

 \fn VasnecovMaterial::setEmissionColor
 \param color
*/
void VasnecovMaterial::setEmissionColor(const QColor &color)
{
    m_emissionColor = color;
}

/*!
 \brief

 \fn VasnecovMaterial::setShininess
 \param shininess
*/
void VasnecovMaterial::setShininess(GLfloat shininess)
{
    m_shininess = shininess;
}

/*!
 \brief

 \fn VasnecovMaterial::ambientColor
 \return QColor
*/
QColor VasnecovMaterial::ambientColor() const
{
    return m_ambientColor;
}
/*!
 \brief

 \fn VasnecovMaterial::diffuseColor
 \return QColor
*/
QColor VasnecovMaterial::diffuseColor() const
{
    return m_diffuseColor;
}
/*!
 \brief

 \fn VasnecovMaterial::specularColor
 \return QColor
*/
QColor VasnecovMaterial::specularColor() const
{
    return m_specularColor;
}
/*!
 \brief

 \fn VasnecovMaterial::emissionColor
 \return QColor
*/
QColor VasnecovMaterial::emissionColor() const
{
    return m_emissionColor;
}
/*!
 \brief

 \fn VasnecovMaterial::shininess
 \return GLfloat
*/
GLfloat VasnecovMaterial::shininess() const
{
    return m_shininess;
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
