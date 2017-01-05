#ifndef GRAPHICSVIEWEX_H
#define GRAPHICSVIEWEX_H

#include <QGraphicsView>
#include <QWheelEvent>
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QGraphicsPathItem>
#include <QDebug>
#include <QScrollBar>

#include "graphicssceneex.h"

#define QVARIANT_PEN_BACKUP_ID 100

class GraphicsViewEx : public QGraphicsView
{
    Q_OBJECT
public:
    bool defaultHandlers;
    double zoomFactor;
    bool dragging;
    bool newItem;
    QPoint startPoint;
    QMap<QGraphicsItem*,QPoint> *posMap;
    QPoint offset;
    QGraphicsRectItem *newRectItem;
    QGraphicsItem *selectedItem;
    QString tagToUse;
    QGraphicsPathItem *newPathItem;
    QPainterPath *painterPath;

    QString (*getCurrentTagStringFunction)(void);
    QColor (*getCurrentTagColorFunction)(void);
    bool (*getIsRectangularSelectionFunction)(void);
    bool (*getIsPolygonalSelectionFunction)(void);


    GraphicsViewEx(QWidget *parent);
    void wheelEvent(QWheelEvent *e);
    void enterEvent(QEvent *e);
    void leaveEvent(QEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);
    void dropEvent(QDropEvent *e); // Needed! Gets called by GraphicsSceneEx!
    void toggleNewItem();
    void setZoomFactor(double newZoomFactor);
    void resetZoom();
    QGraphicsRectItem *addItem(QRectF rect,QString tag);
    QGraphicsPathItem *addItem(QPainterPath path,QString tag);

public slots:
    void highlightItem(QGraphicsItem *item);
    void unhighlightItem(QGraphicsItem *item);

signals:
    void wheelEx(QWheelEvent *e);
    void mouseEnterEx(QEvent *e);
    void mouseLeaveEx(QEvent *e);
    void mouseMoveEx(QMouseEvent *e);
    void mouseDownEx(QMouseEvent *e);
    void mouseUpEx(QMouseEvent *e);
    void mouseDoubleClickEx(QMouseEvent *e);
    void dropEx(QDropEvent *e);
    void rectItemAdded(QGraphicsRectItem *rectItem);
    void pathItemAdded(QGraphicsPathItem *pathItem);
    void selectedItemChanged(QGraphicsItem *item);
};

#endif // GRAPHICSVIEWEX_H
