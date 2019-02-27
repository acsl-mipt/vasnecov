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

VasnecovTerrain::VasnecovTerrain(VasnecovPipeline *pipeline, const QString& name) :
    VasnecovElement(pipeline, name)
{}
VasnecovTerrain::~VasnecovTerrain()
{}

void VasnecovTerrain::setPoints(const std::vector <QVector3D> &points, const std::vector<QVector3D>& colors)
{
    _indices.clear();
    _points = points;
    if(colors.size() == points.size())
        _colors = colors;
    else
        _colors.clear();

    if(_points.empty())
        return;

    size_t lineSize = std::sqrt(_points.size());
    if((lineSize * lineSize) != _points.size())
    {
        _points.clear();
        Vasnecov::problem("Terrain is not squad");
        return;
    }

    _indices.reserve(lineSize * 2);
    for(size_t i = 0; i < lineSize; ++i)
    {
        _indices.push_back(std::vector<GLuint>());
        for(size_t j = 0; j < lineSize; ++j)
        {
            _indices.back().push_back(j + i * lineSize);
        }
    }
}

void VasnecovTerrain::clearPoints()
{
    _points.clear();
}

GLuint VasnecovTerrain::pointsAmount() const
{
    return _points.size();
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
        pure_pipeline->drawElements(VasnecovPipeline::PolyLine,
                                    &indVec,
                                    &_points,
                                    nullptr,
                                    nullptr,
                                    &_colors);
    }
}
