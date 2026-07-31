#include "MainWindow.h"

#include "FieldWindow.h"
#include "ProjectManager.h"
#include "ProsUploader.h"
#include "UploadButton.h"
#include "UsbManager.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QRegularExpression>
#include <QSplitter>
#include <QTextCursor>
#include <QTextStream>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Create backend objects once.
    projectManager = new ProjectManager(this);
    usbManager = new UsbManager(this);
    prosUploader = new ProsUploader(this);

    // Create all widgets.
    createUi();

    // Populate the USB port list.
    refreshPorts();

    //--------------------------------------------------
    // PROS terminal output
    //--------------------------------------------------

    connect(
        prosUploader,
        &ProsUploader::outputReady,
        this,
        [this](const QString &text)
        {
            terminal->moveCursor(QTextCursor::End);
            terminal->insertPlainText(text);
            terminal->moveCursor(QTextCursor::End);
        }
    );

    //--------------------------------------------------
    // Upload percentage animation
    //--------------------------------------------------

    connect(
        prosUploader,
        &ProsUploader::progressChanged,
        this,
        [this](int percentage)
        {
            uploadButton->setProgress(percentage);
        }
    );

    //--------------------------------------------------
    // Disable controls while PROS is running
    //--------------------------------------------------

    connect(
        prosUploader,
        &ProsUploader::busyChanged,
        this,
        [this](bool busy)
        {
            cleanButton->setEnabled(!busy);
            buildButton->setEnabled(!busy);
            uploadButton->setEnabled(!busy);
            buildUploadButton->setEnabled(!busy);

            browseButton->setEnabled(!busy);
            portCombo->setEnabled(!busy);
            refreshUsbButton->setEnabled(!busy);

            // Abort is only available while a process is running.
            abortButton->setEnabled(busy);
        }
    );

    //--------------------------------------------------
    // Operation finished
    //--------------------------------------------------

    connect(
        prosUploader,
        &ProsUploader::operationFinished,
        this,
        [this](const QString &operation, bool success)
        {
            if (operation == "Upload")
            {
                if (success)
                {
                    uploadButton->setProgress(100);
                    robotStatus->setText("🟢 Upload complete");

                    QTimer::singleShot(
                        1200,
                        this,
                        [this]()
                        {
                            if (!prosUploader->isBusy())
                            {
                                uploadButton->setProgress(0);
                            }
                        }
                    );
                }
                else
                {
                    uploadButton->setProgress(0);
                    robotStatus->setText("🔴 Upload failed");
                }
            }
            else if (operation == "Build")
            {
                robotStatus->setText(
                    success
                        ? "🟢 Build successful"
                        : "🔴 Build failed"
                );
            }
            else if (operation == "Clean")
            {
                robotStatus->setText(
                    success
                        ? "🟢 Clean successful"
                        : "🔴 Clean failed"
                );
            }
        }
    );
    connect(
        prosUploader,
        &ProsUploader::operationCancelled,
        this,
        [this](const QString &operation)
        {
            uploadButton->setProgress(0);

            robotStatus->setText(
                "🟠 " + operation + " aborted"
            );

            terminal->appendPlainText(
                operation + " was aborted."
            );
        }
    );
    //--------------------------------------------------
    // Clean button
    //--------------------------------------------------

    connect(
        cleanButton,
        &QPushButton::clicked,
        this,
        [this]()
        {
            if (!prepareProsOperation())
            {
                return;
            }

            prosUploader->clean();
        }
    );

    //--------------------------------------------------
    // Build button
    //--------------------------------------------------

    connect(
        buildButton,
        &QPushButton::clicked,
        this,
        [this]()
        {
            if (!prepareProsOperation())
            {
                return;
            }

            prosUploader->build();
        }
    );

    //--------------------------------------------------
    // Upload button
    //--------------------------------------------------

    connect(
        uploadButton,
        &QPushButton::clicked,
        this,
        [this]()
        {
            if (!prepareProsOperation())
            {
                return;
            }

            uploadButton->setProgress(0);
            robotStatus->setText("🟡 Uploading...");

            prosUploader->upload();
        }
    );

    //--------------------------------------------------
    // Build and upload button
    //--------------------------------------------------

    connect(
        buildUploadButton,
        &QPushButton::clicked,
        this,
        [this]()
        {
            if (!prepareProsOperation())
            {
                return;
            }

            uploadButton->setProgress(0);
            robotStatus->setText("🟡 Building...");

            prosUploader->buildAndUpload();
        }
    );
    //--------------------------------------------------
// Abort button
//--------------------------------------------------

connect(
    abortButton,
    &QPushButton::clicked,
    this,
    [this]()
    {
        if (!prosUploader->isBusy())
        {
            return;
        }

        abortButton->setEnabled(false);

        robotStatus->setText(
            "🟠 Aborting..."
        );

        terminal->appendPlainText(
            "\nStopping the current PROS operation..."
        );

        prosUploader->cancel();
    }
);
    //--------------------------------------------------
    // Refresh USB ports
    //--------------------------------------------------

    connect(
        refreshUsbButton,
        &QPushButton::clicked,
        this,
        [this]()
        {
            refreshPorts();
        }
    );

    //--------------------------------------------------
    // Browse for robot source file
    //--------------------------------------------------

    connect(
        browseButton,
        &QPushButton::clicked,
        this,
        [this]()
        {
            const QString file =
                QFileDialog::getOpenFileName(
                    this,
                    "Select Robot Source File",
                    QString(),
                    "Source Files (*.cpp *.cc *.cxx *.h *.hpp)"
                );

            if (file.isEmpty())
            {
                return;
            }

            projectPath->setText(file);
            loadAutonomousFunctions(file);

            terminal->appendPlainText("");
            terminal->appendPlainText("Loaded source file:");
            terminal->appendPlainText(file);

            QString errorMessage;

            if (prosUploader->setSourceFile(
                    file,
                    &errorMessage
                ))
            {
                robotStatus->setText(
                    "🟢 Source and PROS project loaded"
                );

                terminal->appendPlainText("");
                terminal->appendPlainText(
                    "PROS project root:"
                );

                terminal->appendPlainText(
                    prosUploader->projectRoot()
                );
            }
            else
            {
                robotStatus->setText(
                    "🟠 Source loaded; no PROS project"
                );

                terminal->appendPlainText("");
                terminal->appendPlainText(errorMessage);
            }
        }
    );

    //--------------------------------------------------
    // Autonomous visualiser
    //--------------------------------------------------

    connect(
        visualizerButton,
        &QPushButton::clicked,
        this,
        [this]()
        {
            const QString sourceFile =
                projectPath->text().trimmed();

            const QString selectedRoutine =
                autonCombo->currentText().trimmed();

            if (sourceFile.isEmpty())
            {
                QMessageBox::warning(
                    this,
                    "No Source File",
                    "Select the robot source file first."
                );

                return;
            }

            if (selectedRoutine.isEmpty() ||
                selectedRoutine == "None" ||
                selectedRoutine.startsWith("No ") ||
                selectedRoutine.startsWith("Could "))
            {
                QMessageBox::warning(
                    this,
                    "No Autonomous Selected",
                    "Select an autonomous routine first."
                );

                return;
            }

            auto *window = new FieldWindow(
                sourceFile,
                selectedRoutine
            );

            window->setAttribute(
                Qt::WA_DeleteOnClose
            );

            window->show();
            window->raise();
            window->activateWindow();
        }
    );
}


void MainWindow::createUi()
{
    resize(1200, 500);
    setWindowTitle("⚡️ Hyper Robot Manager ⚡️");

    auto *central = new QWidget(this);
    setCentralWidget(central);

    auto *mainLayout = new QVBoxLayout(central);

    //--------------------------------------------------
    // Top bar
    //--------------------------------------------------

    auto *topLayout = new QHBoxLayout();

    topLayout->addWidget(new QLabel("Project"));

    projectPath = new QLineEdit();
    projectPath->setPlaceholderText(
        "Select Robot Source..."
    );

    topLayout->addWidget(projectPath);

    browseButton = new QPushButton("Browse");
    topLayout->addWidget(browseButton);

    topLayout->addWidget(new QLabel("USB"));

    portCombo = new QComboBox();
    topLayout->addWidget(portCombo);

    refreshUsbButton = new QPushButton("Refresh");
    topLayout->addWidget(refreshUsbButton);

    topLayout->addStretch();

    robotStatus = new QLabel("🔴 No Robot");
    topLayout->addWidget(robotStatus);

    mainLayout->addLayout(topLayout);

    //--------------------------------------------------
    // Main splitter
    //--------------------------------------------------

    auto *splitter = new QSplitter(Qt::Horizontal);
    mainLayout->addWidget(splitter);

    //--------------------------------------------------
    // Left panel
    //--------------------------------------------------

    auto *leftWidget = new QWidget();
    auto *leftLayout = new QVBoxLayout(leftWidget);

    //--------------------------------------------------
    // Driver control
    //--------------------------------------------------

    auto *driverGroup =
        new QGroupBox("Driver Control");

    auto *driverLayout =
        new QVBoxLayout(driverGroup);

    mainRadio = new QRadioButton("Main");
    arcadeRadio = new QRadioButton("Arcade");
    atacRadio = new QRadioButton("ATAC");

    mainRadio->setChecked(true);

    driverLayout->addWidget(mainRadio);
    driverLayout->addWidget(arcadeRadio);
    driverLayout->addWidget(atacRadio);

    leftLayout->addWidget(driverGroup);

    //--------------------------------------------------
    // Robot mode
    //--------------------------------------------------

    auto *modeGroup =
        new QGroupBox("Robot Mode");

    auto *modeLayout =
        new QVBoxLayout(modeGroup);

    matchRadio = new QRadioButton("Match");
    skillsRadio = new QRadioButton("Skills");

    matchRadio->setChecked(true);

    modeLayout->addWidget(matchRadio);
    modeLayout->addWidget(skillsRadio);

    leftLayout->addWidget(modeGroup);

    //--------------------------------------------------
    // Autonomous
    //--------------------------------------------------

    auto *autonGroup =
        new QGroupBox("Autonomous");

    auto *autonLayout =
        new QVBoxLayout(autonGroup);

    autonCombo = new QComboBox();
    autonCombo->addItem("None");

    autonLayout->addWidget(autonCombo);
    leftLayout->addWidget(autonGroup);

    //--------------------------------------------------
    // Features
    //--------------------------------------------------

    auto *featureGroup =
        new QGroupBox("Features");

    auto *featureLayout =
        new QVBoxLayout(featureGroup);

    skillsPrepCheck =
        new QCheckBox("Skills Prep");

    postAutonCheck =
        new QCheckBox("Post Auton");

    gpsCheck =
        new QCheckBox("GPS");

    visionCheck =
        new QCheckBox("Vision");

    featureLayout->addWidget(skillsPrepCheck);
    featureLayout->addWidget(postAutonCheck);
    featureLayout->addWidget(gpsCheck);
    featureLayout->addWidget(visionCheck);

    leftLayout->addWidget(featureGroup);
    leftLayout->addStretch();

    splitter->addWidget(leftWidget);

    //--------------------------------------------------
    // Right panel
    //--------------------------------------------------

    auto *rightWidget = new QWidget();
    auto *rightLayout = new QVBoxLayout(rightWidget);

    terminal = new QPlainTextEdit();
    terminal->setReadOnly(true);

    terminal->appendPlainText(
        "Hyper Robot Manager"
    );

    terminal->appendPlainText(
        "----------------------------"
    );

    terminal->appendPlainText("Ready.");

    rightLayout->addWidget(terminal);

    //--------------------------------------------------
    // Bottom buttons
    //--------------------------------------------------

    auto *buttonLayout = new QHBoxLayout();

    cleanButton = new QPushButton("Clean");
    buildButton = new QPushButton("Build");

    uploadButton = new UploadButton();

    buildUploadButton =
        new QPushButton("Build && Upload");

    abortButton =
        new QPushButton("Abort");

    // It should only be enabled while PROS is running.
    abortButton->setEnabled(false);

    visualizerButton =
        new QPushButton("Auton Visualizer");

    buttonLayout->addWidget(cleanButton);
    buttonLayout->addWidget(buildButton);
    buttonLayout->addWidget(uploadButton);
    buttonLayout->addWidget(buildUploadButton);
    buttonLayout->addWidget(abortButton);
    buttonLayout->addWidget(visualizerButton);

    rightLayout->addLayout(buttonLayout);

    splitter->addWidget(rightWidget);

    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 3);
}


void MainWindow::loadAutonomousFunctions(
    const QString &filename
)
{
    autonCombo->clear();
    autonCombo->addItem("None");

    QFile file(filename);

    if (!file.open(
            QIODevice::ReadOnly |
            QIODevice::Text
        ))
    {
        autonCombo->addItem(
            "Could not read source file"
        );

        return;
    }

    QTextStream input(&file);
    const QString text = input.readAll();

    file.close();

    /*
     * Find functions written in this form:
     *
     * void defaultLeft(...)
     * void testRight90(...)
     */
    const QRegularExpression functionRegex(
        R"(\bvoid\s+([A-Za-z_][A-Za-z0-9_]*)\s*\()"
    );

    QRegularExpressionMatchIterator matches =
        functionRegex.globalMatch(text);

    bool foundAutonomous = false;

    while (matches.hasNext())
    {
        /*
         * 'match' must be created inside this loop.
         */
        const QRegularExpressionMatch match =
            matches.next();

        const QString name =
            match.captured(1);

        const QString lowerName =
            name.toLower();

        /*
         * Ignore wrapper functions and general robot
         * lifecycle functions.
         */
        if (lowerName == "auton" ||
            lowerName == "autonomous" ||
            lowerName == "skillsauton" ||
            lowerName == "run" ||
            lowerName == "skillsprep" ||
            lowerName == "postauton" ||
            lowerName == "initialize" ||
            lowerName == "disabled" ||
            lowerName == "competitioninitialize" ||
            lowerName == "opcontrol")
        {
            continue;
        }

        /*
         * Include function names that appear to represent
         * autonomous routines.
         */
        if (lowerName.contains("left") ||
            lowerName.contains("right") ||
            lowerName.contains("auton") ||
            lowerName.contains("sector") ||
            lowerName.contains("test") ||
            lowerName.contains("skills") ||
            lowerName.contains("default"))
        {
            /*
             * Prevent duplicate entries.
             */
            if (autonCombo->findText(name) == -1)
            {
                autonCombo->addItem(name);
                foundAutonomous = true;
            }
        }
    }

    if (!foundAutonomous)
    {
        autonCombo->addItem(
            "No autonomous routines found"
        );
    }
}


void MainWindow::refreshPorts()
{
    portCombo->clear();

    // Empty data means PROS should automatically detect
    // the connected communication port.
    portCombo->addItem(
        "Automatic detection",
        QString()
    );

    const QStringList ports =
        usbManager->availablePorts();

    for (QString port : ports)
    {
#if defined(Q_OS_MACOS) || defined(Q_OS_LINUX)
        if (!port.startsWith("/dev/"))
        {
            port.prepend("/dev/");
        }
#endif

        portCombo->addItem(
            port,
            port
        );
    }
}


bool MainWindow::prepareProsOperation()
{
    const QString sourceFile =
        projectPath->text().trimmed();

    if (sourceFile.isEmpty())
    {
        QMessageBox::warning(
            this,
            "No Robot Code Selected",
            "Select your robot C++ file first."
        );

        return false;
    }

    QString errorMessage;

    if (!prosUploader->setSourceFile(
            sourceFile,
            &errorMessage
        ))
    {
        QMessageBox::warning(
            this,
            "PROS Project Not Found",
            errorMessage
        );

        return false;
    }

    prosUploader->setPort(selectedPort());

    terminal->appendPlainText("");
    terminal->appendPlainText(
        "PROS project: " +
        prosUploader->projectRoot()
    );

    return true;
}


QString MainWindow::selectedPort() const
{
    QString port =
        portCombo->currentData().toString();

    if (port.isEmpty())
    {
        port = portCombo->currentText();
    }

    if (port.startsWith(
            "Automatic",
            Qt::CaseInsensitive
        ) ||
        port.startsWith(
            "No ",
            Qt::CaseInsensitive
        ))
    {
        return {};
    }

    return port;
}