#include "UploadButton.h"

#include <QPainter>
#include <QPainterPath>
#include <QRandomGenerator>

UploadButton::UploadButton(QWidget *parent)
    : QPushButton(parent)
{
    setMinimumHeight(50);

    glowTimer.setInterval(20);

    connect(&glowTimer,
            &QTimer::timeout,
            this,
            &UploadButton::animateGlow);
}
void UploadButton::setProgress(int value)
{
    progress = qBound(0, value, 100);

    if(progress == 0 || progress == 100)
        glowTimer.stop();
    else
        glowTimer.start();

    update();
}
void UploadButton::animateGlow()
{
    if(glowGrowing)
        glowStrength += 0.03;
    else
        glowStrength -= 0.03;

    if(glowStrength > 1.25)
        glowGrowing = false;

    if(glowStrength < 0.85)
        glowGrowing = true;

    update();
}
void UploadButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    p.setRenderHint(QPainter::Antialiasing);
        QRect r = rect();

    p.setPen(Qt::NoPen);
    p.setBrush(QColor(35,35,35));

    p.drawRoundedRect(r.adjusted(1,1,-1,-1),8,8);
        QColor glow(0,220,255);

    glow.setAlpha(80 * glowStrength);

    p.setPen(QPen(glow,6));

    p.drawRoundedRect(r.adjusted(3,3,-3,-3),8,8);
        QPainterPath bolt;

    bolt.moveTo(0.55,0.05);
    bolt.lineTo(0.35,0.45);
    bolt.lineTo(0.52,0.45);
    bolt.lineTo(0.40,0.95);
    bolt.lineTo(0.72,0.40);
    bolt.lineTo(0.53,0.40);
    bolt.closeSubpath();
        QRectF area(20,8,28,34);

    QTransform t;

    t.translate(area.left(),area.top());

    t.scale(area.width(),area.height());

    bolt = t.map(bolt);
        p.setPen(QPen(Qt::white,2));

    p.drawPath(bolt);
    QRectF bounds = bolt.boundingRect();
    double fillY =
        bounds.bottom()
        - bounds.height() * progress / 100.0;
    QRectF clip(
    bounds.left(),
    fillY,
    bounds.width(),
    bounds.bottom() - fillY
    );

    p.save();

    p.setClipRect(clip);

    p.fillPath(
        bolt,
        QColor(0,220,255)
    );

    p.restore();
    p.setPen(Qt::white);

    QFont f = font();

    f.setBold(true);

    p.setFont(f);

    QString text;

    if(progress==0)
        text="Upload";

    else if(progress==100)
        text="Done";

    else
        text=QString("Uploading %1%")
                .arg(progress);

    p.drawText(rect(),
            Qt::AlignCenter,
            text);
    if(progress > 0 && progress < 100)
    {
        p.setPen(QPen(QColor(120,220,255), 2));

        for(int i = 0; i < 3; i++)
        {
            int x = QRandomGenerator::global()->bounded(width());
            int y = QRandomGenerator::global()->bounded(height());

            p.drawLine(x, y, x + 4, y - 4);
            p.drawLine(x + 4, y - 4, x + 8, y + 2);
        }
    }
}