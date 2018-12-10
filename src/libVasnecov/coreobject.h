/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "types.h"
#include "vasnecovpipeline.h"

class VasnecovPipeline;

namespace Vasnecov
{
    class CoreObject
    {
    public:
        CoreObject(VasnecovPipeline* pipeline,
                   const QString& name = QString()) :
            raw_wasUpdated(false),
            m_name(name),
            m_isHidden(false),

            pure_pipeline(pipeline)
        {}
        virtual ~CoreObject(){}

    public:
        // Название
        void setName(const QString& name);
        QString name() const;

        // Видимость
        virtual void setVisible(GLboolean visible = true);
        void setHidden(GLboolean hidden = true);
        void hide(); // Включение/выключение отрисовки
        void show();
        GLboolean isVisible() const;
        GLboolean isHidden() const; // Скрыт ли элемент

    protected:
        // Методы без мьютексов, вызываемые методами, защищенными своими мьютексами. Префикс designer
        GLboolean designerIsVisible() const;

    protected:
        // Методы, вызываемые на этапе обнолвения данных. Т.е. могут трогать любые данные
        virtual GLenum renderUpdateData(); // обновление данных, вызов должен быть обёрнут мьютексом

        virtual void renderDraw() = 0; // Собственно, отрисовка элемента

        GLboolean updaterIsUpdateFlag(GLenum flag) const;
        void updaterSetUpdateFlag(GLenum flag);
        void updaterClearUpdateFlag();
        void updaterRemoveUpdateFlag(GLenum flag);

    protected:
        // Методы, вызываемые рендерером (прямое обращение к основным данным без мьютексов). Префикс render
        // Для их сокрытия методы объявлены protected, а класс Рендерера сделан friend
        const QString& renderName() const;

        GLboolean renderIsVisible() const;
        GLboolean renderIsHidden() const;

    protected:
        GLenum raw_wasUpdated;

        QString     m_name; // Наименование
        GLboolean   m_isHidden; // Флаг на отрисовку

        VasnecovPipeline* const pure_pipeline; // Указатель конвейера, через который ведётся отрисовка

        enum Updated // Изменение данных
        {
            Flags		= 0x0001,
            Name		= 0x0002
        };
    private:
        Q_DISABLE_COPY(CoreObject)
    };


    inline void CoreObject::setName(const QString& name)
    {
        m_name = name;
    }
    inline QString CoreObject::name() const
    {
        return m_name;
    }

    inline void CoreObject::setVisible(GLboolean visible)
    {
        m_isHidden = !visible;
    }
    inline void CoreObject::setHidden(GLboolean hidden)
    {
        setVisible(!hidden);
    }
    inline void CoreObject::hide()
    {
        setVisible(false);
    }
    inline void CoreObject::show()
    {
        setVisible(true);
    }
    inline GLboolean CoreObject::isVisible() const
    {
        return !m_isHidden;
    }
    inline GLboolean CoreObject::isHidden() const
    {
        return !isVisible();
    }

    inline GLboolean CoreObject::designerIsVisible() const
    {
        return !m_isHidden;
    }

    inline GLenum CoreObject::renderUpdateData()
    {
        GLenum updated = raw_wasUpdated;

        if(raw_wasUpdated)
        {
            raw_wasUpdated = 0;
            pure_pipeline->setSomethingWasUpdated();
        }

        return updated;
    }

    inline GLboolean CoreObject::updaterIsUpdateFlag(GLenum flag) const
    {
        return (raw_wasUpdated & flag) != 0;
    }
    inline void CoreObject::updaterSetUpdateFlag(GLenum flag)
    {
        raw_wasUpdated |= flag;
    }
    inline void CoreObject::updaterClearUpdateFlag()
    {
        raw_wasUpdated = 0;
    }
    inline void CoreObject::updaterRemoveUpdateFlag(GLenum flag)
    {
        raw_wasUpdated = raw_wasUpdated &~ flag;
    }

    inline const QString& CoreObject::renderName() const
    {
        return m_name;
    }
    inline GLboolean CoreObject::renderIsVisible() const
    {
        return !m_isHidden;
    }
    inline GLboolean CoreObject::renderIsHidden() const
    {
        return m_isHidden;
    }
}
