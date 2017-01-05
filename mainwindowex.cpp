#include "mainwindowex.h"

MainWindowEx::MainWindowEx(QWidget *parent) : QMainWindow(parent)
{

}

void MainWindowEx::closeEvent(QCloseEvent *event)
{
    QMainWindow::closeEvent(event);
}

void MainWindowEx::keyPressEvent(QKeyEvent *event)
{
    keyDownEx(event);
    QMainWindow::keyPressEvent(event);
}

