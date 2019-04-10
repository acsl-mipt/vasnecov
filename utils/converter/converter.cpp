#include <QApplication>
#include <QGLWidget>
#include <QOpenGLWidget>
#include <QGraphicsView>
#include "ConverterWidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    ConverterWidget *view = new ConverterWidget();

    view->show();

    int res = app.exec();

    return res;
}
