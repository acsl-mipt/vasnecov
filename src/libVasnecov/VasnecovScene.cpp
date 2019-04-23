/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "VasnecovScene.h"
#include "VasnecovUniverse.h"
#include <QPainter>

VasnecovScene::VasnecovScene(QObject *parent) :
    QGraphicsScene(parent),
    m_width(0),
    m_height(0),
    m_universe(nullptr)
{
}
void VasnecovScene::drawBackground(QPainter *painter, const QRectF &)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    m_width = painter->device()->width() * painter->device()->devicePixelRatioF();
    m_height = painter->device()->height() * painter->device()->devicePixelRatioF();
#else
    m_width = painter->device()->width() * painter->device()->devicePixelRatio();
    m_height = painter->device()->height() * painter->device()->devicePixelRatio();
#endif

    painter->beginNativePainting();
    if(m_universe)
    {
        m_universe->renderDrawAll(m_width, m_height);
    }
}
void VasnecovScene::setUniverse(VasnecovUniverse *universe)
{
    if(universe)
    {
        m_universe = universe;
        m_universe->renderInitialize();
    }
}
