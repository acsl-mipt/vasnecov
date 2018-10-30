#pragma once

#include <Vasnecov>

class Scene: public VasnecovScene
{
    Q_OBJECT

public:
    Scene(VasnecovUniverse *universe, QObject *parent = 0);
    void createModels();

private slots:
    void animatePropellers();

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

private:
    std::vector<QVector3D> figureFromObj(const QString& fileName);

private:
    VasnecovUniverse *m_universe;
    VasnecovWorld *m_world;
    VasnecovProduct *m_bomberModel;
    VasnecovProduct *m_propellers[4];

    Q_DISABLE_COPY(Scene)
};

