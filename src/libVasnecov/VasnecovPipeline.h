/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// "Виртуализация" конвейера OpenGL
#pragma once

#include <vector>
#include <QColor>
#include <QMatrix4x4>
#include <QVector2D>
#include "Types.h"

class QGLContext;

namespace Vasnecov
{
    enum RotationOrders
    {
        RotationOrderZXY = 1,
        RotationOrderZYX,
        RotationOrderXYZ
    };

    struct Config
    {
        RotationOrders rotationOrder;

        Config() :
            rotationOrder(RotationOrderZXY)
        {}
    };
}

class VasnecovPipeline
{
    enum MaterialColoringTypes
    {
        AmbientAndDiffuse = GL_AMBIENT_AND_DIFFUSE,
        Ambient = GL_AMBIENT,
        Diffuse = GL_DIFFUSE,
        Specular = GL_SPECULAR,
        Emission = GL_EMISSION
    };

public:
    enum ElementDrawingMethods
    {
        Points =		GL_POINTS,

        Lines =			GL_LINES,
        LoopLine =		GL_LINE_LOOP,
        PolyLine =		GL_LINE_STRIP,

        Triangles =		GL_TRIANGLES,
        FanTriangle =	GL_TRIANGLE_FAN,
        StripTriangle = GL_TRIANGLE_STRIP
    };
    struct CameraAttributes
    {
        QVector3D eye;
        QVector3D center;
        QVector3D up;

        CameraAttributes():
            eye(),
            center(),
            up(0.0f, 0.0f, 1.0f)
        {}
    };

public:
    explicit VasnecovPipeline(QGLContext* context = nullptr);

    void initialize(QGLContext* context = nullptr);

    void clearAll();
    void clearZBuffer();

    void setPerspective(const Vasnecov::Perspective& perspective, const CameraAttributes& camera);
    void setOrtho(const Vasnecov::Ortho& ortho, const CameraAttributes& camera);
    void setPerspective(const Vasnecov::Perspective& perspective);
    void setOrtho(const Vasnecov::Ortho& ortho);
    void setOrtho2D(); // "Технолический" режим, в матрицу m_P не учитывается
    void unsetOrtho2D(); // Возвращение прошлой матрицы проекции
    void setViewport(GLint x, GLint y, GLsizei width, GLsizei height);

    void setIdentityMatrixP();
    void setMatrixP(const QMatrix4x4& P);
    void addMatrixP(const QMatrix4x4& P); // Домножение на матрицу
    const QMatrix4x4 matrixP() const;

    void setIdentityMatrixMV(); // Задание единичной модельно-видовой матрицы
    void setMatrixMV(const QMatrix4x4& MV);
    void setMatrixMV(const QMatrix4x4* MV);
    void addMatrixMV(const QMatrix4x4& MV);
    void addMatrixMV(const QMatrix4x4* MV);
    void setMatrixOrtho2D(const QMatrix4x4& MV, const QVector2D& offset = QVector2D());
    QVector4D projectPoint(const QMatrix4x4& MV, const QVector3D& point = QVector3D());

    void setBackgroundColor(const QColor& color = QColor(0, 0, 0, 0));
    void setColor(const QColor& color = QColor(255, 255, 255, 255));

    void enableTexture2D(GLuint m_texture2D, GLboolean strong = false);
    void disableTexture2D(GLboolean strong = false);

    void setAmbientColor(const QColor& color);

    void enableLamps(GLboolean strong = false);
    void disableLamps(GLboolean strong = false);
    void activateLamps(GLboolean lamps = true, GLboolean strong = false);

    void enableDepth(GLboolean strong = false);
    void disableDepth(GLboolean strong = false);
    void activateDepth(GLboolean depth = true, GLboolean strong = false);

    void enableMaterialColoring(MaterialColoringTypes type, GLboolean strong = false);
    void disableMaterialColoring(GLboolean strong = false);

    void enableBackFaces(GLboolean strong = false);
    void disableBackFaces(GLboolean strong = false);

    void enableBlending(GLboolean strong = false);
    void disableBlending(GLboolean strong = false);
    void enableSmoothShading(GLboolean strong = false);
    void disableSmoothShading(GLboolean strong = false);
    void enableNormalization(GLboolean strong = false);
    void disableNormalization(GLboolean strong = false);

    void enableConcreteLamp(GLuint lamp, GLboolean strong = false);
    void disableConcreteLamp(GLuint lamp, GLboolean strong = false);
    void disableAllConcreteLamps(GLboolean strong = false);

    void enableLineStipple(GLint factor, GLushort pattern, GLboolean strong = false);
    void disableLineStipple(GLboolean strong = false);

    void setMaterialColors(const QColor& ambient,
                           const QColor& diffuse,
                           const QColor& specular,
                           const QColor& emission,
                           GLfloat shininess);
    void setMaterialAmbientColor(const QColor& color);
    void setMaterialDiffuseColor(const QColor& color);
    void setMaterialSpecularColor(const QColor& color);
    void setMaterialEmissionColor(const QColor& color);
    void setMaterialShininess(GLfloat shininess);
    void resetMaterialColors();

    void setDrawingType(Vasnecov::PolygonDrawingTypes type, GLboolean strong = false);
    void setLineWidth(float width);
    void setPointSize(float size);

    void drawElements(ElementDrawingMethods         method,
                      const std::vector<GLuint>*    indices,
                      const std::vector<QVector3D>* vertices,
                      const std::vector<QVector3D>* normals = nullptr,
                      const std::vector<QVector2D>* textures = nullptr,
                      const std::vector<QVector3D>* colors = nullptr) const;

    void setSomethingWasUpdated() {m_wasSomethingUpdated = true;}

//	Vasnecov::Config &config();
//	void setConfig(const Vasnecov::Config config);

protected:
    void applyMaterialColors();
    void setCamera(const CameraAttributes& camera);

    void setContext(const QGLContext* context);

    void clearSomethingUpdates() {m_wasSomethingUpdated = false;}
    bool wasSomethingUpdated() const {return m_wasSomethingUpdated;}

protected:
    const QGLContext* m_context;

    QColor m_backgroundColor; // Цвет задника
    QColor m_color; // Цвет отрисовки
    Vasnecov::PolygonDrawingTypes m_drawingType; // Тип отрисовки
    GLuint m_texture2D; // Индекс активной текстуры

    QMatrix4x4 m_P;
    GLint m_viewX;
    GLint m_viewY;
    GLsizei m_viewWidth;
    GLsizei m_viewHeight;

    std::vector<GLuint> m_activatedLamps;

    GLboolean m_flagTexture2D; // Включены 2D текстуры
    GLboolean m_flagLight; // Включен свет
    GLboolean m_flagDepth; // Тест глубины
    GLboolean m_materialColoring; // Раскраска по цвету
    GLboolean m_backFaces; // Отображение задних граней
    GLboolean m_blending; // Смещивание (для прозрачности)
    GLboolean m_normalizing;
    GLboolean m_smoothShading; // Плавное смещивание
    GLboolean m_lineStipple;

    QColor m_ambientColor;
    MaterialColoringTypes m_materialColoringType;

    QColor m_materialColorAmbient;
    QColor m_materialColorDiffuse;
    QColor m_materialColorSpecular;
    QColor m_materialColorEmission;
    GLfloat m_materialShininess;

    GLfloat m_lineWidth;
    GLfloat m_pointSize;

    GLint       m_lineStippleFactor;
    GLushort    m_lineStipplePattern;

    bool m_wasSomethingUpdated;

//	Vasnecov::Config m_config;

    friend class VasnecovUniverse;

    static const GLenum m_face = GL_FRONT;

private:
    Q_DISABLE_COPY(VasnecovPipeline)
};

inline void VasnecovPipeline::setContext(const QGLContext *context)
{
    m_context = context;
}

inline void VasnecovPipeline::clearAll()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}
inline void VasnecovPipeline::clearZBuffer()
{
    glClear(GL_DEPTH_BUFFER_BIT);
}

inline void VasnecovPipeline::setIdentityMatrixP()
{
    m_P.setToIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
}
inline void VasnecovPipeline::setMatrixP(const QMatrix4x4 &P)
{
    m_P = P;
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(m_P.constData());
    glMatrixMode(GL_MODELVIEW);
}
inline void VasnecovPipeline::addMatrixP(const QMatrix4x4 &P)
{
    m_P = m_P * P;
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(m_P.constData());
    glMatrixMode(GL_MODELVIEW);
}
inline const QMatrix4x4 VasnecovPipeline::matrixP() const
{
    return m_P;
}
inline void VasnecovPipeline::setIdentityMatrixMV()
{
    glLoadIdentity();
}
inline void VasnecovPipeline::setMatrixMV(const QMatrix4x4 &MV)
{
    glLoadMatrixf(MV.constData());
}
inline void VasnecovPipeline::setMatrixMV(const QMatrix4x4 *MV)
{
    setMatrixMV(*MV);
}
inline void VasnecovPipeline::addMatrixMV(const QMatrix4x4 &MV)
{
    glMultMatrixf(MV.constData());
}
inline void VasnecovPipeline::addMatrixMV(const QMatrix4x4 *MV)
{
    addMatrixMV(*MV);
}
inline void VasnecovPipeline::setBackgroundColor(const QColor &color)
{
    m_backgroundColor = color;
    glClearColor(m_backgroundColor.redF(), m_backgroundColor.greenF(), m_backgroundColor.blueF(), 0.0f); // Цвет очистки экрана
}

inline void VasnecovPipeline::enableTexture2D(GLuint texture, GLboolean strong)
{
    if(!m_flagTexture2D || strong)
    {
        m_flagTexture2D = true;
        glEnable(GL_TEXTURE_2D);
    }
    if(texture != m_texture2D)
    {
        m_texture2D = texture;
        glBindTexture(GL_TEXTURE_2D, m_texture2D);
    }
}
inline void VasnecovPipeline::disableTexture2D(GLboolean strong)
{
    if(m_flagTexture2D || strong)
    {
        m_flagTexture2D = false;
        glDisable(GL_TEXTURE_2D);
    }
}

inline void VasnecovPipeline::enableLamps(GLboolean strong)
{
    if(!m_flagLight || strong)
    {
        m_flagLight = true;
        glEnable(GL_LIGHTING);
    }
}
inline void VasnecovPipeline::disableLamps(GLboolean strong)
{
    if(m_flagLight || strong)
    {
        m_flagLight = false;
        glDisable(GL_LIGHTING);
    }
}
inline void VasnecovPipeline::activateLamps(GLboolean lamps, GLboolean strong)
{
    if(lamps)
    {
        enableLamps(strong);
    }
    else
    {
        disableLamps(strong);
    }
}

inline void VasnecovPipeline::enableDepth(GLboolean strong)
{
    if(!m_flagDepth || strong)
    {
        m_flagDepth = true;
        glEnable(GL_DEPTH_TEST);
    }
}
inline void VasnecovPipeline::disableDepth(GLboolean strong)
{
    if(m_flagDepth || strong)
    {
        m_flagDepth = false;
        glDisable(GL_DEPTH_TEST);
    }
}
inline void VasnecovPipeline::activateDepth(GLboolean depth, GLboolean strong)
{
    if(depth)
    {
        enableDepth(strong);
    }
    else
    {
        disableDepth(strong);
    }
}

inline void VasnecovPipeline::enableMaterialColoring(MaterialColoringTypes type, GLboolean strong)
{
    if(!m_materialColoring || strong)
    {
        m_materialColoring = true;
        glEnable(GL_COLOR_MATERIAL); // Включить раскраску с помощью glColor
    }
    if((m_materialColoringType != type) || strong)
    {
        m_materialColoringType = type;
        glColorMaterial(GL_FRONT, m_materialColoringType);
    }
}
inline void VasnecovPipeline::disableMaterialColoring(GLboolean strong)
{
    if(m_materialColoring || strong)
    {
        m_materialColoring = false;
        glDisable(GL_COLOR_MATERIAL);
    }
}

inline void VasnecovPipeline::enableBackFaces(GLboolean strong)
{
    if(!m_backFaces || strong)
    {
        m_backFaces = true;
        glDisable(GL_CULL_FACE); // Включить задние грани, т.е. ВЫключить их отбрасывание
    }
}
inline void VasnecovPipeline::disableBackFaces(GLboolean strong)
{
    if(m_backFaces || strong)
    {
        m_backFaces = false;
        glEnable(GL_CULL_FACE);
    }
}

inline void VasnecovPipeline::enableBlending(GLboolean strong)
{
    if(!m_blending || strong)
    {
        m_blending = true;
        glEnable(GL_BLEND);

        if(strong)
        {
            // Ибо Qt че-то своё мутит с прозрачностью
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Альфа-прозрачность
#ifndef _MSC_VER
            glBlendEquation(GL_FUNC_ADD);
#endif
        }
    }
}
inline void VasnecovPipeline::disableBlending(GLboolean strong)
{
    if(m_blending || strong)
    {
        m_blending = false;
        glDisable(GL_BLEND);
    }
}
inline void VasnecovPipeline::enableSmoothShading(GLboolean strong)
{
    if(!m_smoothShading || strong)
    {
        m_smoothShading = true;
        glShadeModel(GL_SMOOTH);
    }
}
inline void VasnecovPipeline::disableSmoothShading(GLboolean strong)
{
    if(m_smoothShading || strong)
    {
        m_smoothShading = false;
        glShadeModel(GL_FLAT);
    }
}

inline void VasnecovPipeline::enableNormalization(GLboolean strong)
{
    if(!m_normalizing || strong)
    {
        m_normalizing = true;
        glEnable(GL_NORMALIZE);
    }
}

inline void VasnecovPipeline::disableNormalization(GLboolean strong)
{
    if(m_normalizing || strong)
    {
        m_normalizing = false;
        glDisable(GL_NORMALIZE);
    }
}

inline void VasnecovPipeline::enableConcreteLamp(GLuint lamp, GLboolean strong)
{
    if(m_activatedLamps.end() == find(m_activatedLamps.begin(), m_activatedLamps.end(), lamp))
    {
        m_activatedLamps.push_back(lamp);
        glEnable(lamp);
    }
    else if(strong)
    {
        glEnable(lamp);
    }
}

inline void VasnecovPipeline::disableConcreteLamp(GLuint lamp, GLboolean strong)
{
    std::vector<GLuint>::iterator lit = find(m_activatedLamps.begin(), m_activatedLamps.end(), lamp);

    if(lit != m_activatedLamps.end())
    {
        m_activatedLamps.erase(lit);
        glDisable(lamp);
    }
    else if(strong)
    {
        glDisable(lamp);
    }
}

inline void VasnecovPipeline::disableLineStipple(GLboolean strong)
{
    if(m_lineStipple || strong)
    {
        m_lineStipple = false;
        glDisable(GL_LINE_STIPPLE);
    }
}

inline void VasnecovPipeline::setMaterialAmbientColor(const QColor &color)
{
    if(color != m_materialColorAmbient)
    {
        m_materialColorAmbient = color;
        GLfloat params[4];
        params[0] = m_materialColorAmbient.redF();
        params[1] = m_materialColorAmbient.greenF();
        params[2] = m_materialColorAmbient.blueF();
        params[3] = m_materialColorAmbient.alphaF();

        glMaterialfv(m_face, GL_AMBIENT, params);
    }
}
inline void VasnecovPipeline::setMaterialDiffuseColor(const QColor &color)
{
    if(color != m_materialColorDiffuse)
    {
        m_materialColorDiffuse = color;
        GLfloat params[4];
        params[0] = m_materialColorDiffuse.redF();
        params[1] = m_materialColorDiffuse.greenF();
        params[2] = m_materialColorDiffuse.blueF();
        params[3] = m_materialColorDiffuse.alphaF();

        glMaterialfv(m_face, GL_DIFFUSE, params);
    }
}
inline void VasnecovPipeline::setMaterialSpecularColor(const QColor &color)
{
    if(color != m_materialColorSpecular)
    {
        m_materialColorSpecular = color;
        GLfloat params[4];
        params[0] = m_materialColorSpecular.redF();
        params[1] = m_materialColorSpecular.greenF();
        params[2] = m_materialColorSpecular.blueF();
        params[3] = m_materialColorSpecular.alphaF();

        glMaterialfv(m_face, GL_SPECULAR, params);
    }
}
inline void VasnecovPipeline::setMaterialEmissionColor(const QColor &color)
{
    if(color != m_materialColorEmission)
    {
        m_materialColorEmission = color;
        GLfloat params[4];
        params[0] = m_materialColorEmission.redF();
        params[1] = m_materialColorEmission.greenF();
        params[2] = m_materialColorEmission.blueF();
        params[3] = m_materialColorEmission.alphaF();

        glMaterialfv(m_face, GL_EMISSION, params);
    }
}
inline void VasnecovPipeline::setMaterialShininess(GLfloat shininess)
{
    if(shininess != m_materialShininess)
    {
        m_materialShininess = shininess;
        glMaterialf(m_face, GL_SHININESS, m_materialShininess);
    }
}

inline void VasnecovPipeline::resetMaterialColors()
{
    m_materialColorAmbient.setRgbF(0.2f, 0.2f, 0.2f, 1.0f);
    m_materialColorDiffuse.setRgbF(0.8f, 0.8f, 0.8f, 1.0f);
    m_materialColorSpecular.setRgbF(0.0, 0.0, 0.0, 1.0);
    m_materialColorEmission.setRgbF(0.0, 0.0, 0.0, 1.0);
    m_materialShininess = 0;

    applyMaterialColors();
}

inline void VasnecovPipeline::setLineWidth(float width)
{
    if(width != m_lineWidth)
    {
        m_lineWidth = width;
        glLineWidth(m_lineWidth);
    }
}
inline void VasnecovPipeline::setPointSize(float size)
{
    if(size != m_pointSize)
    {
        m_pointSize = size;
        glPointSize(m_pointSize);
    }
}
//inline Vasnecov::Config &VasnecovPipeline::config()
//{
//	return m_config;
//}
//inline void VasnecovPipeline::setConfig(const Vasnecov::Config config)
//{
//	m_config = config;
//}

inline void VasnecovPipeline::setDrawingType(Vasnecov::PolygonDrawingTypes type, GLboolean strong)
{
    if((m_drawingType != type) || strong)
    {
        m_drawingType = type;

        if(m_backFaces)
        {
            glPolygonMode(GL_FRONT_AND_BACK, m_drawingType);
        }
        else
        {
            glPolygonMode(GL_FRONT, m_drawingType);
        }
    }
}
