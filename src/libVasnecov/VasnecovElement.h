/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Базовый класс для всех элементов сцены, которые можно нарисовать
#pragma once

#include <QQuaternion>
#include "CoreObject.h"
#include "VasnecovPipeline.h"

class VasnecovAbstractElement : public Vasnecov::CoreObject
{
public:
    VasnecovAbstractElement(VasnecovPipeline* pipeline, const QString& name = QString());

public:
    // Методы, вызываемые извне (защищенные мьютексами). Без префикса.
    // Координаты
    virtual void setCoordinates(const QVector3D& coordinates);
    void setCoordinates(GLfloat x, GLfloat y, GLfloat z);
    virtual void incrementCoordinates(const QVector3D& increment); // Приращение координат
    void incrementCoordinates(GLfloat x, GLfloat y, GLfloat z);
    QVector3D coordinates() const;
    // Углы (по умолчанию, задаются в градусах. В них же хранятся)
    virtual void setAngles(const QVector3D& angles);
    void setAngles(GLfloat x, GLfloat y, GLfloat z);
    virtual void incrementAngles(const QVector3D& increment);
    void incrementAngles(GLfloat x, GLfloat y, GLfloat z);
    void setAnglesRad(const QVector3D& angles);
    void setAnglesRad(GLfloat x, GLfloat y, GLfloat z);
    void incrementAnglesRad(const QVector3D& increment);
    void incrementAnglesRad(GLfloat x, GLfloat y, GLfloat z);
    QVector3D angles() const;

    virtual void setPositionFromElement(const VasnecovAbstractElement* element);
    void attachToElement(const VasnecovAbstractElement* element);
    void detachFromOtherElement();

protected:
    // Методы без мьютексов, вызываемые методами, защищенными своими мьютексами. Префикс designer
    QMatrix4x4 designerMatrixMs() const;
    const QMatrix4x4* designerExportingMatrix() const;
    virtual void designerUpdateMatrixMs();
    GLboolean designerRemoveThisAlienMatrix(const QMatrix4x4* alienMs); // Обнуление чужой матрицы, равной заданной параметром

protected:
    // Методы, вызываемые рендерером (прямое обращение к основным данным без мьютексов). Префикс render
    // Для их сокрытия методы объявлены protected, а класс Рендерера сделан friend
    virtual GLenum renderUpdateData(); // обновление данных, вызов должен быть обёрнут мьютексом

    void renderApplyTranslation() const; // Выполнение позиционирования элемента
    const QMatrix4x4& renderMatrixMs() const;

    QVector3D renderCoordinates() const;
    QVector3D renderAngles() const;

protected:
    QVector3D raw_coordinates;
    QVector3D raw_angles;
    QQuaternion raw_qX, raw_qY, raw_qZ;

    Vasnecov::MutualData<QMatrix4x4> m_Ms;
    Vasnecov::MutualData<const QMatrix4x4*> m_alienMs;

    enum Updated // Изменение данных
    {
        MatrixMs		= 0x0008,
        AlienMatrix 	= 0x0010
    };

private:
    Q_DISABLE_COPY(VasnecovAbstractElement)
};

class VasnecovElement : public VasnecovAbstractElement
{
public:
    VasnecovElement(VasnecovPipeline* pipeline, const QString& name = "");

public:
    // Методы, вызываемые извне (защищенные мьютексами)
    // Цвет
    virtual void setColor(const QColor& color);
    void setColor(GLint r, GLint g, GLint b, GLint a = 255);
    void setColorF(GLfloat r, GLfloat g, GLfloat b, GLfloat a = 1.0f);
    void setColor(QRgb rgb);
    void setColorWithAlpha(QRgb rgba); // С прозрачностью, в формате #AARRGGBB
    void setColorAlpha(GLint alpha);
    void setColorAlphaF(GLfloat alpha);
    QColor color() const;

    virtual void setScale(GLfloat scale = 1.0f);
    GLfloat scale() const;

    // Прозрачность
    GLboolean isTransparency() const; // Является ли прозрачной

protected:
    // Методы без мьютексов, вызываемые методами, защищенными своими мьютексами
    virtual void designerUpdateMatrixMs();

protected:
    // Методы, вызываемые рендерером (прямое обращение к основным данным без мьютексов)
    virtual GLenum renderUpdateData(); // обновление данных, вызов должен быть обёрнут мьютексом

    GLfloat renderDistance() const;
    QColor renderColor() const;
    GLfloat renderScale() const;

    GLboolean renderIsTransparency() const;
    virtual GLfloat renderCalculateDistanceToPlane(const QVector3D& planePoint, const QVector3D& normal);

    static bool renderCompareByReverseDistance(VasnecovElement* first, VasnecovElement* second);
    static bool renderCompareByDirectDistance(VasnecovElement* first, VasnecovElement* second);

protected:
    Vasnecov::MutualData<QColor> m_color; // Цвет
    Vasnecov::MutualData<GLfloat> m_scale; // Масштаб
    Vasnecov::MutualData<GLboolean> m_isTransparency; // Прозрачность

    GLfloat pure_distance; // Расстояние от ЦМ объекта до плоскости камеры (для сортировки)

    enum Updated// Изменение данных
    {
        Color		 = 0x0040,
        Scale		 = 0x0080,
        Transparency = 0x0100
    };

    friend class VasnecovUniverse;
    friend class VasnecovWorld;

private:
    Q_DISABLE_COPY(VasnecovElement)
};

//==================================================================================================
inline void VasnecovAbstractElement::renderApplyTranslation() const
{
    // Если есть чужая матрица трансформаций, то перемножается со своей, иначе используем только свою.
    if(m_alienMs.pure())
    {
        pure_pipeline->setMatrixMV(m_alienMs.pure());
        pure_pipeline->addMatrixMV(m_Ms.pure());
    }
    else
    {
        pure_pipeline->setMatrixMV(m_Ms.pure());
    }
}
inline QMatrix4x4 VasnecovAbstractElement::designerMatrixMs() const
{
    return m_Ms.raw();
}
inline const QMatrix4x4 *VasnecovAbstractElement::designerExportingMatrix() const
{
    const QMatrix4x4 *Ms(&m_Ms.raw());
    return Ms;
}

inline GLboolean VasnecovAbstractElement::designerRemoveThisAlienMatrix(const QMatrix4x4 *alienMs)
{
    if(m_alienMs.raw() == alienMs)
    {
        m_alienMs.set(nullptr);
        return true;
    }
    return false;
}
inline const QMatrix4x4 &VasnecovAbstractElement::renderMatrixMs() const
{
    return m_Ms.pure();
}

inline GLfloat VasnecovElement::renderDistance() const
{
    return pure_distance;
}
inline QColor VasnecovElement::renderColor() const
{
    return m_color.pure();
}

inline GLfloat VasnecovElement::renderScale() const
{
    return m_scale.pure();
}

inline GLboolean VasnecovElement::renderIsTransparency() const
{
    return m_isTransparency.pure();
}
