#include "ConverterWidget.h"
#include "ui_ConverterWidget.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QLineEdit>

#include "libVasnecov/VasnecovMesh.h"

const static char* widnowTitleText = "Vasnecov mesh converter";

ConverterWidget::ConverterWidget(QWidget *parent)
    :QWidget(parent)
    , _ui(new Ui::ConverterWidget)
    , _mesh(nullptr)
    , _status(Nothing)
{
    _ui->setupUi(this);

    setWindowTitle(widnowTitleText);
    connect(this, &ConverterWidget::statusChanged, this, &ConverterWidget::handleStatus);
}

ConverterWidget::~ConverterWidget()
{
    delete _ui;
}

void ConverterWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        _ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void ConverterWidget::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
}

void ConverterWidget::on_selectButton_clicked()
{
    setWindowTitle(widnowTitleText);

    QString dirPath = _lastPath.isEmpty() ? QDir::homePath() : _lastPath;
    QString filePath = QFileDialog::getOpenFileName(this, "Open model", dirPath, "OBJ files (*.obj)");
    if(filePath.isEmpty())
    {
        _ui->labelResult->setText("File is not selected");
        _ui->path->setText(QString());

        _objFilePath.clear();
        _lastPath.clear();
    }
    else
    {
        _ui->path->setText(filePath);
        _objFilePath = _ui->path->text();

        QFileInfo fileInfo(_objFilePath);
        _lastPath = fileInfo.absolutePath();
    }
}

void ConverterWidget::on_convertButton_clicked()
{
    _status = Nothing;
    setWindowTitle(widnowTitleText);

    if(_objFilePath.isEmpty())
    {
        _ui->labelResult->setText("File is not selected");
        return;
    }

    _vmfFilePath = _objFilePath;
    const QString objFormat = ".obj";
    const QString vmfFormat = ".vmf";
    if(!_vmfFilePath.endsWith(objFormat, Qt::CaseInsensitive) || _vmfFilePath.length() < (objFormat.size() + 1))
    {
        _ui->labelResult->setText("File is not selected");
        _vmfFilePath.clear();
        return;
    }
    _vmfFilePath.replace(_vmfFilePath.size() - objFormat.size(), objFormat.size(), vmfFormat);

    _ui->labelResult->setText("Start reading...");
    setWindowTitle(QString("%1. Working...").arg(widnowTitleText));
    _status = Selected;
    emit statusChanged();
}

void ConverterWidget::handleStatus()
{
    repaint();

    if(_status == Selected)
        readFile();
    else if(_status == Read)
        writeFile();
}

void ConverterWidget::prepareFile()
{
    if(_mesh != nullptr)
    {
        delete _mesh;
    }
    _mesh = new VasnecovMesh(_objFilePath);
}

void ConverterWidget::readFile()
{
    prepareFile();

    if(!_mesh->loadModel())
    {
        _ui->labelResult->setText("Can't load model");
        _status = Nothing;
        setWindowTitle(QString("%1. Failed").arg(widnowTitleText));
        return;
    }

    _ui->labelResult->setText("Model is read. Start converting...");
    _status = Read;

    emit statusChanged();
}

void ConverterWidget::writeFile()
{
    if(!_mesh->writeRawModel(_vmfFilePath))
    {
        _ui->labelResult->setText("Can't convert model");
        setWindowTitle(QString("%1. Failed").arg(widnowTitleText));
        return;
    }

    _ui->labelResult->setText(QString("Model saved to:\n%1").arg(_vmfFilePath));
    _status = Written;
    setWindowTitle(QString("%1. Converted").arg(widnowTitleText));
}
