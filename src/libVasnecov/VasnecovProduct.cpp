/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "VasnecovProduct.h"
#include "Technologist.h"
#include "VasnecovMaterial.h"
#include "VasnecovMesh.h"

VasnecovProduct::VasnecovProduct(VasnecovPipeline *pipeline, VasnecovProduct::ProductTypes type, VasnecovProduct* parent, GLuint level) :
    VasnecovElement(pipeline),
    raw_M1(),
    raw_ownVisible(true),

    m_type(raw_wasUpdated, Type, type),
    m_parent(raw_wasUpdated, Parent, parent),
    m_level(raw_wasUpdated, Level, level),

    m_mesh(raw_wasUpdated, Mesh, nullptr),
    m_material(raw_wasUpdated, Material, nullptr),
    m_children(raw_wasUpdated, Children),

    m_drawingBox(raw_wasUpdated, DrawingBox, false)
{
    init();
}
VasnecovProduct::VasnecovProduct(VasnecovPipeline *pipeline, const QString& name, VasnecovProduct::ProductTypes type, VasnecovProduct *parent, GLuint level) :
    VasnecovElement(pipeline, name),
    raw_M1(),
    raw_ownVisible(true),

    m_type(raw_wasUpdated, Type, type),
    m_parent(raw_wasUpdated, Parent, parent),
    m_level(raw_wasUpdated, Level, level),

    m_mesh(raw_wasUpdated, Mesh, nullptr),
    m_material(raw_wasUpdated, Material, nullptr),
    m_children(raw_wasUpdated, Children),

    m_drawingBox(raw_wasUpdated, DrawingBox, false)
{
    init();
}
VasnecovProduct::VasnecovProduct(VasnecovPipeline *pipeline, const QString& name, VasnecovMesh *mesh, VasnecovProduct *parent, GLuint level) :
    VasnecovElement(pipeline, name),
    raw_M1(),
    raw_ownVisible(true),

    m_type(raw_wasUpdated, Type, ProductTypePart), // т.к. меш может быть только у детали
    m_parent(raw_wasUpdated, Parent, parent),
    m_level(raw_wasUpdated, Level, level),

    m_mesh(raw_wasUpdated, Mesh, mesh),
    m_material(raw_wasUpdated, Material, nullptr),
    m_children(raw_wasUpdated, Children),

    m_drawingBox(raw_wasUpdated, DrawingBox, false)
{
    init();
}
VasnecovProduct::VasnecovProduct(VasnecovPipeline *pipeline, const QString& name, VasnecovMesh *mesh, VasnecovMaterial *material, VasnecovProduct *parent, GLuint level) :
    VasnecovElement(pipeline, name),
    raw_M1(),
    raw_ownVisible(true),

    m_type(raw_wasUpdated, Type, ProductTypePart), // т.к. меш может быть только у детали
    m_parent(raw_wasUpdated, Parent, parent),
    m_level(raw_wasUpdated, Level, level),

    m_mesh(raw_wasUpdated, Mesh, mesh),
    m_material(raw_wasUpdated, Material, material),
    m_children(raw_wasUpdated, Children),

    m_drawingBox(raw_wasUpdated, DrawingBox, false)
{
    init();
}
VasnecovProduct::~VasnecovProduct()
{
    /*
     * Деструктор можно вызывать только в потоке рендеринга.
     *
     * Продукт удаляется из списка родителя извне.
     * Меш не удаляется, т.к. сохраняется для других продуктов на будущее.
     * Материал удаляется извне, если не используется другими продуктами.
     * Дети все убиваются рекурсивно тоже извне.
     */
}

void VasnecovProduct::setVisible(GLboolean visible)
{
    designerOwnSetVisible(visible);
}

void VasnecovProduct::setPositionFromElement(const VasnecovAbstractElement *element)
{
    VasnecovAbstractElement::setPositionFromElement(element);

    if(element)
    {
        designerUpdateChildrenMatrix();
    }
}

void VasnecovProduct::showDrawingBox(bool show)
{
    m_drawingBox.set(show);
}

void VasnecovProduct::switchDrawingBox()
{
    m_drawingBox.set(!m_drawingBox.raw());
}

void VasnecovProduct::designerOwnSetVisible(bool visible)
{
    raw_ownVisible = visible;

    if(m_parent.raw())
    {
        designerSetVisibleFromParent(m_parent.raw()->designerIsVisible() && raw_ownVisible);
    }
    else
    {
        designerSetVisibleFromParent(raw_ownVisible);
    }
}

void VasnecovProduct::designerSetVisibleFromParent(bool visible)
{
    bool trueVis(false);

    if(visible && raw_ownVisible)
    {
        trueVis = true;
    }

    m_isHidden.set(!trueVis);

    if(!m_children.raw().empty())
    {
        for(std::vector<VasnecovProduct *>::const_iterator cit = m_children.raw().begin();
            cit != m_children.raw().end(); ++cit)
        {
            (*cit)->designerSetVisibleFromParent(trueVis);
        }
    }
}

void VasnecovProduct::designerUpdateMatrixMs()
{
    QMatrix4x4 newMatrix(raw_M1);
    newMatrix.translate(raw_coordinates);

    QQuaternion qRot;
    qRot = raw_qZ * raw_qX * raw_qY;

    newMatrix.rotate(qRot);

    if(m_scale.raw() != 1.0f)
    {
        newMatrix.scale(m_scale.raw(), m_scale.raw(), m_scale.raw());
    }

    m_Ms.set(newMatrix);
}

GLfloat VasnecovProduct::renderCalculateDistanceToPlane(const QVector3D &planePoint, const QVector3D &normal)
{
    QVector3D centerPoint;
    if(m_mesh.pure())
    {
        centerPoint = m_mesh.pure()->massCenter();
    }

    if(m_alienMs.pure())
    {
        centerPoint = (*m_alienMs.pure()) * m_Ms.pure() * centerPoint;
    }
    else
    {
        centerPoint = m_Ms.pure() * centerPoint;
    }

    pure_distance = centerPoint.distanceToPlane(planePoint, normal);

    return pure_distance;
}
GLenum VasnecovProduct::renderUpdateData()
{
    // Проверка прозрачности
    GLboolean transp = false;
    if(m_material.raw() && m_material.raw()->renderTextureD())
    {
        transp = m_material.raw()->renderTextureD()->isTransparency();
    }
    if(m_color.raw().alphaF() < 1.0)
    {
        transp = true;
    }
    m_isTransparency.set(transp);

    // Далее, как обычно
    GLenum updated(raw_wasUpdated);

    if(raw_wasUpdated)
    {
        pure_pipeline->setSomethingWasUpdated();

        // Копирование сырых данных в основные
        m_type.update();
        m_parent.update();
        m_level.update();

        m_mesh.update();
        m_material.update();

        m_children.update();

        m_drawingBox.update();

        VasnecovElement::renderUpdateData();
    }

    return updated;
}
void VasnecovProduct::renderDraw()
{
    if(!m_isHidden.pure() &&
       m_type.pure() == ProductTypePart &&
       m_mesh.pure())
    {
        renderApplyTranslation();

        if(m_material.pure())
        {
            m_material.pure()->renderDraw();
        }
        else
        {
            pure_pipeline->disableTexture2D();
            pure_pipeline->setColor(m_color.pure());
        }

        if(m_scale.pure() != 1.0f)
            pure_pipeline->enableNormalization();

        if(m_drawingBox.pure())
        {
            m_mesh.pure()->drawBorderBox(pure_pipeline);
        }
        m_mesh.pure()->drawModel(pure_pipeline);

        if(m_scale.pure() != 1.0f)
            pure_pipeline->disableNormalization();
    }
}
GLboolean VasnecovProduct::designerAddChild(VasnecovProduct *child)
{
    GLboolean res(false);
    if(child)
    {
        if(m_type.raw() == ProductTypeAssembly &&
           m_level.raw() <= Vasnecov::cfg_elementMaxLevel)
        {
            std::vector<VasnecovProduct *> chs = m_children.raw();
            chs.push_back(child);
            m_children.set(chs);

            child->designerSetMatrixM1Recursively(m_Ms.raw());
            child->designerSetColorRecursively(m_color.raw());

            res = true;
        }
    }

    return res;
}
GLboolean VasnecovProduct::designerRemoveChild(VasnecovProduct *child)
{
    GLboolean res(false);
    if(child)
    {
        if(m_type.raw() == ProductTypeAssembly)
        {
            std::vector<VasnecovProduct *> chs = m_children.raw();

            for(std::vector<VasnecovProduct *>::iterator cit = chs.begin();
                cit != chs.end(); ++cit)
            {
                if((*cit) == child)
                {
                    chs.erase(cit);
                    m_children.set(chs);

                    res = true;
                    break;
                }
            }
        }
    }

    return res;
}
std::vector<VasnecovProduct *> VasnecovProduct::designerAllChildren()
{
    std::vector<VasnecovProduct *> children;

    if(m_type.raw() == ProductTypeAssembly)
    {
        for(std::vector<VasnecovProduct *>::const_iterator cit = m_children.raw().begin();
            cit != m_children.raw().end(); ++cit)
        {
            children.push_back(*cit);

            std::vector<VasnecovProduct *> grandchildren = (*cit)->designerAllChildren();
            children.insert(children.end(), grandchildren.begin(), grandchildren.end());
        }
    }
    return children;
}
void VasnecovProduct::setMaterial(VasnecovMaterial *material)
{
    if(material)
    {
        if(m_type.raw() == ProductTypePart)
        {
            m_material.set(material);
        }
    }
}
VasnecovMaterial *VasnecovProduct::material() const
{
    VasnecovMaterial * material(m_material.raw());
    return material;
}
void VasnecovProduct::setMesh(VasnecovMesh *mesh)
{
    if(mesh)
    {
        if(m_type.raw() == ProductTypePart)
        {
            m_mesh.set(mesh);
        }
    }
}
VasnecovMesh *VasnecovProduct::mesh() const
{
    VasnecovMesh *mesh(m_mesh.raw());
    return mesh;
}
GLuint VasnecovProduct::level() const
{
    GLuint level(m_level.raw());
    return level;
}
VasnecovProduct::ProductTypes VasnecovProduct::type() const
{
    VasnecovProduct::ProductTypes type(m_type.raw());
    return type;
}
VasnecovProduct *VasnecovProduct::parent() const
{
    VasnecovProduct *parent(m_parent.raw());
    return parent;
}

void VasnecovProduct::changeParent(VasnecovProduct *newParent)
{
    if(m_parent.raw())
    {
        m_parent.raw()->designerRemoveChild(this);
    }
    if(newParent)
    {
        m_parent.set(newParent);
        newParent->designerAddChild(this);
    }
    else // Нет родителя - элемент глобальный
    {
        designerSetMatrixM1Recursively(QMatrix4x4());
    }
}
std::vector<VasnecovProduct *> VasnecovProduct::children() const
{
    std::vector<VasnecovProduct *> children(m_children.raw());
    return children;
}
void VasnecovProduct::setColor(const QColor &color)
{
    if(m_material.raw() == nullptr && m_color.raw() == color)
        return;
    if(m_material.raw() != nullptr &&
       (m_material.raw()->ambientColor() == color && m_material.raw()->diffuseColor() == color))
       return;

    designerSetColorRecursively(color);
}
void VasnecovProduct::setCoordinates(const QVector3D &coordinates)
{
    if(raw_coordinates != coordinates)
    {
        raw_coordinates = coordinates;

        designerUpdateMatrixMs();
        designerUpdateChildrenMatrix();
    }
}
void VasnecovProduct::incrementCoordinates(const QVector3D &increment)
{
    if(increment.x() != 0.0f || increment.y() != 0.0f || increment.z() != 0.0f)
    {
        raw_coordinates += increment;

        designerUpdateMatrixMs();
        designerUpdateChildrenMatrix();
    }
}
QVector3D VasnecovProduct::globalCoordinates()
{
    QVector3D coordinates(m_Ms.raw()(0, 3), m_Ms.raw()(1, 3), m_Ms.raw()(2, 3));
    return coordinates;
}
void VasnecovProduct::setAngles(const QVector3D &angles)
{
    if(raw_angles != angles)
    {
        GLenum rotate(0);

        if(raw_angles.x() != angles.x())
        {
            raw_angles.setX(Vasnecov::trimAngle(angles.x()));
            raw_qX = raw_qX.fromAxisAndAngle(1.0, 0.0, 0.0, raw_angles.x());
            rotate |= Vasnecov::RotationX;
        }
        if(raw_angles.y() != angles.y())
        {
            raw_angles.setY(Vasnecov::trimAngle(angles.y()));
            raw_qY = raw_qY.fromAxisAndAngle(0.0, 1.0, 0.0, raw_angles.y());
            rotate |= Vasnecov::RotationY;
        }
        if(raw_angles.z() != angles.z())
        {
            raw_angles.setZ(Vasnecov::trimAngle(angles.z()));
            raw_qZ = raw_qZ.fromAxisAndAngle(0.0, 0.0, 1.0, raw_angles.z());
            rotate |= Vasnecov::RotationZ;
        }

        if(rotate)
        {
            designerUpdateMatrixMs();
            designerUpdateChildrenMatrix();
        }
    }
}
void VasnecovProduct::incrementAngles(const QVector3D &increment)
{
    if(increment.x() != 0.0f || increment.y() != 0.0f || increment.z() != 0.0f)
    {
        GLenum rotate(0);

        if(increment.x() != 0.0)
        {
            raw_angles.setX(Vasnecov::trimAngle(raw_angles.x() + increment.x()));
            raw_qX = raw_qX.fromAxisAndAngle(1.0, 0.0, 0.0, raw_angles.x());
            rotate |= Vasnecov::RotationX;
        }
        if(increment.y() != 0.0)
        {
            raw_angles.setY(Vasnecov::trimAngle(raw_angles.y() + increment.y()));
            raw_qY = raw_qY.fromAxisAndAngle(0.0, 1.0, 0.0, raw_angles.y());
            rotate |= Vasnecov::RotationY;
        }
        if(increment.z() != 0.0)
        {
            raw_angles.setZ(Vasnecov::trimAngle(raw_angles.z() + increment.z()));
            raw_qZ = raw_qZ.fromAxisAndAngle(0.0, 0.0, 1.0, raw_angles.z());
            rotate |= Vasnecov::RotationZ;
        }

        if(rotate)
        {
            designerUpdateMatrixMs();
            designerUpdateChildrenMatrix();
        }
    }
}
void VasnecovProduct::setScale(GLfloat scale)
{
    if(m_scale.set(scale))
    {
        designerUpdateMatrixMs();
        designerUpdateChildrenMatrix();
    }
}
void VasnecovProduct::designerSetMatrixM1(const QMatrix4x4 &M1)
{
    if(raw_M1 != M1)
    {
        designerSetMatrixM1Recursively(M1);
    }
}
void VasnecovProduct::designerSetColorRecursively(const QColor &color)
{
    m_color.set(color);

    if(!m_children.raw().empty())
    {
        for(std::vector<VasnecovProduct *>::const_iterator cit = m_children.raw().begin();
            cit != m_children.raw().end();
            ++cit)
        {
            (*cit)->designerSetColorRecursively(color);
        }
    }
    if(m_type.raw() == ProductTypePart && m_material.raw())
    {
        m_material.raw()->designerSetAmbientAndDiffuseColor(color);
    }
}
void VasnecovProduct::designerUpdateChildrenMatrix()
{
    if(!m_children.raw().empty())
    {
        for(std::vector<VasnecovProduct *>::const_iterator cit = m_children.raw().begin();
            cit != m_children.raw().end();
            ++cit)
        {
            // Матрица преобразований элемента передается дочерним в качестве матрицы начальных преобразований
            (*cit)->designerSetMatrixM1Recursively(m_Ms.raw());
        }
    }
}
void VasnecovProduct::designerSetMatrixM1Recursively(const QMatrix4x4 &M1)
{
    designerUpdateMatrixM1(M1);
    designerUpdateChildrenMatrix();
}
