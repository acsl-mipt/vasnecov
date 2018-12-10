/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vector>
#include "technologist.h"

class VasnecovLamp;
class VasnecovProduct;
class VasnecovFigure;
class VasnecovLabel;
class VasnecovMaterial;

namespace Vasnecov
{
    // Обёртка контейнера списков указателей на элементы
    template <typename T>
    class ElementBox
    {
    public:
        ElementBox();
        virtual ~ElementBox(){}

        virtual GLboolean synchronize(); // Синхронизация чистых данных с грязными

        T* findElement(T* element) const;
        virtual GLboolean addElement(T* element, GLboolean check = false);
        virtual GLboolean removeElement(T* element);
        GLuint removeElements(const std::vector<T*>& deletingList);
        const std::vector<T*>& raw() const;
        const std::vector<T*>& pure() const;
        GLboolean hasPure() const;
        GLuint rawCount() const;

        template <typename F>
        void forEachPure(F fun) const
        {
            for_each(m_pure.begin(), m_pure.end(), fun);
        }

    protected:
        std::vector<T*> m_raw;
        std::vector<T*> m_buffer;
        std::vector<T*> m_pure;
        GLboolean m_wasUpdated;
        const GLenum m_flag; // Флаг обновления

    private:
        Q_DISABLE_COPY(ElementBox)
    };

    template <typename T>
    ElementBox<T>::ElementBox() :
        m_wasUpdated(false),
        m_flag()
    {}

    template <typename T>
    GLboolean ElementBox<T>::addElement(T *element, GLboolean check)
    {
        if(element)
        {
            GLboolean added = true;
            if(check)
            {
                // Поиск дубликатов
                if(m_raw.end() != find(m_raw.begin(), m_raw.end(), element))
                {
                    added = false;
                }
            }
            if(added)
            {
                m_raw.push_back(element);
                m_buffer = m_raw;
                m_wasUpdated = true;
                return true;
            }
        }
        Vasnecov::problem("Неверный элемент либо дублирование данных");
        return false;
    }

    template <typename T>
    GLboolean ElementBox<T>::synchronize()
    {
        if(m_wasUpdated)
        {
            m_pure.swap(m_buffer);
            m_wasUpdated = false;
            return true;
        }
        return false;
    }

    template <typename T>
    const std::vector<T*> &ElementBox<T>::raw() const
    {
        return m_raw;
    }
    template <typename T>
    const std::vector<T *> &ElementBox<T>::pure() const
    {
        return m_pure;
    }
    template <typename T>
    GLboolean ElementBox<T>::hasPure() const
    {
        return !m_pure.empty();
    }
    template <typename T>
    GLuint ElementBox<T>::rawCount() const
    {
        return static_cast<GLuint>(m_pure.size());
    }

    template <typename T>
    T *ElementBox<T>::findElement(T *element) const
    {
        if (!element)
            return nullptr;
        if(find(m_raw.begin(), m_raw.end(), element) != m_raw.end())
        {
            return element;
        }
        return nullptr;
    }

    template <typename T>
    GLboolean ElementBox<T>::removeElement(T *element)
    {
        if (!element)
            return false;

        const auto i = std::find_if(m_raw.begin(), m_raw.end(), [=](const T* v) { return v == element; });
        if (i == m_raw.end())
            return false;

        m_raw.erase(i);
        m_buffer = m_raw;
        m_wasUpdated = true;
        return true;
    }

    template <typename T>
    GLuint ElementBox<T>::removeElements(const std::vector<T*> &deletingList)
    {
        GLuint count = 0;

        for(auto& i: deletingList)
        {
            if (removeElement(i))
            {
                ++count;
            }
        }
        return count;
    }
}

namespace Vasnecov
{
    template <template <typename> class C,
              class ELamp = VasnecovLamp,
              class EProduct = VasnecovProduct,
              class EFigure = VasnecovFigure,
              class ELabel = VasnecovLabel>
    class ElementList
    {
    public:
        ElementList() {}
        virtual ~ElementList(){}

        VasnecovLamp* findRawElement(VasnecovLamp* lamp) const {return m_lamps.findElement(lamp);}
        VasnecovProduct* findRawElement(VasnecovProduct* product) const {return m_products.findElement(product);}
        VasnecovFigure* findRawElement(VasnecovFigure* figure) const {return m_figures.findElement(figure);}
        VasnecovLabel* findRawElement(VasnecovLabel* label) const {return m_labels.findElement(label);}

        GLboolean addElement(VasnecovLamp* lamp, GLboolean check = false) {return m_lamps.addElement(lamp, check);}
        GLboolean addElement(VasnecovProduct* product, GLboolean check = false) {return m_products.addElement(product, check);}
        GLboolean addElement(VasnecovFigure* figure, GLboolean check = false) {return m_figures.addElement(figure, check);}
        GLboolean addElement(VasnecovLabel* label, GLboolean check = false) {return m_labels.addElement(label, check);}

        GLboolean removeElement(VasnecovLamp* lamp) {return m_lamps.removeElement(lamp);}
        GLboolean removeElement(VasnecovProduct* product) {return m_products.removeElement(product);}
        GLboolean removeElement(VasnecovFigure* figure) {return m_figures.removeElement(figure);}
        GLboolean removeElement(VasnecovLabel* label) {return m_labels.removeElement(label);}

        GLuint removeElements(const std::vector<VasnecovLamp*>& deletingList) {return m_lamps.removeElements(deletingList);}
        GLuint removeElements(const std::vector<VasnecovProduct*>& deletingList) {return m_products.removeElements(deletingList);}
        GLuint removeElements(const std::vector<VasnecovFigure*>& deletingList) {return m_figures.removeElements(deletingList);}
        GLuint removeElements(const std::vector<VasnecovLabel*>& deletingList) {return m_labels.removeElements(deletingList);}

        GLboolean synchronizeLamps() {return m_lamps.synchronize();}
        GLboolean synchronizeProducts() {return m_products.synchronize();}
        GLboolean synchronizeFigures() {return m_figures.synchronize();}
        GLboolean synchronizeLabels() {return m_labels.synchronize();}

        const std::vector<VasnecovLamp*>& rawLamps() const {return m_lamps.raw();}
        const std::vector<VasnecovProduct*>& rawProducts() const {return m_products.raw();}
        const std::vector<VasnecovFigure*>& rawFigures() const {return m_figures.raw();}
        const std::vector<VasnecovLabel*>& rawLabels() const {return m_labels.raw();}

        const std::vector<VasnecovLamp*>& pureLamps() const {return m_lamps.pure();}
        const std::vector<VasnecovProduct*>& pureProducts() const {return m_products.pure();}
        const std::vector<VasnecovFigure*>& pureFigures() const {return m_figures.pure();}
        const std::vector<VasnecovLabel*>& pureLabels() const {return m_labels.pure();}

        GLboolean hasPureLamps() const {return m_lamps.hasPure();}
        GLboolean hasPureProducts() const {return m_products.hasPure();}
        GLboolean hasPureFigures() const {return m_figures.hasPure();}
        GLboolean hasPureLabels() const {return m_labels.hasPure();}

        GLuint rawLampsCount() const {return m_lamps.rawCount();}
        GLuint rawProductsCount() const {return m_products.rawCount();}
        GLuint rawFiguresCount() const {return m_figures.rawCount();}
        GLuint rawLabelsCount() const {return m_labels.rawCount();}

        template <typename F>
        void forEachPureLamp(F fun) const {m_lamps.forEachPure(fun);}
        template <typename F>
        void forEachPureProduct(F fun) const {m_products.forEachPure(fun);}
        template <typename F>
        void forEachPureFigure(F fun) const {m_figures.forEachPure(fun);}
        template <typename F>
        void forEachPureLabel(F fun) const {m_labels.forEachPure(fun);}

        virtual GLboolean synchronizeAll()
        {
            GLboolean res = false;

            res |= m_lamps.synchronize();
            res |= m_products.synchronize();
            res |= m_figures.synchronize();
            res |= m_labels.synchronize();

            return res;
        }

    protected:
        C<ELamp> m_lamps;
        C<EProduct> m_products;
        C<EFigure> m_figures;
        C<ELabel> m_labels;
    };
}
