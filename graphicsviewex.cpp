#include "graphicsviewex.h"

GraphicsViewEx::GraphicsViewEx(QWidget *parent)
    : QGraphicsView(parent)
{
    setMouseTracking(true);
    setScene(new GraphicsSceneEx(this)); // We need to pass a parent!
    posMap=new QMap<QGraphicsItem*,QPoint>();
    defaultHandlers=true;
    zoomFactor=1.0;
    dragging=false;
    startPoint=QPoint();
    newItem=false;
    newRectItem=0;
    newPathItem=0;
    painterPath=0;
    selectedItem=0;
    tagToUse="";
}

void GraphicsViewEx::wheelEvent(QWheelEvent *e)
{
    if(!defaultHandlers||newItem)
    {
        wheelEx(e);
        return;
    }
    if(dragging)
        dragging=false;
    if(e->delta()>0)
    {
        double newZoomFactor=zoomFactor+(zoomFactor/10);
        scale(newZoomFactor/zoomFactor,newZoomFactor/zoomFactor);
        zoomFactor=newZoomFactor;
    }
    else
    {
        double newZoomFactor=zoomFactor-(zoomFactor/10);
        scale(newZoomFactor/zoomFactor,newZoomFactor/zoomFactor);
        zoomFactor=newZoomFactor;
    }
    wheelEx(e);
}

void GraphicsViewEx::enterEvent(QEvent *e)
{
    QGraphicsView::enterEvent(e);
    mouseEnterEx(e);
}

void GraphicsViewEx::leaveEvent(QEvent *e)
{
    QGraphicsView::leaveEvent(e);
    if(dragging)
        dragging=false;
    if(newItem)
    {
        toggleNewItem();
    }
    mouseLeaveEx(e);
}

void GraphicsViewEx::mouseMoveEvent(QMouseEvent *e)
{
    QGraphicsView::mouseMoveEvent(e);
    if(!defaultHandlers)
        goto EmitEvent;
    if(newItem)
    {
        if(getIsRectangularSelectionFunction())
        {
            if(newRectItem!=0)
            {
                // !!!!! No setPos here !!!!!
                int x=newRectItem->rect().x();
                int y=newRectItem->rect().y();
                QPoint scenePoint=QPointF(mapToScene(e->pos())).toPoint();
                int w=scenePoint.x()-x;
                int h=scenePoint.y()-y;
                if(w<0)
                    w=0;
                if(h<0)
                    h=0;
                newRectItem->setRect(x,y,w,h);
            }
        }
        else
        {
            if(newPathItem!=0)
            {
                if(!getIsPolygonalSelectionFunction())
                {
                    QPoint scenePoint=QPointF(mapToScene(e->pos())).toPoint();
                    painterPath->lineTo(scenePoint);
                    newPathItem->setPath(*painterPath);
                }
            }
        }
        goto EmitEvent;
    }
    if(dragging)
    {
        int x=offset.x()+(startPoint.x()-e->x());
        int y=offset.y()+(startPoint.y()-e->y());
        int xDelta=abs(x-offset.x());
        int yDelta=abs(y-offset.y());
        if(x<0)
            x=0;
        if(x>horizontalScrollBar()->maximum())
            x=horizontalScrollBar()->maximum();
        if(y<0)
            y=0;
        if(y>verticalScrollBar()->maximum())
            y=verticalScrollBar()->maximum();

        if(xDelta<10&&yDelta<10)
            goto EmitEvent;
        // Hack.
        horizontalScrollBar()->setValue(x);
        verticalScrollBar()->setValue(y);
    }
    EmitEvent:
    mouseMoveEx(e);
}

void GraphicsViewEx::mousePressEvent(QMouseEvent *e)
{
    QGraphicsItem *nSI=itemAt(e->pos());
    if(selectedItem!=nSI)
    {
        if(selectedItem!=0)
            unhighlightItem(selectedItem);
        selectedItem=nSI;
        if(nSI!=0)
            highlightItem(nSI);
        selectedItemChanged(nSI);
    }
    QGraphicsView::mousePressEvent(e);
    if(e->button()&Qt::RightButton)
    {
        tagToUse=getCurrentTagStringFunction();
        if(tagToUse.length()>0)
        {
            newItem=true;
        }
    }
    if(newItem&&e->button()==Qt::RightButton)
    {
        if(getIsRectangularSelectionFunction())
        {
            // !!!!! No setPos here !!!!!
            newRectItem=new QGraphicsRectItem();
            newRectItem->setPen(QPen(QBrush(getCurrentTagColorFunction()),1));
            QPointF scenePoint=mapToScene(e->pos());
            scene()->addItem(newRectItem);
            newRectItem->setRect(scenePoint.x(),scenePoint.y(),0,0);
        }
        else
        {
            if(!getIsPolygonalSelectionFunction()||newPathItem==0)
            {
                newPathItem=new QGraphicsPathItem();
                newPathItem->setPen(QPen(QBrush(getCurrentTagColorFunction()),1));
                QPointF scenePoint=mapToScene(e->pos());
                painterPath=new QPainterPath(scenePoint);
                scene()->addItem(newPathItem);
                newPathItem->setPath(*painterPath);
            }
            else // if(getIsPolygonalSelectionFunction())
            {
                QPoint scenePoint=QPointF(mapToScene(e->pos())).toPoint();
                painterPath->lineTo(scenePoint);
                newPathItem->setPath(*painterPath);
            }

        }
        goto SkipDragActions;
    }
    if(zoomFactor*scene()->width()<geometry().width()&&zoomFactor*scene()->height()<geometry().height()) // geometry: boundaries
        goto SkipDragActions;
    dragging=true;
    startPoint=e->pos();
    posMap->clear();
    offset=QPoint(horizontalScrollBar()->value(),verticalScrollBar()->value());
    SkipDragActions:
    mouseDownEx(e);
}

void GraphicsViewEx::mouseReleaseEvent(QMouseEvent *e)
{
    QGraphicsView::mouseReleaseEvent(e);
    if(newItem)
    {
        if(getIsRectangularSelectionFunction())
        {
            toggleNewItem();
            //scene()->removeItem(newRectItem); // Already removed in toggleNewItem().

            QRectF nIR=newRectItem->rect();

            QGraphicsRectItem *substitute=addItem(nIR,tagToUse);

            rectItemAdded(substitute);
            delete newRectItem;
            selectedItem=0; // Needed.
            newRectItem=0; // Tested, works. Do not use new QGraphicsRectItem(newRectItem), this item wouldn't be in the scene's items.
        }
        else if(!getIsPolygonalSelectionFunction()||e->button()==Qt::LeftButton)
        {
            toggleNewItem();
            QPainterPath nPP=newPathItem->path();
            nPP.closeSubpath();
            QGraphicsPathItem *substitute=addItem(nPP,tagToUse);

            pathItemAdded(substitute);
            delete newPathItem;
            selectedItem=0; // Needed.
            newPathItem=0; // Tested, works. Do not use new QGraphicsPathItem(newPathItem), this item wouldn't be in the scene's items.
        }
    }
    dragging=false;
    mouseUpEx(e);
}

void GraphicsViewEx::mouseDoubleClickEvent(QMouseEvent *e)
{
    QGraphicsView::mouseDoubleClickEvent(e);
    mouseDoubleClickEx(e);
}

void GraphicsViewEx::dropEvent(QDropEvent *e)
{
    dropEx(e);
}

void GraphicsViewEx::toggleNewItem()
{
    if(newItem)
    {
        newItem=false;
        setCursor(QCursor(Qt::ArrowCursor));
        if(newRectItem!=0)
            scene()->removeItem(newRectItem);
        if(newPathItem!=0)
            scene()->removeItem(newPathItem);
    }
    else
    {
        newItem=true;
        if(dragging)
            dragging=false;
        setCursor(QCursor(Qt::CrossCursor));
    }
}

void GraphicsViewEx::setZoomFactor(double newZoomFactor)
{
    scale(newZoomFactor/zoomFactor,newZoomFactor/zoomFactor);
    zoomFactor=newZoomFactor;
}

void GraphicsViewEx::resetZoom()
{
    setZoomFactor(1.0);
}

QGraphicsRectItem *GraphicsViewEx::addItem(QRectF rect, QString tag)
{
    QGraphicsRectItem *rectItem=new QGraphicsRectItem();
    QBrush brush(getCurrentTagColorFunction());

    QGraphicsSimpleTextItem *textItem=new QGraphicsSimpleTextItem(tag);
    textItem->setData(Qt::UserRole,QVariant((qulonglong)rectItem));
    textItem->setPos(QPoint(rect.x()+5,rect.y()+5));
    textItem->setFont(QFont(textItem->font().family(),12));
    textItem->setBrush(brush);
    textItem->setParentItem(rectItem);
    scene()->addItem(textItem);

    rectItem->setData(Qt::UserRole,QVariant((qulonglong)textItem));
    rectItem->setData(Qt::UserRole+2/*Yes, 2*/,QVariant(tag));
    rectItem->setRect(rect);
    rectItem->setPen(QPen(brush,1));
    scene()->addItem(rectItem);
    return rectItem;
}

QGraphicsPathItem *GraphicsViewEx::addItem(QPainterPath path, QString tag)
{
    QGraphicsPathItem *pathItem=new QGraphicsPathItem();
    QBrush brush(getCurrentTagColorFunction());

    QGraphicsSimpleTextItem *textItem=new QGraphicsSimpleTextItem(tag);
    textItem->setData(Qt::UserRole,QVariant((qulonglong)pathItem));
    QPainterPath::Element firstPoint=path.elementAt(0);
    textItem->setPos(QPoint(firstPoint.x+5,firstPoint.y+5));
    textItem->setFont(QFont(textItem->font().family(),12));
    textItem->setBrush(brush);
    textItem->setParentItem(pathItem);
    scene()->addItem(textItem);

    pathItem->setData(Qt::UserRole,QVariant((qulonglong)textItem));
    pathItem->setData(Qt::UserRole+2/*Yes, 2*/,QVariant(tag));
    pathItem->setPath(path);
    pathItem->setPen(QPen(brush,1));
    scene()->addItem(pathItem);
    return pathItem;
}

void GraphicsViewEx::highlightItem(QGraphicsItem *item)
{
    const QBrush highlightingBrush=QBrush(0xFFFF0000);
    const QPen highlightingPen=QPen(highlightingBrush,3);

    const QBrush textHighlightingBrush=highlightingBrush;
    const QPen textHighlightingPen=QPen(textHighlightingBrush,1);

    if(item==0)
        return;

    int itemType=item->type();
    if(itemType==9 /*QGraphicsSimpleTextItem::type()*/)
    {
        QGraphicsSimpleTextItem *textItem=(QGraphicsSimpleTextItem*)item;
        textItem->setData(Qt::UserRole+100,QVariant(textItem->pen())); // Back up standard (non-highlighting) pen.
        textItem->setPen(textHighlightingPen);

        QVariant secondItemRef=item->data(Qt::UserRole);
        if(!secondItemRef.isNull())
        {
            QGraphicsItem *secondItemProto=(QGraphicsItem*)secondItemRef.toULongLong();
            int secondItemType=secondItemProto->type();

            if(secondItemType==2 /*QGraphicsPathItem::type()*/)
            {
                QGraphicsPathItem *secondItem=(QGraphicsPathItem*)secondItemProto;
                secondItem->setData(Qt::UserRole+100,QVariant(secondItem->pen())); // Back up standard (non-highlighting) pen.
                secondItem->setPen(highlightingPen);
            }
            else if(secondItemType==3 /*QGraphicsRectItem::type()*/)
            {
                QGraphicsRectItem *secondItem=(QGraphicsRectItem*)secondItemProto;
                secondItem->setData(Qt::UserRole+100,QVariant(secondItem->pen())); // Back up standard (non-highlighting) pen.
                secondItem->setPen(highlightingPen);
            }
        }
    }
    else if(itemType==2 /*QGraphicsPathItem::type()*/)
    {
        QGraphicsPathItem *castItem=(QGraphicsPathItem*)item;
        castItem->setData(Qt::UserRole+100,QVariant(castItem->pen())); // Back up standard (non-highlighted) pen.
        castItem->setPen(highlightingPen);

        QVariant textItemRef=item->data(Qt::UserRole);
        if(!textItemRef.isNull())
        {
            QGraphicsSimpleTextItem *textItem=(QGraphicsSimpleTextItem*)item->data(Qt::UserRole).toULongLong();
            textItem->setData(Qt::UserRole+100,QVariant(textItem->pen())); // Back up standard (non-highlighting) pen.
            textItem->setPen(textHighlightingPen);
        }
    }
    else if(itemType==3 /*QGraphicsRectItem::type()*/)
    {
        QGraphicsRectItem *castItem=(QGraphicsRectItem*)item;
        castItem->setData(Qt::UserRole+100,QVariant(castItem->pen())); // Back up standard (non-highlighted) pen.
        castItem->setPen(highlightingPen);

        QVariant textItemRef=item->data(Qt::UserRole);
        if(!textItemRef.isNull())
        {
            QGraphicsSimpleTextItem *textItem=(QGraphicsSimpleTextItem*)item->data(Qt::UserRole).toULongLong();
            textItem->setData(Qt::UserRole+100,QVariant(textItem->pen())); // Back up standard (non-highlighting) pen.
            textItem->setPen(textHighlightingPen);
        }
    }
}

void GraphicsViewEx::unhighlightItem(QGraphicsItem *item)
{
    const QBrush defaultBrush=QBrush(0xFF000000);
    const QPen defaultPen=QPen(defaultBrush,1);

    const QBrush defaultTextBrush=defaultBrush;
    const QPen defaultTextPen=QPen(defaultTextBrush,1);

    if(item==0)
        return;

    int itemType=item->type();
    if(itemType==9 /*QGraphicsSimpleTextItem::type()*/)
    {
        QGraphicsSimpleTextItem *textItem=(QGraphicsSimpleTextItem*)item;
        QVariant penBackup=textItem->data(Qt::UserRole+QVARIANT_PEN_BACKUP_ID);
        textItem->setPen(penBackup.isNull()?defaultTextPen:penBackup.value<QPen>());

        QVariant secondItemRef=item->data(Qt::UserRole);
        if(!secondItemRef.isNull())
        {
            QGraphicsItem *secondItemProto=(QGraphicsItem*)secondItemRef.toULongLong();
            int secondItemType=secondItemProto->type();

            if(secondItemType==2 /*QGraphicsPathItem::type()*/)
            {
                QGraphicsPathItem *secondItem=(QGraphicsPathItem*)secondItemProto;
                QVariant penBackup=secondItem->data(Qt::UserRole+QVARIANT_PEN_BACKUP_ID);
                secondItem->setPen(penBackup.isNull()?defaultPen:penBackup.value<QPen>());
            }
            else if(secondItemType==3 /*QGraphicsRectItem::type()*/)
            {
                QGraphicsRectItem *secondItem=(QGraphicsRectItem*)secondItemProto;
                QVariant penBackup=secondItem->data(Qt::UserRole+QVARIANT_PEN_BACKUP_ID);
                secondItem->setPen(penBackup.isNull()?defaultPen:penBackup.value<QPen>());
            }
        }
    }
    else if(itemType==2 /*QGraphicsPathItem::type()*/)
    {
        QGraphicsPathItem *castItem=(QGraphicsPathItem*)item;
        QVariant penBackup=castItem->data(Qt::UserRole+QVARIANT_PEN_BACKUP_ID);
        castItem->setPen(penBackup.isNull()?defaultPen:penBackup.value<QPen>());

        QVariant textItemRef=item->data(Qt::UserRole);
        if(!textItemRef.isNull())
        {
            QGraphicsSimpleTextItem *textItem=(QGraphicsSimpleTextItem*)item->data(Qt::UserRole).toULongLong();
            QVariant textPenBackup=textItem->data(Qt::UserRole+QVARIANT_PEN_BACKUP_ID);
            textItem->setPen(textPenBackup.isNull()?defaultTextPen:textPenBackup.value<QPen>());
        }
    }
    else if(itemType==3 /*QGraphicsRectItem::type()*/)
    {
        QGraphicsRectItem *castItem=(QGraphicsRectItem*)item;
        QVariant penBackup=castItem->data(Qt::UserRole+QVARIANT_PEN_BACKUP_ID);
        castItem->setPen(penBackup.isNull()?defaultPen:penBackup.value<QPen>());

        QVariant textItemRef=item->data(Qt::UserRole);
        if(!textItemRef.isNull())
        {
            QGraphicsSimpleTextItem *textItem=(QGraphicsSimpleTextItem*)item->data(Qt::UserRole).toULongLong();
            QVariant textPenBackup=textItem->data(Qt::UserRole+QVARIANT_PEN_BACKUP_ID);
            textItem->setPen(textPenBackup.isNull()?defaultTextPen:textPenBackup.value<QPen>());
        }
    }
}
