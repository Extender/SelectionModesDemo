#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow *MainWindow::instance = 0;

MainWindow::MainWindow(QWidget *parent) :
    MainWindowEx(parent),
    ui(new Ui::MainWindow)
{
    srand(time(0));

    MainWindow::instance=this;
    ui->setupUi(this);
    ui->rectangularSelectionBtn->setChecked(true);
    scene=new GraphicsSceneEx();
    ui->graphicsView->getCurrentTagStringFunction=[]()->QString{return QString::number(rand());};
    ui->graphicsView->getCurrentTagColorFunction=[]()->QColor{return MainWindow::instance->currentTagColor;};
    ui->graphicsView->getIsRectangularSelectionFunction=[]()->bool{return MainWindow::instance->isRectangularSelection();};
    ui->graphicsView->getIsPolygonalSelectionFunction=[]()->bool{return MainWindow::instance->isPolygonalSelection();};

    QPixmap bgPixmap(":/res/colors.jpg");
    pixmapItem=new QGraphicsPixmapItem(bgPixmap);
    bgImage=new QImage(bgPixmap.toImage());
    bgImageWidth=bgImage->width();
    bgImageHeight=bgImage->height();
    bgImageData=qImageToBitmapData(bgImage);
    imageVisible=true;

    scene->addItem(pixmapItem);
    ui->graphicsView->setScene(scene);

    saveDialog=new QFileDialog(this);

    saveDialog->setAcceptMode(QFileDialog::AcceptSave);
    QStringList filters;
    filters<<"JPEG image (*.jpg)"
           <<"PNG image (*.png)"
           <<"GIF image (*.gif)"
           <<"Bitmap (*.bmp)"
           <<"All files (*.*)";
    saveDialog->setNameFilters(filters);
    saveDialog->setDirectory(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    connect(saveDialog,SIGNAL(fileSelected(QString)),this,SLOT(saveDialogFileSelected(QString)));

    connect(this,SIGNAL(keyDownEx(QKeyEvent*)),this,SLOT(keyDownHandler(QKeyEvent*)));
    connect(ui->extractSelectedAreaBtn,SIGNAL(clicked(bool)),this,SLOT(extractSelectedAreaBtnClicked()));
    connect(ui->toggleImageBtn,SIGNAL(clicked(bool)),this,SLOT(toggleImageBtnClicked()));
    connect(ui->graphicsView,SIGNAL(rectItemAdded(QGraphicsRectItem*)),this,SLOT(itemGraphicsViewRectItemAdded(QGraphicsRectItem*)));
    connect(ui->graphicsView,SIGNAL(pathItemAdded(QGraphicsPathItem*)),this,SLOT(itemGraphicsViewPathItemAdded(QGraphicsPathItem*)));

    generateNewTagColor();
}

MainWindow::~MainWindow()
{
    delete ui;
}

uint32_t *MainWindow::qImageToBitmapData(QImage *image)
{
    int32_t width=image->width();
    int32_t height=image->height();
    uint32_t *out=(uint32_t*)malloc(width*height*sizeof(uint32_t));
    for(int32_t y=0;y<height;y++)
    {
        int32_t offset=y*width;
        QRgb *scanLine=(QRgb*)image->scanLine(y); // Do not free!
        for(int32_t x=0;x<width;x++)
        {
            QRgb color=scanLine[x];
            uint32_t alpha=qAlpha(color);
            uint32_t red=qRed(color);
            uint32_t green=qGreen(color);
            uint32_t blue=qBlue(color);
            out[offset+x]=(alpha<<24)|(red<<16)|(green<<8)|blue;
        }
        // Do not free "scanLine"!
    }
    return out;
}

void MainWindow::keyDownHandler(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Delete)
    {
        QGraphicsItem *item=ui->graphicsView->selectedItem;
        if(item==0)
            return;

        QGraphicsSimpleTextItem *textItem;

        int itemType=item->type();
        if(itemType==9 /*QGraphicsSimpleTextItem::type()*/)
        {
            textItem=(QGraphicsSimpleTextItem*)item;
            item=(QGraphicsItem*)item->data(Qt::UserRole).toULongLong();
        }
        else if(itemType==2 /*QGraphicsPathItem::type()*/ || itemType==3 /*QGraphicsRectItem::type()*/)
        {
            textItem=(QGraphicsSimpleTextItem*)item->data(Qt::UserRole).toULongLong();
        }
        else
            return;

        //selectedItem->tags->removeOne((std::pair<char*,QRectF>*)rectItem->data(Qt::UserRole+1).toULongLong()); // Do not use removeAt, as the indexes may shift.

        scene->removeItem(textItem);
        scene->removeItem(item);

        delete textItem;
        delete item;

        ui->graphicsView->selectedItem=0;
        //saveCurrentTags();
    }
}

void MainWindow::toggleImageBtnClicked()
{
    if(imageVisible)
    {
        imageVisible=false;
        pixmapItem->setVisible(false);
    }
    else
    {
        imageVisible=true;
        pixmapItem->setVisible(true);
    }
}

bool MainWindow::isRectangularSelection()
{
    return ui->rectangularSelectionBtn->isChecked();
}

bool MainWindow::isPolygonalSelection()
{
    return ui->polygonalSelectionBtn->isChecked();
}

void MainWindow::itemGraphicsViewRectItemAdded(QGraphicsRectItem *rectItem)
{
    char *tag=strdup(rectItem->data(Qt::UserRole+2/*Yes, 2*/).toString().toStdString().c_str());
    std::pair<char*,QRectF> *p=new std::pair<char*,QRectF>(tag,rectItem->rect());
    rectItem->setData(Qt::UserRole+1,QVariant((qulonglong)p));
    generateNewTagColor();
    // Do not free "tag"
}

void MainWindow::itemGraphicsViewPathItemAdded(QGraphicsPathItem *pathItem)
{
    char *tag=strdup(pathItem->data(Qt::UserRole+2/*Yes, 2*/).toString().toStdString().c_str());
    std::pair<char*,QPainterPath> *p=new std::pair<char*,QPainterPath>(tag,pathItem->path());
    pathItem->setData(Qt::UserRole+1,QVariant((qulonglong)p));
    generateNewTagColor();
    // Do not free "tag"
}

void MainWindow::extractSelectedAreaBtnClicked()
{
    QGraphicsItem *item=ui->graphicsView->selectedItem;
    if(item==0)
        goto Error;

    QGraphicsSimpleTextItem *textItem;

    int itemType=item->type();
    if(itemType==9 /*QGraphicsSimpleTextItem::type()*/)
    {
        textItem=(QGraphicsSimpleTextItem*)item;
        item=(QGraphicsItem*)item->data(Qt::UserRole).toULongLong();
    }
    else if(itemType==2 /*QGraphicsPathItem::type()*/)
    {
        textItem=(QGraphicsSimpleTextItem*)item->data(Qt::UserRole).toULongLong();
        std::pair<char*,QPainterPath> *p=(std::pair<char*,QPainterPath>*)item->data(Qt::UserRole+1).toULongLong();
        pathToSave=p->second;
    }
    else if(itemType==3 /*QGraphicsRectItem::type()*/)
    {
        textItem=(QGraphicsSimpleTextItem*)item->data(Qt::UserRole).toULongLong();
        qDebug()<<"Address: "<<item->data(Qt::UserRole+1).toULongLong();
        std::pair<char*,QRectF> *p=(std::pair<char*,QRectF>*)item->data(Qt::UserRole+1).toULongLong();
        rectToSave=p->second;
    }
    else
    {
        goto Error;
    }
    goto NoError;
    Error:
    QMessageBox::critical(this,"Error","No area selected! Please left-click on an area to select it.");
    return;
    NoError:

    savePath=item->type()==2 /*QGraphicsPathItem::type()*/;


    saveDialog->exec();
}

void MainWindow::generateNewTagColor()
{
    currentTagColor=QColor(rand()%255,rand()%255,rand()%255);
}

void MainWindow::saveDialogFileSelected(QString path)
{
    QFileInfo inf(path);
    bool png=inf.suffix().compare(QString("png"),Qt::CaseInsensitive)==0;

    QBrush filledPixelBrush=QBrush(0xffff0000);
    QBrush emptyPixelBrush=QBrush(png?0x00000000 /*Supports transparency*/:0xffffffff);


    /*uint32_t *data=(uint32_t*)malloc(bgImageWidth*bgImageHeight*sizeof(uint32_t));
    for(int y=0;y<bgImageHeight;y++)
    {
        size_t offset=y*bgImageWidth;
        for(int x=0;x<bgImageWidth;x++)
        {
            bool fill=(savePath?pathToSave.contains(QPointF((qreal)x,(qreal)y)):rectToSave.contains(QPointF((qreal)x,(qreal)y)));
            data[offset+x]=(fill?(imageVisible?bgImageData[offset+x]:filledPixelColor):emptyPixelColor);
        }
    }
    */
    QImage newImg=QImage(bgImageWidth,bgImageHeight,QImage::Format_ARGB32);
    QPainter painter;
    painter.begin(&newImg);
    if(isRectangularSelection())
    {
        painter.fillRect(0,0,bgImageWidth,bgImageHeight,emptyPixelBrush);
        if(imageVisible)
        {
            painter.setClipping(true);
            painter.setClipRect(rectToSave);
            painter.drawImage(0,0,*bgImage);
        }
        else
            painter.fillRect(rectToSave,filledPixelBrush);
    }
    else
    {
        painter.fillRect(0,0,bgImageWidth,bgImageHeight,emptyPixelBrush);
        if(imageVisible)
        {
            painter.setClipping(true);
            painter.setClipPath(pathToSave);
            painter.drawImage(0,0,*bgImage);
        }
        else
            painter.fillPath(pathToSave,filledPixelBrush);
    }
    painter.save();
    painter.end();
    const char *format=inf.suffix().toStdString().c_str();
    newImg.save(path,format,100);
    // free(data);
}
