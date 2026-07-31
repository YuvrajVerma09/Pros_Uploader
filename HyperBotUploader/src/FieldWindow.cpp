#include "FieldWindow.h"

#include <QFile>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QRegularExpression>
#include <QTextStream>
#include <QtMath>
#include <QMouseEvent>
#include <QEvent>

namespace
{
double normaliseHeading(double heading)
{
    while (heading > 180.0)
    {
        heading -= 360.0;
    }

    while (heading <= -180.0)
    {
        heading += 360.0;
    }

    return heading;
}
}


FieldWindow::FieldWindow(
    const QString &sourceFile,
    const QString &selectedRoutine,
    QWidget *parent
    
)
    : QMainWindow(parent),
      sourceFilePath(sourceFile),
      routineName(selectedRoutine)
{
    resize(1050, 800);

    setWindowTitle(
        "Autonomous Visualizer - " +
        routineName
    );
    setMouseTracking(true);
    /*
     * Try both possible Qt resource paths.
     */
    if (!fieldImage.load(":/overrideField.png"))
    {
        fieldImage.load(
            ":/resources/overrideField.png"
        );
    }

    /*
     * Estimated starting location.
     *
     * Left routines begin on the left.
     * Right routines begin on the right.
     * Other routines begin in the centre.
     */
    const QString lowerRoutine =
        routineName.toLower();

    if (lowerRoutine.contains("left"))
    {
        startX = 36.0;
    }
    else if (lowerRoutine.contains("right"))
    {
        startX = 108.0;
    }
    else
    {
        startX = 72.0;
    }

    startY = 12.0;
    startHeading = 0.0;

    createAutonomousPath();
}


QString FieldWindow::extractFunctionBody(
    QString sourceText
) const
{
    //--------------------------------------------------
    // Remove block comments
    //--------------------------------------------------

    const QRegularExpression blockComments(
        R"(/\*.*?\*/)",
        QRegularExpression::DotMatchesEverythingOption
    );

    sourceText.remove(blockComments);

    //--------------------------------------------------
    // Remove single-line comments
    //--------------------------------------------------

    const QRegularExpression lineComments(
        R"(//[^\n\r]*)"
    );

    sourceText.remove(lineComments);

    //--------------------------------------------------
    // Find the selected function
    //--------------------------------------------------

    const QString escapedName =
        QRegularExpression::escape(routineName);

    const QRegularExpression functionExpression(
        QString(
            R"(\bvoid\s+%1\s*\([^)]*\)\s*(?:override\s*)?\{)"
        ).arg(escapedName)
    );

    const QRegularExpressionMatch match =
        functionExpression.match(sourceText);

    if (!match.hasMatch())
    {
        return {};
    }

    /*
     * capturedEnd() points just after the opening brace.
     */
    const qsizetype openingBrace =
        match.capturedEnd() - 1;

    int braceDepth = 0;

    for (qsizetype index = openingBrace;
         index < sourceText.size();
         ++index)
    {
        const QChar character =
            sourceText.at(index);

        if (character == '{')
        {
            ++braceDepth;
        }
        else if (character == '}')
        {
            --braceDepth;

            if (braceDepth == 0)
            {
                return sourceText.mid(
                    openingBrace + 1,
                    index - openingBrace - 1
                );
            }
        }
    }

    return {};
}


void FieldWindow::createAutonomousPath()
{
    poses.clear();
    movementCommandCount = 0;
    errorMessage.clear();

    //--------------------------------------------------
    // Open robot source
    //--------------------------------------------------

    QFile sourceFile(sourceFilePath);

    if (!sourceFile.open(
            QIODevice::ReadOnly |
            QIODevice::Text
        ))
    {
        errorMessage =
            "Could not open the selected source file.";

        update();
        return;
    }

    QTextStream stream(&sourceFile);

    const QString sourceText =
        stream.readAll();

    sourceFile.close();

    //--------------------------------------------------
    // Extract selected routine
    //--------------------------------------------------

    const QString functionBody =
        extractFunctionBody(sourceText);

    if (functionBody.isEmpty())
    {
        errorMessage =
            "Could not find the selected autonomous "
            "function.";

        update();
        return;
    }

    //--------------------------------------------------
    // Starting pose
    //--------------------------------------------------

    double robotX = startX;
    double robotY = startY;
    double robotHeading = startHeading;

    poses.append({
        QPointF(robotX, robotY),
        robotHeading
    });

    //--------------------------------------------------
    // Find movement commands in source-code order
    //--------------------------------------------------

    const QRegularExpression movementExpression(
        R"((?:this\s*->\s*)?cm\s*->\s*drive\s*\.\s*pid\s*\.\s*(lateral|turn|uTurn)\s*\(\s*([-+]?(?:\d+(?:\.\d*)?|\.\d+))?)"
    );

    QRegularExpressionMatchIterator matches =
        movementExpression.globalMatch(
            functionBody
        );

    while (matches.hasNext())
    {
        const QRegularExpressionMatch match =
            matches.next();

        const QString command =
            match.captured(1);

        //--------------------------------------------------
        // U-turn
        //--------------------------------------------------

        if (command == "uTurn")
        {
            robotHeading =
                normaliseHeading(
                    robotHeading + 180.0
                );

            poses.append({
                QPointF(robotX, robotY),
                robotHeading
            });

            ++movementCommandCount;
            continue;
        }

        //--------------------------------------------------
        // Read numeric first argument
        //--------------------------------------------------

        bool conversionSucceeded = false;

        const double value =
            match.captured(2).toDouble(
                &conversionSucceeded
            );

        if (!conversionSucceeded)
        {
            continue;
        }

        //--------------------------------------------------
        // Relative turn
        //--------------------------------------------------

        if (command == "turn")
        {
            /*
             * Positive angles rotate clockwise.
             * Negative angles rotate anticlockwise.
             */
            robotHeading =
                normaliseHeading(
                    robotHeading + value
                );

            poses.append({
                QPointF(robotX, robotY),
                robotHeading
            });

            ++movementCommandCount;
        }

        //--------------------------------------------------
        // Forward or reverse movement
        //--------------------------------------------------

        else if (command == "lateral")
        {
            /*
             * Zero degrees points toward the top of the
             * visualizer.
             */
            const double radians =
                qDegreesToRadians(robotHeading);

            robotX += value * qSin(radians);
            robotY += value * qCos(radians);

            poses.append({
                QPointF(robotX, robotY),
                robotHeading
            });

            ++movementCommandCount;
        }
    }

    if (movementCommandCount == 0)
    {
        errorMessage =
            "No supported movement commands were found "
            "inside this routine.";
    }

    update();
}

void FieldWindow::mouseMoveEvent(
    QMouseEvent *event
)
{
    const QPointF mousePosition =
        event->position();

    mouseScreenPosition = mousePosition;

    if (currentFieldRectangle.isEmpty() ||
        !currentFieldRectangle.contains(mousePosition))
    {
        mouseIsOnField = false;
        update();

        QMainWindow::mouseMoveEvent(event);
        return;
    }

    //--------------------------------------------------
    // Convert screen pixels to field inches
    //--------------------------------------------------

    constexpr double fieldSizeInches = 144.0;

    const double relativeX =
        (mousePosition.x() -
         currentFieldRectangle.left()) /
        currentFieldRectangle.width();

    const double relativeY =
        (currentFieldRectangle.bottom() -
         mousePosition.y()) /
        currentFieldRectangle.height();

    double fieldX =
        relativeX * fieldSizeInches;

    double fieldY =
        relativeY * fieldSizeInches;

    /*
     * Keep the result inside the official field limits.
     */
    fieldX = qBound(
        0.0,
        fieldX,
        fieldSizeInches
    );

    fieldY = qBound(
        0.0,
        fieldY,
        fieldSizeInches
    );

    mouseFieldPosition =
        QPointF(fieldX, fieldY);

    mouseIsOnField = true;

    /*
     * Repaint to update the crosshair and label.
     */
    update();

    QMainWindow::mouseMoveEvent(event);
}
void FieldWindow::leaveEvent(
    QEvent *event
)
{
    mouseIsOnField = false;
    update();

    QMainWindow::leaveEvent(event);
}
void FieldWindow::paintEvent(
    QPaintEvent *event
)
{
    Q_UNUSED(event);

    QPainter painter(this);

    painter.setRenderHint(
        QPainter::Antialiasing
    );

    painter.setRenderHint(
        QPainter::SmoothPixmapTransform
    );

    painter.fillRect(
        rect(),
        QColor(30, 30, 34)
    );

    //--------------------------------------------------
    // Draw field image
    //--------------------------------------------------

    QRectF imageRectangle;

    if (!fieldImage.isNull())
    {
        QSize imageSize = fieldImage.size();

        imageSize.scale(
            qMax(1, width() - 40),
            qMax(1, height() - 90),
            Qt::KeepAspectRatio
        );

        imageRectangle = QRectF(
            QPointF(0, 0),
            QSizeF(imageSize)
        );

        imageRectangle.moveCenter(
            QPointF(
                width() / 2.0,
                height() / 2.0 + 20.0
            )
        );

        painter.drawPixmap(
            imageRectangle.toRect(),
            fieldImage
        );
    }
    else
    {
        /*
         * The path can still be displayed when the field
         * PNG has not loaded.
         */
        const double side =
            qMin(
                static_cast<double>(width() - 40),
                static_cast<double>(height() - 90)
            );

        imageRectangle = QRectF(
            (width() - side) / 2.0,
            (height() - side) / 2.0 + 20.0,
            side,
            side
        );

        painter.fillRect(
            imageRectangle,
            QColor(65, 65, 70)
        );
    }

    //--------------------------------------------------
    // Playable 12 ft × 12 ft field rectangle
    //--------------------------------------------------

    const double fieldSide =
        qMin(
            imageRectangle.width(),
            imageRectangle.height()
        );

    const QRectF fieldRectangle(
        imageRectangle.center().x() -
            fieldSide / 2.0,

        imageRectangle.center().y() -
            fieldSide / 2.0,

        fieldSide,
        fieldSide
    );
    currentFieldRectangle = fieldRectangle;
    painter.setPen(
        QPen(
            QColor(255, 255, 255, 150),
            2
        )
    );

    painter.setBrush(Qt::NoBrush);
    painter.drawRect(fieldRectangle);

    //--------------------------------------------------
    // Map inches to pixels
    //--------------------------------------------------

    const auto mapToScreen =
        [&fieldRectangle](const QPointF &point)
    {
        constexpr double fieldSize = 144.0;

        const double screenX =
            fieldRectangle.left() +
            point.x() / fieldSize *
                fieldRectangle.width();

        const double screenY =
            fieldRectangle.bottom() -
            point.y() / fieldSize *
                fieldRectangle.height();

        return QPointF(screenX, screenY);
    };

    //--------------------------------------------------
    // Information bar
    //--------------------------------------------------

    const QRectF informationRectangle(
        15,
        10,
        width() - 30,
        40
    );

    painter.setPen(Qt::NoPen);
    painter.setBrush(
        QColor(15, 15, 18, 220)
    );

    painter.drawRoundedRect(
        informationRectangle,
        8,
        8
    );

    painter.setPen(Qt::white);

    painter.drawText(
        informationRectangle.adjusted(
            12,
            0,
            -12,
            0
        ),
        Qt::AlignVCenter |
            Qt::AlignLeft,
        QString(
            "%1 | %2 movement commands | "
            "Start: (%3, %4) in, %5°"
        )
            .arg(routineName)
            .arg(movementCommandCount)
            .arg(startX, 0, 'f', 0)
            .arg(startY, 0, 'f', 0)
            .arg(startHeading, 0, 'f', 0)
    );

    //--------------------------------------------------
    // Error
    //--------------------------------------------------

    if (!errorMessage.isEmpty())
    {
        painter.setPen(Qt::white);

        painter.setBrush(
            QColor(130, 35, 35, 225)
        );

        const QRectF errorRectangle(
            fieldRectangle.left() + 20,
            fieldRectangle.center().y() - 35,
            fieldRectangle.width() - 40,
            70
        );

        painter.drawRoundedRect(
            errorRectangle,
            10,
            10
        );

        painter.drawText(
            errorRectangle.adjusted(
                15,
                10,
                -15,
                -10
            ),
            Qt::AlignCenter |
                Qt::TextWordWrap,
            errorMessage
        );

        return;
    }

    if (poses.isEmpty())
    {
        return;
    }

    //--------------------------------------------------
    // Create path line
    //--------------------------------------------------

    QPainterPath path;

    path.moveTo(
        mapToScreen(
            poses.first().position
        )
    );

    QPointF previousPosition =
        poses.first().position;

    for (qsizetype index = 1;
         index < poses.size();
         ++index)
    {
        const QPointF position =
            poses.at(index).position;

        /*
         * A turn changes heading but not position, so it
         * does not add another line segment.
         */
        if (position != previousPosition)
        {
            path.lineTo(
                mapToScreen(position)
            );

            previousPosition = position;
        }
    }

    //--------------------------------------------------
    // Clip the path to the field
    //--------------------------------------------------

    painter.save();
    painter.setClipRect(fieldRectangle);

    //--------------------------------------------------
    // Path glow
    //--------------------------------------------------

    painter.setPen(
        QPen(
            QColor(0, 210, 255, 65),
            12,
            Qt::SolidLine,
            Qt::RoundCap,
            Qt::RoundJoin
        )
    );

    painter.drawPath(path);

    //--------------------------------------------------
    // Main path line
    //--------------------------------------------------

    painter.setPen(
        QPen(
            QColor(70, 225, 255),
            4,
            Qt::SolidLine,
            Qt::RoundCap,
            Qt::RoundJoin
        )
    );

    painter.drawPath(path);

    //--------------------------------------------------
    // Movement and turn markers
    //--------------------------------------------------

    int movementNumber = 0;

    for (qsizetype index = 1;
         index < poses.size();
         ++index)
    {
        const Pose &previous =
            poses.at(index - 1);

        const Pose &current =
            poses.at(index);

        const QPointF screenPoint =
            mapToScreen(current.position);

        if (current.position != previous.position)
        {
            ++movementNumber;

            painter.setPen(
                QPen(Qt::white, 1)
            );

            painter.setBrush(
                QColor(30, 30, 34, 225)
            );

            painter.drawEllipse(
                screenPoint,
                10,
                10
            );

            painter.drawText(
                QRectF(
                    screenPoint.x() - 10,
                    screenPoint.y() - 10,
                    20,
                    20
                ),
                Qt::AlignCenter,
                QString::number(movementNumber)
            );
        }
        else
        {
            /*
             * Orange circle marks a turn.
             */
            painter.setPen(
                QPen(
                    QColor(255, 185, 60),
                    3
                )
            );

            painter.setBrush(Qt::NoBrush);

            painter.drawEllipse(
                screenPoint,
                7,
                7
            );
        }
    }

    //--------------------------------------------------
    // Start marker
    //--------------------------------------------------

    const QPointF startPoint =
        mapToScreen(
            poses.first().position
        );

    painter.setPen(
        QPen(Qt::white, 2)
    );

    painter.setBrush(
        QColor(55, 210, 105)
    );

    painter.drawEllipse(
        startPoint,
        9,
        9
    );

    //--------------------------------------------------
    // Final robot position
    //--------------------------------------------------

    const Pose &finalPose =
        poses.last();

    const QPointF finalPoint =
        mapToScreen(
            finalPose.position
        );

    painter.save();

    painter.translate(finalPoint);

    /*
     * Qt screen rotation is clockwise, matching the
     * convention used above.
     */
    painter.rotate(finalPose.heading);

    painter.setPen(
        QPen(Qt::white, 2)
    );

    painter.setBrush(
        QColor(220, 65, 65, 220)
    );

    painter.drawRoundedRect(
        QRectF(
            -14,
            -14,
            28,
            28
        ),
        4,
        4
    );

    /*
     * Yellow line shows the robot's final heading.
     */
    painter.setPen(
        QPen(
            QColor(255, 230, 80),
            4,
            Qt::SolidLine,
            Qt::RoundCap
        )
    );

    painter.drawLine(
        QPointF(0, 0),
        QPointF(0, -20)
    );

    painter.restore();
    painter.restore();

    //--------------------------------------------------
    // Out-of-bounds warning
    //--------------------------------------------------

    bool leavesField = false;

    for (const Pose &pose : poses)
    {
        if (pose.position.x() < 0 ||
            pose.position.x() > 144 ||
            pose.position.y() < 0 ||
            pose.position.y() > 144)
        {
            leavesField = true;
            break;
        }
    }

    if (leavesField)
    {
        painter.setPen(Qt::white);

        painter.setBrush(
            QColor(155, 60, 25, 230)
        );

        const QRectF warningRectangle(
            15,
            height() - 42,
            width() - 30,
            28
        );

        painter.drawRoundedRect(
            warningRectangle,
            7,
            7
        );

        painter.drawText(
            warningRectangle,
            Qt::AlignCenter,
            "Warning: calculated path leaves the "
            "12 ft × 12 ft field."
        );
    }
    //--------------------------------------------------
// Mouse coordinate display
//--------------------------------------------------

if (mouseIsOnField &&
    currentFieldRectangle.contains(
        mouseScreenPosition
    ))
    {
        painter.save();

        //--------------------------------------------------
        // Crosshair
        //--------------------------------------------------

        painter.setClipRect(
            currentFieldRectangle
        );

        painter.setPen(
            QPen(
                QColor(255, 255, 255, 150),
                1,
                Qt::DashLine
            )
        );

        painter.drawLine(
            QPointF(
                currentFieldRectangle.left(),
                mouseScreenPosition.y()
            ),
            QPointF(
                currentFieldRectangle.right(),
                mouseScreenPosition.y()
            )
        );

        painter.drawLine(
            QPointF(
                mouseScreenPosition.x(),
                currentFieldRectangle.top()
            ),
            QPointF(
                mouseScreenPosition.x(),
                currentFieldRectangle.bottom()
            )
        );

        //--------------------------------------------------
        // Cursor marker
        //--------------------------------------------------

        painter.setPen(
            QPen(Qt::white, 2)
        );

        painter.setBrush(
            QColor(40, 190, 255, 210)
        );

        painter.drawEllipse(
            mouseScreenPosition,
            5,
            5
        );

        painter.restore();

        //--------------------------------------------------
        // Coordinate label
        //--------------------------------------------------

        const QString coordinateText =
            QString("X: %1 in   Y: %2 in")
                .arg(
                    mouseFieldPosition.x(),
                    0,
                    'f',
                    1
                )
                .arg(
                    mouseFieldPosition.y(),
                    0,
                    'f',
                    1
                );

        QFontMetrics fontMetrics(
            painter.font()
        );

        const QSize textSize =
            fontMetrics.size(
                Qt::TextSingleLine,
                coordinateText
            );

        QRectF labelRectangle(
            mouseScreenPosition.x() + 14,
            mouseScreenPosition.y() - 36,
            textSize.width() + 20,
            textSize.height() + 12
        );

        //--------------------------------------------------
        // Prevent the label leaving the window
        //--------------------------------------------------

        if (labelRectangle.right() >
            width() - 5)
        {
            labelRectangle.moveRight(
                mouseScreenPosition.x() - 14
            );
        }

        if (labelRectangle.top() < 5)
        {
            labelRectangle.moveTop(
                mouseScreenPosition.y() + 14
            );
        }

        if (labelRectangle.bottom() >
            height() - 5)
        {
            labelRectangle.moveBottom(
                height() - 5
            );
        }

        painter.setPen(
            QPen(
                QColor(255, 255, 255, 190),
                1
            )
        );

        painter.setBrush(
            QColor(15, 15, 18, 225)
        );

        painter.drawRoundedRect(
            labelRectangle,
            6,
            6
        );

        painter.setPen(Qt::white);

        painter.drawText(
            labelRectangle,
            Qt::AlignCenter,
            coordinateText
        );
    }
}