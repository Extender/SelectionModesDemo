#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <stdint.h>
#include <ctime>
#include <QMainWindow>
#include <QFileDialog>
#include <QStandardPaths>
#include <QFileInfo>
#include <QMessageBox>

#include "mainwindowex.h"
#include "graphicssceneex.h"
#include "graphicsviewex.h"
#include "extcolordefs.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public MainWindowEx
{
    Q_OBJECT

public:
    static MainWindow *instance;
    GraphicsSceneEx *scene;
    QFileDialog *saveDialog;
    bool savePath;
    QRectF rectToSave;
    QPainterPath pathToSave;
    QColor currentTagColor;
    QGraphicsPixmapItem *pixmapItem;
    bool imageVisible;
    int bgImageWidth;
    int bgImageHeight;
    QImage *bgImage;
    uint32_t *bgImageData;

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    static uint32_t *qImageToBitmapData(QImage *image);
    bool isRectangularSelection();
    bool isPolygonalSelection();

public slots:
    void extractSelectedAreaBtnClicked();
    void generateNewTagColor();
    void keyDownHandler(QKeyEvent *event);
    void toggleImageBtnClicked();
    void saveDialogFileSelected(QString path);
    void itemGraphicsViewRectItemAdded(QGraphicsRectItem *rectItem);
    void itemGraphicsViewPathItemAdded(QGraphicsPathItem *pathItem);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
