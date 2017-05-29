/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "vasnecovlamp.h"
#ifndef _MSC_VER
    #pragma GCC diagnostic warning "-Weffc++"
#endif

/*!
 \brief

 \fn VasnecovLamp::VasnecovLamp
 \param mutex
 \param pipeline
 \param name
 \param type
 \param index
*/
VasnecovLamp::VasnecovLamp(QMutex *mutex, VasnecovPipeline *pipeline, const std::string &name, VasnecovLamp::LampTypes type, GLuint index) :
    VasnecovAbstractElement(mutex, pipeline, name),
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
    QMutexLocker locker(mtx_data);

    m_type.set(type);
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
    QMutexLocker locker(mtx_data);

    m_ambientColor.set(color);
}

/*!
 \brief

 \fn VasnecovLamp::setDiffuseColor
 \param color
*/
void VasnecovLamp::setDiffuseColor(const QColor &color)
{
    QMutexLocker locker(mtx_data);

    m_diffuseColor.set(color);
}

/*!
 \brief

 \fn VasnecovLamp::setSpecularColor
 \param color
*/
void VasnecovLamp::setSpecularColor(const QColor &color)
{
    QMutexLocker locker(mtx_data);

    m_specularColor.set(color);
}

/*!
 \brief

 \fn VasnecovLamp::setSpotDirection
 \param direction
*/
void VasnecovLamp::setSpotDirection(const QVector3D &direction)
{
    QMutexLocker locker(mtx_data);

    if(m_spotDirection.set(direction))
    {
        m_type.set(LampTypeHeadlight);
    }
}

/*!
 \brief

 \fn VasnecovLamp::setSpotExponent
 \param exponent
*/
void VasnecovLamp::setSpotExponent(GLfloat exponent)
{
    QMutexLocker locker(mtx_data);

    if(m_spotExponent.set(exponent))
    {
        m_type.set(LampTypeHeadlight);
    }
}

/*!
 \brief

 \fn VasnecovLamp::setSpotAngle
 \param angle
*/
void VasnecovLamp::setSpotAngle(GLfloat angle)
{
    QMutexLocker locker(mtx_data);

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

        m_type.set(LampTypeHeadlight);
    }
}

/*!
 \brief

 \fn VasnecovLamp::setConstantAttenuation
 \param attenuation
*/
void VasnecovLamp::setConstantAttenuation(GLfloat attenuation)
{
    QMutexLocker locker(mtx_data);

    m_constantAttenuation.set(attenuation);
}

/*!
 \brief

 \fn VasnecovLamp::setLinearAttenuation
 \param attenuation
*/
void VasnecovLamp::setLinearAttenuation(GLfloat attenuation)
{
    QMutexLocker locker(mtx_data);

    m_linearAttenuation.set(attenuation);
}

/*!
 \brief

 \fn VasnecovLamp::setQuadraticAttenuation
 \param attenuation
*/
void VasnecovLamp::setQuadraticAttenuation(GLfloat attenuation)
{
    QMutexLocker locker(mtx_data);

    m_quadraticAttenuation.set(attenuation);
}
/*!
 \brief

 \fn VasnecovLamp::renderUpdateData
 \return GLenum
*/
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

/*!
 \brief

 \fn VasnecovLamp::renderDraw
*/
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

        if(m_type.pure() == LampTypeCelestial)
        {
            // Для направленного источника чужая матрица задаёт углы, направление же считывается со своей
            params[0] = m_Ms.pure()(0, 3);
            params[1] = m_Ms.pure()(1, 3);
            params[2] = m_Ms.pure()(2, 3);
            params[3] = 0;
            glLightfv(pure_index, GL_POSITION, params);
        }
        else if(m_type.pure() == LampTypeSpot ||
                m_type.pure() == LampTypeHeadlight)
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

            if(m_type.pure() == LampTypeHeadlight)
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

#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
