/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "Technologist.h"
#include "VasnecovTerrain.h"
#include "VasnecovTexture.h"
#include <QFile>
#include <QSize>
#include <QImage>
#include <QVector2D>

#include <cmath>

VasnecovTerrain::VasnecovTerrain(VasnecovPipeline *pipeline, const QString& name)
    : VasnecovElement(pipeline, name)
    , _type(TypeSurface)
    , _lineSize(0)
    , _texture(new VasnecovTextureDiffuse(QImage()))
    , _textureZone(0.0, 0.0, 1.0, 1.0)
    , _isTextureEnabled(false)
{}
VasnecovTerrain::~VasnecovTerrain()
{
    delete _texture;
}

void VasnecovTerrain::setPoints(std::vector <QVector3D>&& points, std::vector<QVector3D>&& colors)
{
    _points = std::move(points);
    if(colors.size() == _points.size())
        _colors = std::move(colors);
    else
        _colors.clear();

    if(_points.empty())
    {
        _lineSize = 0;
        _colors.clear();
        _indices.clear();
        return;
    }

    _lineSize = std::sqrt(_points.size());
    if((_lineSize * _lineSize) != _points.size())
    {
        _lineSize = 0;
        _points.clear();
        _colors.clear();
        _indices.clear();
        Vasnecov::problem("Terrain is not squad");
        return;
    }

    updateCornerPoints();
    updateNormals();
    updateTextures();
    updateIndices();
}

void VasnecovTerrain::clearPoints()
{
    _points.clear();
    _colors.clear();
    _indices.clear();
}

bool VasnecovTerrain::isEmpty() const
{
    return _points.empty();
}

GLuint VasnecovTerrain::pointsAmount() const
{
    return _points.size();
}

void VasnecovTerrain::setType(VasnecovTerrain::Types type)
{
    if(type == _type)
        return;

    _type = type;

    updateCornerPoints();
    updateNormals();
    updateTextures();
    updateIndices();
}

void VasnecovTerrain::enableImage(bool enable)
{
    if(_isTextureEnabled == enable)
        return;

    _isTextureEnabled = enable;

    updaterSetUpdateFlag(Image);
}

void VasnecovTerrain::disableImage(bool disable)
{
    enableImage(!disable);
}

void VasnecovTerrain::setImage(const QImage& image)
{
    if(image.isNull())
    {
        Vasnecov::problem("3D: Terrain texture is empty");
        return;
    }

    if((image.width() & (image.width() - 1)) != 0 || (image.height() & (image.height() - 1)) != 0)
    {
        Vasnecov::problem("3D: Terrain texture has incorrect size");
        return;
    }

    _texture->setImage(image);

    updaterSetUpdateFlag(Image);
}

void VasnecovTerrain::loadImage()
{
    _texture->loadImage();
}

void VasnecovTerrain::setImageZone(GLfloat x, GLfloat y, GLfloat width, GLfloat height)
{
    QRectF rect(x, y, width, height);
    if(rect == _textureZone)
        return;

    _textureZone = rect;

    updaterSetUpdateFlag(Zone);
    updateTextures();
}

void VasnecovTerrain::renderDraw()
{
    if(m_isHidden.pure() || _indices.empty())
        return;

    renderApplyTranslation();

    if(_colors.empty())
        pure_pipeline->setColor(m_color.pure());

    if(_type == TypeSurface)
    {
        bool textured = _isTextureEnabled && _texture != nullptr;
        if(textured)
        {
            pure_pipeline->setColor(QColor(255, 255, 255, 255));
            pure_pipeline->enableTexture2D(_texture->id());
        }

        if(m_scale.pure() != 1.0f)
            pure_pipeline->enableNormalization();

        for(auto& indValue : _indices)
        {
            if(textured)
            {
                pure_pipeline->drawElements(VasnecovPipeline::Triangles,
                                            &indValue,
                                            &_points,
                                            &_normals,
                                            &_textures,
                                            nullptr);
            }
            else
            {
                pure_pipeline->disableTexture2D();
                pure_pipeline->drawElements(VasnecovPipeline::Triangles,
                                            &indValue,
                                            &_points,
                                            &_normals,
                                            nullptr,
                                            &_colors);
            }
        }

        if(m_scale.pure() != 1.0f)
            pure_pipeline->disableNormalization();
        if(textured)
        {
            pure_pipeline->disableTexture2D();
        }
    }
    else if(_type == TypeMesh)
    {
        for(size_t i = 0; i < _indices.size() - 1; ++i)
        {
            pure_pipeline->drawElements(VasnecovPipeline::PolyLine,
                                        &_indices[i],
                                        &_points,
                                        &_normals,
                                        nullptr,
                                        &_colors);
        }
        pure_pipeline->drawElements(VasnecovPipeline::Lines,
                                    &_indices.back(),
                                    &_points,
                                    &_normals,
                                    nullptr,
                                    &_colors);
    }
}

void VasnecovTerrain::updateCornerPoints()
{
    if(_points.empty())
        return;

    GLuint vecSize = _lineSize * _lineSize;
    if(_points.size() > vecSize)
        _points.erase(_points.begin() + vecSize, _points.end());
    if(!_colors.empty() && _colors.size() > vecSize)
        _colors.erase(_colors.begin() + vecSize, _colors.end());

    if(_type == TypeMesh)
    {
        // Corner vertices
        _points.reserve(_points.size() + 4);

        size_t cornInd = 0;
        _points.push_back(QVector3D(_points[cornInd].x(), _points[cornInd].y(), 0.0f));

        cornInd = (_lineSize - 1) * _lineSize;
        _points.push_back(QVector3D(_points[cornInd].x(), _points[cornInd].y(), 0.0f));

        cornInd = _lineSize * _lineSize - 1;
        _points.push_back(QVector3D(_points[cornInd].x(), _points[cornInd].y(), 0.0f));

        cornInd = _lineSize - 1;
        _points.push_back(QVector3D(_points[cornInd].x(), _points[cornInd].y(), 0.0f));

        if(!_colors.empty())
        {
            _colors.reserve(_colors.size() + 4);
            for(size_t j = 0; j < 4; ++j)
                _colors.push_back(QVector3D(0.0f, 0.65f, 1.0f));
        }
    }
    else if(_type == TypeSurface)
    {
        // Vertical flats
        _points.reserve(_points.size() + _lineSize * 4 * 2);
        if(!_colors.empty())
            _colors.reserve(_colors.size() + _lineSize * 4 * 2);

        // YZ left plane
        size_t cornInd(0);
        for(size_t i = 0; i < _lineSize; ++i)
        {
            cornInd = i * _lineSize;
            _points.push_back(_points[cornInd]); // Copies for correct normals
            _points.push_back(QVector3D(_points[cornInd].x(), _points[cornInd].y(), 0.0f));

            if(!_colors.empty())
            {
                _colors.push_back(_colors[cornInd]);
                _colors.push_back(QVector3D(0.0f, 0.65f, 1.0f));
            }
        }
        // XZ bottom plane
        for(size_t i = 0; i < _lineSize; ++i)
        {
            cornInd = (_lineSize - 1) * _lineSize + i;
            _points.push_back(_points[cornInd]);
            _points.push_back(QVector3D(_points[cornInd].x(), _points[cornInd].y(), 0.0f));

            if(!_colors.empty())
            {
                _colors.push_back(_colors[cornInd]);
                _colors.push_back(QVector3D(0.0f, 0.65f, 1.0f));
            }
        }
        // YZ right plane
        for(size_t i = 0; i < _lineSize; ++i)
        {
            cornInd = (_lineSize * _lineSize - 1) - (i * _lineSize);
            _points.push_back(_points[cornInd]);
            _points.push_back(QVector3D(_points[cornInd].x(), _points[cornInd].y(), 0.0f));

            if(!_colors.empty())
            {
                _colors.push_back(_colors[cornInd]);
                _colors.push_back(QVector3D(0.0f, 0.65f, 1.0f));
            }
        }
        // XZ top plane
        for(size_t i = 0; i < _lineSize; ++i)
        {
            cornInd = _lineSize - 1 - i;
            _points.push_back(_points[cornInd]);
            _points.push_back(QVector3D(_points[cornInd].x(), _points[cornInd].y(), 0.0f));

            if(!_colors.empty())
            {
                _colors.push_back(_colors[cornInd]);
                _colors.push_back(QVector3D(0.0f, 0.65f, 1.0f));
            }
        }
    }
}

void VasnecovTerrain::updateNormals()
{
    _normals.clear();

    if(_points.empty())
        return;

    _normals.reserve(_points.size());

    for (GLuint row = 0; row < _lineSize; ++row)
    {
        for (GLuint col = 0; col < _lineSize; ++col)
        {
            std::size_t index = row * _lineSize + col;
            if(row == _lineSize - 1 && col == _lineSize - 1) // Right-bottom corner
            {
                QVector3D a = _points[index - 1] - _points[index];
                QVector3D b = _points[index - 1] - _points[index - _lineSize];
                _normals.push_back(QVector3D::normal(a, b));
            }
            else if(row == _lineSize - 1) // Bottom line
            {
                QVector3D a = _points[index + 1] - _points[index];
                QVector3D b = _points[index + 1] - _points[index - _lineSize];
                _normals.push_back(QVector3D::normal(b, a));
            }
            else if(col == _lineSize - 1) // Right line
            {
                QVector3D a = _points[index - 1] - _points[index];
                QVector3D b = _points[index - 1] - _points[index + _lineSize];
                _normals.push_back(QVector3D::normal(b, a));
            }
            else
            {
                QVector3D a = _points[index + 1] - _points[index];
                QVector3D b = _points[index + 1] - _points[index + _lineSize];
                _normals.push_back(QVector3D::normal(a, b));
            }
        }
    }

    if(_type == TypeMesh)
    {
        _normals.reserve(_normals.size() + 4);
        _normals.push_back(QVector3D(-1.0f,  1.0f, 0.0f));
        _normals.push_back(QVector3D(-1.0f, -1.0f, 0.0f));
        _normals.push_back(QVector3D( 1.0f, -1.0f, 0.0f));
        _normals.push_back(QVector3D( 1.0f,  1.0f, 0.0f));
    }
    else if (_type == TypeSurface)
    {
        // Vertical planes
        for(size_t i = 0; i < _lineSize; ++i)
        {
            _normals.push_back(QVector3D(-1.0f, 0.0f, 0.0f));
            _normals.push_back(_normals.back());
        }
        for(size_t i = 0; i < _lineSize; ++i)
        {
            _normals.push_back(QVector3D(0.0f, -1.0f, 0.0f));
            _normals.push_back(_normals.back());
        }
        for(size_t i = 0; i < _lineSize; ++i)
        {
            _normals.push_back(QVector3D(1.0f, 0.0f, 0.0f));
            _normals.push_back(_normals.back());
        }
        for(size_t i = 0; i < _lineSize; ++i)
        {
            _normals.push_back(QVector3D(0.0f, 1.0f, 0.0f));
            _normals.push_back(_normals.back());
        }
    }
}

void VasnecovTerrain::updateTextures()
{
    _textures.clear();

    if(_points.empty())
        return;

    _textures.reserve(_points.size());

    for (GLuint row = 0; row < _lineSize; ++row)
    {
        for (GLuint col = 0; col < _lineSize; ++col)
        {
            _textures.push_back(QVector2D(static_cast<float>(col)/_lineSize * _textureZone.width()  + _textureZone.x(),
                                          static_cast<float>(row)/_lineSize * _textureZone.height() + _textureZone.y()));
        }
    }
}

void VasnecovTerrain::updateIndices()
{
    _indices.clear();

    if(_points.empty())
        return;

    if(_type == TypeMesh)
    {
        _indices.reserve(_lineSize * _lineSize * 2);
        // Horizontal (X)
        for(GLuint i = 0; i < _lineSize; ++i)
        {
            _indices.push_back(std::vector<GLuint>());
            for(GLuint j = 0; j < _lineSize; ++j)
            {
                _indices.back().push_back(j + i * _lineSize);
            }
        }
        // Vertical (Y)
        for(GLuint i = 0; i < _lineSize; ++i)
        {
            _indices.push_back(std::vector<GLuint>());
            for(GLuint j = 0; j < _lineSize; ++j)
            {
                _indices.back().push_back(j * _lineSize + i);
            }
        }

        // Ground mesh
        _indices.push_back(std::vector<GLuint>());
        _indices.back().reserve(5);
        for(GLuint i = _lineSize * _lineSize; i < _lineSize * _lineSize + 4; ++i)
            _indices.back().push_back(i);
        _indices.back().push_back(_lineSize * _lineSize);

        // Vertical lines from ground
        _indices.push_back(std::vector<GLuint>());
        std::vector<GLuint>* ind = &_indices.back();
        ind->reserve(2 * 4);

        ind->push_back(0);
        ind->push_back(_lineSize * _lineSize);

        ind->push_back((_lineSize - 1) * _lineSize);
        ind->push_back(_lineSize * _lineSize + 1);

        ind->push_back(_lineSize * _lineSize - 1);
        ind->push_back(_lineSize * _lineSize + 2);

        ind->push_back(_lineSize - 1);
        ind->push_back(_lineSize * _lineSize + 3);
    }
    else if (_type == TypeSurface)
    {
        {
            _indices.push_back(std::vector<GLuint>());
            std::vector<GLuint>& ind = _indices.back();
            ind.resize((_lineSize - 1)*(_lineSize - 1) * 6);

            #pragma omp parallel
            {
                #pragma omp for
                for (GLint row = 0; row < GLint(_lineSize - 1); ++row)
                {
                    for (GLint col = 0; col < GLint(_lineSize - 1); ++col)
                    {
                        GLuint v1 = row * _lineSize + col;
                        GLuint v2 = v1 + _lineSize;
                        GLuint v3 = v1 + 1;
                        GLuint x = 6 * (v1 - row);
                        // First triangle
                        ind[x + 0] = v1;
                        ind[x + 1] = v2;
                        ind[x + 2] = v3;
                        // Second triangle
                        ind[x + 3] = v3;
                        ind[x + 4] = v2;
                        ind[x + 5] = v2 + 1;
                    }
                }
            }
        }

        // Corners
        {
            _indices.push_back(std::vector<GLuint>());
            std::vector<GLuint>& ind = _indices.back();
            ind.reserve(6 * (_lineSize * 2 * 4 - 2));

            GLuint pSize = _lineSize * _lineSize;
            for (GLuint i = 0; i < _lineSize * 2 * 4 - 2; i=i+2)
            {
                ind.push_back(pSize + i);
                ind.push_back(pSize + i + 1);
                ind.push_back(pSize + i + 2);

                ind.push_back(pSize + i + 2);
                ind.push_back(pSize + i + 1);
                ind.push_back(pSize + i + 3);
            }
        }
    }
}
