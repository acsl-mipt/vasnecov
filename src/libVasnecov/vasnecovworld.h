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
#ifndef VASNECOV_WORLD_H
#define VASNECOV_WORLD_H

#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
#include "elementlist.h"
#include "vasnecovlamp.h"
#ifndef _MSC_VER
    #pragma GCC diagnostic warning "-Weffc++"
#endif

class VasnecovProduct;
class VasnecovLabel;
class VasnecovFigure;

class QSize;
class QRect;

namespace Vasnecov
{
    const GLsizei cfg_worldWidthMin = 16;
    const GLsizei cfg_worldHeightMin = 16;
    const GLsizei cfg_worldWidthMax = 2048;
    const GLsizei cfg_worldHeightMax = 2048;
}

class VasnecovWorld : public Vasnecov::CoreObject
{
    // Список контейнеров списков
    class WorldElementList : public Vasnecov::ElementList<Vasnecov::ElementBox>
    {};


public:
    VasnecovWorld(QMutex* mutex,
                  VasnecovPipeline* pipeline,
                  GLint mx, GLint my,
                  GLsizei width, GLsizei height,
                  const std::string& name = std::string());
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

    Vasnecov::Camera camera() const;

    Vasnecov::Line unprojectPointToLine(const QPointF& point);
    Vasnecov::Line unprojectPointToLine(GLfloat x, GLfloat y);

protected:
    // Списки содержимого
    VasnecovLamp* designerFindElement(VasnecovLamp* lamp) const;
    VasnecovProduct* designerFindElement(VasnecovProduct* product) const;
    VasnecovFigure* designerFindElement(VasnecovFigure* figure) const;
    VasnecovLabel* designerFindElement(VasnecovLabel* label) const;

    // Добавление новых элементов в списки
    GLboolean designerAddElement(VasnecovLamp* lamp, GLboolean check = false);
    GLboolean designerAddElement(VasnecovProduct* product, GLboolean check = false);
    GLboolean designerAddElement(VasnecovFigure* figure, GLboolean check = false);
    GLboolean designerAddElement(VasnecovLabel* label, GLboolean check = false);

    GLboolean designerRemoveElement(VasnecovLamp* lamp);
    GLboolean designerRemoveElement(VasnecovProduct* product);
    GLboolean designerRemoveElement(VasnecovFigure* figure);
    GLboolean designerRemoveElement(VasnecovLabel* label);

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
    Vasnecov::MutualData<Vasnecov::WorldParameters> m_parameters; // Характеристики мира
    Vasnecov::MutualData<Vasnecov::Perspective> m_perspective; // Характеристики вида при перспективной проекции
    Vasnecov::MutualData<Vasnecov::Ortho> m_ortho; // Характеристики вида при ортогональной проекции
    Vasnecov::MutualData<Vasnecov::Camera> m_camera; // камера мира
    Vasnecov::MutualData<QMatrix4x4> m_projectionMatrix;

    Vasnecov::LightModel m_lightModel;
    WorldElementList m_elements;

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

inline VasnecovLamp* VasnecovWorld::designerFindElement(VasnecovLamp* lamp) const
{
    return m_elements.findRawElement(lamp);
}
inline VasnecovProduct* VasnecovWorld::designerFindElement(VasnecovProduct* product) const
{
    return m_elements.findRawElement(product);
}
inline VasnecovFigure* VasnecovWorld::designerFindElement(VasnecovFigure* figure) const
{
    return m_elements.findRawElement(figure);
}
inline VasnecovLabel* VasnecovWorld::designerFindElement(VasnecovLabel* label) const
{
    return m_elements.findRawElement(label);
}

inline GLboolean VasnecovWorld::designerAddElement(VasnecovLamp* lamp, GLboolean check)
{
    return m_elements.addElement(lamp, check);
}
inline GLboolean VasnecovWorld::designerAddElement(VasnecovProduct* product, GLboolean check)
{
    return m_elements.addElement(product, check);
}
inline GLboolean VasnecovWorld::designerAddElement(VasnecovFigure* figure, GLboolean check)
{
    return m_elements.addElement(figure, check);
}
inline GLboolean VasnecovWorld::designerAddElement(VasnecovLabel* label, GLboolean check)
{
    return m_elements.addElement(label, check);
}

inline GLboolean VasnecovWorld::designerRemoveElement(VasnecovLamp* lamp)
{
    return m_elements.removeElement(lamp);
}
inline GLboolean VasnecovWorld::designerRemoveElement(VasnecovProduct* product)
{
    return m_elements.removeElement(product);
}
inline GLboolean VasnecovWorld::designerRemoveElement(VasnecovFigure* figure)
{
    return m_elements.removeElement(figure);
}
inline GLboolean VasnecovWorld::designerRemoveElement(VasnecovLabel* label)
{
    return m_elements.removeElement(label);
}

inline void VasnecovWorld::renderSwitchLamps() const
{
    if(m_parameters.pure().light)
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
    return m_parameters.pure();
}

inline const Vasnecov::Perspective& VasnecovWorld::renderPerspective() const
{
    return m_perspective.pure();
}

inline const Vasnecov::Ortho& VasnecovWorld::renderOrtho() const
{
    return m_ortho.pure();
}

inline const Vasnecov::Camera& VasnecovWorld::renderCamera() const
{
    return m_camera.pure();
}

#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
#endif // VASNECOV_WORLD_H
