/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Описание текстур
#ifndef VASNECOVTEXTURE_H
#define VASNECOVTEXTURE_H

#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
#include "types.h"
#ifndef _MSC_VER
    #pragma GCC diagnostic warning "-Weffc++"
#endif

class QImage;

class VasnecovTexture
{
public:
    explicit VasnecovTexture(QImage *image);
    virtual ~VasnecovTexture();
    virtual GLboolean loadImage() = 0; // Загрузка данных из файла

    void setImage(QImage *image);

    GLuint id() const;
    const QImage *image() const;
    GLsizei width() const {return m_width;}
    GLsizei height() const {return m_height;}
    GLboolean isTransparency() const;

protected:
    GLuint m_id;
    QImage *m_image;
    GLsizei m_width, m_height;
    GLboolean m_isTransparency;

private:
    Q_DISABLE_COPY(VasnecovTexture)
};


class VasnecovTextureDiffuse : public VasnecovTexture
{
public:
    explicit VasnecovTextureDiffuse(QImage *image);
    GLboolean loadImage();
};


class VasnecovTextureInterface : public VasnecovTexture
{
public:
    explicit VasnecovTextureInterface(QImage *image);
    GLboolean loadImage();
};

class VasnecovTextureNormal : public VasnecovTextureInterface // Карта нормалей. Когда-нибудь я её наконец-то реализую
{
public:
    explicit VasnecovTextureNormal(QImage *image);
};

//==================================================================================================
inline void VasnecovTexture::setImage(QImage *image)
{
    m_image = image;
}

inline GLuint VasnecovTexture::id() const
{
    return m_id;
}
inline const QImage *VasnecovTexture::image() const
{
    return m_image;
}
inline GLboolean VasnecovTexture::isTransparency() const
{
    return m_isTransparency;
}

#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
#endif // VASNECOVTEXTURE_H
