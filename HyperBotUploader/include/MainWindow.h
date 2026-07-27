#pragma once
#include "ProjectManager.h"
#include <QMainWindow>
#include "UsbManager.h"
#include "UploadButton.h"

// Widgets
class QLabel;
class QLineEdit;
class QPushButton;
class QPlainTextEdit;
class QComboBox;
class QCheckBox;
class QRadioButton;
class QTimer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void createUi();

    // Top Bar
    QLabel* robotStatus;
    QLineEdit* projectPath;
    QPushButton* browseButton;
    

    // Driver Control
    QRadioButton* mainRadio;
    QRadioButton* arcadeRadio;
    QRadioButton* atacRadio;

    // Robot Mode
    QRadioButton* matchRadio;
    QRadioButton* skillsRadio;

    // Autonomous
    QComboBox* autonCombo;

    // Features
    QCheckBox* skillsPrepCheck;
    QCheckBox* postAutonCheck;
    QCheckBox* gpsCheck;
    QCheckBox* visionCheck;

    // Terminal
    QPlainTextEdit* terminal;

    // Bottom Buttons
    QPushButton* cleanButton;
    QPushButton* buildButton;
    UploadButton *uploadButton;
    QPushButton* buildUploadButton;

    ProjectManager* projectManager;

    QPushButton *visualizerButton;
    void loadAutonomousFunctions(const QString& filename);
    void refreshPorts();
    QComboBox* portCombo;
    QPushButton* refreshUsbButton;
    UsbManager* usbManager;

    QTimer *uploadProgressTimer;
    int uploadProgress = 0;
};