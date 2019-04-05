/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "VasnecovElement.h"

class VasnecovTerrain : public VasnecovElement
{
public:
    enum Types
    {
        TypeMesh = 0,
        TypeSurface
    };

    VasnecovTerrain(VasnecovPipeline* pipeline, const QString& name = QString());
    ~VasnecovTerrain();

    void setPoints(std::vector<QVector3D>&& points, std::vector<QVector3D>&& colors = std::vector<QVector3D>());
    void clearPoints();
    bool isEmpty() const;
    GLuint pointsAmount() const;

    void setType(Types type);
    Types type() const {return _type;}

    void enableImage(bool enable = true);
    void disableImage(bool disable = true);
    bool isImageEnabled() const {return _isTextureEnabled;}

    void setImage(const QImage& image);
    void loadImage();
    void setImageZone(GLfloat x, GLfloat y, GLfloat width, GLfloat height);

protected:
    void renderDraw();

private:
    void updateCornerPoints();
    void updateNormals();
    void updateTextures();
    void updateIndices();

    Types                               _type;
    std::vector<QVector3D>              _points;
    std::vector<QVector3D>              _colors;
    std::vector<std::vector<GLuint>>    _indices;
    std::vector<QVector3D>              _normals;
    std::vector<QVector2D>              _textures;
    GLuint                              _lineSize;

    VasnecovTexture*                    _texture;
    QRectF                              _textureZone;
    bool                                _isTextureEnabled;

    enum Updated
    {
        Type		= 0x0200,
        Points		= 0x0400,
        Thickness	= 0x0800,
        Lighting	= 0x1000,
        Depth		= 0x2000,
        Image       = 0x4000,
        Zone        = 0x8000,
    };

    friend class VasnecovUniverse;
    friend class VasnecovWorld;

private:
    Q_DISABLE_COPY(VasnecovTerrain)
};
