/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "VasnecovLabel.h"
#include "VasnecovTexture.h"
#include <QImage>

VasnecovLabel::VasnecovLabel(VasnecovPipeline *pipeline, const QString& name, const QVector2D &size, VasnecovTexture* texture) :
    VasnecovElement(pipeline, name),
    m_position(size.x() * 0.5f, size.y() * 0.5f),
    m_texturePosition(),

    m_indices(6),
    m_vertices(4),
    m_textures(4),

    m_texture(texture),
    m_personalTexture(false),
    raw_dataLabel(m_texture, size),
    m_offset(raw_wasUpdated, Offset, QVector2D())
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
VasnecovLabel::~VasnecovLabel()
{
    updaterRemoveOldPersonalTexture();
}

QSizeF VasnecovLabel::size() const
{
    return QSize(raw_dataLabel.size.x(), raw_dataLabel.size.y());
}
void VasnecovLabel::setTextureZone(GLfloat x, GLfloat y, GLfloat width, GLfloat height)
{
    if(raw_dataLabel.texturePoint != QVector2D(x ,y) ||
       raw_dataLabel.textureZone != QVector2D(width, height))
    {
        if(width == 0.0f)
        {
            width = raw_dataLabel.size.x();
        }
        if(height == 0.0f)
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
GLboolean VasnecovLabel::setTexture(VasnecovTexture *texture)
{
    if(texture)
    {
        if(raw_dataLabel.texture != texture)
        {
            raw_dataLabel.texture = texture;
            updaterSetUpdateFlag(Texture);
            return true;
        }
    }
    return false;
}
GLboolean VasnecovLabel::setTexture(VasnecovTexture *texture, GLfloat x, GLfloat y, GLfloat width, GLfloat height)
{
    if(setTexture(texture))
    {
        setTextureZone(x, y, width, height);
        return true;
    }
    return false;
}
GLboolean VasnecovLabel::setImage(const QImage &image)
{
    if(!image.isNull())
    {
        if((image.width() & (image.width() - 1)) == 0 && (image.height() & (image.height() - 1)) == 0)
        {
            // Делается копия картинки на куче (которая удалится сама текстурой после загрузки), чтобы не было проблем с многопоточностью
            QImage newImage = image.copy(); // Необходимо вызывать именно copy() из-за особенностей копирования QImage

            raw_dataLabel.texture = new VasnecovTextureInterface(newImage);
            updaterSetUpdateFlag(Image);
            return true;
        }
    }
    return false;
}
GLboolean VasnecovLabel::setImage(const QImage &image, GLfloat x, GLfloat y, GLfloat width, GLfloat height)
{
    if(setImage(image))
    {
        setTextureZone(x, y, width, height);
        return true;
    }
    return false;
}

void VasnecovLabel::setOffset(const QVector2D& offset)
{
    m_offset.set(offset);
}

void VasnecovLabel::setOffset(const QPointF& offset)
{
    m_offset.set(QVector2D(offset));
}

void VasnecovLabel::setOffset(GLfloat x, GLfloat y)
{
    m_offset.set(QVector2D(x, y));
}

QVector2D VasnecovLabel::offset() const
{
    return m_offset.raw();
}
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

        if(texWidth != 0.0f && texHeight != 0.0f)
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
            m_position.setX(raw_dataLabel.size.x() * 0.5f);
            m_position.setY(raw_dataLabel.size.y() * 0.5f);

            ok = updaterCalculateTexturePosition();

            updated |= Size;
        }
        if(updaterIsUpdateFlag(Zone))
        {
            ok = updaterCalculateTexturePosition();

            updated |= Zone;
        }

        updated |= m_offset.update();

        updated |= VasnecovElement::renderUpdateData();
    }

    if(!ok)
    {
        updaterSetUpdateFlag(Zone);
    }

    return updated;
}
void VasnecovLabel::renderDraw()
{
    if(!m_isHidden.pure() && m_texture)
    {
        // Позиционирование
        if(m_alienMs.pure())
        {
            pure_pipeline->setMatrixOrtho2D(m_Ms.pure() * (*m_alienMs.pure()), m_offset.pure());
        }
        else
        {
            pure_pipeline->setMatrixOrtho2D(m_Ms.pure(), m_offset.pure());
        }

        // Растровая часть
        pure_pipeline->setColor(m_color.pure());
        pure_pipeline->enableTexture2D(m_texture->id());

        pure_pipeline->drawElements(VasnecovPipeline::Triangles, &m_indices, &m_vertices, nullptr, &m_textures);
    }
}

void VasnecovLabel::updaterRemoveOldPersonalTexture()
{
    if(m_personalTexture)
    {
        delete m_texture;
        m_texture = nullptr;
        m_personalTexture = false;
    }
}
