/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "vasnecovlamp.h"

/*!
 \brief

 \fn VasnecovLamp::VasnecovLamp
 \param pipeline
 \param name
 \param type
 \param index
*/
VasnecovLamp::VasnecovLamp(VasnecovPipeline* pipeline, const QString& name, VasnecovLamp::LampTypes type, GLuint index) :
    VasnecovAbstractElement(pipeline, name),
    m_type(type),

    m_ambientColor(0, 0, 0, 255),
    m_diffuseColor(255, 255, 255, 255),
    m_specularColor(255, 255, 255, 255),

    m_spotDirection(0.0f, 0.0f, -1.0f),
    m_spotExponent(0.0f),
    m_spotAngle(180.0f),
    m_spotCosAngle(-1.0f),

    m_constantAttenuation(1.0f),
    m_linearAttenuation(0.0f),
    m_quadraticAttenuation(0.0f),

    pure_index(index)
{
    if(type == LampTypeCelestial)
    {
        QVector3D v(0.0, 0.0, 1.0);
        raw_coordinates = v;
        designerUpdateMatrixMs();
    }
}

/*!
 \brief

 \fn VasnecovLamp::setType
 \param type
*/
void VasnecovLamp::setType(LampTypes type)
{
    m_type = type;
}

/*!
 \brief

 \fn VasnecovLamp::setCelestialDirection
 \param direction
*/
void VasnecovLamp::setCelestialDirection(const QVector3D &direction)
{
    setType(LampTypeCelestial);
    setCoordinates(direction);
}
/*!
 \brief

 \fn VasnecovLamp::setCelestialDirection
 \param x
 \param y
 \param z
*/
void VasnecovLamp::setCelestialDirection(GLfloat x, GLfloat y, GLfloat z)
{
    setType(LampTypeCelestial);
    setCoordinates(x, y, z);
}

/*!
 \brief

 \fn VasnecovLamp::setAmbientColor
 \param color
*/
void VasnecovLamp::setAmbientColor(const QColor &color)
{
    m_ambientColor = color;
}

/*!
 \brief

 \fn VasnecovLamp::setDiffuseColor
 \param color
*/
void VasnecovLamp::setDiffuseColor(const QColor &color)
{
    m_diffuseColor = color;
}

/*!
 \brief

 \fn VasnecovLamp::setSpecularColor
 \param color
*/
void VasnecovLamp::setSpecularColor(const QColor &color)
{
    m_specularColor = color;
}

/*!
 \brief

 \fn VasnecovLamp::setSpotDirection
 \param direction
*/
void VasnecovLamp::setSpotDirection(const QVector3D &direction)
{
    if(m_spotDirection != direction)
    {
        m_spotDirection = direction;
        m_type = LampTypeHeadlight;
    }
}

/*!
 \brief

 \fn VasnecovLamp::setSpotExponent
 \param exponent
*/
void VasnecovLamp::setSpotExponent(GLfloat exponent)
{
    if(m_spotExponent != exponent)
    {
        m_spotExponent = exponent;
        m_type = LampTypeHeadlight;
    }
}

/*!
 \brief

 \fn VasnecovLamp::setSpotAngle
 \param angle
*/
void VasnecovLamp::setSpotAngle(GLfloat angle)
{
    if (m_spotAngle == angle)
        return;
    m_spotAngle = angle;

    if(angle != 180.0f)
    {
        m_spotCosAngle = cos(angle * c_degToRad);
    }
    else
    {
        m_spotCosAngle = -1.0f;
    }

    m_type = LampTypeHeadlight;
}

/*!
 \brief

 \fn VasnecovLamp::setConstantAttenuation
 \param attenuation
*/
void VasnecovLamp::setConstantAttenuation(GLfloat attenuation)
{
    m_constantAttenuation = attenuation;
}

/*!
 \brief

 \fn VasnecovLamp::setLinearAttenuation
 \param attenuation
*/
void VasnecovLamp::setLinearAttenuation(GLfloat attenuation)
{
    m_linearAttenuation = attenuation;
}

/*!
 \brief

 \fn VasnecovLamp::setQuadraticAttenuation
 \param attenuation
*/
void VasnecovLamp::setQuadraticAttenuation(GLfloat attenuation)
{
    m_quadraticAttenuation = attenuation;
}
/*!
 \brief

 \fn VasnecovLamp::renderUpdateData
 \return GLenum
*/
GLenum VasnecovLamp::renderUpdateData()
{
    GLenum updated = raw_wasUpdated;

    if(raw_wasUpdated)
    {
        pure_pipeline->setSomethingWasUpdated();
        VasnecovAbstractElement::renderUpdateData();
    }

    return updated;
}

/*!
 \brief

 \fn VasnecovLamp::renderDraw
*/
void VasnecovLamp::renderDraw()
{
    if(!m_isHidden)
    {
        pure_pipeline->enableConcreteLamp(pure_index);

        GLfloat params[4];

        params[0] = m_ambientColor.redF();
        params[1] = m_ambientColor.greenF();
        params[2] = m_ambientColor.blueF();
        params[3] = m_ambientColor.alphaF();
        glLightfv(pure_index, GL_AMBIENT, params);

        params[0] = m_diffuseColor.redF();
        params[1] = m_diffuseColor.greenF();
        params[2] = m_diffuseColor.blueF();
        params[3] = m_diffuseColor.alphaF();
        glLightfv(pure_index, GL_DIFFUSE, params);

        params[0] = m_specularColor.redF();
        params[1] = m_specularColor.greenF();
        params[2] = m_specularColor.blueF();
        params[3] = m_specularColor.alphaF();
        glLightfv(pure_index, GL_SPECULAR, params);


        renderApplyTranslation();

        if(m_type == LampTypeCelestial)
        {
            // Для направленного источника чужая матрица задаёт углы, направление же считывается со своей
            params[0] = m_Ms(0, 3);
            params[1] = m_Ms(1, 3);
            params[2] = m_Ms(2, 3);
            params[3] = 0;
            glLightfv(pure_index, GL_POSITION, params);
        }
        else if(m_type == LampTypeSpot ||
                m_type == LampTypeHeadlight)
        {
            params[0] = 0;
            params[1] = 0;
            params[2] = 0;
            params[3] = 1.0;
            glLightfv(pure_index, GL_POSITION, params);

            params[0] = m_constantAttenuation;
            glLightfv(pure_index, GL_CONSTANT_ATTENUATION, params);

            params[0] = m_linearAttenuation;
            glLightfv(pure_index, GL_LINEAR_ATTENUATION, params);

            params[0] = m_quadraticAttenuation;
            glLightfv(pure_index, GL_QUADRATIC_ATTENUATION, params);

            if(m_type == LampTypeHeadlight)
            {
                params[0] = m_spotDirection.x();
                params[1] = m_spotDirection.y();
                params[2] = m_spotDirection.z();
                glLightfv(pure_index, GL_SPOT_DIRECTION, params);

                params[0] = m_spotExponent;
                glLightfv(pure_index, GL_SPOT_EXPONENT, params);

                params[0] = m_spotAngle;
                glLightfv(pure_index, GL_SPOT_CUTOFF, params);
            }
        }
    }
    else
    {
        pure_pipeline->disableConcreteLamp(pure_index);
    }
}
