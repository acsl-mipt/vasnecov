/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Класс материала для наложения на меши
#ifndef VASNECOVMATERIAL_H
#define VASNECOVMATERIAL_H

#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
#include <QColor>
#include "coreobject.h"
#include "vasnecovtexture.h"
#ifndef _MSC_VER
    #pragma GCC diagnostic warning "-Weffc++"
#endif

class VasnecovTexture;
class VasnecovPipeline;

// TODO: make MaterialManager which will control current material in pipeline.
// TODO: add material types (default, defaultColoring, etc...)
class VasnecovMaterial : public Vasnecov::CoreObject
{
public:
    VasnecovMaterial(VasnecovPipeline* pipeline,
                     const std::string& name = std::string());
    VasnecovMaterial(VasnecovPipeline* pipeline,
                     VasnecovTexture* textureD,
                     VasnecovTexture* textureN = nullptr,
                     const std::string& name = std::string());

public:
    VasnecovTexture* textureD() const { return m_textureD; }
    VasnecovTexture* textureN() const { return m_textureN; }

    void setAmbientColor(const QColor& color) { m_ambientColor = color; }
    void setDiffuseColor(const QColor& color) { m_diffuseColor = color; }
    void setSpecularColor(const QColor& color) { m_specularColor = color; }
    void setEmissionColor(const QColor& color) { m_emissionColor = color; }
    void setShininess(GLfloat shininess) { m_shininess = shininess; }
    QColor ambientColor() const { return m_ambientColor; }
    QColor diffuseColor() const { return m_diffuseColor; }
    QColor specularColor() const { return m_specularColor; }
    QColor emissionColor() const { return m_emissionColor; }
    GLfloat shininess() const { return m_shininess; }

protected:
    void designerSetAmbientAndDiffuseColor(const QColor& color);

protected:
    GLenum renderUpdateData() override;
    void renderDraw() override;

protected:

    enum MaterialType // TODO: complete material type-control
    {
        Default = 1,
        DefaultDiffuseTextured
    };

    VasnecovTexture* m_textureD;
    VasnecovTexture* m_textureN;

    QColor m_ambientColor;
    QColor m_diffuseColor;
    QColor m_specularColor;
    QColor m_emissionColor;
    GLfloat m_shininess;

    enum
    {
        TextureD	= 0x0008,
        TextureN	= 0x0010,

        Ambient		= 0x0020,
        Diffuse		= 0x0040,
        Specular	= 0x0080,
        Emission	= 0x0100,
        Shininess	= 0x0200
    };

    friend class VasnecovUniverse;
    friend class VasnecovWorld;
    friend class VasnecovProduct;

private:
    Q_DISABLE_COPY(VasnecovMaterial)
};

#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
#endif // VASNECOVMATERIAL_H
