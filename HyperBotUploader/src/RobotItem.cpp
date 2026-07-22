
#include "RobotItem.h"

#include <QPainter>
#include <QGraphicsScene>
RobotItem::RobotItem()
{
}

QRectF RobotItem::boundingRect() const
{
    return QRectF(-9,-9,18,18);
}

void RobotItem::paint(QPainter *painter,
                      const QStyleOptionGraphicsItem *,
                      QWidget *)
{
    painter->setBrush(Qt::red);

    painter->drawRect(-9,-9,18,18);

    painter->setPen(QPen(Qt::black,1));

    painter->drawLine(0,0,9,0);
}

void RobotItem::setPose(double x, double y, double heading)
{
    setPos(x, y);
    setRotation(-heading);
}