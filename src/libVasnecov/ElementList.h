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
#include "Technologist.h"
#include "VasnecovLamp.h"
#include "VasnecovProduct.h"
#include "VasnecovFigure.h"
#include "VasnecovTerrain.h"
#include "VasnecovLabel.h"

class VasnecovLamp;
class VasnecovProduct;
class VasnecovFigure;
class VasnecovTerrain;
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
            for_each(_pure.begin(), _pure.end(), fun);
        }

    protected:
        std::vector<T*>     _raw, _buffer, _pure;
        GLboolean           _wasUpdated;
        const GLenum        _flag; // Флаг обновления

    private:
        Q_DISABLE_COPY(ElementBox)
    };


    template <typename T>
    ElementBox<T>::ElementBox() :
        _raw(), _buffer(), _pure(),
        _wasUpdated(false),
        _flag()
    {}

    template <typename T>
    GLboolean ElementBox<T>::addElement(T *element, GLboolean check)
    {
        if(element)
        {
            GLboolean added(true);
            if(check)
            {
                // Поиск дубликатов
                if(_raw.end() != find(_raw.begin(), _raw.end(), element))
                {
                    added = false;
                }
            }
            if(added)
            {
                _raw.push_back(element);
                _buffer = _raw;
                _wasUpdated = true;
                return true;
            }
        }
        Vasnecov::problem("Incorrect element or data duplicate");
        return false;
    }

    template <typename T>
    GLboolean ElementBox<T>::synchronize()
    {
        if(_wasUpdated)
        {
            _pure.swap(_buffer);
            _wasUpdated = false;
            return true;
        }
        return false;
    }

    template <typename T>
    const std::vector<T *> &ElementBox<T>::raw() const
    {
        return _raw;
    }
    template <typename T>
    const std::vector<T *> &ElementBox<T>::pure() const
    {
        return _pure;
    }
    template <typename T>
    GLboolean ElementBox<T>::hasPure() const
    {
        return !_pure.empty();
    }
    template <typename T>
    GLuint ElementBox<T>::rawCount() const
    {
        return static_cast<GLuint>(_pure.size());
    }

    template <typename T>
    T *ElementBox<T>::findElement(T *element) const
    {
        if(element)
        {
            if(find(_raw.begin(), _raw.end(), element) != _raw.end())
            {
                return element;
            }
        }
        return nullptr;
    }

    template <typename T>
    GLboolean ElementBox<T>::removeElement(T *element)
    {
        if(element)
        {
            for(typename std::vector<T *>::iterator eit = _raw.begin();
                eit != _raw.end(); ++eit)
            {
                if((*eit) == element)
                {
                    _raw.erase(eit);
                    _buffer = _raw;
                    _wasUpdated = true;
                    return true;
                }
            }
        }

        return false;
    }
    template <typename T>
    GLuint ElementBox<T>::removeElements(const std::vector<T *> &deletingList)
    {
        GLuint count(0);

        for(typename std::vector<T *>::const_iterator dit = deletingList.begin();
            dit != deletingList.end(); ++dit)
        {
            if(*dit)
            {
                if(this->removeElement(*dit))
                {
                    ++count;
                }
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
              class ETerrain = VasnecovTerrain,
              class ELabel = VasnecovLabel>
    class ElementList
    {
    public:
        ElementList() :
                _lamps(),
                _products(),
                _figures(),
                _labels()
        {}
        virtual ~ElementList(){}

        VasnecovLamp* findRawElement(VasnecovLamp* element) const       {return _lamps.findElement(element);}
        VasnecovProduct* findRawElement(VasnecovProduct* element) const {return _products.findElement(element);}
        VasnecovFigure* findRawElement(VasnecovFigure* element) const   {return _figures.findElement(element);}
        VasnecovTerrain* findRawElement(VasnecovTerrain* element) const {return _terrains.findElement(element);}
        VasnecovLabel* findRawElement(VasnecovLabel* element) const     {return _labels.findElement(element);}

        GLboolean addElement(VasnecovLamp* element, GLboolean check = false)    {return _lamps.addElement(element, check);}
        GLboolean addElement(VasnecovProduct* element, GLboolean check = false) {return _products.addElement(element, check);}
        GLboolean addElement(VasnecovFigure* element, GLboolean check = false)  {return _figures.addElement(element, check);}
        GLboolean addElement(VasnecovTerrain* element, GLboolean check = false) {return _terrains.addElement(element, check);}
        GLboolean addElement(VasnecovLabel* element, GLboolean check = false)   {return _labels.addElement(element, check);}

        GLboolean removeElement(VasnecovLamp* element)      {return _lamps.removeElement(element);}
        GLboolean removeElement(VasnecovProduct* element)   {return _products.removeElement(element);}
        GLboolean removeElement(VasnecovFigure* element)    {return _figures.removeElement(element);}
        GLboolean removeElement(VasnecovTerrain* element)   {return _terrains.removeElement(element);}
        GLboolean removeElement(VasnecovLabel* element)     {return _labels.removeElement(element);}

        GLuint removeElements(const std::vector<VasnecovLamp*>& deletingList)       {return _lamps.removeElements(deletingList);}
        GLuint removeElements(const std::vector<VasnecovProduct*>& deletingList)    {return _products.removeElements(deletingList);}
        GLuint removeElements(const std::vector<VasnecovFigure*>& deletingList)     {return _figures.removeElements(deletingList);}
        GLuint removeElements(const std::vector<VasnecovTerrain*>& deletingList)    {return _terrains.removeElements(deletingList);}
        GLuint removeElements(const std::vector<VasnecovLabel*>& deletingList)      {return _labels.removeElements(deletingList);}

        GLboolean synchronizeLamps()    {return _lamps.synchronize();}
        GLboolean synchronizeProducts() {return _products.synchronize();}
        GLboolean synchronizeFigures()  {return _figures.synchronize();}
        GLboolean synchronizeTerrains() {return _terrains.synchronize();}
        GLboolean synchronizeLabels()   {return _labels.synchronize();}

        const std::vector<VasnecovLamp*>& rawLamps() const          {return _lamps.raw();}
        const std::vector<VasnecovProduct*>& rawProducts() const    {return _products.raw();}
        const std::vector<VasnecovFigure*>& rawFigures() const      {return _figures.raw();}
        const std::vector<VasnecovTerrain*>& rawTerrains() const    {return _terrains.raw();}
        const std::vector<VasnecovLabel*>& rawLabels() const        {return _labels.raw();}

        const std::vector<VasnecovLamp*>& pureLamps() const         {return _lamps.pure();}
        const std::vector<VasnecovProduct*>& pureProducts() const   {return _products.pure();}
        const std::vector<VasnecovFigure*>& pureFigures() const     {return _figures.pure();}
        const std::vector<VasnecovTerrain*>& pureTerrains() const   {return _terrains.pure();}
        const std::vector<VasnecovLabel*>& pureLabels() const       {return _labels.pure();}

        GLboolean hasPureLamps() const      {return _lamps.hasPure();}
        GLboolean hasPureProducts() const   {return _products.hasPure();}
        GLboolean hasPureFigures() const    {return _figures.hasPure();}
        GLboolean hasPureTerrains() const   {return _terrains.hasPure();}
        GLboolean hasPureLabels() const     {return _labels.hasPure();}

        GLuint rawLampsCount() const    {return _lamps.rawCount();}
        GLuint rawProductsCount() const {return _products.rawCount();}
        GLuint rawFiguresCount() const  {return _figures.rawCount();}
        GLuint rawTerrainsCount() const {return _terrains.rawCount();}
        GLuint rawLabelsCount() const   {return _labels.rawCount();}

        template <typename F>
        void forEachPureLamp(F fun) const       {_lamps.forEachPure(fun);}
        template <typename F>
        void forEachPureProduct(F fun) const    {_products.forEachPure(fun);}
        template <typename F>
        void forEachPureFigure(F fun) const     {_figures.forEachPure(fun);}
        template <typename F>
        void forEachPureTerrain(F fun) const    {_terrains.forEachPure(fun);}
        template <typename F>
        void forEachPureLabel(F fun) const      {_labels.forEachPure(fun);}

        virtual GLboolean synchronizeAll()
        {
            GLboolean res(false);

            res |= _lamps.synchronize();
            res |= _products.synchronize();
            res |= _figures.synchronize();
            res |= _terrains.synchronize();
            res |= _labels.synchronize();

            return res;
        }

    protected:
        C<ELamp>    _lamps;
        C<EProduct> _products;
        C<EFigure>  _figures;
        C<ETerrain> _terrains;
        C<ELabel>   _labels;
    };
}
