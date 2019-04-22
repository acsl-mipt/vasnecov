/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Класс представления 3D-мира.
// Аналог Модели в модели MVC.
#pragma once

#include "ElementList.h"
#include "LightModel.h"
#include "CoreObject.h"

class VasnecovLamp;
class VasnecovProduct;
class VasnecovLabel;
class VasnecovFigure;

class QSize;
class QRect;

namespace Vasnecov
{
    const GLsizei cfg_worldWidthMin = 16;
    const GLsizei cfg_worldHeightMin = 16;
    const GLsizei cfg_worldWidthMax = 4096;
    const GLsizei cfg_worldHeightMax = 4096;
}

class VasnecovWorld : public Vasnecov::CoreObject
{
    // Список контейнеров списков
    class WorldElementList : public Vasnecov::ElementList<Vasnecov::ElementBox>
    {};


public:
    VasnecovWorld(VasnecovPipeline* pipeline,
                  GLint mx, GLint my,
                  GLsizei width, GLsizei height,
                  const QString& name = QString());
    ~VasnecovWorld();

public:
    void setDrawingType(Vasnecov::PolygonDrawingTypes type);
    GLboolean setProjection(Vasnecov::WorldTypes type);
    GLboolean setWindow(GLint x, GLint y, GLsizei width, GLsizei height);
    GLboolean setParameters(Vasnecov::WorldParameters parameters);
    Vasnecov::WorldParameters worldParameters() const;
    Vasnecov::WorldTypes projection() const;

    GLint x() const;
    GLint y() const;
    GLsizei width() const;
    GLsizei height() const;
    QSize size() const;
    QRect window() const; // x, y - left bottom point

    void setDepth();
    void unsetDepth();
    GLboolean depth() const;
    void switchDepth();

    void setLight();
    void unsetLight();
    GLboolean light() const;
    void switchLight();

    GLboolean setPerspective(GLfloat angle, GLfloat frontBorder, GLfloat backBorder); // Задать характеристики перспективной проекции
    Vasnecov::Perspective perspective() const;
    Vasnecov::Ortho ortho() const;

    // TODO: add camera (operator) position with angles (quaternions) & use setCameraAngles in local CS
    void setCamera(const Vasnecov::Camera& camera);
    // Задать положение камеры
    void setCameraPosition(const QVector3D& position);
    void setCameraPosition(GLfloat x, GLfloat y, GLfloat z);
    // Задать точку, в которую смотрит камера
    void setCameraTarget(const QVector3D& target);
    void setCameraTarget(GLfloat x, GLfloat y, GLfloat z);
    // Задать углы направления взгляда камеры
    void setCameraAngles(GLfloat yaw, GLfloat pitch);
    void setCameraAngles(GLfloat yaw, GLfloat pitch, GLfloat roll); // с креном камеры
    // Перемещение камеры
    void flyCamera(const QVector3D& step);
    void flyCamera(GLfloat x, GLfloat y, GLfloat z);
    void moveCamera(const QVector3D& step);
    void moveCamera(GLfloat x, GLfloat y, GLfloat z);
    // Вращение камеры
    void rotateCamera(GLfloat yaw, GLfloat pitch);
    void rotateCamera(GLfloat yaw, GLfloat pitch, GLfloat roll);
    // Задать крен камеры
    void setCameraRoll(GLfloat roll);
    void tiltCamera(GLfloat roll);

    QVector3D cameraPosition() const;
    QVector3D cameraTarget() const;
    float cameraRoll() const;

    Vasnecov::Camera camera() const;

    Vasnecov::Line unprojectPointToLine(const QPointF& point);
    Vasnecov::Line unprojectPointToLine(const QVector2D& point);
    Vasnecov::Line unprojectPointToLine(GLfloat x, GLfloat y);
    QVector2D projectVectorToPoint(const QVector3D& vector); // Vector from 3D to screen position

protected:
    // Списки содержимого
    template<typename T>
    T* designerFindElement(T* element) const;

    // Добавление новых элементов в списки
    template<typename T>
    GLboolean designerAddElement(T* element, GLboolean check = false);

    template<typename T>
    GLboolean designerRemoveElement(T* element);

    void designerUpdateOrtho();

protected:
    // Вызовы из рендерера
    GLenum renderUpdateData();
    void renderDraw();

    void renderSwitchLamps() const;
    VasnecovPipeline::CameraAttributes renderCalculateCamera() const;

    const Vasnecov::WorldParameters& renderWorldParameters() const;
    const Vasnecov::Perspective& renderPerspective() const;
    const Vasnecov::Ortho& renderOrtho() const;
    const Vasnecov::Camera& renderCamera() const;

    template <typename T>
    static void renderDrawElement(T* element)
    {
        if(element)
        {
            element->renderDraw();
        }
    }

private:
    Vasnecov::MutualData<Vasnecov::WorldParameters> _parameters; // Характеристики мира
    Vasnecov::MutualData<Vasnecov::Perspective>     _perspective; // Характеристики вида при перспективной проекции
    Vasnecov::MutualData<Vasnecov::Ortho>           _ortho; // Характеристики вида при ортогональной проекции
    Vasnecov::MutualData<Vasnecov::Camera>          _camera; // камера мира
    Vasnecov::MutualData<QMatrix4x4>                _projectionMatrix;

    Vasnecov::LightModel                            _lightModel;
    WorldElementList                                _elements;

    friend class VasnecovUniverse;

    enum Updated
    {
        Products		= 0x0010,
        Figures			= 0x0020,
        Labels			= 0x0040,
        Lamps			= 0x0080,
        Parameters		= 0x0100,
        Perspective		= 0x0200,
        Ortho			= 0x0400,
        Cameras			= 0x0800,
        Flags			= 0x1000,
        Matrix          = 0x2000
    };

private:
    Q_DISABLE_COPY(VasnecovWorld)
};

template<typename T>
T* VasnecovWorld::designerFindElement(T* element) const
{
    return _elements.findRawElement(element);
}

template<typename T>
GLboolean VasnecovWorld::designerAddElement(T* element, GLboolean check)
{
    return _elements.addElement(element, check);
}

template<typename T>
GLboolean VasnecovWorld::designerRemoveElement(T* element)
{
    return _elements.removeElement(element);
}

inline void VasnecovWorld::renderSwitchLamps() const
{
    if(_parameters.pure().light())
    {
        pure_pipeline->enableLamps();
    }
    else
    {
        pure_pipeline->disableLamps();
    }
}

inline const Vasnecov::WorldParameters& VasnecovWorld::renderWorldParameters() const
{
    return _parameters.pure();
}

inline const Vasnecov::Perspective& VasnecovWorld::renderPerspective() const
{
    return _perspective.pure();
}

inline const Vasnecov::Ortho& VasnecovWorld::renderOrtho() const
{
    return _ortho.pure();
}

inline const Vasnecov::Camera& VasnecovWorld::renderCamera() const
{
    return _camera.pure();
}
