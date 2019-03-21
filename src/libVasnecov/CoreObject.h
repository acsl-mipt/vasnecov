/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "Types.h"
#include "VasnecovPipeline.h"

#define SINGLE_THREAD_REALIZATION 1

namespace Vasnecov
{
    // Обёртка для данных, используемых в нескольких потоках
    // Используемый тип должен иметь операторы присваивания и !=
    template <typename T>
    class MutualData
    {
    public:
        MutualData(GLenum &wasUpdated, const GLenum flag) :
            m_raw(),
            m_pure(),
            m_flag(flag),
            m_wasUpdated(wasUpdated)
        {}
        MutualData(GLenum &wasUpdated, const GLenum flag, const T &data) :
            m_raw(data),
            m_pure(data),
            m_flag(flag),
            m_wasUpdated(wasUpdated)
        {}
        GLboolean set(const T &value)
        {
            if(m_raw != value)
            {
                m_raw = value;
                m_wasUpdated |= m_flag; // Добавка своего флага в общий (внешний)
                return true;
            }
            return false;
        }
        MutualData<T> &operator=(const T &value)
        {
            set(value);
            return *this;
        }
        void synchronizeRaw()
        {
            m_raw = m_pure;
        }
        GLenum update()
        {
            if((m_wasUpdated & m_flag) != 0)
            {
                m_pure = m_raw;
                m_wasUpdated = m_wasUpdated &~ m_flag; // Удаление своего флага из общего
                return m_flag;
            }
            return 0;
        }
        const T &raw() const
        {
            return m_raw;
        }
        // Для прямого изменения сложных типов
        T &editableRaw()
        {
            m_wasUpdated |= m_flag;
            return m_raw;
        }
        T &editablePure()
        {
            m_wasUpdated |= m_flag;
            return m_pure;
        }

        const T &pure() const
        {
            return m_pure;
        }

    private:
        T m_raw; // Грязные данные - из внешнего потока
        T m_pure; // Чистые - для рендеринга
        const GLenum m_flag; // Флаг. Идентификатор, выдаваемый результатом синхронизации update()
        GLenum &m_wasUpdated; // Ссылка на общий флаг обновлений
    };

}

class VasnecovPipeline;

namespace Vasnecov
{
    class CoreObject
    {
    public:
        CoreObject(VasnecovPipeline* pipeline,
                   const QString& name = QString()) :
            raw_wasUpdated(false),

            m_name(raw_wasUpdated, Name, name),
            m_isHidden(raw_wasUpdated, Flags, false),

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

        MutualData<QString> m_name; // Наименование
        MutualData<GLboolean> m_isHidden; // Флаг на отрисовку

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
        m_name.set(name);
    }
    inline QString CoreObject::name() const
    {
        QString name(m_name.raw());
        return name;
    }

    inline void CoreObject::setVisible(GLboolean visible)
    {
        m_isHidden.set(!visible);
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
        GLboolean visible(!m_isHidden.raw());
        return visible;
    }
    inline GLboolean CoreObject::isHidden() const
    {
        return !isVisible();
    }

    inline GLboolean CoreObject::designerIsVisible() const
    {
        return !m_isHidden.raw();
    }

    inline GLenum CoreObject::renderUpdateData()
    {
        GLenum updated(raw_wasUpdated);

        if(raw_wasUpdated)
        {
            // Копирование сырых данных в основные
            m_name.update();
            m_isHidden.update();

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
        return m_name.pure();
    }
    inline GLboolean CoreObject::renderIsVisible() const
    {
        return !m_isHidden.pure();
    }
    inline GLboolean CoreObject::renderIsHidden() const
    {
        return m_isHidden.pure();
    }
}
