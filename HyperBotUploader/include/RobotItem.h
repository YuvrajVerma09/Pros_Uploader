#pragma once

#include <QGraphicsItem>

class RobotItem : public QGraphicsItem
{
public:
    RobotItem();

    QRectF boundingRect() const override;

    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *,
               QWidget *) override;

    void setPose(double x,
                 double y,
                 double heading);

private:
    double m_x;
    double m_y;
    double m_heading;
};