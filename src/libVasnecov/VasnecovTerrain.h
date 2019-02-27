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
    VasnecovTerrain(VasnecovPipeline* pipeline, const QString& name = QString());
    ~VasnecovTerrain();

    void setPoints(const std::vector<QVector3D>& points, const std::vector<QVector3D>& colors = std::vector<QVector3D>());
    void clearPoints();
    GLuint pointsAmount() const;

protected:
    void renderDraw();

private:
    std::vector<QVector3D>              _points;
    std::vector<QVector3D>              _colors;
    std::vector<std::vector<GLuint>>    _indices;

    enum Updated
    {
        Type		= 0x0200,
        Points		= 0x0400,
        Thickness	= 0x0800,
        Lighting	= 0x1000,
        Depth		= 0x2000
    };

    friend class VasnecovUniverse;
    friend class VasnecovWorld;

private:
    Q_DISABLE_COPY(VasnecovTerrain)
};
