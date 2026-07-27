#pragma once

#include <QPushButton>
#include <QTimer>

class UploadButton : public QPushButton
{
    Q_OBJECT

public:
    explicit UploadButton(QWidget *parent = nullptr);

    void setProgress(int value);

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void animateGlow();

private:
    int progress = 0;

    double glowStrength = 1.0;
    bool glowGrowing = true;

    QTimer glowTimer;
};