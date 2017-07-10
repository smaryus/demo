#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <cassert>
#include <string>

#include <QLineEdit>

#include "search.h"

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_search( new Search() )
{
    ui->setupUi(this);

    connect(ui->searchLine, SIGNAL(textChanged(const QString &)),
            this, SLOT(textChanged(const QString &)));

    connect(this, SIGNAL(searchFinish(const QString &, bool)),
            this, SLOT(onSearchFinish(const QString &, bool)));

    m_search->setCallback([this](const std::string& result, bool match) {

        emit this->searchFinish(QString(result.c_str()), match);
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::textChanged(const QString& content)
{
    assert( m_search );

    m_search->search(content.toStdString());
    ui->resultsList->clear();
}

void MainWindow::onSearchFinish(const QString& result, bool exactMatch)
{
    ui->resultsList->addItem(result);
    auto item = ui->resultsList->item(ui->resultsList->count() - 1);

    item->setTextColor( exactMatch ? Qt::black : Qt::blue );
}
