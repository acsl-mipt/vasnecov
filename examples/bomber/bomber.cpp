#include <QApplication>
#include <QGLWidget>
#include <QGraphicsView>
#include "Scene.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    VasnecovUniverse *universe = new VasnecovUniverse();

    QGLWidget *widget = new QGLWidget(QGLFormat(QGL::SampleBuffers));
    widget->makeCurrent();

    Scene *scene = new Scene(universe, &app);
    universe->setContext(widget->context());
    scene->setUniverse(universe);

    QGraphicsView *view = new QGraphicsView(scene);
    view->resize(800, 600);
    view->setViewport(widget);
    view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    scene->createModels();

    view->setWindowTitle("Bomber - Vasnecov example");
    view->show();

    int res = app.exec();

    delete view;
    delete universe;

    return res;
}
