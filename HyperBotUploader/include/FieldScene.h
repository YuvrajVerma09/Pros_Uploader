#pragma once

#include <QGraphicsScene>

class RobotItem;

class FieldScene : public QGraphicsScene
{
    Q_OBJECT

public:

    explicit FieldScene(QObject *parent=nullptr);

private:

    void drawGrid();

    void drawWalls();

    RobotItem *robot;
};