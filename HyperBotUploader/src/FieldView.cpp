#include "FieldView.h"
#include "FieldScene.h"

FieldView::FieldView(QWidget *parent)
    : QGraphicsView(parent)
{
    scene = new FieldScene(this);

    setScene(scene);

    setRenderHint(QPainter::Antialiasing);

    setDragMode(QGraphicsView::ScrollHandDrag);

    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    scale(5,5);
}