/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Базовый класс для всех элементов сцены, которые можно нарисовать
#ifndef VASNECOVELEMENT_H
#define VASNECOVELEMENT_H

#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
#include <QQuaternion>
#include "coreobject.h"
#include "vasnecovpipeline.h"
#ifndef _MSC_VER
    #pragma GCC diagnostic warning "-Weffc++"
#endif

class VasnecovAbstractElement : public Vasnecov::CoreObject
{
public:
    VasnecovAbstractElement(VasnecovPipeline* pipeline, const std::string& name = std::string());

public:
    // Методы, вызываемые извне (защищенные мьютексами). Без префикса.
    // Координаты
    virtual void setCoordinates(const QVector3D& coordinates);
    void setCoordinates(GLfloat x, GLfloat y, GLfloat z);
    virtual void incrementCoordinates(const QVector3D& increment); // Приращение координат
    void incrementCoordinates(GLfloat x, GLfloat y, GLfloat z);
    const QVector3D& coordinates() const;
    // Углы (по умолчанию, задаются в градусах. В них же хранятся)
    virtual void setAngles(const QVector3D& angles);
    void setAngles(GLfloat x, GLfloat y, GLfloat z);
    virtual void incrementAngles(const QVector3D& increment);
    void incrementAngles(GLfloat x, GLfloat y, GLfloat z);
    void setAnglesRad(const QVector3D& angles);
    void setAnglesRad(GLfloat x, GLfloat y, GLfloat z);
    void incrementAnglesRad(const QVector3D& increment);
    void incrementAnglesRad(GLfloat x, GLfloat y, GLfloat z);
    const QVector3D& angles() const;

    virtual void setPositionFromElement(const VasnecovAbstractElement* element);
    void attachToElement(const VasnecovAbstractElement* element);
    void detachFromOtherElement();

protected:
    // Методы без мьютексов, вызываемые методами, защищенными своими мьютексами. Префикс designer
    const QMatrix4x4& matrixMs() const { return m_Ms; }
    virtual void designerUpdateMatrixMs();
    GLboolean designerRemoveThisAlienMatrix(const QMatrix4x4* alienMs); // Обнуление чужой матрицы, равной заданной параметром

protected:
    // Методы, вызываемые рендерером (прямое обращение к основным данным без мьютексов). Префикс render
    // Для их сокрытия методы объявлены protected, а класс Рендерера сделан friend
    GLenum renderUpdateData() override; // обновление данных, вызов должен быть обёрнут мьютексом

    void renderApplyTranslation() const; // Выполнение позиционирования элемента

protected:
    QVector3D raw_coordinates;
    QVector3D raw_angles;
    QQuaternion raw_qX, raw_qY, raw_qZ;

    QMatrix4x4        m_Ms;
    const QMatrix4x4* m_alienMs;

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
    VasnecovElement(VasnecovPipeline* pipeline, const std::string& name = "");

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
    GLfloat scale() const { return m_scale; }

    // Прозрачность
    GLboolean isTransparency() const { return m_isTransparency; } // Является ли прозрачной

protected:
    // Методы без мьютексов, вызываемые методами, защищенными своими мьютексами
    void designerUpdateMatrixMs() override;

protected:
    // Методы, вызываемые рендерером (прямое обращение к основным данным без мьютексов)
    GLenum renderUpdateData() override; // обновление данных, вызов должен быть обёрнут мьютексом

    GLfloat renderDistance() const { return pure_distance; }

    virtual GLfloat renderCalculateDistanceToPlane(const QVector3D& planePoint, const QVector3D& normal);

    static bool renderCompareByReverseDistance(VasnecovElement* first, VasnecovElement* second);
    static bool renderCompareByDirectDistance(VasnecovElement* first, VasnecovElement* second);

protected:
    QColor    m_color; // Цвет
    GLfloat   m_scale; // Масштаб
    GLboolean m_isTransparency; // Прозрачность

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
    if(m_alienMs)
    {
        pure_pipeline->setMatrixMV(*m_alienMs);
        pure_pipeline->addMatrixMV(m_Ms);
    }
    else
    {
        pure_pipeline->setMatrixMV(m_Ms);
    }
}

inline GLboolean VasnecovAbstractElement::designerRemoveThisAlienMatrix(const QMatrix4x4 *alienMs)
{
    if (m_alienMs != alienMs)
        return false;
    m_alienMs = nullptr;
    return true;
}


#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
#endif // VASNECOVELEMENT_H
