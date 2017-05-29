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
    VasnecovMaterial(QMutex *mutex,
                     VasnecovPipeline *pipeline,
                     const std::string &name = std::string());
    VasnecovMaterial(QMutex *mutex,
                     VasnecovPipeline *pipeline,
                     VasnecovTexture *textureD,
                     VasnecovTexture *textureN = 0,
                     const std::string &name = std::string());

public:
    void setTextureD(VasnecovTexture *textureD);
    VasnecovTexture *textureD() const;
    void setTextureN(VasnecovTexture *textureN);
    VasnecovTexture *textureN() const;

    void setAmbientColor(const QColor &color);
    void setDiffuseColor(const QColor &color);
    void setSpecularColor(const QColor &color);
    void setEmissionColor(const QColor &color);
    void setShininess(GLfloat shininess);
    QColor ambientColor() const;
    QColor diffuseColor() const;
    QColor specularColor() const;
    QColor emissionColor() const;
    GLfloat shininess() const;

protected:
    void designerSetAmbientAndDiffuseColor(const QColor &color);

protected:
    GLenum renderUpdateData();
    void renderDraw();

    VasnecovTexture *renderTextureD() const;
    VasnecovTexture *renderTextureN() const;

    QColor renderAmbientColor() const;
    QColor renderDiffuseColor() const;
    QColor renderSpecularColor() const;
    QColor renderEmissionColor() const;
    GLfloat renderShininess() const;

protected:

    enum MaterialType // TODO: complete material type-control
    {
        Default = 1,
        DefaultDiffuseTextured
    };

    Vasnecov::MutualData<VasnecovTexture *> m_textureD;
    Vasnecov::MutualData<VasnecovTexture *> m_textureN;

    Vasnecov::MutualData<QColor> m_ambientColor;
    Vasnecov::MutualData<QColor> m_diffuseColor;
    Vasnecov::MutualData<QColor> m_specularColor;
    Vasnecov::MutualData<QColor> m_emissionColor;
    Vasnecov::MutualData<GLfloat> m_shininess;

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

inline VasnecovTexture *VasnecovMaterial::renderTextureD() const
{
    return m_textureD.pure();
}
inline VasnecovTexture *VasnecovMaterial::renderTextureN() const
{
    return m_textureN.pure();
}

inline QColor VasnecovMaterial::renderAmbientColor() const
{
    return m_ambientColor.pure();
}
inline QColor VasnecovMaterial::renderDiffuseColor() const
{
    return m_diffuseColor.pure();
}
inline QColor VasnecovMaterial::renderSpecularColor() const
{
    return m_specularColor.pure();
}
inline QColor VasnecovMaterial::renderEmissionColor() const
{
    return m_emissionColor.pure();
}
inline GLfloat VasnecovMaterial::renderShininess() const
{
    return m_shininess.pure();
}

#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
#endif // VASNECOVMATERIAL_H
