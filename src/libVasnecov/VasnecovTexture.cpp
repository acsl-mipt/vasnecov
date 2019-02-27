/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "VasnecovTexture.h"
#include <QImage>
#include <GL/glu.h>
#include "Technologist.h"

VasnecovTexture::VasnecovTexture(const QImage& image):
    m_id(0),
    m_image(image),
    m_width(0),	m_height(0),
    m_isTransparency(false)
{
}
VasnecovTexture::~VasnecovTexture()
{
    if(m_id)
    {
        glDeleteTextures(1, &m_id);
    }
}
VasnecovTextureDiffuse::VasnecovTextureDiffuse(const QImage& image) :
    VasnecovTexture(image)
{
}
GLboolean VasnecovTextureDiffuse::loadImage()
{
    if(!m_image.isNull())
    {
        m_width = m_image.width();
        m_height = m_image.height();

        // Создание и инициализация текстуры
        glGenTextures(1, &m_id);
        glBindTexture(GL_TEXTURE_2D, m_id);

        if(m_image.hasAlphaChannel())
        {
            m_isTransparency = true;
            gluBuild2DMipmaps(GL_TEXTURE_2D, 4, m_width, m_height, GL_BGRA_EXT, GL_UNSIGNED_BYTE, m_image.bits());
        }
        else
        {
            gluBuild2DMipmaps(GL_TEXTURE_2D, 3, m_width, m_height, GL_BGRA_EXT, GL_UNSIGNED_BYTE, m_image.bits());
        }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        m_image = QImage();

        return static_cast<GLboolean>(m_id);
    }

    Vasnecov::problem("Can't load texture");
    return 0;
}
VasnecovTextureInterface::VasnecovTextureInterface(const QImage& image) :
    VasnecovTexture(image)
{
}
GLboolean VasnecovTextureInterface::loadImage()
{
    if(!m_image.isNull())
    {
        m_width = m_image.width();
        m_height = m_image.height();

        // Создание и инициализация текстуры
        glGenTextures(1, &m_id);
        glBindTexture(GL_TEXTURE_2D, m_id);

        if(m_image.hasAlphaChannel())
        {
            m_isTransparency = true;
            glTexImage2D(GL_TEXTURE_2D, 0, 4, m_width, m_height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, m_image.bits());
        }
        else
        {
            glTexImage2D(GL_TEXTURE_2D, 0, 3, m_width, m_height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, m_image.bits());
        }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // После загрузки класс сам удаляет более ненужный QImage
        m_image = QImage();

        return static_cast<GLboolean>(m_id);
    }

    Vasnecov::problem("Can't load texture");
    return 0;
}
VasnecovTextureNormal::VasnecovTextureNormal(const QImage& image) :
    VasnecovTextureInterface(image)
{
}
