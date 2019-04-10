#include <QApplication>
#include <QGLWidget>
#include <QOpenGLWidget>
#include <QGraphicsView>
#include "MainWidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWidget *view = new MainWidget();

    view->setWindowTitle("Vasnecov mesh converter");
    view->show();

    int res = app.exec();

    return res;
}
