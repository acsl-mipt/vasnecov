#pragma once

#include <Vasnecov>
#include <VasnecovProduct>

class EmptyModel
{
public:
    EmptyModel(VasnecovUniverse *universe,
                  VasnecovWorld *world);
    EmptyModel();

    virtual ~EmptyModel(){}

    void setTools(VasnecovUniverse *universe, VasnecovWorld *world);

    void setUniverse(VasnecovUniverse *universe);
    VasnecovUniverse *universe() const {return _universe;}
    void setWorld(VasnecovWorld *world);
    VasnecovWorld *world() const {return _world;}

    virtual void setVisible(bool visible = true);
    bool isVisible() const {return _visible;}

    void setPosition(const QVector3D &coord, const QVector3D &angles);
    virtual void setCoordinates(const QVector3D &coord);
    QVector3D coordinates() const {return _coordinates;}
    virtual void setAngles(const QVector3D &angles);
    QVector3D angles() const {return _angles;}

    virtual void updateDrawing(); // Изменение отображения по различным факторам

protected:
    bool hasTools() const {return _universe && _world;}

protected:
    VasnecovUniverse *_universe;
    VasnecovWorld *_world;
    QVector3D _coordinates, _angles;
    bool _visible;

private:
    Q_DISABLE_COPY(EmptyModel)
};

class ProductModel : public EmptyModel
{
public:
    static float angleToRange360(float deg);
    static float angleToRange180(float deg);
    static float angleToRange90(float deg);

    ProductModel(VasnecovUniverse *universe,
                    VasnecovWorld *world);
    ProductModel();
    ~ProductModel() {removeModel();}

    void setVisible(bool visible = true);

    void setCoordinates(const QVector3D &coord);
    void setAngles(const QVector3D &angles);

    virtual bool create(const QString &name, VasnecovProduct *parent = 0);
    bool create(const QString &name, ProductModel *parent);
    virtual void clear() {removeModel();}
    void removeModel(); // Удаление элементов

    bool hasWorkModel() const {return _universe && _world && _model;}

protected:
    bool isChildrenCorrect(bool ok);

protected:
    VasnecovProduct *model() const {return _model;}
    VasnecovProduct *modelParent() const;

private:
    VasnecovProduct *_model;

private:
    Q_DISABLE_COPY(ProductModel)
};

inline void EmptyModel::setTools(VasnecovUniverse *universe, VasnecovWorld *world)
{
    _universe = universe;
    _world = world;
}

inline void EmptyModel::setUniverse(VasnecovUniverse *universe)
{
    _universe = universe;
}

inline void EmptyModel::setWorld(VasnecovWorld *world)
{
    _world = world;
}

inline void ProductModel::removeModel()
{
    if(_universe)
    {
        _universe->removeProduct(_model);
        _model = 0;
    }
}

inline VasnecovProduct *ProductModel::modelParent() const
{
    if(_model)
    {
        return _model->parent();
    }
    return 0;
}
