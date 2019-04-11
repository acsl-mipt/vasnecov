/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "VasnecovMaterial.h"
#include "VasnecovPipeline.h"

VasnecovMaterial::VasnecovMaterial(VasnecovPipeline *pipeline,
                                   VasnecovTexture *textureD,
                                   VasnecovTexture *textureN,
                                   const QString &name)
    : Vasnecov::CoreObject(pipeline, name)
    , m_textureD(raw_wasUpdated, TextureD, textureD)
    , m_textureN(raw_wasUpdated, TextureN, textureN)

    , m_ambientColor(raw_wasUpdated, Ambient, QColor(Qt::lightGray))
    , m_diffuseColor(raw_wasUpdated, Diffuse, textureD == nullptr ? QColor(204, 204, 204, 255) : QColor(Qt::white))
    , m_specularColor(raw_wasUpdated, Specular, QColor(0, 0, 0, 255))
    , m_emissionColor(raw_wasUpdated, Emission, QColor(0, 0, 0, 255))
    , m_shininess(raw_wasUpdated, Shininess, 0)
{
}

VasnecovMaterial::VasnecovMaterial(VasnecovPipeline *pipeline,
                                   const QString &name)
    : VasnecovMaterial(pipeline, nullptr, nullptr, name)
{
}

void VasnecovMaterial::setTextureD(VasnecovTexture *textureD)
{
    m_textureD.set(textureD);
    m_diffuseColor.set(textureD == nullptr ? QColor(204, 204, 204, 255) : QColor(Qt::white));
}

VasnecovTexture *VasnecovMaterial::textureD() const
{
    VasnecovTexture *texture(m_textureD.raw());
    return texture;
}
void VasnecovMaterial::setTextureN(VasnecovTexture *textureN)
{
    m_textureN.set(textureN);
}
VasnecovTexture *VasnecovMaterial::textureN() const
{
    VasnecovTexture *texture(m_textureN.raw());
    return texture;
}
void VasnecovMaterial::setAmbientColor(const QColor &color)
{
    m_ambientColor.set(color);
}
void VasnecovMaterial::setDiffuseColor(const QColor &color)
{
    m_diffuseColor.set(color);
}
void VasnecovMaterial::setSpecularColor(const QColor &color)
{
    m_specularColor.set(color);
}
void VasnecovMaterial::setEmissionColor(const QColor &color)
{
    m_emissionColor.set(color);
}
void VasnecovMaterial::setShininess(GLfloat shininess)
{
    m_shininess.set(shininess);
}
QColor VasnecovMaterial::ambientColor() const
{
    QColor color(m_ambientColor.raw());
    return color;
}
QColor VasnecovMaterial::diffuseColor() const
{
    QColor color(m_diffuseColor.raw());
    return color;
}
QColor VasnecovMaterial::specularColor() const
{
    QColor color(m_specularColor.raw());
    return color;
}
QColor VasnecovMaterial::emissionColor() const
{
    QColor color(m_emissionColor.raw());
    return color;
}
GLfloat VasnecovMaterial::shininess() const
{
    GLfloat shininess(m_shininess.raw());
    return shininess;
}
void VasnecovMaterial::designerSetAmbientAndDiffuseColor(const QColor &color)
{
    m_ambientColor.set(color);
    m_diffuseColor.set(color);
}
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
