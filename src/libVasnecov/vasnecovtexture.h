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

#include <QImage>

class VasnecovTexture
{
public:
    explicit VasnecovTexture(const QImage& image);
    virtual ~VasnecovTexture();
    virtual GLboolean loadImage() = 0; // Загрузка данных из файла

    void setImage(const QImage& image) { m_image = image; };
    GLuint id() const { return m_id; }
    const QImage& image() const { return m_image; };
    GLsizei width() const {return m_width;}
    GLsizei height() const {return m_height;}
    GLboolean isTransparency() const { return m_isTransparency; }

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
    GLboolean loadImage() override;
};


class VasnecovTextureInterface : public VasnecovTexture
{
public:
    explicit VasnecovTextureInterface(const QImage& image);
    GLboolean loadImage() override;
};

class VasnecovTextureNormal : public VasnecovTextureInterface // Карта нормалей. Когда-нибудь я её наконец-то реализую
{
public:
    explicit VasnecovTextureNormal(const QImage& image);
};


#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
#endif // VASNECOVTEXTURE_H
