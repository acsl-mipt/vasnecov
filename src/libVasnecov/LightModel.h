#pragma once

#include <QColor>

namespace Vasnecov
{
    // Модель характеристик освещения
    class LightModel
    {
    public:
        explicit LightModel() :
            m_ambientColor(51, 51, 51, 255)
        {
            m_ambientColor.setRgbF(0.2, 0.2, 0.2, 1.0);
        }
        void setAmbientColor(const QColor& color)
        {
            m_ambientColor = color;
        }
        QColor ambientColor() const
        {
            return m_ambientColor;
        }

    protected:
        QColor m_ambientColor;
    };

    enum LampTypes
    {
        LampTypeCelestial = 1,
        LampTypeSpot,
        LampTypeHeadlight
    };
}
