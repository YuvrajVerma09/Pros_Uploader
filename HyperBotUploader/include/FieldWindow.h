#pragma once

#include <QMainWindow>
#include <QPixmap>

class FieldWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit FieldWindow(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QPixmap fieldImage;
};