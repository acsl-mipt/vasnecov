/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "vasnecovlabel.h"
#include <QImage>
#include "vasnecovtexture.h"
#ifndef _MSC_VER
    #pragma GCC diagnostic warning "-Weffc++"
#endif

/*!
 \brief

 \fn VasnecovLabel::VasnecovLabel
 \param mutex
 \param pipeline
 \param name
 \param size
 \param texture
*/
VasnecovLabel::VasnecovLabel(QMutex *mutex, VasnecovPipeline *pipeline, const GLstring &name, const QVector2D &size, VasnecovTexture *texture) :
    VasnecovElement(mutex, pipeline, name),
    m_position(size.x()*0.5, size.y()*0.5),
    m_texturePosition(),

    m_indices(6),
    m_vertices(4),
    m_textures(4),

    m_texture(texture),
    m_personalTexture(false),
    raw_dataLabel(m_texture, size)
{
    if(m_texture)
    {
        if(!updaterCalculateTexturePosition())
        {
            updaterSetUpdateFlag(Zone);
        }
    }

    m_indices[0] = 0;
    m_indices[1] = 1;
    m_indices[2] = 2;
    m_indices[3] = 2;
    m_indices[4] = 3;
    m_indices[5] = 0;
}
/*!
 \brief

 \fn VasnecovLabel::~VasnecovLabel
*/
VasnecovLabel::~VasnecovLabel()
{
    updaterRemoveOldPersonalTexture();
}

/*!
 \brief

 \fn VasnecovLabel::setTextureZone
 \param x
 \param y
 \param width
 \param height
*/
void VasnecovLabel::setTextureZone(GLfloat x, GLfloat y, GLfloat width, GLfloat height)
{
    QMutexLocker locker(mtx_data);

    if(raw_dataLabel.texturePoint != QVector2D(x ,y) ||
       raw_dataLabel.textureZone != QVector2D(width, height))
    {
        if(width == 0)
        {
            width = raw_dataLabel.size.x();
        }
        if(height == 0)
        {
            height = raw_dataLabel.size.y();
        }
        raw_dataLabel.texturePoint.setX(x);
        raw_dataLabel.texturePoint.setY(y);
        raw_dataLabel.textureZone.setX(width);
        raw_dataLabel.textureZone.setY(height);

        updaterSetUpdateFlag(Zone);
    }
}

/*!
 \brief

 \fn VasnecovLabel::setTexture
 \param texture
 \return GLboolean
*/
GLboolean VasnecovLabel::setTexture(VasnecovTexture *texture)
{
    if(texture)
    {
        QMutexLocker locker(mtx_data);

        if(raw_dataLabel.texture != texture)
        {
            raw_dataLabel.texture = texture;
            updaterSetUpdateFlag(Texture);
            return true;
        }
    }
    return false;
}

/*!
 \brief

 \fn VasnecovLabel::setTexture
 \param texture
 \param x
 \param y
 \param width
 \param height
 \return GLboolean
*/
GLboolean VasnecovLabel::setTexture(VasnecovTexture *texture, GLfloat x, GLfloat y, GLfloat width, GLfloat height)
{
    if(setTexture(texture))
    {
        setTextureZone(x, y, width, height);
        return true;
    }
    return false;
}
/*!
 \brief

 \fn VasnecovLabel::setImage
 \param image
 \return GLboolean
*/
GLboolean VasnecovLabel::setImage(const QImage &image)
{
    if(!image.isNull())
    {
        if((image.width() & (image.width() - 1)) == 0 && (image.height() & (image.height() - 1)) == 0)
        {
            // Делается копия картинки на куче (которая удалится сама текстурой после загрузки), чтобы не было проблем с многопоточностью
            QImage *newImage = new QImage();
            *newImage = image.copy(); // Необходимо вызывать именно copy() из-за особенностей копирования QImage

            QMutexLocker locker(mtx_data);

            raw_dataLabel.texture = new VasnecovTextureInterface(newImage);
            updaterSetUpdateFlag(Image);
            return true;
        }
    }
    return false;
}
/*!
 \brief

 \fn VasnecovLabel::setImage
 \param image
 \param x
 \param y
 \param width
 \param height
 \return GLboolean
*/
GLboolean VasnecovLabel::setImage(const QImage &image, GLfloat x, GLfloat y, GLfloat width, GLfloat height)
{
    if(setImage(image))
    {
        setTextureZone(x, y, width, height);
        return true;
    }
    return false;
}

/*!
 \brief Обновляет зону текстуры, которая накладывается на полигон.

  Положение отсчитывается из левой нижней точки текстуры по правой тройке.

*/
bool VasnecovLabel::updaterCalculateTexturePosition()
{
    if(m_texture)
    {
        GLfloat texWidth = m_texture->width();
        GLfloat texHeight = m_texture->height();

        if(!m_texture->id()) // Текстура еще не загрузилась, значит будем пробовать в следующий раз
        {
            return false;
        }

        if(texWidth != 0.0 && texHeight != 0.0)
        {
            GLfloat width(raw_dataLabel.textureZone.x());
            GLfloat height(raw_dataLabel.textureZone.y());

            if(raw_dataLabel.textureZone.isNull())
            {
                // Размеры зоны текстуры по размеру самой метки
                width = raw_dataLabel.size.x();
                height = raw_dataLabel.size.y();
            }

            m_texturePosition[0].setX(raw_dataLabel.texturePoint.x()/texWidth);
            m_texturePosition[0].setY((texHeight-(raw_dataLabel.texturePoint.y() + height))/texHeight);
            m_texturePosition[1].setY((texHeight-raw_dataLabel.texturePoint.y())/texHeight);
            m_texturePosition[1].setX((raw_dataLabel.texturePoint.x() + width)/texWidth);

            m_vertices[0] = QVector3D(-m_position.x(), -m_position.y(), 0.0f);
            m_vertices[1] = QVector3D(m_position.x(), -m_position.y(), 0.0f);
            m_vertices[2] = QVector3D(m_position.x(), m_position.y(), 0.0f);
            m_vertices[3] = QVector3D(-m_position.x(), m_position.y(), 0.0f);

            m_textures[0] = QVector2D(m_texturePosition[0].x(), m_texturePosition[1].y());
            m_textures[1] = QVector2D(m_texturePosition[1].x(), m_texturePosition[1].y());
            m_textures[2] = QVector2D(m_texturePosition[1].x(), m_texturePosition[0].y());
            m_textures[3] = QVector2D(m_texturePosition[0].x(), m_texturePosition[0].y());
        }
    }

    return true;
}

/*!
 \brief

 \fn VasnecovLabel::renderUpdateData
 \return GLenum
*/
GLenum VasnecovLabel::renderUpdateData()
{
    GLenum updated(false);
    GLboolean ok(true);

    if(raw_wasUpdated)
    {
        pure_pipeline->setSomethingWasUpdated();

        if(updaterIsUpdateFlag(Image))
        {
            // Текстура на куче, загруженная непосредственно в Метку
            // Сначала удаляется старая текстура
            updaterRemoveOldPersonalTexture();

            m_texture = raw_dataLabel.texture;

            m_texture->loadImage();
            m_personalTexture = true;

            ok = updaterCalculateTexturePosition();

            updated |= Image;
        }
        if(updaterIsUpdateFlag(Texture))
        {
            updaterRemoveOldPersonalTexture();

            // Текстура из списка в Universe
            m_texture = raw_dataLabel.texture;

            ok = updaterCalculateTexturePosition();

            updated |= Texture;
        }
        if(updaterIsUpdateFlag(Size))
        {
            // Полигон метки рисуется из центра
            m_position.setX(raw_dataLabel.size.x()*0.5);
            m_position.setY(raw_dataLabel.size.y()*0.5);

            ok = updaterCalculateTexturePosition();

            updated |= Size;
        }
        if(updaterIsUpdateFlag(Zone))
        {
            ok =updaterCalculateTexturePosition();

            updated |= Zone;
        }

        updated |= VasnecovElement::renderUpdateData();
    }

    if(!ok)
    {
        updaterSetUpdateFlag(Zone);
    }

    return updated;
}
/*!
 \brief

 \fn VasnecovLabel::renderDraw
*/
void VasnecovLabel::renderDraw()
{
    if(!m_isHidden.pure() && m_texture)
    {
        // Позиционирование
        if(m_alienMs.pure())
        {
            pure_pipeline->setMatrixOrtho2D(m_Ms.pure() * (*m_alienMs.pure()));
        }
        else
        {
            pure_pipeline->setMatrixOrtho2D(m_Ms.pure());
        }

        // Растровая часть
        pure_pipeline->setColor(m_color.pure());
        pure_pipeline->enableTexture2D(m_texture->id());

        pure_pipeline->drawElements(VasnecovPipeline::Triangles, &m_indices, &m_vertices, 0, &m_textures);
    }
}

void VasnecovLabel::updaterRemoveOldPersonalTexture()
{
    if(m_personalTexture)
    {
        delete m_texture;
        m_texture = 0;
        m_personalTexture = false;
    }
}

#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
