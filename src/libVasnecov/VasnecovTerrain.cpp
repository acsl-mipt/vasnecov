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

VasnecovTerrain::VasnecovTerrain(VasnecovPipeline *pipeline, const QString& name) :
    VasnecovElement(pipeline, name)
{
}
VasnecovTerrain::~VasnecovTerrain()
{
}

void VasnecovTerrain::setPoints(const std::vector <QVector3D> &points)
{
    m_points = points;
}

void VasnecovTerrain::clearPoints()
{
    m_points.clear();
}

GLuint VasnecovTerrain::pointsAmount() const
{
    return m_points.size();
}

void VasnecovTerrain::renderDraw()
{

}
