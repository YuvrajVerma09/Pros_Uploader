#pragma once

#include <QGraphicsView>

class FieldScene;

class FieldView : public QGraphicsView
{
    Q_OBJECT

public:

    explicit FieldView(QWidget *parent = nullptr);

private:

    FieldScene *scene;
};