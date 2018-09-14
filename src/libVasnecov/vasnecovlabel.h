/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Метка. Принцип билборда
#ifndef VASNECOVLABEL_H
#define VASNECOVLABEL_H

#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
#include <QVector2D>
#include "vasnecovelement.h"
#ifndef _MSC_VER
    #pragma GCC diagnostic warning "-Weffc++"
#endif

class VasnecovTexture;

namespace Vasnecov
{
    struct LabelAttributes
    {
        QVector2D size;
        QVector2D texturePoint, textureZone; // поцизия на текстуре, размер зоны
        VasnecovTexture* texture;

        LabelAttributes(VasnecovTexture* p_texture, QVector2D p_size) :
            size(p_size),
            texturePoint(), textureZone(p_size),
            texture(p_texture)
        {
        }

    private:
        Q_DISABLE_COPY(LabelAttributes)
    };
}

class VasnecovLabel : public VasnecovElement
{
public:
    VasnecovLabel(VasnecovPipeline* pipeline,
                  const std::string& name,
                  const QVector2D& size,
                  VasnecovTexture* texture = nullptr);
    ~VasnecovLabel();

    void setSize(GLfloat width, GLfloat height);
    // Задаёт позицию метки на текстуре. Для случаев, когда от текстуры используется только кусочек
    void setTextureZone(GLfloat x, GLfloat y, GLfloat width = 0.0, GLfloat height = 0.0);

    GLboolean setTexture(VasnecovTexture* texture);
    GLboolean setTexture(VasnecovTexture* texture, GLfloat x, GLfloat y, GLfloat width = 0.0, GLfloat height = 0.0);
    GLboolean setImage(const QImage& image);
    GLboolean setImage(const QImage& image, GLfloat x, GLfloat y, GLfloat width = 0.0, GLfloat height = 0.0);

protected:
    VasnecovTexture* designerTexture() const {return raw_dataLabel.texture;}

protected:
    // Методы, которые вызываются на этапе обновления данных, т.е. имеют доступ и к сырым, и к нормальным
    bool updaterCalculateTexturePosition();
    void updaterRemoveOldPersonalTexture();

protected:
    GLenum renderUpdateData();
    void renderDraw();

    VasnecovTexture* texture() const {return m_texture;}

protected:
    QVector2D m_position;
    QVector2D m_texturePosition[2]; // координата прямоугольника на текстуре

    std::vector<GLuint> m_indices;
    std::vector<QVector3D> m_vertices;
    std::vector<QVector2D> m_textures;

    VasnecovTexture* m_texture;
    bool m_personalTexture; // Текстура создается Меткой сама только для себя

    Vasnecov::LabelAttributes raw_dataLabel;

    enum Updated// Изменение данных
    {
        Image		= 0x0200,
        Texture		= 0x0400,
        Size		= 0x0800,
        Zone		= 0x1000
    };

    friend class VasnecovWorld;
    friend class VasnecovUniverse;

private:
    Q_DISABLE_COPY(VasnecovLabel)
};


inline void VasnecovLabel::setSize(GLfloat width, GLfloat height)
{
    raw_dataLabel.size.setX(width);
    raw_dataLabel.size.setY(height);
}



#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
#endif // VASNECOVLABEL_H
