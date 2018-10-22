#include <QApplication>
#include <QGLWidget>
#include <QOpenGLWidget>
#include <QGraphicsView>
#include "Scene.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    VasnecovUniverse *universe = new VasnecovUniverse;

    QGLWidget *widget = new QGLWidget();
//    QOpenGLWidget *widget = new QOpenGLWidget();
    widget->makeCurrent();

//    QSurfaceFormat format;
////    format.setDepthBufferSize(24);
////    format.setStencilBufferSize(8);
//    format.setVersion(1, 1);
////    format.setProfile(QSurfaceFormat::CompatibilityProfile);
//    widget->setFormat(format); // must be called before the widget or its parent window gets shown

    Scene *scene = new Scene(universe, &app);

    QGraphicsView *view = new QGraphicsView(scene);
    view->resize(1850, 950);
    view->setViewport(widget);
    view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    scene->createModels();

    view->setWindowTitle("DronesFormation - Vasnecov example");
    view->show();

    int res = app.exec();

    delete view;
    delete universe;

    return res;
}
