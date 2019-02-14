#include "Scene.h"

#include <QFile>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QQuaternion>
#include <QTimer>

#include <VasnecovFigure>
#include <VasnecovLamp>

#include "BomberModel.h"

Scene::Scene(VasnecovUniverse *univ, QObject *parent) :
    VasnecovScene(parent),
    _world(nullptr),
    _bomberModel(nullptr),
    _bomberModel1(nullptr),
    _bomberModel2(nullptr),
    _ground(nullptr),
    _bm1SignZ(1)
{
    VasnecovScene::setUniverse(univ);

    if(!universe())
        qCritical("Wrong Universe!");
}

Scene::~Scene()
{
    delete _bomberModel;
}

void Scene::drawBackground(QPainter *painter, const QRectF &rect)
{
    if(_world)
    {
        Vasnecov::WorldParameters params = _world->worldParameters();
        int w(params.width()), h(params.height());

        if(windowWidth() != w || windowHeight() != h)
        {
            _world->setWindow(0, 0, windowWidth(), windowHeight());
        }
    }

    VasnecovScene::drawBackground(painter, rect);
}

void Scene::createModels()
{
    if(universe())
    {
        qInfo("Models loaded: %d", universe()->loadMeshes());

        _world = universe()->addWorld(0, 0, 1850, 950);
        if(_world)
        {
            _world->setPerspective(30.0f, 0.1f, 20000.0f);
            _world->setCameraPosition(2., 2., 2.);
            _world->setCameraTarget(0, 0., 0.);
            _world->setName("MainWorld");

            VasnecovLamp *sun = universe()->addLamp("Sun", _world);
            if(sun)
            {
                sun->setCelestialDirection(0.0, 0.4, 1.0);
            }

            _ground = universe()->addFigure("Ground", _world);
            if(_ground)
            {
                _ground->createSquareGrid(100, 100, QColor(50, 50, 50, 200), 101, 101);
            }
            VasnecovFigure *circle = universe()->addFigure("Circle", _world);
            if(circle)
            {
                circle->createCircle(2.0, QColor(100, 100, 100, 200), 128);
            }

            VasnecovFigure *axis = universe()->addFigure("Axis", _world);
            if(axis)
            {
                axis->setColor(0x00AA00);
                axis->setType(VasnecovFigure::TypeLines);
                axis->setThickness(3);

                axis->addLastPoint(QVector3D(0.0, 0.0, 0.0));
                axis->addLastPoint(QVector3D(0.4, 0.0, 0.0));
                axis->addLastPoint(QVector3D(0.0, 0.0, 0.0));
                axis->addLastPoint(QVector3D(0.0, 0.4, 0.0));
                axis->addLastPoint(QVector3D(0.0, 0.0, 0.0));
                axis->addLastPoint(QVector3D(0.0, 0.0, 0.4));
            }

            _bomberModel = new BomberModel(universe(), _world);

            if(_bomberModel && _bomberModel->create("Bomber 0"))
            {
                _bomberModel->setCoordinates(QVector3D(0, 0.5, .75));
            }

            BomberModel *b0_ghost = new BomberModel(universe(), _world);
            if(b0_ghost && b0_ghost->create("Bomber 0 Ghost") && _bomberModel)
            {
                b0_ghost->setCoordinates(_bomberModel->coordinates() + QVector3D(.1, .1, 0));
                b0_ghost->setGhostMode();

                if(_bomberModel)
                {
                    _bomberModel->setTheoreticalPosition(b0_ghost->coordinates());
                }
            }

            _bomberModel1 = new BomberModel(universe(), _world);
            if(_bomberModel1 && _bomberModel1->create("Bomber 1"))
            {
                _bomberModel1->setCoordinates(QVector3D(0.5, -0.5, 0.0));
                _bomberModel1->setAngles(QVector3D(0, 0, 120));
            }

            _bomberModel2 = new BomberModel(universe(), _world);
            if(_bomberModel2 && _bomberModel2->create("Bomber 2"))
            {
                _bomberModel2->setCoordinates(QVector3D(-1.3, -1.5, 0.0));
                _bomberModel2->setAngles(QVector3D(0, 60, 0));

                _bomberModel2->rotatePropellers(135);
            }

            QTimer::singleShot(100, this, &Scene::animatePropellers);
        }
        else
        {
            qCritical("Can't create world");
        }
    }
}

void Scene::animatePropellers()
{
    if(_bomberModel)
    {
        _bomberModel->rotatePropellers(10);
    }
    if(_bomberModel1)
    {
        QVector3D newPos = _bomberModel1->coordinates() + QVector3D(0.0f, 0.0f, 0.005f * _bm1SignZ);
        if(newPos.z() > 2.0)
        {
            _bm1SignZ = -1;
            newPos.setZ(2.0);
        }
        else if(newPos.z() < 0.0)
        {
            _bm1SignZ = 1;
            newPos.setZ(0.0);
        }

        _bomberModel1->setCoordinates(newPos);

        _bomberModel1->rotatePropellers(20);
    }
    if(_bomberModel2)
    {
//        _bomberModel2->rotatePropellers(1);
    }

    update();

    QTimer::singleShot(10, this, &Scene::animatePropellers);
}

void Scene::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_Up:
            if(event->modifiers() & Qt::ControlModifier)
                moveCopter(0.0, 0.0, 0.1);
            else if(event->modifiers() & Qt::ShiftModifier)
                moveCopter(0.1, 0.0, 0.0);
            else
                driveCopter(0.1, 0.0, 0.0);
            break;
        case Qt::Key_Down:
            if(event->modifiers() & Qt::ControlModifier)
                moveCopter(0.0, 0.0, -0.1);
            else
                moveCopter(-0.1, 0.0, 0.0);
            break;
        case Qt::Key_Left:
            if(event->modifiers() & Qt::ControlModifier)
                rotateCopter(0.0, 0.0, 3);
            else
                moveCopter(0.0, 0.1, 0.0);
            break;
        case Qt::Key_Right:
            if(event->modifiers() & Qt::ControlModifier)
                rotateCopter(0.0, 0.0, -3);
            else
                moveCopter(0.0, -0.1, 0.0);
            break;
        default:
            break;
    }

    VasnecovScene::keyPressEvent(event);
}
void Scene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(_world)
    {
        QPointF diff = event->lastScenePos() - event->scenePos();

        float angle(0.0f);
        if(diff.x() > 0.0)
        {
            angle = 5.0f;
        }
        else if (diff.x() < 0.0)
        {
            angle = -5.0f;
        }

        float z(0.0f);
        if(diff.y() > 0.0)
        {
            z = 0.1f;
        }
        else if (diff.y() < 0.0)
        {
            z = -0.1f;
        }

        Vasnecov::Camera camera = _world->camera();
        QQuaternion qua = QQuaternion::fromAxisAndAngle(QVector3D(0.0f, 0.0f, 1.0f), angle);
        QVector3D newPos = qua.rotatedVector(camera.position()) + QVector3D(0.0f, 0.0f, z);

        if(newPos.z() < 0.0f)
            newPos.setZ(0.0f);

        _world->setCameraPosition(newPos);
    }

    VasnecovScene::mouseMoveEvent(event);
}

void Scene::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    zoomView(-event->delta() * 0.015);

    VasnecovScene::wheelEvent(event);
}

bool Scene::zoomView(float zoom)
{
    if(_world)
    {

        float dist = _world->camera().position().length();

        if(dist > 0 && zoom != 0.0)
        {
            QVector3D pos = _world->camera().position();
            float d = dist;

            float dArg = pow(d / 0.5e-6, 0.25f);
            d = 0.5e-6 * pow(dArg + zoom, 4.0f);
//            d = correctViewDistance(d);

            float k = d/dist;
            pos = pos * k;

            _world->setCameraPosition(pos);
            dist = d;

            return true;
        }
    }

    return false;
}

void Scene::moveCopter(float x, float y, float z)
{
    if(x || y || z)
    {
        if(_bomberModel)
        {
            QVector3D newPos = _bomberModel->coordinates() + QVector3D(x*.25, y*0.25, z*0.25);
            if(newPos.z() < 0.0f)
                newPos.setZ(0.0f);

            _bomberModel->setCoordinates(newPos);
//            update();
        }
    }
}

void Scene::rotateCopter(float x, float y, float z)
{
    if(x || y || z)
    {
        if(_bomberModel)
        {
            _bomberModel->setAngles(_bomberModel->angles() + QVector3D(x, y, z));
        }
    }
}

void Scene::driveCopter(float x, float y, float z)
{
    if(x || y || z)
    {
        if(_bomberModel)
        {
            QVector3D rotPos(x*.25, y*0.25, z*0.25);
            QQuaternion qua = QQuaternion::fromAxisAndAngle(QVector3D(0, 0, 1),_bomberModel->angles().z());
            rotPos = qua.rotatedVector(rotPos);

            _bomberModel->setCoordinates(_bomberModel->coordinates() + rotPos);
        }
    }
}

std::vector<QVector3D> Scene::figureFromObj(const QString &fileName)
{
    std::vector<QVector3D> points;

    QFile file(fileName);

    if(file.open(QIODevice::ReadOnly))
    {
        std::vector<QVector3D> verticies;
        std::vector<QSize> indicies;

        int nLine(0);
        while(true)
        {
            QByteArray line = file.readLine();
            if(line.isEmpty())
                break;

            ++nLine;

            QString text = QString::fromUtf8(line);
            text = text.simplified();

            if(!text.isEmpty())
            {
                QVector<QStringRef> refs = text.splitRef(' ');

                if(refs.at(0) == "v")
                {
                    float x(0.0f), y(0.0f), z(0.0f);
                    for(int i = 1; i < refs.size(); ++i)
                    {
                        if(i == 1)
                        {
                            x = refs.at(i).toFloat();
                        }
                        else if(i == 2)
                        {
                            y = refs.at(i).toFloat();
                        }
                        else if(i == 3)
                        {
                            z = refs.at(i).toFloat();
                        }
                        else
                        {
                            break;
                        }
                    }
                    verticies.push_back(QVector3D(x, y, z));
//                    qDebug("%f %f %f", x, y, z);
                }
                else if(refs.at(0) == "l")
                {
                    int first(0), last(0);
                    for(int i = 1; i < refs.size(); ++i)
                    {
                        if(i == 1)
                        {
                            first = refs.at(i).toUInt();
                        }
                        else if(i == 2)
                        {
                            last = refs.at(i).toUInt();
                        }
                        else
                        {
                            break;
                        }
                    }
                    indicies.push_back(QSize(first, last));
//                    qDebug("%d %d", first, last);
                }
            }
        }

        points.reserve(indicies.size() * 2);
        for(uint i = 0; i < indicies.size(); ++i)
        {
            uint first = static_cast<uint>(indicies.at(i).width() - 1);
            uint last  = static_cast<uint>(indicies.at(i).height() - 1);

            if(first < verticies.size() && last < verticies.size())
            {
                points.push_back(verticies.at(first));
                points.push_back(verticies.at(last));
            }
        }
        qDebug("Strings readed: %d", nLine);
    }

    return points;
}
