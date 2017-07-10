#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>

class Search;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void searchFinish(const QString &, bool);

private slots:
    void textChanged(const QString &);
    void onSearchFinish(const QString &, bool exactMatch);


private:
    Ui::MainWindow *ui;
    std::unique_ptr<Search> m_search;
};

#endif // MAINWINDOW_H
