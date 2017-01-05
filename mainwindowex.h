#ifndef MAINWINDOWEX_H
#define MAINWINDOWEX_H

#include <QMainWindow>

class MainWindowEx : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindowEx(QWidget *parent = 0);
    virtual void closeEvent(QCloseEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);

signals:
    void keyDownEx(QKeyEvent *event);

public slots:
};

#endif // MAINWINDOWEX_H
