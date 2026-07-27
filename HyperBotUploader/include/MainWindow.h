#pragma once

#include <QMainWindow>
#include <QString>

// Qt classes
class QLabel;
class QLineEdit;
class QPushButton;
class QPlainTextEdit;
class QComboBox;
class QCheckBox;
class QRadioButton;

// Your classes
class ProjectManager;
class UsbManager;
class ProsUploader;
class UploadButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void createUi();
    void loadAutonomousFunctions(const QString &filename);
    void refreshPorts();

    bool prepareProsOperation();
    QString selectedPort() const;

    //--------------------------------------------------
    // Top bar
    //--------------------------------------------------

    QLabel *robotStatus = nullptr;
    QLineEdit *projectPath = nullptr;
    QPushButton *browseButton = nullptr;

    QComboBox *portCombo = nullptr;
    QPushButton *refreshUsbButton = nullptr;

    //--------------------------------------------------
    // Driver controls
    //--------------------------------------------------

    QRadioButton *mainRadio = nullptr;
    QRadioButton *arcadeRadio = nullptr;
    QRadioButton *atacRadio = nullptr;

    //--------------------------------------------------
    // Robot mode
    //--------------------------------------------------

    QRadioButton *matchRadio = nullptr;
    QRadioButton *skillsRadio = nullptr;

    //--------------------------------------------------
    // Autonomous
    //--------------------------------------------------

    QComboBox *autonCombo = nullptr;

    //--------------------------------------------------
    // Features
    //--------------------------------------------------

    QCheckBox *skillsPrepCheck = nullptr;
    QCheckBox *postAutonCheck = nullptr;
    QCheckBox *gpsCheck = nullptr;
    QCheckBox *visionCheck = nullptr;

    //--------------------------------------------------
    // Terminal and buttons
    //--------------------------------------------------

    QPlainTextEdit *terminal = nullptr;

    QPushButton *cleanButton = nullptr;
    QPushButton *buildButton = nullptr;
    UploadButton *uploadButton = nullptr;
    QPushButton *buildUploadButton = nullptr;
    QPushButton *visualizerButton = nullptr;

    //--------------------------------------------------
    // Backend objects
    //--------------------------------------------------

    ProjectManager *projectManager = nullptr;
    UsbManager *usbManager = nullptr;

    // This is the declaration that is currently missing.
    ProsUploader *prosUploader = nullptr;
};