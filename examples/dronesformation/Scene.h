#pragma once

#include <Vasnecov>

class BomberModel;

class Scene: public VasnecovScene
{
    Q_OBJECT

public:
    Scene(VasnecovUniverse *univ, QObject *parent = 0);
    ~Scene();

    void drawBackground(QPainter *painter, const QRectF &rect);
    void createModels();

    static std::vector<QVector3D> figureFromObj(const QString &fileName);

private slots:
    void animatePropellers();

protected:
    void keyPressEvent(QKeyEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void wheelEvent(QGraphicsSceneWheelEvent *event);
    bool zoomView(float zoom);
    void moveCopter(float x, float y, float z);
    void rotateCopter(float x, float y, float z);
    void driveCopter(float x, float y, float z);

private:

private:
    VasnecovWorld *_world;
    BomberModel *_bomberModel;
    BomberModel *_bomberModel1;
    BomberModel *_bomberModel2;

    VasnecovFigure *_ground;
    int _bm1SignZ;

    Q_DISABLE_COPY(Scene)
};
