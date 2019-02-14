#include "BomberModel.h"
#include <QPainter>
#include <QFont>
#include <VasnecovFigure>
#include <VasnecovLabel>
#include "Scene.h" // TODO: refactor

BomberModel::BomberModel(VasnecovUniverse *universe, VasnecovWorld *world) :
    ProductModel(universe, world),
    _beams(0),
    _body(0),
    _hat(0),
    _battery(0),
    _propellers{0},
    _outletBody(0),
    _outletCircles{0},
    _projectionLine(0),
    _mistakeLine(0),
    _track(0),
    _trackPoints(),
    _theoreticalPosition(0),
    _theoreticalLabel(0),
    _theoreticalLabelImage(64, 32, QImage::Format_ARGB32_Premultiplied)
{}

BomberModel::BomberModel() :
    ProductModel(),
    _beams(0),
    _body(0),
    _hat(0),
    _battery(0),
    _propellers{0},
    _outletBody(0),
    _outletCircles{0},
    _projectionLine(0),
    _mistakeLine(0),
    _track(0),
    _trackPoints(),
    _theoreticalPosition(0),
    _theoreticalLabel(0),
    _theoreticalLabelImage(64, 32, QImage::Format_ARGB32_Premultiplied)
{}

BomberModel::~BomberModel()
{
    removeModel();
    removeBomber();
}

void BomberModel::setVisible(bool visible)
{
    if(visible != isVisible())
    {
        ProductModel::setVisible(visible);

        if(_outletBody)
        {
            _outletBody->setVisible(visible);
        }
        for(uint i = 0; i < _propellersCount; ++i)
        {
            if(_outletCircles[i])
                _outletCircles[i]->setVisible(false); // FIXME: debug
        }
        if(_projectionLine)
        {
            _projectionLine->setVisible(visible);
        }
    }
}

void BomberModel::setCoordinates(const QVector3D &coord)
{
    if(model())
    {
        if(coord != coordinates())
        {
            if(coord != coordinates())
            ProductModel::setCoordinates(coord);

            QVector3D outletCoord = coord;
            outletCoord.setZ(0.0f);

            if(_outletBody && _projectionLine)
            {
                _outletBody->setCoordinates(outletCoord);

                if(coord.z() != 0.0f)
                {
                    _projectionLine->setPoints(std::vector<QVector3D>{QVector3D(), QVector3D(0.0f, 0.0f, coord.z())});
                }
            }
            for(uint i = 0; i < _propellersCount; ++i)
            {
                if(_outletCircles[i])
                {
                    if(_outletBody)
                    {
                        float xS = 1.0;
                        float yS = 1.0;
                        if(i%2)
                            xS = -1.0;
                        if(i>1)
                            yS = -1.0;

                        QVector3D pos = _outletBody->coordinates();
                        pos.setX(pos.x() + 135 * 0.001 * xS);
                        pos.setY(pos.y() + 135 * 0.001 * yS);

                        _outletCircles[i]->setCoordinates(pos);
                    }
                }
            }

            increaseTrack(coord);
            recalculateMistakeLine();
            redrawLabel();
        }
    }
}

void BomberModel::setAngles(const QVector3D &angles)
{
    if(model() && angles != coordinates())
    {
        ProductModel::setAngles(angles);

        QVector3D outletAngles = angles;
        outletAngles.setX(0.0f);
        outletAngles.setY(0.0f);

        if(_outletBody)
        {
            _outletBody->setAngles(outletAngles);
        }
        for(uint i = 0; i < _propellersCount; ++i)
        {
            if(_outletCircles[i])
            {
                _outletCircles[i]->setAngles(outletAngles); // FIXME: recalculate pos-vector at angle changing
            }
        }
    }
}

bool BomberModel::create(const QString &name, VasnecovProduct *parent)
{
    if(ProductModel::create(name, parent))
    {
        VasnecovMaterial *m = universe()->addMaterial();
        m->setAmbientColor(QColor(255,0,0,255));
        m->setDiffuseColor(QColor(255,0,0,255));
//        m->setSpecularColor(QColor(255,0,0,255));


        _beams = universe()->addPart("Beams", world(), "bomber_beams", m, model());
        _body = universe()->addPart("Body", world(), "bomber_body", model());
        _hat = universe()->addPart("Hat", world(), "bomber_hat", model());
        _battery = universe()->addPart("Battery", world(), "bomber_battery", model());

        if(_beams && _body && _hat && _battery)
        {
//            _beams->setColor(QColor(50, 50, 60, 255));
            _body->setColor(QColor(190, 190, 190, 255));
            _hat->setColor(QColor(255, 255, 0, 255));
            _battery->setColor(QColor(0, 0, 255, 255));
        }

        for(uint i = 0; i < _propellersCount; ++i)
        {
            float xS = 1.0;
            float yS = 1.0;
            if(i%2)
                xS = -1.0;
            if(i>1)
                yS = -1.0;

            _propellers[i] = universe()->addPart("Propeller " + QString::number(i), world(), "bomber_propeller", model());
            if(_propellers[i])
            {
                _propellers[i]->setColor(QColor(0, 155, 155, 255));
                _propellers[i]->setCoordinates(QVector3D(135.f*0.001 * xS, 135.f*0.001 * yS, 40.f*0.001));
            }

            _motors[i] = universe()->addPart("Motor " + QString::number(i), world(), "bomber_motor", model());
            if(_motors[i])
            {
                _motors[i]->setColor(QColor(30, 30, 30, 255));
                _motors[i]->setCoordinates(QVector3D(135.f*0.001 * xS, 135.f*0.001 * yS, 5.f*0.001));
            }
        }

        // Outlets
        _outletBody = universe()->addFigure("Outlet body", world());
        if(_outletBody)
        {
            _outletBody->setPoints(Scene::figureFromObj("stuff/meshes/outlet_bomber_body.obj"));

            _outletBody->setColor(QColor(0, 100, 255, 100));
            _outletBody->setType(VasnecovFigure::TypeLines);
            _outletBody->setAngles(0, 0, 180);
            _outletBody->setThickness(1);

            if(model())
            {
                QVector3D pos = model()->coordinates();
                QVector3D ang = model()->angles();
                pos.setZ(0.0f);

                ang.setX(0.0f);
                ang.setY(0.0f);
                ang.setZ(ang.z() + 180);

                _outletBody->setCoordinates(pos);
                _outletBody->setAngles(ang);
            }
        }

        std::vector<QVector3D> points = Scene::figureFromObj("stuff/meshes/outlet_bomber_propeller.obj");
        for(uint i = 0; i < _propellersCount; ++i)
        {
            _outletCircles[i] = universe()->addFigure("Outlet circle", world());
            if(_outletCircles[i])
            {
                _outletCircles[i]->setColor(QColor(0, 0, 150, 155));
                _outletCircles[i]->setType(VasnecovFigure::TypeLines);
                _outletCircles[i]->setPoints(points);
                _outletCircles[i]->setThickness(2);
                _outletCircles[i]->setScale(0.001);

                if(_outletBody)
                {
                    float xS = 1.0;
                    float yS = 1.0;
                    if(i%2)
                        xS = -1.0;
                    if(i>1)
                        yS = -1.0;

                    QVector3D pos = _outletBody->coordinates();
                    QVector3D ang = _outletBody->angles();
                    pos.setX(pos.x() + 135 * 0.001 * xS);
                    pos.setY(pos.y() + 135 * 0.001 * yS);

                    _outletCircles[i]->setCoordinates(pos);
                    _outletCircles[i]->setAngles(ang);
                }

                _outletCircles[i]->hide(); // FIXME: debug
            }
        }

        _projectionLine = universe()->addFigure("Projection Line", world());
        if(_projectionLine)
        {
            _projectionLine->setColor(QColor(0, 255, 255, 100));
            _projectionLine->setType(VasnecovFigure::TypeLines);
            _projectionLine->setThickness(1);

            if(_outletBody)
            {
                _projectionLine->attachToElement(_outletBody);
            }
        }

        _mistakeLine = universe()->addFigure("Mistake Line", world());
        if(_mistakeLine)
        {
            _mistakeLine->setColor(QColor(255, 255, 0, 255));
            _mistakeLine->setType(VasnecovFigure::TypeLines);
            _mistakeLine->setThickness(3);
        }

        _track = universe()->addFigure("Track", world());
        if(_track)
        {
            _track->setColor(QColor(0, 0, 255, 255));
            _track->setType(VasnecovFigure::TypePolyline);
            _track->setThickness(1);
        }

        _theoreticalLabel = universe()->addLabel("TPos", world(), 64, 32);

        bool good(true);
        good = good &&
               _beams &&
               _body &&
               _hat &&
               _battery &&
               _outletBody &&
               _projectionLine &&
               _mistakeLine &&
               _theoreticalLabel &&
               _track;

        for(uint i = 0; i < _propellersCount; ++i)
        {
            good = good && _propellers[i];
            good = good && _motors[i];
            good = good && _outletCircles[i];
        }

        return isChildrenCorrect(good);
    }
    return false;
}

void BomberModel::clear()
{
    removeModel(); // Removing childrens
    removeBomber();
}

void BomberModel::rotatePropellers(float angle)
{
    if(hasWorkModel())
    {
        for(uint i = 0; i < _propellersCount; ++i)
        {
            if(_propellers[i])
            {
                if(i == 1 || i == 2)
                    _propellers[i]->incrementAngles(QVector3D(0, 0, angle));
                else
                    _propellers[i]->incrementAngles(QVector3D(0, 0, -angle));
            }
        }
    }
}

void BomberModel::setGhostMode(bool ghost)
{
    if(hasWorkModel())
    {
        if(ghost)
        {
            // FIXME: refactor!
//            _beams->setColor(QColor(50, 50, 60, 50));
//            _body->setColor(QColor(190, 190, 190, 50));
//            _hat->setColor(QColor(255, 255, 0, 50));
//            _battery->setColor(QColor(0, 0, 255, 50));

//            for(uint i = 0; i < _propellersCount; ++i)
//            {
//                _propellers[i]->setColor(QColor(0, 155, 155, 50));
//                _motors[i]->setColor(QColor(30, 30, 30, 50));

//                _propellers[i]->hide();
//            }

            _beams->setColor(QColor(255, 255, 255, 50));
            _body->setColor(QColor(255, 255, 255, 50));
            _hat->setColor(QColor(255, 255, 255, 50));
            _battery->setColor(QColor(255, 255, 255, 50));

            if(_outletBody)
                _outletBody->hide();

            for(uint i = 0; i < _propellersCount; ++i)
            {
                _propellers[i]->setColor(QColor(0, 155, 155, 50));
                _motors[i]->setColor(QColor(255, 255, 255, 50));

                _propellers[i]->hide();

                if(_outletCircles[i])
                    _outletCircles[i]->hide();
                if(_projectionLine)
                    _projectionLine->hide();
            }
        }
        else
        {
            _beams->setColor(QColor(50, 50, 60, 50));
            _body->setColor(QColor(190, 190, 190, 50));
            _hat->setColor(QColor(255, 255, 0, 50));
            _battery->setColor(QColor(0, 0, 255, 50));

            if(_outletBody)
                _outletBody->show();

            for(uint i = 0; i < _propellersCount; ++i)
            {
                _propellers[i]->setColor(QColor(0, 155, 155, 50));
                _motors[i]->setColor(QColor(30, 30, 30, 50));

                _propellers[i]->show();

                if(_outletCircles[i])
                    _outletCircles[i]->hide(); // FIXME: debug
                if(_projectionLine)
                    _projectionLine->show();
            }
        }
    }
}

void BomberModel::setTheoreticalPosition(const QVector3D &position)
{
    if(!_theoreticalPosition)
    {
        _theoreticalPosition = new QVector3D(position);
    }
    else if(*_theoreticalPosition != position)
    {
        *_theoreticalPosition = position;
    }
    else
    {
        return;
    }

    recalculateMistakeLine();
    redrawLabel();
}

void BomberModel::clearTheoreticalPosition()
{
    delete _theoreticalPosition;
    _theoreticalPosition = 0;
}

void BomberModel::clearTrack()
{
    _trackPoints.clear();
    if(_track)
        _track->clearPoints();
}

void BomberModel::removeBomber()
{
    if(universe())
    {
        _beams = 0;
        _body = 0;
        _hat = 0;
        _battery = 0;

        universe()->removeFigure(_outletBody);
        universe()->removeFigure(_projectionLine);
        universe()->removeFigure(_mistakeLine);
        universe()->removeFigure(_track);
        _outletBody = 0;
        _projectionLine = 0;
        _mistakeLine = 0;
        _track = 0;

        _trackPoints.clear();
        delete _theoreticalPosition;

        universe()->removeLabel(_theoreticalLabel);
        _theoreticalLabel = 0;

        for(uint i = 0; i < _propellersCount; ++i)
        {
            _propellers[i] = 0;
            _motors[i]= 0;

            universe()->removeFigure(_outletCircles[i]);
            _outletCircles[i] = 0;
        }
    }
}

void BomberModel::redrawLabel()
{
    if(_theoreticalLabel && _theoreticalPosition)
    {
        _theoreticalLabelImage.fill(QColor(0,0,0,150));
//        _theoreticalLabelImage.fill(0);
        QPainter painter(&_theoreticalLabelImage);
        QFont font("Roboto", 10, QFont::Normal);

        painter.setFont(font);
        painter.setPen(QColor(Qt::white));

        QString text = QString::number(coordinates().distanceToPoint(*_theoreticalPosition), 'f', 3) + "m";
        painter.drawText(QRectF(0, 0, 64, 32), Qt::AlignHCenter | Qt::AlignVCenter,  text);

        _theoreticalLabel->setCoordinates(*_theoreticalPosition);
        _theoreticalLabel->setImage(_theoreticalLabelImage);

//        qDebug("Image: %d %d", _theoreticalLabelImage.size().width(), _theoreticalLabelImage.size().height());
    }
}

void BomberModel::recalculateMistakeLine()
{
    if(_mistakeLine && _theoreticalPosition)
    {
        _mistakeLine->setPoints(std::vector<QVector3D>{coordinates(), *_theoreticalPosition});
    }
}

void BomberModel::increaseTrack(const QVector3D &point)
{
    // FIXME: refactor
    if(_trackPoints.size() > 512)
    {
        _trackPoints.erase(_trackPoints.begin());
    }

    if(!_trackPoints.empty())
    {
        if(point == _trackPoints.back())
        {
            return;
        }
    }

    _trackPoints.push_back(point);

    if(_track)
    {
        _track->setPoints(_trackPoints);
    }
}
