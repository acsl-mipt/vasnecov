/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Сцена для вывода OpenGL (для связки View-Scene)
#pragma once

#include <QGraphicsScene>
#include "Types.h"

class VasnecovUniverse;
class VasnecovWorld;

class VasnecovLamp;
class VasnecovProduct;
class VasnecovFigure;
class VasnecovLabel;
class VasnecovMaterial;
class VasnecovTexture;

class VasnecovScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit VasnecovScene(QObject* parent = nullptr);
    VasnecovUniverse* universe() const;

public slots:
    virtual void setUniverse(VasnecovUniverse* universe);
    virtual bool removeUniverse();

protected:
    virtual void drawBackground(QPainter* painter, const QRectF&);
    GLsizei windowWidth()  {return m_width;}
    GLsizei windowHeight() {return m_height;}

private:
    // Геометрия окна
    GLsizei m_width;
    GLsizei m_height;

    VasnecovUniverse* m_universe;

    Q_DISABLE_COPY(VasnecovScene)
};


inline VasnecovUniverse *VasnecovScene::universe() const
{
    return m_universe;
}
inline bool VasnecovScene::removeUniverse()
{
    if(m_universe)
    {
        m_universe = nullptr;
        return true;
    }
    else
    {
        return false;
    }
}
