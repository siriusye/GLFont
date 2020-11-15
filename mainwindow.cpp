#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "glwidget.h"

#include <QtDebug>
#include <QFileDialog>

#include <QLineEdit>
#include <QBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->actionExit, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(ui->actionLoad_Font, SIGNAL(triggered()), this, SLOT(loadFont()));

    this->resize(QSize(800, 600));

    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setVersion(3, 3);

    GLWidget* glWidget = new GLWidget(this);
    glWidget->setFormat(format);

    QLineEdit* lineEdit = new QLineEdit(this);
    QGridLayout* gridLayout = new QGridLayout;
    gridLayout->addWidget(lineEdit, 0, 0);
    gridLayout->addWidget(glWidget, 1, 0);
    ui->centralwidget->setLayout(gridLayout);

    connect(lineEdit, SIGNAL(textChanged(const QString&)), glWidget, SLOT(updateText(const QString&)));
    connect(this, SIGNAL(fontChanged(const QString&)), glWidget, SLOT(updateFont(const QString&)));
    connect(ui->actionOutline, SIGNAL(toggled(bool)), glWidget, SLOT(toggleOutline(bool)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadFont()
{
    QString fileName;
    if(currentPath.length() == 0)
    {
        fileName = QFileDialog::getOpenFileName(this,
            tr("Load font"), "", tr("Font Files (*.ttf)"));
    }
    else
    {
        fileName = QFileDialog::getOpenFileName(this,
            tr("Load font"), currentPath, tr("Font Files (*.ttf)"));
    }

    currentPath = fileName.left(fileName.lastIndexOf('\\'));
    emit fontChanged(fileName);
}
