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
#include <QFile>
#include <QSize>
#include <cmath>

VasnecovTerrain::VasnecovTerrain(VasnecovPipeline *pipeline, const QString& name)
    : VasnecovElement(pipeline, name)
    , _type(TypeSurface)
    , _lineSize(0)
{}
VasnecovTerrain::~VasnecovTerrain()
{}

void VasnecovTerrain::setPoints(const std::vector <QVector3D> &points, const std::vector<QVector3D>& colors)
{
    _points = points;
    if(colors.size() == points.size())
        _colors = colors;
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
    updateIndices();
}

void VasnecovTerrain::clearPoints()
{
    _points.clear();
    _colors.clear();
    _indices.clear();
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
    updateIndices();
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
        for(auto& indValue : _indices)
        {
            pure_pipeline->drawElements(VasnecovPipeline::Triangles,
                                        &indValue,
                                        &_points,
                                        &_normals,
                                        nullptr,
                                        &_colors);
        }
//        pure_pipeline->drawElements(VasnecovPipeline::Triangles,
//                                    &_indices[0],
//                                    &_points,
//                                    &_normals,
//                                    nullptr,
//                                    &_colors);
//        pure_pipeline->drawElements(VasnecovPipeline::Points,
//                                    &_indices[1],
//                                    &_points,
//                                    &_normals,
//                                    nullptr,
//                                    &_colors);
    }
    else if(_type == TypeMesh)
    {
        for(size_t i = 0; i < _indices.size() - 1; ++i)
        {
            pure_pipeline->drawElements(VasnecovPipeline::PolyLine,
                                        &_indices[i],
                                        &_points,
                                        nullptr,
                                        nullptr,
                                        &_colors);
        }
        pure_pipeline->drawElements(VasnecovPipeline::Lines,
                                    &_indices.back(),
                                    &_points,
                                    nullptr,
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
            QVector3D a = _points[row * _lineSize + col + 1] - _points[row * _lineSize + col];
            QVector3D b = _points[row * _lineSize + col + 1] - _points[(row + 1) * _lineSize + col];
            _normals.push_back(QVector3D::normal(a, b));
        }
    }

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
        _indices.push_back(std::vector<GLuint>());

        std::vector<GLuint>* ind = &_indices.back();
        ind->reserve(_points.size() * 3);

        for (GLuint row = 0; row < _lineSize - 1; ++row)
        {
            for (GLuint col = 0; col < _lineSize - 1; ++col)
            {
                // First triangle
                ind->push_back(row * _lineSize + col);
                ind->push_back((row + 1) * _lineSize + col);
                ind->push_back(*(ind->end() - 2) + 1);

                // Second triangle
                ind->push_back(*(ind->end() - 1));
                ind->push_back(*(ind->end() - 3));
                ind->push_back(*(ind->end() - 1) + 1);
            }
        }

        // Corners
        _indices.push_back(std::vector<GLuint>());
        ind = &_indices.back();
        ind->reserve(_lineSize * 4 * 3);

        GLuint pSize = _lineSize * _lineSize;
        for(GLuint i = 0; i < _lineSize * 2 * 4 - 1; ++++i)
        {
            ind->push_back(pSize + i);
            ind->push_back(pSize + i + 1);
            ind->push_back(pSize + i + 2);

            ind->push_back(pSize + i + 2);
            ind->push_back(pSize + i + 1);
            ind->push_back(pSize + i + 3);
        }
    }
}


