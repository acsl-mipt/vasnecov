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

#include "vasnecovelement.h"

namespace Vasnecov
{
    // Модель характеристик освещения
    class LightModel
    {
    public:
        LightModel() :
            m_ambientColor(51, 51, 51, 255)
        {
            m_ambientColor.setRgbF(0.2, 0.2, 0.2, 1.0);
        }
        void setAmbientColor(const QColor& color)
        {
            m_ambientColor = color;
        }
        QColor ambientColor() const
        {
            return m_ambientColor;
        }

    protected:
        QColor m_ambientColor;
    };
}

class VasnecovLamp : public VasnecovAbstractElement
{
public:
    enum LampTypes
    {
        LampTypeCelestial = 1,
        LampTypeSpot,
        LampTypeHeadlight
    };

public:
    VasnecovLamp(VasnecovPipeline* pipeline, const QString& name, VasnecovLamp::LampTypes type, GLuint index);

public:
    void setType(LampTypes type);

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
    LampTypes m_type;

    QColor m_ambientColor;
    QColor m_diffuseColor;
    QColor m_specularColor;

    QVector3D m_spotDirection;
    GLfloat m_spotExponent;
    GLfloat m_spotAngle;
    GLfloat m_spotCosAngle;

    GLfloat m_constantAttenuation;
    GLfloat m_linearAttenuation;
    GLfloat m_quadraticAttenuation;

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
