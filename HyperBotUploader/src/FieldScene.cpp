#include "FieldScene.h"
#include "RobotItem.h"

#include <QGraphicsPixmapItem>
#include <QPixmap>

FieldScene::FieldScene(QObject *parent)
    : QGraphicsScene(parent)
{
    QPixmap field(":/overrideField.png");

    auto *background = addPixmap(field);

    setSceneRect(background->boundingRect());

    robot = new RobotItem();

    addItem(robot);

    constexpr double FIELD_SIZE = 144.0;

    double scale = sceneRect().width() / FIELD_SIZE;

    double px = 24 * scale;
    double py = sceneRect().height() - 24 * scale;

    robot->setPose(px, py, 90);
}
void FieldScene::drawGrid()
{
    QPen pen(Qt::lightGray);

    for(int i=0;i<=144;i+=12)
    {
        addLine(i,0,i,144,pen);

        addLine(0,i,144,i,pen);
    }
}
void FieldScene::drawWalls()
{
    QPen pen(Qt::black);

    pen.setWidthF(0.5);

    addRect(0,0,144,144,pen);
}