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
        return;

    updateIndices();
}

void VasnecovTerrain::clearPoints()
{
    _points.clear();
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
    updateIndices();
}

void VasnecovTerrain::renderDraw()
{
    if(m_isHidden.pure())
        return;

    renderApplyTranslation();

    if(_colors.empty())
        pure_pipeline->setColor(m_color.pure());

    for(auto& indVec : _indices)
    {
        VasnecovPipeline::ElementDrawingMethods type = VasnecovPipeline::PolyLine;
        if(_type == TypeSurface)
            type = VasnecovPipeline::Triangles;

        pure_pipeline->drawElements(type,
                                    &indVec,
                                    &_points,
                                    nullptr,
                                    nullptr,
                                    &_colors);
    }
}

void VasnecovTerrain::updateIndices()
{
    _indices.clear();

    GLuint lineSize = std::sqrt(_points.size());
    if((lineSize * lineSize) != _points.size())
    {
        _points.clear();
        Vasnecov::problem("Terrain is not squad");
        return;
    }

    if(_type == TypeMesh)
    {
        _indices.reserve(lineSize * lineSize * 2);
        // Horizontal (X)
        for(GLuint i = 0; i < lineSize; ++i)
        {
            _indices.push_back(std::vector<GLuint>());
            for(GLuint j = 0; j < lineSize; ++j)
            {
                _indices.back().push_back(j + i * lineSize);
            }
        }
        // Vertical (Y)
        for(GLuint i = 0; i < lineSize; ++i)
        {
            _indices.push_back(std::vector<GLuint>());
            for(GLuint j = 0; j < lineSize; ++j)
            {
                _indices.back().push_back(j * lineSize + i);
            }
        }
    }
    else if (_type == TypeSurface)
    {
        _indices.push_back(std::vector<GLuint>());
        _indices.back().reserve(_points.size() * 3);

        for (GLuint raw = 0; raw < lineSize - 1; ++raw)
        {
            for (GLuint col = 0; col < lineSize - 1; ++col)
            {
                // First triangle
                _indices.back().push_back(raw * lineSize + col);
                _indices.back().push_back((raw + 1) * lineSize + col);
                _indices.back().push_back(*(_indices.back().end() - 2) + 1);

                // Second triangle
                _indices.back().push_back(*(_indices.back().end() - 1));
                _indices.back().push_back(*(_indices.back().end() - 3));
                _indices.back().push_back(*(_indices.back().end() - 1) + 1);
            }
        }
    }
}
