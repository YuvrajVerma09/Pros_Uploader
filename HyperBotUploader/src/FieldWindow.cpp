#include "FieldWindow.h"

#include <QPainter>
#include <QDebug>

FieldWindow::FieldWindow(QWidget *parent)
    : QMainWindow(parent)
{
    resize(1000, 1000);

    setWindowTitle("Autonomous Visualizer");

    fieldImage.load("programming/HyperBotUploader/resources/overrideField.png");

    qDebug() << "Null:" << fieldImage.isNull();
    qDebug() << "Size:" << fieldImage.size();

    if(fieldImage.isNull())
        qDebug() << "Failed to load field image!";
    else
        qDebug() << "Field image loaded successfully.";
}

void FieldWindow::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    painter.fillRect(rect(), QColor(40,40,40));

    if(!fieldImage.isNull())
    {
        QRect target(20,20,width()-40,height()-40);

        painter.drawPixmap(target, fieldImage);
    }

    //------------------------------------------
    // Robot
    //------------------------------------------

    painter.setBrush(Qt::red);
    painter.setPen(QPen(Qt::white,3));

    QRect robot(width()/2-25,height()/2-25,50,50);

    painter.drawRect(robot);

    painter.setPen(QPen(Qt::black,5));

    painter.drawLine(robot.center(),
                     QPoint(robot.center().x(),
                            robot.top()));
}