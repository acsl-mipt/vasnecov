#pragma once

#include <QWidget>

class VasnecovMesh;

namespace Ui {
    class ConverterWidget;
}

class ConverterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConverterWidget(QWidget *parent = nullptr);
    ~ConverterWidget() override;

protected:
    void changeEvent(QEvent *e) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void on_selectButton_clicked();
    void on_convertButton_clicked();
    void handleStatus();

signals:
    void statusChanged();

private:
    void prepareFile();
    void readFile();
    void writeFile();

    Ui::ConverterWidget*    _ui;
    VasnecovMesh*           _mesh;
    QString                 _lastPath;

    enum Status
    {
        Nothing = 0,
        Selected,
        Read,
        Written,
    };
    Status                  _status;
    QString                 _objFilePath;
    QString                 _vmfFilePath;

    Q_DISABLE_COPY(ConverterWidget)
};
