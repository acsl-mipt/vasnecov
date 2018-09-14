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
VasnecovMaterial::VasnecovMaterial(VasnecovPipeline *pipeline,
                                   const std::string &name) :
    Vasnecov::CoreObject(pipeline, name),
    m_textureD(raw_wasUpdated, TextureD, nullptr),
    m_textureN(raw_wasUpdated, TextureN, nullptr),

    m_ambientColor(raw_wasUpdated, Ambient, QColor(51, 51, 51, 255)),
    m_diffuseColor(raw_wasUpdated, Diffuse, QColor(204, 204, 204, 255)),
    m_specularColor(raw_wasUpdated, Specular, QColor(0, 0, 0, 255)),
    m_emissionColor(raw_wasUpdated, Emission, QColor(0, 0, 0, 255)),
    m_shininess(raw_wasUpdated, Shininess, 0)
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
    m_textureD(raw_wasUpdated, TextureD, textureD),
    m_textureN(raw_wasUpdated, TextureN, textureN),

    m_ambientColor(raw_wasUpdated, Ambient, QColor(51, 51, 51, 255)),
    m_diffuseColor(raw_wasUpdated, Diffuse, QColor(204, 204, 204, 255)),
    m_specularColor(raw_wasUpdated, Specular, QColor(0, 0, 0, 255)),
    m_emissionColor(raw_wasUpdated, Emission, QColor(0, 0, 0, 255)),
    m_shininess(raw_wasUpdated, Shininess, 0)
{
}

/*!
 \brief

 \fn VasnecovMaterial::setTextureD
 \param textureD
*/
void VasnecovMaterial::setTextureD(VasnecovTexture *textureD)
{
    m_textureD.set(textureD);
}

/*!
 \brief

 \fn VasnecovMaterial::textureD
 \return VasnecovTexture
*/
VasnecovTexture *VasnecovMaterial::textureD() const
{
    VasnecovTexture *texture(m_textureD.raw());
    return texture;
}

/*!
 \brief

 \fn VasnecovMaterial::setTextureN
 \param textureN
*/
void VasnecovMaterial::setTextureN(VasnecovTexture *textureN)
{
    m_textureN.set(textureN);
}

/*!
 \brief

 \fn VasnecovMaterial::textureN
 \return VasnecovTexture
*/
VasnecovTexture *VasnecovMaterial::textureN() const
{
    VasnecovTexture *texture(m_textureN.raw());
    return texture;
}

/*!
 \brief

 \fn VasnecovMaterial::setAmbientColor
 \param color
*/
void VasnecovMaterial::setAmbientColor(const QColor &color)
{
    m_ambientColor.set(color);
}

/*!
 \brief

 \fn VasnecovMaterial::setDiffuseColor
 \param color
*/
void VasnecovMaterial::setDiffuseColor(const QColor &color)
{
    m_diffuseColor.set(color);
}

/*!
 \brief

 \fn VasnecovMaterial::setSpecularColor
 \param color
*/
void VasnecovMaterial::setSpecularColor(const QColor &color)
{
    m_specularColor.set(color);
}

/*!
 \brief

 \fn VasnecovMaterial::setEmissionColor
 \param color
*/
void VasnecovMaterial::setEmissionColor(const QColor &color)
{
    m_emissionColor.set(color);
}

/*!
 \brief

 \fn VasnecovMaterial::setShininess
 \param shininess
*/
void VasnecovMaterial::setShininess(GLfloat shininess)
{
    m_shininess.set(shininess);
}

/*!
 \brief

 \fn VasnecovMaterial::ambientColor
 \return QColor
*/
QColor VasnecovMaterial::ambientColor() const
{
    QColor color(m_ambientColor.raw());
    return color;
}
/*!
 \brief

 \fn VasnecovMaterial::diffuseColor
 \return QColor
*/
QColor VasnecovMaterial::diffuseColor() const
{
    QColor color(m_diffuseColor.raw());
    return color;
}
/*!
 \brief

 \fn VasnecovMaterial::specularColor
 \return QColor
*/
QColor VasnecovMaterial::specularColor() const
{
    QColor color(m_specularColor.raw());
    return color;
}
/*!
 \brief

 \fn VasnecovMaterial::emissionColor
 \return QColor
*/
QColor VasnecovMaterial::emissionColor() const
{
    QColor color(m_emissionColor.raw());
    return color;
}
/*!
 \brief

 \fn VasnecovMaterial::shininess
 \return GLfloat
*/
GLfloat VasnecovMaterial::shininess() const
{
    GLfloat shininess(m_shininess.raw());
    return shininess;
}

/*!
 \brief

 \fn VasnecovMaterial::designerSetAmbientAndDiffuseColor
 \param color
*/
void VasnecovMaterial::designerSetAmbientAndDiffuseColor(const QColor &color)
{
    m_ambientColor.set(color);
    m_diffuseColor.set(color);
}

/*!
 \brief

 \fn VasnecovMaterial::renderUpdateData
 \return GLenum
*/
GLenum VasnecovMaterial::renderUpdateData()
{
    GLenum updated(raw_wasUpdated);

    if(raw_wasUpdated)
    {
        pure_pipeline->setSomethingWasUpdated();

        // Копирование сырых данных в основные
        m_textureD.update();
        m_textureN.update();

        m_ambientColor.update();
        m_diffuseColor.update();
        m_specularColor.update();
        m_emissionColor.update();
        m_shininess.update();

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
    pure_pipeline->setMaterialColors(m_ambientColor.pure(),
                                     m_diffuseColor.pure(),
                                     m_specularColor.pure(),
                                     m_emissionColor.pure(),
                                     m_shininess.pure());

    if(m_textureD.pure())
    {
        pure_pipeline->enableTexture2D(m_textureD.pure()->id());
    }
    else
    {
        pure_pipeline->disableTexture2D();
    }
}

#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
