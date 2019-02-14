/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Описание текстур
#pragma once

#include "Types.h"

#include <QImage>

class VasnecovTexture
{
public:
    explicit VasnecovTexture(const QImage& image);
    virtual ~VasnecovTexture();
    virtual GLboolean loadImage() = 0; // Загрузка данных из файла

    void setImage(const QImage& image);

    GLuint id() const;
    const QImage& image() const;
    GLsizei width() const {return m_width;}
    GLsizei height() const {return m_height;}
    GLboolean isTransparency() const;

protected:
    GLuint m_id;
    QImage m_image;
    GLsizei m_width, m_height;
    GLboolean m_isTransparency;

private:
    Q_DISABLE_COPY(VasnecovTexture)
};


class VasnecovTextureDiffuse : public VasnecovTexture
{
public:
    explicit VasnecovTextureDiffuse(const QImage& image);
    GLboolean loadImage();
};


class VasnecovTextureInterface : public VasnecovTexture
{
public:
    explicit VasnecovTextureInterface(const QImage& image);
    GLboolean loadImage();
};

class VasnecovTextureNormal : public VasnecovTextureInterface // Карта нормалей. Когда-нибудь я её наконец-то реализую
{
public:
    explicit VasnecovTextureNormal(const QImage& image);
};

//==================================================================================================
inline void VasnecovTexture::setImage(const QImage& image)
{
    m_image = image;
}

inline GLuint VasnecovTexture::id() const
{
    return m_id;
}
inline const QImage& VasnecovTexture::image() const
{
    return m_image;
}
inline GLboolean VasnecovTexture::isTransparency() const
{
    return m_isTransparency;
}
