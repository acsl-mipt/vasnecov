/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef COREOBJECT_H
#define COREOBJECT_H

#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
#include "types.h"
#include "vasnecovpipeline.h"
#ifndef _MSC_VER
    #pragma GCC diagnostic warning "-Weffc++"
#endif

class VasnecovPipeline;

namespace Vasnecov
{
    class CoreObject
    {
    public:
        CoreObject(VasnecovPipeline* pipeline, const std::string& name = std::string()) :
            raw_wasUpdated(false),
            m_name(name),
            m_isHidden(false),
            pure_pipeline(pipeline)
        {}
        virtual ~CoreObject(){}

    public:
        // Название
        void setName(const std::string &name) { m_name = name; }
        const std::string& name() const { return m_name; }

        // Видимость
        virtual void setVisible(GLboolean visible = true) { m_isHidden = !visible; }
        void show() { setVisible(true); }
        GLboolean isVisible() const { return !m_isHidden; }
        void setHidden(GLboolean hidden = true) { setVisible(!hidden); }
        GLboolean isHidden() const { return !isVisible(); }
        void hide() { setVisible(false); }

    protected:
        // Методы без мьютексов, вызываемые методами, защищенными своими мьютексами. Префикс designer
        GLboolean designerIsVisible() const { return !m_isHidden; }

    protected:
        // Методы, вызываемые на этапе обнолвения данных. Т.е. могут трогать любые данные
        virtual GLenum renderUpdateData(); // обновление данных, вызов должен быть обёрнут мьютексом

        virtual void renderDraw() = 0; // Собственно, отрисовка элемента

        GLboolean updaterIsUpdateFlag(GLenum flag) const { return (raw_wasUpdated & flag) != 0; }
        void updaterSetUpdateFlag(GLenum flag) { raw_wasUpdated |= flag; }
        void updaterClearUpdateFlag() { raw_wasUpdated = 0; }
        void updaterRemoveUpdateFlag(GLenum flag) { raw_wasUpdated = raw_wasUpdated & ~flag; }

    protected:
        // Методы, вызываемые рендерером (прямое обращение к основным данным без мьютексов). Префикс render
        // Для их сокрытия методы объявлены protected, а класс Рендерера сделан friend
        std::string CoreObject::renderName() const { return m_name; }

        GLboolean renderIsVisible() const { return !m_isHidden; }
        GLboolean renderIsHidden() const { return m_isHidden; }

    protected:
        GLenum raw_wasUpdated;

        std::string m_name; // Наименование
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

    inline GLenum CoreObject::renderUpdateData()
    {
        if(raw_wasUpdated)
        {
            raw_wasUpdated = 0;
            pure_pipeline->setSomethingWasUpdated();
            return true;
        }
        return false;
    }

}

#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
#endif // COREOBJECT_H
