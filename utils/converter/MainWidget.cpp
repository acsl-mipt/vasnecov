#include "MainWidget.h"
#include "ui_MainWidget.h"

#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>

#include "libVasnecov/VasnecovMesh.h"

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);
}

MainWidget::~MainWidget()
{
    delete ui;
}

void MainWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWidget::on_selectButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Open model", QDir::homePath(), "OBJ files (*.obj)");
    if(filePath.isEmpty())
    {
        ui->labelResult->setText("File is not selected");
        ui->path->setText(QString());
    }
    else
    {
        ui->path->setText(filePath);
    }
}

void MainWidget::on_convertButton_clicked()
{
    if(ui->path->text().isEmpty())
    {
        ui->labelResult->setText("File is not selected");
        return;
    }

    QString newPath = ui->path->text();
    const QString objFormat = ".obj";
    const QString vmfFormat = ".vmf";
    if(!newPath.endsWith(objFormat, Qt::CaseInsensitive) || newPath.length() < (objFormat.size() + 1))
    {
        ui->labelResult->setText("File is not selected");
        return;
    }
    newPath.replace(newPath.size() - objFormat.size(), objFormat.size(), vmfFormat);

    ui->labelResult->setText("Start reading...");

    VasnecovMesh mesh(ui->path->text());
    if(!mesh.loadModel())
    {
        ui->labelResult->setText("Can't load model");
        return;
    }
    ui->labelResult->setText("Model is readed. Start converting...");

    if(!mesh.writeRawModel(newPath))
    {
        ui->labelResult->setText("Can't convert model");
        return;
    }

    ui->labelResult->setText(QString("Model saved to:\n%1").arg(newPath));
}
