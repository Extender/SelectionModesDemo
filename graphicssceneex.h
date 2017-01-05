#ifndef GRAPHICSSCENEEX_H
#define GRAPHICSSCENEEX_H

#include <QGraphicsScene>
#include "graphicsviewex.h"
#include <QGraphicsSceneDragDropEvent>
#include <QMimeData>

class GraphicsSceneEx : public QGraphicsScene
{
    Q_OBJECT
public:

    explicit GraphicsSceneEx(QObject *parent = 0);

signals:

public slots:

};

#endif // GRAPHICSSCENEEX_H
