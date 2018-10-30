#include "ProductModel.h"

EmptyModel::EmptyModel(VasnecovUniverse *universe, VasnecovWorld *world) :
    _universe(universe),
    _world(world),
    _coordinates(), _angles(),
    _visible(true)
{}

EmptyModel::EmptyModel() :
    _universe(0),
    _world(0),
    _coordinates(), _angles(),
    _visible(true)
{}

void EmptyModel::setVisible(bool visible)
{
    if(_visible != visible)
    {
        _visible = visible;
    }
}

void EmptyModel::setPosition(const QVector3D &coord, const QVector3D &angles)
{
    setCoordinates(coord);
    setAngles(angles);
}

void EmptyModel::setCoordinates(const QVector3D &coord)
{
    if(_coordinates != coord)
    {
        _coordinates = coord;
    }
}

void EmptyModel::setAngles(const QVector3D &angles)
{
    if(_angles != angles)
    {
        _angles = angles;
    }
}

void EmptyModel::updateDrawing()
{}

float ProductModel::angleToRange360(float deg)
{
    if(deg > 0)
    {
        deg = fmod(deg, 360);
    }
    if(deg < 0)
    {
        deg = fmod(deg, 360) + 360;
    }
    return deg;
}

float ProductModel::angleToRange180(float deg)
{
    deg = angleToRange360(deg);
    if((360 - deg) < 180) // (-180; 180]
    {
        deg = deg - 360;
    }
    return deg;
}

float ProductModel::angleToRange90(float deg)
{
    deg = angleToRange180(deg);
    if(deg < 0.0)
    {
        deg = 0.0;
    }
    if(deg > 90.0)
    {
        deg = 90.0;
    }
    return deg;
}

ProductModel::ProductModel(VasnecovUniverse *universe, VasnecovWorld *world) :
    EmptyModel(universe, world),
    _model(0)
{}

ProductModel::ProductModel() :
    EmptyModel(),
    _model(0)
{}

void ProductModel::setVisible(bool visible)
{
    if(_model && visible != _visible)
    {
        _visible = visible;
        _model->setVisible(visible);
    }
}

void ProductModel::setCoordinates(const QVector3D &coord)
{
    if(_model && _coordinates != coord)
    {
        _coordinates = coord;
        _model->setCoordinates(_coordinates);
    }
}

void ProductModel::setAngles(const QVector3D &angles)
{
    if(_model && _angles != angles)
    {
        _angles = angles;
        _model->setAngles(_angles);
    }
}

bool ProductModel::create(const QString &name, VasnecovProduct *parent)
{
    if(_model)
    {
        qWarning("%s: 3D-model is exist", qPrintable(name));
        return false;
    }

    if(hasTools())
    {
        _model = _universe->addAssembly(name, _world, parent);
        if(_model)
        {
            return true;
        }
        qWarning("%s: can't create node 3D-model", qPrintable(name));
    }

    qWarning("%s: need to set tools for creating",qPrintable(name));
    return false;
}

bool ProductModel::create(const QString &name, ProductModel *parent)
{
    return create(name, parent->model());
}

bool ProductModel::isChildrenCorrect(bool ok)
{
    if(!ok)
    {
        clear();
        qWarning("Can't create 3D-model, wrong children");
    }
    return ok;
}
