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
 \param mutex
 \param pipeline
 \param type
 \param parent
 \param level
*/
VasnecovProduct::VasnecovProduct(QMutex *mutex, VasnecovPipeline *pipeline, VasnecovProduct::ProductTypes type, VasnecovProduct* parent, GLuint level) :
    VasnecovElement(mutex, pipeline),
    raw_M1(),
    raw_ownVisible(true),

    m_type(raw_wasUpdated, Type, type),
    m_parent(raw_wasUpdated, Parent, parent),
    m_level(raw_wasUpdated, Level, level),

    m_mesh(raw_wasUpdated, Mesh, 0),
    m_material(raw_wasUpdated, Material, 0),
    m_children(raw_wasUpdated, Children),

    m_drawingBox(raw_wasUpdated, DrawingBox, false)
{
    init();
}

/*!
 \brief

 \fn VasnecovProduct::VasnecovProduct
 \param mutex
 \param pipeline
 \param name
 \param type
 \param parent
 \param level
*/
VasnecovProduct::VasnecovProduct(QMutex *mutex, VasnecovPipeline *pipeline, std::string name, VasnecovProduct::ProductTypes type, VasnecovProduct *parent, GLuint level) :
    VasnecovElement(mutex, pipeline, name),
    raw_M1(),
    raw_ownVisible(true),

    m_type(raw_wasUpdated, Type, type),
    m_parent(raw_wasUpdated, Parent, parent),
    m_level(raw_wasUpdated, Level, level),

    m_mesh(raw_wasUpdated, Mesh, 0),
    m_material(raw_wasUpdated, Material, 0),
    m_children(raw_wasUpdated, Children),

    m_drawingBox(raw_wasUpdated, DrawingBox, false)
{
    init();
}
/*!
 \brief

 \fn VasnecovProduct::VasnecovProduct
 \param mutex
 \param pipeline
 \param name
 \param mesh
 \param parent
 \param level
*/
VasnecovProduct::VasnecovProduct(QMutex *mutex, VasnecovPipeline *pipeline, std::string name, VasnecovMesh *mesh, VasnecovProduct *parent, GLuint level) :
    VasnecovElement(mutex, pipeline, name),
    raw_M1(),
    raw_ownVisible(true),

    m_type(raw_wasUpdated, Type, ProductTypePart), // т.к. меш может быть только у детали
    m_parent(raw_wasUpdated, Parent, parent),
    m_level(raw_wasUpdated, Level, level),

    m_mesh(raw_wasUpdated, Mesh, mesh),
    m_material(raw_wasUpdated, Material, 0),
    m_children(raw_wasUpdated, Children),

    m_drawingBox(raw_wasUpdated, DrawingBox, false)
{
    init();
}

/*!
 \brief

 \fn VasnecovProduct::VasnecovProduct
 \param mutex
 \param pipeline
 \param name
 \param mesh
 \param material
 \param parent
 \param level
*/
VasnecovProduct::VasnecovProduct(QMutex *mutex, VasnecovPipeline *pipeline, std::string name, VasnecovMesh *mesh, VasnecovMaterial *material, VasnecovProduct *parent, GLuint level) :
    VasnecovElement(mutex, pipeline, name),
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
    QMutexLocker locker(mtx_data);

    designerOwnSetVisible(visible);
}

void VasnecovProduct::setPositionFromElement(const VasnecovAbstractElement *element)
{
    VasnecovAbstractElement::setPositionFromElement(element);

    if(element)
    {
        QMutexLocker locker(mtx_data);
        designerUpdateChildrenMatrix();
    }
}

void VasnecovProduct::switchDrawingBox()
{
    QMutexLocker locker(mtx_data);

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

    if(m_scale.raw() != 1.0)
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
        centerPoint = m_mesh.pure()->cm();
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
/*!
 \brief

 \fn VasnecovProduct::renderUpdateData
 \return GLenum
*/
GLenum VasnecovProduct::renderUpdateData()
{
    // Проверка прозрачности
    GLboolean transp = false;
    if(m_material.raw() && m_material.raw()->renderTextureD())
    {
        transp = m_material.raw()->renderTextureD()->isTransparency();
    }
    if(m_color.raw().alphaF() < 1.0f)
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

/*!
 \brief Отрисовка продукта через OpenGL-конвейер.

 \note Отрисовщик не возвращает конвейер в исходное состояние.
*/
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

        if(m_drawingBox.pure())
        {
            m_mesh.pure()->drawBorderBox();
        }
        m_mesh.pure()->drawModel();
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

/*!
 \brief

 \fn VasnecovProduct::designerRemoveChild
 \param child
 \return GLboolean
*/
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
/*!
 \brief

 \fn VasnecovProduct::designerAllChildren
 \return std::vector<VasnecovProduct *>
*/
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
/*!
 \brief

 \fn VasnecovProduct::setMaterial
 \param material
*/
void VasnecovProduct::setMaterial(VasnecovMaterial *material)
{
    if(material)
    {
        QMutexLocker locker(mtx_data);

        if(m_type.raw() == ProductTypePart)
        {
            m_material.set(material);
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
    QMutexLocker locker(mtx_data);

    VasnecovMaterial * material(m_material.raw());
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
        QMutexLocker locker(mtx_data);

        if(m_type.raw() == ProductTypePart)
        {
            m_mesh.set(mesh);
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
    QMutexLocker locker(mtx_data);

    VasnecovMesh *mesh(m_mesh.raw());
    return mesh;
}

/*!
 \brief

 \fn VasnecovProduct::level
 \return GLuint
*/
GLuint VasnecovProduct::level() const
{
    QMutexLocker locker(mtx_data);

    GLuint level(m_level.raw());
    return level;
}
/*!
 \brief

 \fn VasnecovProduct::type
 \return VasnecovProduct::ProductTypes
*/
VasnecovProduct::ProductTypes VasnecovProduct::type() const
{
    QMutexLocker locker(mtx_data);

    VasnecovProduct::ProductTypes type(m_type.raw());
    return type;
}

/*!
 \brief

 \fn VasnecovProduct::parent
 \return VasnecovProduct
*/
VasnecovProduct *VasnecovProduct::parent() const
{
    QMutexLocker locker(mtx_data);

    VasnecovProduct *parent(m_parent.raw());
    return parent;
}

void VasnecovProduct::changeParent(VasnecovProduct *newParent)
{
    QMutexLocker locker(mtx_data);

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

/*!
 \brief

 \fn VasnecovProduct::children
 \return std::vector<VasnecovProduct *>
*/
std::vector<VasnecovProduct *> VasnecovProduct::children() const
{
    QMutexLocker locker(mtx_data);

    std::vector<VasnecovProduct *> children(m_children.raw());
    return children;
}

/*!
 \brief

 \fn VasnecovProduct::setColor
 \param color
*/
void VasnecovProduct::setColor(const QColor &color)
{
    QMutexLocker locker(mtx_data);

    if(m_color.raw() != color)
    {
        designerSetColorRecursively(color);
    }
}
/*!
 \brief

 \fn VasnecovProduct::setCoordinates
 \param coordinates
*/
void VasnecovProduct::setCoordinates(const QVector3D &coordinates)
{
    QMutexLocker locker(mtx_data);

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
    if(increment.x() != 0.0 || increment.y() != 0.0 || increment.z() != 0.0)
    {
        QMutexLocker locker(mtx_data);

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
    QMutexLocker locker(mtx_data);

    QVector3D coordinates(m_Ms.raw()(0, 3), m_Ms.raw()(1, 3), m_Ms.raw()(2, 3));
    return coordinates;
}

/*!
 \brief

 \fn VasnecovProduct::setAngles
 \param angles
*/
void VasnecovProduct::setAngles(const QVector3D &angles)
{
    QMutexLocker locker(mtx_data);

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

/*!
 \brief

 \fn VasnecovProduct::incrementAngles
 \param increment
*/
void VasnecovProduct::incrementAngles(const QVector3D &increment)
{
    if(increment.x() != 0.0 || increment.y() != 0.0 || increment.z() != 0.0)
    {
        QMutexLocker locker(mtx_data);

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

/*!
 \brief

 \fn VasnecovProduct::setScale
 \param scale
*/
void VasnecovProduct::setScale(GLfloat scale)
{
    QMutexLocker locker(mtx_data);

    if(m_scale.set(scale))
    {
        designerUpdateMatrixMs();
        designerUpdateChildrenMatrix();
    }
}

/*!
 \brief

 \fn VasnecovProduct::designerSetMatrixM1
 \param M1
*/
void VasnecovProduct::designerSetMatrixM1(const QMatrix4x4 &M1)
{
    if(raw_M1 != M1)
    {
        designerSetMatrixM1Recursively(M1);
    }
}

/*!
 \brief

 \fn VasnecovProduct::designerSetColorRecursively
 \param color
*/
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
/*!
 \brief

 \fn VasnecovProduct::designerUpdateChildrenMatrix
*/
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
