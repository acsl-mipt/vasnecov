/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "VasnecovLamp.h"

VasnecovLamp::VasnecovLamp(VasnecovPipeline* pipeline, const QString& name, Vasnecov::LampTypes type, GLuint index) :
    VasnecovAbstractElement(pipeline, name),
    m_type(raw_wasUpdated, Type, type),

    m_ambientColor(raw_wasUpdated, Ambient, QColor(0, 0, 0, 255)),
    m_diffuseColor(raw_wasUpdated, Diffuse, QColor(255, 255, 255, 255)),
    m_specularColor(raw_wasUpdated, Specular, QColor(255, 255, 255, 255)),

    m_spotDirection(raw_wasUpdated, Direction, QVector3D(0.0f, 0.0f, -1.0f)),
    m_spotExponent(raw_wasUpdated, Exponent, 0.0f),
    m_spotAngle(raw_wasUpdated, Angle, 180.0f),
    m_spotCosAngle(raw_wasUpdated, CosAngle, -1.0f),

    m_constantAttenuation(raw_wasUpdated, CAttenuation, 1.0f),
    m_linearAttenuation(raw_wasUpdated, LAttenuation, 0.0f),
    m_quadraticAttenuation(raw_wasUpdated, QAttenuation, 0.0f),

    pure_index(index)
{
    if(type == Vasnecov::LampTypeCelestial)
    {
        QVector3D v(0.0, 0.0, 1.0);
        raw_coordinates = v;
        designerUpdateMatrixMs();
    }
}

VasnecovLamp::~VasnecovLamp()
{}
void VasnecovLamp::setType(Vasnecov::LampTypes type)
{
    m_type.set(type);
}
void VasnecovLamp::setCelestialDirection(const QVector3D &direction)
{
    setType(Vasnecov::LampTypeCelestial);
    setCoordinates(direction);
}
void VasnecovLamp::setCelestialDirection(GLfloat x, GLfloat y, GLfloat z)
{
    setType(Vasnecov::LampTypeCelestial);
    setCoordinates(x, y, z);
}
void VasnecovLamp::setAmbientColor(const QColor &color)
{
    m_ambientColor.set(color);
}
void VasnecovLamp::setDiffuseColor(const QColor &color)
{
    m_diffuseColor.set(color);
}
void VasnecovLamp::setSpecularColor(const QColor &color)
{
    m_specularColor.set(color);
}
void VasnecovLamp::setSpotDirection(const QVector3D &direction)
{
    if(m_spotDirection.set(direction))
    {
        m_type.set(Vasnecov::LampTypeHeadlight);
    }
}
void VasnecovLamp::setSpotExponent(GLfloat exponent)
{
    if(m_spotExponent.set(exponent))
    {
        m_type.set(Vasnecov::LampTypeHeadlight);
    }
}
void VasnecovLamp::setSpotAngle(GLfloat angle)
{
    if(m_spotAngle.set(angle))
    {
        if(angle != 180.0f)
        {
            m_spotCosAngle.set(cos(angle * c_degToRad));
        }
        else
        {
            m_spotCosAngle.set(-1.0f);
        }

        m_type.set(Vasnecov::LampTypeHeadlight);
    }
}
void VasnecovLamp::setConstantAttenuation(GLfloat attenuation)
{
    m_constantAttenuation.set(attenuation);
}
void VasnecovLamp::setLinearAttenuation(GLfloat attenuation)
{
    m_linearAttenuation.set(attenuation);
}
void VasnecovLamp::setQuadraticAttenuation(GLfloat attenuation)
{
    m_quadraticAttenuation.set(attenuation);
}
GLenum VasnecovLamp::renderUpdateData()
{
    GLenum updated(raw_wasUpdated);

    if(raw_wasUpdated)
    {
        pure_pipeline->setSomethingWasUpdated();

        // Копирование сырых данных в основные
        m_type.update();

        m_ambientColor.update();
        m_diffuseColor.update();
        m_specularColor.update();

        m_spotDirection.update();
        m_spotExponent.update();
        m_spotAngle.update();
        m_spotCosAngle.update();

        m_constantAttenuation.update();
        m_linearAttenuation.update();
        m_quadraticAttenuation.update();

        VasnecovAbstractElement::renderUpdateData();
    }

    return updated;
}
void VasnecovLamp::renderDraw()
{
    if(!m_isHidden.pure())
    {
        pure_pipeline->enableConcreteLamp(pure_index);

        GLfloat params[4];

        params[0] = m_ambientColor.pure().redF();
        params[1] = m_ambientColor.pure().greenF();
        params[2] = m_ambientColor.pure().blueF();
        params[3] = m_ambientColor.pure().alphaF();
        glLightfv(pure_index, GL_AMBIENT, params);

        params[0] = m_diffuseColor.pure().redF();
        params[1] = m_diffuseColor.pure().greenF();
        params[2] = m_diffuseColor.pure().blueF();
        params[3] = m_diffuseColor.pure().alphaF();
        glLightfv(pure_index, GL_DIFFUSE, params);

        params[0] = m_specularColor.pure().redF();
        params[1] = m_specularColor.pure().greenF();
        params[2] = m_specularColor.pure().blueF();
        params[3] = m_specularColor.pure().alphaF();
        glLightfv(pure_index, GL_SPECULAR, params);


        renderApplyTranslation();

        if(m_type.pure() == Vasnecov::LampTypeCelestial)
        {
            // Для направленного источника чужая матрица задаёт углы, направление же считывается со своей
            params[0] = m_Ms.pure()(0, 3);
            params[1] = m_Ms.pure()(1, 3);
            params[2] = m_Ms.pure()(2, 3);
            params[3] = 0;
            glLightfv(pure_index, GL_POSITION, params);
        }
        else if(m_type.pure() == Vasnecov::LampTypeSpot ||
                m_type.pure() == Vasnecov::LampTypeHeadlight)
        {
            params[0] = 0;
            params[1] = 0;
            params[2] = 0;
            params[3] = 1.0;
            glLightfv(pure_index, GL_POSITION, params);

            params[0] = m_constantAttenuation.pure();
            glLightfv(pure_index, GL_CONSTANT_ATTENUATION, params);

            params[0] = m_linearAttenuation.pure();
            glLightfv(pure_index, GL_LINEAR_ATTENUATION, params);

            params[0] = m_quadraticAttenuation.pure();
            glLightfv(pure_index, GL_QUADRATIC_ATTENUATION, params);

            if(m_type.pure() == Vasnecov::LampTypeHeadlight)
            {
                params[0] = m_spotDirection.pure().x();
                params[1] = m_spotDirection.pure().y();
                params[2] = m_spotDirection.pure().z();
                glLightfv(pure_index, GL_SPOT_DIRECTION, params);

                params[0] = m_spotExponent.pure();
                glLightfv(pure_index, GL_SPOT_EXPONENT, params);

                params[0] = m_spotAngle.pure();
                glLightfv(pure_index, GL_SPOT_CUTOFF, params);
            }
        }
    }
    else
    {
        pure_pipeline->disableConcreteLamp(pure_index);
    }
}
