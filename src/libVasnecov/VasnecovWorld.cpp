/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "Configuration.h"
#include "Technologist.h"
#include "VasnecovFigure.h"
#include "VasnecovLabel.h"
#include "VasnecovLamp.h"
#include "VasnecovMaterial.h"
#include "VasnecovProduct.h"
#include "VasnecovTerrain.h"
#include "VasnecovWorld.h"

#include <QSize>
#include <QRect>

VasnecovWorld::VasnecovWorld(VasnecovPipeline* pipeline,
                             GLint mx, GLint my,
                             GLsizei width, GLsizei height,
                             const QString& name) :
    Vasnecov::CoreObject(pipeline, name),
    _parameters(raw_wasUpdated, Parameters),
    _perspective(raw_wasUpdated, Perspective),
    _ortho(raw_wasUpdated, Ortho),
    _camera(raw_wasUpdated, Cameras),
    _projectionMatrix(raw_wasUpdated, Matrix),
    _lightModel(),

    _elements()
{
    _parameters.editableRaw().setX(mx);
    _parameters.editableRaw().setY(my);
    _parameters.editableRaw().setWidth(width);
    _parameters.editableRaw().setHeight(height);

    // Перспективная проекция
    _perspective.editableRaw().ratio = static_cast<GLfloat>(width)/height;

    designerUpdateOrtho();

    renderUpdateData();
}
VasnecovWorld::~VasnecovWorld()
{
}
void VasnecovWorld::designerUpdateOrtho()
{
    // Ортогональная проекция (по данным перспективной и положению камеры)
    GLfloat dist;
    dist = (_camera.raw().target() - _camera.raw().position()).length();

    if(dist == 0)
    {
        dist = 1.0f;
    }

    Vasnecov::Ortho ortho;

    ortho.top = tan(_perspective.raw().angle*c_degToRad*0.5f)*dist;
    ortho.bottom = - ortho.top;
    ortho.left = _perspective.raw().ratio*ortho.bottom;
    ortho.right = _perspective.raw().ratio*ortho.top;
    ortho.front = -dist*10.0f; // NOTE: it's stupidy, but i don't know how to make it correct
    ortho.back = (_perspective.raw().backBorder+dist)*10.0f;

    _ortho.set(ortho);
}
void VasnecovWorld::setCameraAngles(GLfloat yaw, GLfloat pitch)
{
    QVector3D vec(1.0, 0.0, 0.0); // Единичный вектор по направлению X

    QQuaternion qYaw = QQuaternion::fromAxisAndAngle(0, 0, 1, yaw);
    QQuaternion qPitch = QQuaternion::fromAxisAndAngle(QVector3D::crossProduct(QVector3D(0, 0, 1), vec).normalized(), pitch);

    vec = qYaw.rotatedVector(vec);
    vec = qPitch.rotatedVector(vec);

    vec *= (_camera.raw().target() - _camera.raw().position()).length(); // Увеличение на длину

    QVector3D target = vec + _camera.raw().position();
    if(_camera.raw().target() != target)
    {
        _camera.editableRaw().setTarget(target);
        designerUpdateOrtho();
    }
}
void VasnecovWorld::setCameraAngles(GLfloat yaw, GLfloat pitch, GLfloat roll)
{
    if(_camera.raw().roll() != roll)
    {
        _camera.editableRaw().setRoll(roll);
    }

    setCameraAngles(yaw, pitch);
}

void VasnecovWorld::flyCamera(const QVector3D &step)
{

    QVector3D forward = (_camera.raw().target() - _camera.raw().position()).normalized();

    bool updated(false);
    QVector3D target, position;

    if(step.x())
    {
        forward *= step.x();

        target = forward + _camera.raw().target();
        position = forward + _camera.raw().position();

        updated = true;
    }
    if(step.y())
    {
        QVector3D side = QVector3D::crossProduct(QVector3D(0, 0, 1), forward).normalized();
        side *= step.y();

        target = side + _camera.raw().target();
        position = side + _camera.raw().position();

        updated = true;
    }
    if(step.z())
    {
        target = QVector3D(0, 0, 1)*step.z() + _camera.raw().target();
        position = QVector3D(0, 0, 1)*step.z() + _camera.raw().position();

        updated = true;
    }

    if(updated)
    {
        GLboolean orth(false);

        if(_camera.raw().target() != target)
        {
            _camera.editableRaw().setTarget(target);
            orth = true;
        }
        if(_camera.raw().position() != position)
        {
            _camera.editableRaw().setPosition(position);
            orth = true;
        }

        if(orth)
            designerUpdateOrtho();
    }
}

void VasnecovWorld::flyCamera(GLfloat x, GLfloat y, GLfloat z)
{
    flyCamera(QVector3D(x, y, z));
}

void VasnecovWorld::moveCamera(const QVector3D &step)
{
    bool updated(false);
    QVector3D target, position;

    // Перемещение в горизонтальной плоскости
    if(step.x() || step.y())
    {
        // Направление на цель в горизональной плоскости (x)
        QVector3D forward = (_camera.raw().target() - _camera.raw().position());
        forward.setZ(0);
        forward.normalize();
        // Направление вбок (y)
        QVector3D side = QVector3D::crossProduct(QVector3D(0, 0, 1), forward).normalized();
        side.normalize();

        forward *= step.x();
        side *= step.y();

        target = forward + side + _camera.raw().target();
        position = forward + side + _camera.raw().position();

        updated = true;

    }
    if(step.z())
    {
        target = QVector3D(0, 0, 1)*step.z() + _camera.raw().target();
        position = QVector3D(0, 0, 1)*step.z() + _camera.raw().position();

        updated = true;
    }

    if(updated)
    {
        GLboolean orth(false);

        if(_camera.raw().target() != target)
        {
            _camera.editableRaw().setTarget(target);
            orth = true;
        }
        if(_camera.raw().position() != position)
        {
            _camera.editableRaw().setPosition(position);
            orth = true;
        }

        if(orth)
            designerUpdateOrtho();
    }
}

void VasnecovWorld::moveCamera(GLfloat x, GLfloat y, GLfloat z)
{
    moveCamera(QVector3D(x, y, z));
}

void VasnecovWorld::rotateCamera(GLfloat yaw, GLfloat pitch)
{
    QVector3D vec = _camera.raw().target() - _camera.raw().position();

    QQuaternion qYaw = QQuaternion::fromAxisAndAngle(0, 0, 1, yaw);
    QQuaternion qPitch = QQuaternion::fromAxisAndAngle(QVector3D::crossProduct(QVector3D(0, 0, 1), vec).normalized(), pitch);

    vec = qYaw.rotatedVector(vec);
    vec = qPitch.rotatedVector(vec);

    QVector3D target = vec + _camera.raw().position();
    if(_camera.raw().target() != target)
    {
        _camera.editableRaw().setTarget(target);
        designerUpdateOrtho();
    }
}

void VasnecovWorld::rotateCamera(GLfloat yaw, GLfloat pitch, GLfloat roll)
{
    if(_camera.raw().roll() != roll)
    {
        _camera.editableRaw().setRoll(roll);
    }

    rotateCamera(yaw, pitch);
}

void VasnecovWorld::setCameraRoll(GLfloat roll)
{
    if(_camera.raw().roll() != roll)
    {
        _camera.editableRaw().setRoll(roll);
        designerUpdateOrtho();
    }
}

void VasnecovWorld::tiltCamera(GLfloat roll)
{
    if(roll != 0.0f)
    {
        _camera.editableRaw().setRoll(_camera.raw().roll() + roll);
        designerUpdateOrtho();
    }
}

QVector3D VasnecovWorld::cameraPosition() const
{
    return _camera.raw().position();
}

QVector3D VasnecovWorld::cameraTarget() const
{
    return _camera.raw().target();
}

float VasnecovWorld::cameraRoll() const
{
    return _camera.raw().roll();
}
GLenum VasnecovWorld::renderUpdateData()
{
    GLenum updated(0);

    // Обновление своих данных
    updated |= _elements.synchronizeAll();

    if(raw_wasUpdated)
    {
        pure_pipeline->setSomethingWasUpdated();

        _parameters.update();
        _perspective.update();
        _ortho.update();
        _camera.update();

        // Matrix is only one object edited by renderer and readed by designer
        _projectionMatrix.synchronizeRaw();

        Vasnecov::CoreObject::renderUpdateData();
    }
    return updated;
}
void VasnecovWorld::renderDraw()
{
    if(m_isHidden.pure())
        return;

    pure_pipeline->clearZBuffer();

    // Задание характеристик мира
    pure_pipeline->activateDepth(_parameters.pure().depth());
    pure_pipeline->setDrawingType(_parameters.pure().drawingType());
    pure_pipeline->setColor(QColor(255, 255, 255, 255));

    // Проецирование и установка камеры
    pure_pipeline->setViewport(_parameters.pure().x(), _parameters.pure().y(), _parameters.pure().width(), _parameters.pure().height());

    if(_parameters.pure().projection() == Vasnecov::WorldTypePerspective) // Перспективная проекция
    {
        pure_pipeline->setPerspective(_perspective.pure(), renderCalculateCamera());
    }
    else // Ортогональная
    {
        pure_pipeline->setOrtho(_ortho.pure(), renderCalculateCamera());
    }

    _projectionMatrix.editablePure() = pure_pipeline->matrixP();

    // Моделирование
    // Обработка источников света
    pure_pipeline->disableAllConcreteLamps(); // Выключение ламп, которые могли быть задействованы
    pure_pipeline->disableLamps();

    GLboolean lampsWork(false);
    if(_elements.hasPureLamps() && _parameters.pure().light())
    {
        pure_pipeline->setAmbientColor(_lightModel.ambientColor());
        pure_pipeline->enableLamps();
        lampsWork = true;

        _elements.forEachPureLamp(renderDrawElement<VasnecovLamp>);
    }

    // Draw terrains
    if(_elements.hasPureTerrains())
    {
        pure_pipeline->enableDepth();

        // Задание материала по умолчанию
        VasnecovMaterial mat(pure_pipeline);
        mat.renderDraw();

        for(auto terrain : _elements.pureTerrains())
            terrain->renderDraw();
    }

    // Рисование фигур (непрозрачных)
    auto startDrawFigures = [this]()
    {
        // Задание материала по умолчанию
        VasnecovMaterial mat(pure_pipeline);
        mat.renderDraw();

        pure_pipeline->disableLamps();
        pure_pipeline->enableBackFaces();
        pure_pipeline->disableTexture2D();
        pure_pipeline->disableSmoothShading();
    };
    auto stopDrawFigures = [this]()
    {
        pure_pipeline->setLineWidth(1.0f);
        pure_pipeline->setPointSize(1.0f);
        pure_pipeline->disableLineStipple();
        pure_pipeline->disableBackFaces();
        pure_pipeline->enableSmoothShading();
        renderSwitchLamps();
    };

    auto checkLighting = [this, lampsWork](const VasnecovFigure* figure)
    {
        if(figure->renderLighting() && lampsWork)
            pure_pipeline->enableLamps();
        else
            pure_pipeline->disableLamps();
    };

    std::vector<VasnecovFigure *> transFigures;
    size_t withoutDepthAmount(0);

    if(_elements.hasPureFigures())
    {
        startDrawFigures();

        transFigures.reserve(_elements.pureFigures().size());
        for(auto figure : _elements.pureFigures())
        {
            if(!figure->renderHasDepth())
            {
                ++withoutDepthAmount;
                continue;
            }

            if(figure->renderIsTransparency())
            {
                transFigures.push_back(figure);
            }
            else
            {
                checkLighting(figure);
                figure->renderDraw();
            }
        }

        stopDrawFigures();
    }

    // Отрисовка непрозрачных изделий (деталей)
    std::vector<VasnecovProduct *> transProducts;

    if(_elements.hasPureProducts())
    {
        transProducts.reserve(_elements.pureProducts().size());
        for(std::vector<VasnecovProduct *>::const_iterator pit = _elements.pureProducts().begin();
            pit != _elements.pureProducts().end(); ++pit)
        {
            VasnecovProduct *prod(*pit);
            if(prod)
            {
                if(prod->renderIsTransparency())
                {
                    transProducts.push_back(prod);
                }
                else
                {
                    prod->renderDraw();
                }
            }
        }
    }

    // Прозрачные и полупрозрачные изделия (детали)
    if(!transProducts.empty())
    {
        if(Vasnecov::cfg_sortTransparency)
        {
            QVector3D viewVector = (_camera.pure().target() - _camera.pure().position()).normalized();
            QVector3D viewPoint = _camera.pure().position();

            for(std::vector<VasnecovProduct *>::const_iterator pit = transProducts.begin();
                pit != transProducts.end(); ++pit)
            {
                VasnecovProduct *prod(*pit);
                if(prod)
                {
                    prod->renderCalculateDistanceToPlane(viewPoint, viewVector);
                }
            }

            std::sort(transProducts.begin(), transProducts.end(), VasnecovElement::renderCompareByReverseDistance);
        }

        for(std::vector<VasnecovProduct *>::const_iterator pit = transProducts.begin();
            pit != transProducts.end(); ++pit)
        {
            VasnecovProduct *prod(*pit);
            if(prod)
            {
                prod->renderDraw();
            }
        }
    }

    // Рисование фигур (прозрачных)
    if(!transFigures.empty())
    {
        if(Vasnecov::cfg_sortTransparency)
        {
            QVector3D viewVector = (_camera.pure().target() - _camera.pure().position()).normalized();
            QVector3D viewPoint = _camera.pure().position();

            for(std::vector<VasnecovFigure *>::iterator fit = transFigures.begin();
                fit != transFigures.end(); ++fit)
            {
                VasnecovFigure *fig(*fit);
                if(fig)
                {
                    fig->renderCalculateDistanceToPlane(viewPoint, viewVector);
                }
            }

            std::sort(transFigures.begin(), transFigures.end(), VasnecovElement::renderCompareByReverseDistance);
        }

        startDrawFigures();

        for(auto figure : transFigures)
        {
            checkLighting(figure);
            figure->renderDraw();
        }
        stopDrawFigures();
    }

    // Draw figures without depth
    if(withoutDepthAmount)
    {
        startDrawFigures();

        for(auto figure : _elements.pureFigures())
        {
            if(figure->renderHasDepth())
                continue;

            checkLighting(figure);
            figure->renderDraw();
        }

        stopDrawFigures();
    }

    // Отрисовка меток
    if(_elements.hasPureLabels())
    {
        pure_pipeline->disableDepth();
        pure_pipeline->clearZBuffer();
        pure_pipeline->disableLamps();
        pure_pipeline->disableBackFaces();

        pure_pipeline->setOrtho2D();
            _elements.forEachPureLabel(renderDrawElement<VasnecovLabel>);
        pure_pipeline->unsetOrtho2D();
    }
}
Vasnecov::WorldParameters VasnecovWorld::worldParameters() const
{
    Vasnecov::WorldParameters parameters(_parameters.raw());
    return parameters;
}

Vasnecov::WorldTypes VasnecovWorld::projection() const
{
    Vasnecov::WorldTypes projection(_parameters.raw().projection());
    return projection;
}

GLint VasnecovWorld::x() const
{
    return _parameters.raw().x();
}

GLint VasnecovWorld::y() const
{
    return _parameters.raw().y();
}

GLsizei VasnecovWorld::width() const
{
    return _parameters.raw().width();
}

GLsizei VasnecovWorld::height() const
{
    return _parameters.raw().height();
}

QSize VasnecovWorld::size() const
{
    return QSize(_parameters.raw().width(), _parameters.raw().height());
}
QRect VasnecovWorld::window() const
{
    return QRect(_parameters.raw().x(),
                 _parameters.raw().y(),
                 _parameters.raw().width(),
                 _parameters.raw().height());
}
void VasnecovWorld::setDrawingType(Vasnecov::PolygonDrawingTypes type)
{
    if(_parameters.raw().drawingType() != type)
    {
        _parameters.editableRaw().setDrawingType(type);
    }
}

void VasnecovWorld::setCameraPosition(const QVector3D &position)
{
    if(_camera.raw().position() != position)
    {
        _camera.editableRaw().setPosition(position);
        designerUpdateOrtho();
    }
}
void VasnecovWorld::setCameraPosition(GLfloat x, GLfloat y, GLfloat z)
{
    setCameraPosition(QVector3D(x, y, z));
}
void VasnecovWorld::setCameraTarget(const QVector3D &target)
{
    if(_camera.raw().target() != target)
    {
        _camera.editableRaw().setTarget(target);
        designerUpdateOrtho();
    }
}
void VasnecovWorld::setCameraTarget(GLfloat x, GLfloat y, GLfloat z)
{
    setCameraTarget(QVector3D(x, y, z));
}
void VasnecovWorld::setCamera(const Vasnecov::Camera &camera)
{
    if(_camera.set(camera))
        designerUpdateOrtho(); // т.к. изменяется расстояние между фронтальными границами
}
Vasnecov::Camera VasnecovWorld::camera() const
{
    Vasnecov::Camera camera(_camera.raw());
    return camera;
}

Vasnecov::Line VasnecovWorld::unprojectPointToLine(const QPointF &point)
{
    return unprojectPointToLine(point.x(), point.y());
}

Vasnecov::Line VasnecovWorld::unprojectPointToLine(const QVector2D& point)
{
    return unprojectPointToLine(point.x(), point.y());
}

Vasnecov::Line VasnecovWorld::unprojectPointToLine(GLfloat x, GLfloat y)
{
    if(x >= _parameters.raw().x() &&
       y >= _parameters.raw().y() &&
       x <= (_parameters.raw().x() + _parameters.raw().width()) &&
       y <= (_parameters.raw().y() + _parameters.raw().height()) &&
       _parameters.raw().width() > 0.0f &&
       _parameters.raw().height() > 0.0f)
    {
        // Поскольку камера умножается только на проективную матрицу, то матрица модели-вида здесь не нужна
        QMatrix4x4 matInverted = _projectionMatrix.raw().inverted();

        // z [0; 1]
        QVector4D viewVector((x - _parameters.raw().x()) * 2.0f / _parameters.raw().width() - 1.0f,
                             (y - _parameters.raw().y()) * 2.0f / _parameters.raw().height() - 1.0f,
                             -1.0f,
                             1.0f);

        QVector4D point1 = matInverted * viewVector;
        if(qFuzzyIsNull(static_cast<double>(point1.w())))
            point1.setW(1.0f);
        point1 /= point1.w();

        viewVector.setZ(1.0f);
        QVector4D point2 = matInverted * viewVector;
        if(qFuzzyIsNull(static_cast<double>(point2.w())))
            point2.setW(1.0f);
        point2 /= point2.w();

        return Vasnecov::Line(point1.toVector3D(), point2.toVector3D());
    }

    return Vasnecov::Line();
}

QVector2D VasnecovWorld::projectVectorToPoint(const QVector3D& vector)
{
    if(_parameters.raw().width() <= 0.0f || _parameters.raw().height() <= 0.0f)
        return QVector2D();

    QVector3D point = _projectionMatrix.raw() * vector;
    return QVector2D((point.x() + 1.0f) * _parameters.raw().width() * 0.5f + _parameters.raw().x(),
                     (point.y() + 1.0f) * _parameters.raw().height() * 0.5f + _parameters.raw().y());
}

Vasnecov::Perspective VasnecovWorld::perspective() const
{
    Vasnecov::Perspective perspective(_perspective.raw());
    return perspective;
}
Vasnecov::Ortho VasnecovWorld::ortho() const
{
    Vasnecov::Ortho orto(_ortho.raw());
    return orto;
}
GLboolean VasnecovWorld::setProjection(Vasnecov::WorldTypes type)
{
    if(type == Vasnecov::WorldTypePerspective || type == Vasnecov::WorldTypeOrthographic)
    {
        if(_parameters.raw().projection() != type)
        {
            _parameters.editableRaw().setProjection(type);
        }
        return 1;
    }
    else
    {
        return 0;
    }
}
GLboolean VasnecovWorld::setWindow(GLint x, GLint y, GLsizei width, GLsizei height)
{
    if(width > Vasnecov::cfg_worldWidthMin && width < Vasnecov::cfg_worldWidthMax &&
       height > Vasnecov::cfg_worldHeightMin && height < Vasnecov::cfg_worldHeightMax)
    {
        GLboolean updated (false);

        if(_parameters.raw().x() != x)
        {
            _parameters.editableRaw().setX(x);
            updated = true;
        }

        if(_parameters.raw().y() != y)
        {
            _parameters.editableRaw().setY(y);
            updated = true;
        }

        if(_parameters.raw().width() != width)
        {
            _parameters.editableRaw().setWidth(width);
            updated = true;
        }

        if(_parameters.raw().height() != height)
        {
            _parameters.editableRaw().setHeight(height);
            updated = true;
        }


        if(!qFuzzyCompare(_perspective.raw().ratio, static_cast<GLfloat>(width)/height))
        {
            _perspective.editableRaw().ratio = static_cast<GLfloat>(width)/height;
            updated = true;
        }

        if(updated)
            designerUpdateOrtho();

        return 1;
    }
    else
    {
        return 0;
    }
}
GLboolean VasnecovWorld::setParameters(Vasnecov::WorldParameters parameters) // TODO: to different parameters
{
    if(parameters.width() > Vasnecov::cfg_worldWidthMin && parameters.width() < Vasnecov::cfg_worldWidthMax &&
       parameters.height() > Vasnecov::cfg_worldHeightMin && parameters.height() < Vasnecov::cfg_worldHeightMax &&
       (parameters.projection() == Vasnecov::WorldTypePerspective ||
        parameters.projection() == Vasnecov::WorldTypeOrthographic))
    {
        GLboolean updated(false);

        updated = _parameters.set(parameters);

        if(!qFuzzyCompare(_perspective.raw().ratio, static_cast<GLfloat>(parameters.width())/parameters.height()))
        {
            _perspective.editableRaw().ratio = static_cast<GLfloat>(parameters.width())/parameters.height();
            updated = true;
        }

        if(updated)
            designerUpdateOrtho();

        return 1;
    }
    else
    {
        return 0;
    }
}
GLboolean VasnecovWorld::setPerspective(GLfloat angle, GLfloat frontBorder, GLfloat backBorder)
{
    if((angle > 0.0f && angle < 180.0f) && frontBorder > 0.0f)
    {
        GLboolean updated(false);

        if(!qFuzzyCompare(_perspective.raw().frontBorder, frontBorder))
        {
            _perspective.editableRaw().frontBorder = frontBorder;
            updated = true;
        }

        if(!qFuzzyCompare(_perspective.raw().backBorder, backBorder))
        {
            _perspective.editableRaw().backBorder = backBorder;
            updated = true;
        }

        if(!qFuzzyCompare(_perspective.raw().angle, angle))
        {
            _perspective.editableRaw().angle = angle;
            updated = true;
        }

        if(updated)
            designerUpdateOrtho();

        return 1;
    }
    else
    {
        return 0;
    }
}
void VasnecovWorld::setDepth()
{
    if(!_parameters.raw().depth())
        _parameters.editableRaw().setDepth(true);
}
void VasnecovWorld::unsetDepth()
{
    if(_parameters.raw().depth())
        _parameters.editableRaw().setDepth(false);
}

GLboolean VasnecovWorld::depth() const
{
    return _parameters.raw().depth();
}

void VasnecovWorld::switchDepth()
{
    _parameters.editableRaw().setDepth(!_parameters.raw().depth());
}
void VasnecovWorld::setLight()
{
    if(!_parameters.raw().light())
        _parameters.editableRaw().setLight(true);
}
void VasnecovWorld::unsetLight()
{
    if(_parameters.editableRaw().light())
        _parameters.editableRaw().setLight(false);
}

GLboolean VasnecovWorld::light() const
{
    return _parameters.raw().light();
}

void VasnecovWorld::switchLight()
{
    _parameters.editableRaw().setLight(!_parameters.raw().light());
}

VasnecovPipeline::CameraAttributes VasnecovWorld::renderCalculateCamera() const
{
    // Расчет направлений камеры
    VasnecovPipeline::CameraAttributes cameraAttr;
    QVector3D forward = (_camera.pure().target() - _camera.pure().position()).normalized();

    cameraAttr.eye = _camera.pure().position();
    cameraAttr.center = _camera.pure().target();

    // Камере (lookAt) не нужен строгий вектор направления вверх. LookAt всё равно берет с него проекцию.
    // Если вектор направления взгляда вертикален, то up-вектор просто направляем по X
    if(_camera.pure().position().x() == _camera.pure().target().x() && _camera.pure().position().y() == _camera.pure().target().y())
    {
        cameraAttr.up = QVector3D(1.0f, 0.0f, 0.0f);
    }
    else // иначе, по Z
    {
        cameraAttr.up = QVector3D(0.0f, 0.0f, 1.0f);
    }

    // Вращение вокруг вектора взгляда
    cameraAttr.up = QQuaternion::fromAxisAndAngle(forward, _camera.pure().roll()).rotatedVector(cameraAttr.up);

    return cameraAttr;
}
