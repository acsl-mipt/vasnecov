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
    , _type(TypeMesh)
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
        _indices.clear();
        return;
    }

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
        // Corner vertices
        _points.reserve(_points.size() + 4);

        size_t cor = 0;
        _points.push_back(QVector3D(_points[cor].x(), _points[cor].y(), 0.0f));

        cor = (lineSize - 1) * lineSize;
        _points.push_back(QVector3D(_points[cor].x(), _points[cor].y(), 0.0f));

        cor = lineSize * lineSize - 1;
        _points.push_back(QVector3D(_points[cor].x(), _points[cor].y(), 0.0f));

        cor = lineSize - 1;
        _points.push_back(QVector3D(_points[cor].x(), _points[cor].y(), 0.0f));

        if(!_colors.empty())
        {
            _colors.reserve(_colors.size() + 4);
            for(int j = 0; j < 4; ++j)
                _colors.push_back(QVector3D(0.0f, 0.65f, 1.0f));
        }

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

        _indices.push_back(std::vector<GLuint>());
        for(GLuint i = lineSize * lineSize; i < lineSize * lineSize + 4; ++i)
            _indices.back().push_back(i);
        _indices.back().push_back(lineSize * lineSize);

        qDebug("%ld (%d); %ld", _points.size(), lineSize, _indices.back().size());
    }
    else if (_type == TypeSurface)
    {
        // Corner flats


        _indices.push_back(std::vector<GLuint>());

        std::vector<GLuint>* ind = &_indices.back();
        ind->reserve(_points.size() * 3);

        for (GLuint raw = 0; raw < lineSize - 1; ++raw)
        {
            for (GLuint col = 0; col < lineSize - 1; ++col)
            {
                // First triangle
                ind->push_back(raw * lineSize + col);
                ind->push_back((raw + 1) * lineSize + col);
                ind->push_back(*(ind->end() - 2) + 1);

                // Second triangle
                ind->push_back(*(ind->end() - 1));
                ind->push_back(*(ind->end() - 3));
                ind->push_back(*(ind->end() - 1) + 1);
            }
        }

        // Corners
//        ind->reserve(ind->size() + () * 4)
    }
}
