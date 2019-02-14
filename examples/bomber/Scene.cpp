#include "Scene.h"
#include <QTimer>
#include <QQuaternion>
#include <QGraphicsSceneMouseEvent>
#include <QFile>

#include <VasnecovFigure>
#include <VasnecovProduct>
#include <VasnecovLamp>

Scene::Scene(VasnecovUniverse *universe, QObject *parent) :
    VasnecovScene(parent),
    m_universe(universe),
    m_world(nullptr),
    m_bomberModel(nullptr),
    m_propellers{nullptr}
{
    if(!m_universe)
        qCritical("Wrong Universe!");
}

void Scene::createModels()
{
    if(m_universe)
    {
        qInfo("Models loaded: %d", m_universe->loadMeshes());

        m_world = m_universe->addWorld(0, 0, 800, 600);
        if(m_world)
        {
            m_world->setPerspective(30.0f, 0.1f, 20000.0f);
            m_world->setCameraPosition(2., 2., 2.);
            m_world->setCameraTarget(0, 0., 0.);
            m_world->setName("MainWorld");

            VasnecovLamp *sun = m_universe->addLamp("Sun", m_world);
            if(sun)
            {
                sun->setCelestialDirection(0.0, 0.4, 1.0);
            }

            m_bomberModel = m_universe->addAssembly("Bomber model", m_world);

            if(m_bomberModel)
            {
                VasnecovProduct *beams = m_universe->addPart("beams", m_world, "bomber_beams", m_bomberModel);
                VasnecovProduct *body = m_universe->addPart("body", m_world, "bomber_body", m_bomberModel);
                VasnecovProduct *hat = m_universe->addPart("hat", m_world, "bomber_hat", m_bomberModel);
                VasnecovProduct *battery = m_universe->addPart("battery", m_world, "bomber_battery", m_bomberModel);

                if(beams && body && hat && battery)
                {
                    beams->setColor(QColor(50, 50, 60, 255));
                    body->setColor(QColor(190, 190, 190, 255));
                    hat->setColor(QColor(255, 255, 0, 255));
                    battery->setColor(QColor(0, 0, 255, 255));
                }

                for(int i = 0; i < 4; ++i)
                {
                    float xS = 1.0;
                    float yS = 1.0;
                    if(i%2)
                        xS = -1.0;
                    if(i>1)
                        yS = -1.0;

                    m_propellers[i] = m_universe->addPart("Propeller " + QString::number(i), m_world, "bomber_propeller", m_bomberModel);
                    if(m_propellers[i])
                    {
                        m_propellers[i]->setColor(QColor(0, 155, 155, 255));
                        m_propellers[i]->setCoordinates(QVector3D(135.f*0.001 * xS, 135.f*0.001 * yS, 40.f*0.001));
                    }

                    VasnecovProduct *motor = m_universe->addPart("Motor " + QString::number(i), m_world, "bomber_motor", m_bomberModel);
                    if(motor)
                    {
                        motor->setCoordinates(QVector3D(135.f*0.001 * xS, 135.f*0.001 * yS, 5.f*0.001));
                        motor->setColor(QColor(30, 30, 30, 255));
                    }
                }

                m_bomberModel->setCoordinates(0, 0.5, .75);



                // Outlet points


                VasnecovFigure *outlet = m_universe->addFigure("", m_world);
                if(outlet)
                {
                    outlet->setColor(QColor(0, 0, 200, 255));
                    outlet->setType(VasnecovFigure::TypeLines);
                    outlet->setPoints(figureFromObj("stuff/meshes/outlet_bomber_body.obj"));
                    outlet->setAngles(0, 0, 180);
                    outlet->setThickness(2);
                    outlet->setScale(0.001);

                    if(m_bomberModel)
                    {
                        QVector3D pos = m_bomberModel->coordinates();
                        QVector3D ang = m_bomberModel->angles();
                        pos.setZ(0.0f);

                        ang.setX(0.0f);
                        ang.setY(0.0f);
                        ang.setZ(ang.z() + 180);

                        outlet->setCoordinates(pos);
                        outlet->setAngles(ang);
                    }
                }

                for(int i = 0; i < 4; ++i)
                {
                    VasnecovFigure *circle = m_universe->addFigure("", m_world);
                    if(circle)
                    {
                        circle->setColor(QColor(0, 0, 150, 255));
                        circle->setType(VasnecovFigure::TypeLines);
                        circle->setPoints(figureFromObj("stuff/meshes/outlet_bomber_propeller.obj"));
                        circle->setThickness(2);
                        circle->setScale(0.001);

                        if(outlet)
                        {
                            float xS = 1.0;
                            float yS = 1.0;
                            if(i%2)
                                xS = -1.0;
                            if(i>1)
                                yS = -1.0;

                            QVector3D pos = outlet->coordinates();
                            QVector3D ang = outlet->angles();
                            pos.setX(pos.x() + 135 * 0.001 * xS);
                            pos.setY(pos.y() + 135 * 0.001 * yS);

                            circle->setCoordinates(pos);
                            circle->setAngles(ang);
                        }
                    }
                }

                QTimer::singleShot(100, this, &Scene::animatePropellers);
            }

            VasnecovFigure *axis = m_universe->addFigure("Axis", m_world);
            if(axis)
            {
                axis->setColor(0x00AA00);
                axis->setType(VasnecovFigure::TypeLines);
                axis->setThickness(1.0);

                axis->addLastPoint(QVector3D(0.0, 0.0, 0.0));
                axis->addLastPoint(QVector3D(0.4, 0.0, 0.0));
                axis->addLastPoint(QVector3D(0.0, 0.0, 0.0));
                axis->addLastPoint(QVector3D(0.0, 0.4, 0.0));
                axis->addLastPoint(QVector3D(0.0, 0.0, 0.0));
                axis->addLastPoint(QVector3D(0.0, 0.0, 0.4));
            }
        }
        else
        {
            qCritical("Can't create world");
        }
    }
}

void Scene::animatePropellers()
{
    for(int i = 0; i < 4; ++i)
    {
        if(m_propellers[i])
        {
            if(i == 1 || i == 2)
                m_propellers[i]->incrementAngles(QVector3D(0, 0, 10));
            else
                m_propellers[i]->incrementAngles(QVector3D(0, 0, -10));
        }
    }
    update();

    QTimer::singleShot(10, this, &Scene::animatePropellers);
}

void Scene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(m_world)
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

        Vasnecov::Camera camera = m_world->camera();
        QQuaternion qua = QQuaternion::fromAxisAndAngle(QVector3D(0.0f, 0.0f, 1.0f), angle);
        m_world->setCameraPosition(qua.rotatedVector(camera.position()) + QVector3D(0.0f, 0.0f, z));
    }

    VasnecovScene::mouseMoveEvent(event);
}

std::vector<QVector3D> Scene::figureFromObj(const QString& fileName)
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
