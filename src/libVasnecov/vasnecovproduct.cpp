/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "vasnecovproduct.h"
#include "technologist.h"
#include "vasnecovmaterial.h"
#include "vasnecovmesh.h"
#ifndef _MSC_VER
    #pragma GCC diagnostic warning "-Weffc++"
#endif

/*!
 \brief

 \fn VasnecovProduct::VasnecovProduct
 \param pipeline
 \param type
 \param parent
 \param level
*/
VasnecovProduct::VasnecovProduct(VasnecovPipeline *pipeline, VasnecovProduct::ProductTypes type, VasnecovProduct* parent, GLuint level) :
    VasnecovElement(pipeline),
    raw_M1(),
    raw_ownVisible(true),

    m_type(type),
    m_parent(parent),
    m_level(level),

    m_mesh(nullptr),
    m_material(nullptr),
    m_children(),

    m_drawingBox(false)
{
    init();
}

/*!
 \brief

 \fn VasnecovProduct::VasnecovProduct
 \param pipeline
 \param name
 \param type
 \param parent
 \param level
*/
VasnecovProduct::VasnecovProduct(VasnecovPipeline *pipeline, const std::string& name, VasnecovProduct::ProductTypes type, VasnecovProduct *parent, GLuint level) :
    VasnecovElement(pipeline, name),
    raw_M1(),
    raw_ownVisible(true),

    m_type(type),
    m_parent(parent),
    m_level(level),

    m_mesh(nullptr),
    m_material(nullptr),
    m_children(),

    m_drawingBox(false)
{
    init();
}
/*!
 \brief

 \fn VasnecovProduct::VasnecovProduct
 \param pipeline
 \param name
 \param mesh
 \param parent
 \param level
*/
VasnecovProduct::VasnecovProduct(VasnecovPipeline *pipeline, const std::string& name, VasnecovMesh *mesh, VasnecovProduct *parent, GLuint level) :
    VasnecovElement(pipeline, name),
    raw_M1(),
    raw_ownVisible(true),

    m_type(ProductTypePart), // т.к. меш может быть только у детали
    m_parent(parent),
    m_level(level),

    m_mesh(mesh),
    m_material(nullptr),
    m_children(),

    m_drawingBox(false)
{
    init();
}

/*!
 \brief

 \fn VasnecovProduct::VasnecovProduct
 \param pipeline
 \param name
 \param mesh
 \param material
 \param parent
 \param level
*/
VasnecovProduct::VasnecovProduct(VasnecovPipeline *pipeline, const std::string& name, VasnecovMesh *mesh, VasnecovMaterial *material, VasnecovProduct *parent, GLuint level) :
    VasnecovElement(pipeline, name),
    raw_M1(),
    raw_ownVisible(true),

    m_type(ProductTypePart), // т.к. меш может быть только у детали
    m_parent(parent),
    m_level(level),

    m_mesh(mesh),
    m_material(material),
    m_drawingBox(false)
{
    init();
}
/*!
 \brief

 \fn VasnecovProduct::~VasnecovProduct
*/
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

void VasnecovProduct::switchDrawingBox()
{
    m_drawingBox = !m_drawingBox;
}

void VasnecovProduct::designerOwnSetVisible(bool visible)
{
    raw_ownVisible = visible;

    if(m_parent)
    {
        designerSetVisibleFromParent(m_parent->designerIsVisible() && raw_ownVisible);
    }
    else
    {
        designerSetVisibleFromParent(raw_ownVisible);
    }
}

void VasnecovProduct::designerSetVisibleFromParent(bool visible)
{
    bool trueVis = false;

    if(visible && raw_ownVisible)
    {
        trueVis = true;
    }

    m_isHidden = !trueVis;

    if(!m_children.empty())
    {
        for(const auto& cit: m_children)
        {
            cit->designerSetVisibleFromParent(trueVis);
        }
    }
}

void VasnecovProduct::designerUpdateMatrixMs()
{
    m_Ms = raw_M1;
    m_Ms.translate(raw_coordinates);

    QQuaternion qRot;
    qRot = raw_qZ * raw_qX * raw_qY;

    m_Ms.rotate(qRot);

    if(m_scale != 1.0f)
    {
        m_Ms.scale(m_scale, m_scale, m_scale);
    }
}

GLfloat VasnecovProduct::renderCalculateDistanceToPlane(const QVector3D &planePoint, const QVector3D &normal)
{
    QVector3D centerPoint;
    if(m_mesh)
    {
        centerPoint = m_mesh->cm();
    }

    if(m_alienMs)
    {
        centerPoint = (*m_alienMs) * m_Ms * centerPoint;
    }
    else
    {
        centerPoint = m_Ms * centerPoint;
    }

    pure_distance = centerPoint.distanceToPlane(planePoint, normal);

    return pure_distance;
}
/*!
 \brief

 \fn VasnecovProduct::renderUpdateData
 \return GLenum
*/
GLenum VasnecovProduct::renderUpdateData()
{
    // Проверка прозрачности
    GLboolean transp = false;
    if(m_material && m_material->renderTextureD())
    {
        transp = m_material->renderTextureD()->isTransparency();
    }
    if(m_color.alphaF() < 1.0)
    {
        transp = true;
    }
    m_isTransparency = transp;

    // Далее, как обычно
    GLenum updated = raw_wasUpdated;

    if(raw_wasUpdated)
    {
        pure_pipeline->setSomethingWasUpdated();

        // Копирование сырых данных в основные
        VasnecovElement::renderUpdateData();
    }

    return updated;
}

/*!
 \brief Отрисовка продукта через OpenGL-конвейер.

 \note Отрисовщик не возвращает конвейер в исходное состояние.
*/
void VasnecovProduct::renderDraw()
{
    if(!m_isHidden &&
       m_type == ProductTypePart &&
       m_mesh)
    {
        renderApplyTranslation();

        if(m_material)
        {
            m_material->renderDraw();
        }
        else
        {
            pure_pipeline->disableTexture2D();
            pure_pipeline->setColor(m_color);
        }

        if(m_drawingBox)
        {
            m_mesh->drawBorderBox();
        }
        m_mesh->drawModel();
    }
}

/*!
 \brief

 \fn VasnecovProduct::designerAddChild
 \param child
 \return GLboolean
*/
GLboolean VasnecovProduct::designerAddChild(VasnecovProduct *child)
{
    GLboolean res = false;
    if (!child)
        return false;

    if (m_type != ProductTypeAssembly || m_level > Vasnecov::cfg_elementMaxLevel)
        return false;

    m_children.push_back(child);
    child->designerSetMatrixM1Recursively(m_Ms);
    child->designerSetColorRecursively(m_color);
    return true;
}

/*!
 \brief

 \fn VasnecovProduct::designerRemoveChild
 \param child
 \return GLboolean
*/
GLboolean VasnecovProduct::designerRemoveChild(VasnecovProduct *child)
{
    if (!child || m_type != ProductTypeAssembly)
        return false;

    const auto i = std::find_if(m_children.begin(), m_children.end(), [=](const VasnecovProduct* v) { return v == child; });
    if (i == m_children.end())
        return false;

    m_children.erase(i);
    return true;
}
/*!
 \brief

 \fn VasnecovProduct::designerAllChildren
 \return std::vector<VasnecovProduct *>
*/
std::vector<VasnecovProduct *> VasnecovProduct::designerAllChildren()
{
    std::vector<VasnecovProduct *> children;
    if (m_type != ProductTypeAssembly)
        return children;

    for(const auto& cit: m_children)
    {
        children.push_back(cit);

        std::vector<VasnecovProduct *> grandchildren = cit->designerAllChildren();
        children.insert(children.end(), grandchildren.begin(), grandchildren.end());
    }
    return children;
}
/*!
 \brief

 \fn VasnecovProduct::setMaterial
 \param material
*/
void VasnecovProduct::setMaterial(VasnecovMaterial *material)
{
    if(material)
    {
        if(m_type == ProductTypePart)
        {
            m_material = material;
        }
    }
}
/*!
 \brief

 \fn VasnecovProduct::material
 \return VasnecovMaterial
*/
VasnecovMaterial *VasnecovProduct::material() const
{
    VasnecovMaterial * material(m_material);
    return material;
}
/*!
 \brief

 \fn VasnecovProduct::setMesh
 \param mesh
*/
void VasnecovProduct::setMesh(VasnecovMesh *mesh)
{
    if(mesh)
    {
        if(m_type == ProductTypePart)
        {
            m_mesh = mesh;
        }
    }
}
/*!
 \brief

 \fn VasnecovProduct::mesh
 \return VasnecovMesh
*/
VasnecovMesh *VasnecovProduct::mesh() const
{
    VasnecovMesh *mesh(m_mesh);
    return mesh;
}

/*!
 \brief

 \fn VasnecovProduct::level
 \return GLuint
*/
GLuint VasnecovProduct::level() const
{
    GLuint level(m_level);
    return level;
}
/*!
 \brief

 \fn VasnecovProduct::type
 \return VasnecovProduct::ProductTypes
*/
VasnecovProduct::ProductTypes VasnecovProduct::type() const
{
    VasnecovProduct::ProductTypes type(m_type);
    return type;
}

/*!
 \brief

 \fn VasnecovProduct::parent
 \return VasnecovProduct
*/
VasnecovProduct *VasnecovProduct::parent() const
{
    VasnecovProduct *parent(m_parent);
    return parent;
}

void VasnecovProduct::changeParent(VasnecovProduct *newParent)
{
    if(m_parent)
    {
        m_parent->designerRemoveChild(this);
    }
    if(newParent)
    {
        m_parent = newParent;
        newParent->designerAddChild(this);
    }
    else // Нет родителя - элемент глобальный
    {
        designerSetMatrixM1Recursively(QMatrix4x4());
    }
}

/*!
 \brief

 \fn VasnecovProduct::children
 \return std::vector<VasnecovProduct *>
*/
const std::vector<VasnecovProduct*>& VasnecovProduct::children() const
{
    return m_children;
}

/*!
 \brief

 \fn VasnecovProduct::setColor
 \param color
*/
void VasnecovProduct::setColor(const QColor &color)
{
    if(m_color != color)
        designerSetColorRecursively(color);
}
/*!
 \brief

 \fn VasnecovProduct::setCoordinates
 \param coordinates
*/
void VasnecovProduct::setCoordinates(const QVector3D &coordinates)
{
    if(raw_coordinates != coordinates)
    {
        raw_coordinates = coordinates;

        designerUpdateMatrixMs();
        designerUpdateChildrenMatrix();
    }
}

/*!
 \brief

 \fn VasnecovProduct::incrementCoordinates
 \param increment
*/
void VasnecovProduct::incrementCoordinates(const QVector3D &increment)
{
    if(increment.x() != 0.0f || increment.y() != 0.0f || increment.z() != 0.0f)
    {
        raw_coordinates += increment;

        designerUpdateMatrixMs();
        designerUpdateChildrenMatrix();
    }
}
/*!
 \brief

 \fn VasnecovProduct::globalCoordinates
 \return QVector3D
*/
QVector3D VasnecovProduct::globalCoordinates()
{
    return QVector3D(m_Ms(0, 3), m_Ms(1, 3), m_Ms(2, 3));
}

/*!
 \brief

 \fn VasnecovProduct::setAngles
 \param angles
*/
void VasnecovProduct::setAngles(const QVector3D &angles)
{
    if (raw_angles == angles)
        return;

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

/*!
 \brief

 \fn VasnecovProduct::incrementAngles
 \param increment
*/
void VasnecovProduct::incrementAngles(const QVector3D &increment)
{
    if (increment.x() == 0.0f && increment.y() == 0.0f && increment.z() == 0.0f)
        return;

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

/*!
 \brief

 \fn VasnecovProduct::setScale
 \param scale
*/
void VasnecovProduct::setScale(GLfloat scale)
{
    if (m_scale == scale)
        return;
    m_scale = scale;
    designerUpdateMatrixMs();
    designerUpdateChildrenMatrix();
}

/*!
 \brief

 \fn VasnecovProduct::designerSetMatrixM1
 \param M1
*/
void VasnecovProduct::designerSetMatrixM1(const QMatrix4x4 &M1)
{
    if(raw_M1 != M1)
        designerSetMatrixM1Recursively(M1);
}

/*!
 \brief

 \fn VasnecovProduct::designerSetColorRecursively
 \param color
*/
void VasnecovProduct::designerSetColorRecursively(const QColor &color)
{
    m_color = color;

    if(!m_children.empty())
    {
        for(const auto& cit: m_children)
        {
            cit->designerSetColorRecursively(color);
        }
    }
    if(m_type == ProductTypePart && m_material)
    {
        m_material->designerSetAmbientAndDiffuseColor(color);
    }
}
/*!
 \brief

 \fn VasnecovProduct::designerUpdateChildrenMatrix
*/
void VasnecovProduct::designerUpdateChildrenMatrix()
{
    if(!m_children.empty())
    {
        for(const auto& cit: m_children)
        {
            // Матрица преобразований элемента передается дочерним в качестве матрицы начальных преобразований
            cit->designerSetMatrixM1Recursively(m_Ms);
        }
    }
}

/*!
 \brief

 \fn VasnecovProduct::designerSetMatrixM1Recursively
 \param M1
*/
void VasnecovProduct::designerSetMatrixM1Recursively(const QMatrix4x4 &M1)
{
    designerUpdateMatrixM1(M1);
    designerUpdateChildrenMatrix();
}
#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
