/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Класс источников света
#pragma once

#include "VasnecovElement.h"
#include "LightModel.h"

#include <QtGlobal>

class VasnecovLamp : public VasnecovAbstractElement
{
public:
    VasnecovLamp(VasnecovPipeline* pipeline, const QString& name, Vasnecov::LampTypes type, GLuint index);
    ~VasnecovLamp();

public:
    void setType(Vasnecov::LampTypes type);

    void setCelestialDirection(const QVector3D& direction); // Обертки для направленного источника света
    void setCelestialDirection(GLfloat x, GLfloat y, GLfloat z);

    void setAmbientColor(const QColor& color);
    void setDiffuseColor(const QColor& color);
    void setSpecularColor(const QColor& color);

    void setSpotDirection(const QVector3D& direction);
    void setSpotExponent(GLfloat exponent);
    void setSpotAngle(GLfloat angle);

    void setConstantAttenuation(GLfloat attenuation);
    void setLinearAttenuation(GLfloat attenuation);
    void setQuadraticAttenuation(GLfloat attenuation);

protected:
    GLenum renderUpdateData();
    void renderDraw();

    void renderSetIndex(GLuint index);
    GLuint renderIndex() const;

protected:
    Vasnecov::MutualData<Vasnecov::LampTypes> m_type;

    Vasnecov::MutualData<QColor> m_ambientColor;
    Vasnecov::MutualData<QColor> m_diffuseColor;
    Vasnecov::MutualData<QColor> m_specularColor;

    Vasnecov::MutualData<QVector3D> m_spotDirection;
    Vasnecov::MutualData<GLfloat> m_spotExponent;
    Vasnecov::MutualData<GLfloat> m_spotAngle;
    Vasnecov::MutualData<GLfloat> m_spotCosAngle;

    Vasnecov::MutualData<GLfloat> m_constantAttenuation;
    Vasnecov::MutualData<GLfloat> m_linearAttenuation;
    Vasnecov::MutualData<GLfloat> m_quadraticAttenuation;

    GLuint pure_index; // Индекс источника (GL_LIGHT0 + n)

    // На 16-битных системах работоспособность сомнительна.
    // Но с другой стороны, какое может быть 3D на 16 битах
    enum Updated // Изменение данных
    {
        Type			= 0x00200,

        Ambient			= 0x00400,
        Diffuse			= 0x00800,
        Specular		= 0x01000,

        Direction		= 0x02000,
        Exponent		= 0x04000,
        Angle			= 0x08000,
        CosAngle		= 0x10000,

        CAttenuation	= 0x20000,
        LAttenuation	= 0x40000,
        QAttenuation	= 0x80000
    };

    friend class VasnecovWorld;
    friend class VasnecovUniverse;

private:
    Q_DISABLE_COPY(VasnecovLamp)
};

inline void VasnecovLamp::renderSetIndex(GLuint index)
{
    pure_index = index;
}
inline GLuint VasnecovLamp::renderIndex() const
{
    return pure_index;
}
