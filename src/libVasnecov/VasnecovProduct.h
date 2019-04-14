/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Класс изделия (деталь или узел), описывающего реальный объект в 3D-мире
#pragma once

#include "VasnecovElement.h"

class VasnecovMaterial;
class VasnecovMesh;

class VasnecovProduct : public VasnecovElement
{
public:
    enum ProductTypes
    {
        ProductTypeUnknown = 0,
        ProductTypeAssembly  = 1, // Узел
        ProductTypePart = 2 // Деталь
    };

public:
    VasnecovProduct(VasnecovPipeline* pipeline,
                    VasnecovProduct::ProductTypes type,
                    VasnecovProduct* parent = nullptr,
                    GLuint level = 0);

    VasnecovProduct(VasnecovPipeline* pipeline,
                    const QString& name,
                    VasnecovProduct::ProductTypes type,
                    VasnecovProduct* parent = nullptr,
                    GLuint level = 0);

    VasnecovProduct(VasnecovPipeline* pipeline,
                    const QString& name,
                    VasnecovMesh* mesh,
                    VasnecovProduct* parent = nullptr,
                    GLuint level = 0);
    VasnecovProduct(VasnecovPipeline* pipeline,
                    const QString& name,
                    VasnecovMesh* mesh,
                    VasnecovMaterial* material,
                    VasnecovProduct* parent = nullptr,
                    GLuint level = 0);
    ~VasnecovProduct();

public:
    // Методы, вызываемые извне (защищенные мьютексами)
    // Интерфейсы свойств
    virtual void setVisible(GLboolean visible = true);

    void setMaterial(VasnecovMaterial* material);
    VasnecovMaterial* material() const;

    void setMesh(VasnecovMesh* mesh);
    VasnecovMesh* mesh() const;

    GLuint level() const;
    VasnecovProduct::ProductTypes type() const;

    VasnecovProduct* parent() const;
    void changeParent(VasnecovProduct* newParent);
    std::vector<VasnecovProduct*> children() const;

    // Изменение цвета
    void setColor(const QColor& color); // Задаёт общий цвет продукта. Если есть материал, то передается в ambient и diffuse материала
    using VasnecovElement::setColor;

    // Привязка координат
    void setCoordinates(const QVector3D& coordinates);
    using VasnecovElement::setCoordinates;
    void incrementCoordinates(const QVector3D& increment);
    using VasnecovElement::incrementCoordinates;
    QVector3D globalCoordinates();
    void setAngles(const QVector3D& angles);
    using VasnecovElement::setAngles;
    virtual void incrementAngles(const QVector3D& increment);
    using VasnecovElement::incrementAngles;
    void setScale(GLfloat scale = 1.0f);
    virtual void setPositionFromElement(const VasnecovAbstractElement* element);

    void showDrawingBox(bool show = true);
    void switchDrawingBox();

protected:
    // Методы, вызываемые внутри методов, вызываемых извне (в состоянии заблокированного мьютекса)
    void designerOwnSetVisible(bool visible);
    void designerSetVisibleFromParent(bool visible);
    GLboolean designerRemoveThisMaterial(const VasnecovMaterial* material);

    GLuint designerLevel() const;

    GLboolean designerAddChild(VasnecovProduct* child); // Добавить дочерний элемент, параметром передается индекс элемента
    GLboolean designerRemoveChild(VasnecovProduct* child);
    std::vector<VasnecovProduct*> designerAllChildren(); // Возвращает общий список всех детей
    VasnecovProduct* designerParent() const;
    VasnecovMaterial* designerMaterial() const;

    void designerSetMatrixM1(const QMatrix4x4& M1);

    void designerSetColorRecursively(const QColor& color);

    void designerUpdateChildrenMatrix(); // Обновляет матрицы детей
    void designerSetMatrixM1Recursively(const QMatrix4x4& M1);

    void designerUpdateMatrixM1(const QMatrix4x4& M1);
    void designerUpdateMatrixMs();

    GLfloat renderCalculateDistanceToPlane(const QVector3D& planePoint, const QVector3D& normal);

protected:
    // Методы, вызываемые рендерером (прямое обращение к основным данным без мьютексов)
    GLenum renderUpdateData();
    void renderDraw();

    VasnecovMaterial* renderMaterial() const;
    VasnecovMesh* renderMesh() const;

    VasnecovProduct::ProductTypes renderType() const;
    GLuint renderLevel() const;

    VasnecovProduct* renderParent() const;
    const std::vector<VasnecovProduct*>* renderChildren() const;

protected:
    QMatrix4x4 raw_M1; // Матрица родительских трансформаций
    bool raw_ownVisible;

    Vasnecov::MutualData<ProductTypes> m_type; // тип: узел, деталь
    Vasnecov::MutualData<VasnecovProduct*> m_parent; // Индекс родительского элемента (если уровень больше нуля, иначе 0)
    Vasnecov::MutualData<GLuint> m_level; // отсчет от нуля (нулевой уровень - корень дерева)

    Vasnecov::MutualData<VasnecovMesh*> m_mesh; // Меш (для детали) - геометрия отрисовки
    Vasnecov::MutualData<VasnecovMaterial*> m_material; // Материал меша

    Vasnecov::MutualData<std::vector<VasnecovProduct*> > m_children; // Список дочерних объектов (для узла)
    Vasnecov::MutualData<GLboolean> m_drawingBox; // TODO: to enum with configuration flags

    enum Updated // Дополнительные флаги изменений. При множественном наследовании могут быть проблемы
    {
        Type		= 0x0200,
        Parent		= 0x0400,
        Level		= 0x0800,
        Mesh		= 0x1000,
        Material	= 0x2000,
        Children	= 0x4000,
        DrawingBox  = 0x8000
    };

    friend class VasnecovUniverse;
    friend class VasnecovWorld;

private:
    void init();
    Q_DISABLE_COPY(VasnecovProduct)
};

inline void VasnecovProduct::init()
{
    if(!m_parent.raw() && m_level.raw())
    {
        m_level.set(0);
    }
}
inline GLboolean VasnecovProduct::designerRemoveThisMaterial(const VasnecovMaterial *material)
{
    if(m_material.raw() != material)
    {
        if(m_type.raw() == ProductTypePart)
        {
            m_material.set(nullptr);
            return true;
        }
    }
    return false;
}

inline GLuint VasnecovProduct::designerLevel() const
{
    return m_level.raw();
}
inline VasnecovProduct *VasnecovProduct::designerParent() const
{
    return m_parent.raw();
}
inline VasnecovMaterial *VasnecovProduct::designerMaterial() const
{
    return m_material.raw();
}

inline void VasnecovProduct::designerUpdateMatrixM1(const QMatrix4x4 &M1)
{
    raw_M1 = M1;

    designerUpdateMatrixMs();
}
inline VasnecovMaterial *VasnecovProduct::renderMaterial() const
{
    return m_material.pure();
}
inline VasnecovMesh *VasnecovProduct::renderMesh() const
{
    return m_mesh.pure();
}

inline VasnecovProduct::ProductTypes VasnecovProduct::renderType() const
{
    return m_type.pure();
}

inline GLuint VasnecovProduct::renderLevel() const
{
    return m_level.pure();
}

inline VasnecovProduct *VasnecovProduct::renderParent() const
{
    return m_parent.pure();
}

inline const std::vector<VasnecovProduct *> *VasnecovProduct::renderChildren() const
{
    return &m_children.pure();
}
