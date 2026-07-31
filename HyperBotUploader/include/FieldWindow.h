#pragma once

#include <QMainWindow>
#include <QPixmap>
#include <QPointF>
#include <QString>
#include <QVector>


class QPaintEvent;
class QMouseEvent;
class QEvent;
class FieldWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit FieldWindow(
        const QString &sourceFile,
        const QString &routineName,
        QWidget *parent = nullptr
    );

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
private:
    struct Pose
    {
        QPointF position;
        double heading = 0.0;
    };

    QString extractFunctionBody(
        QString sourceText
    ) const;

    void createAutonomousPath();

    QPixmap fieldImage;

    QString sourceFilePath;
    QString routineName;
    QString errorMessage;

    QVector<Pose> poses;

    int movementCommandCount = 0;

    // Starting pose in field inches.
    double startX = 50.0;
    double startY = 12.0;
    double startHeading = 0.0;

    QRectF currentFieldRectangle;

    bool mouseIsOnField = false;
    QPointF mouseFieldPosition;
    QPointF mouseScreenPosition;
};