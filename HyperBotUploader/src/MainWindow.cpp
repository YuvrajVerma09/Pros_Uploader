#include "MainWindow.h"
#include "ProjectManager.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSplitter>
#include <QVBoxLayout>
#include <QWidget>
#include "FieldWindow.h"
#include <QFile>
#include <QRegularExpression>
#include <QTextStream>
#include "UsbManager.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Create backend
    projectManager = new ProjectManager(this);
    usbManager = new UsbManager(this);

    
    

    // Build the interface
    createUi();
    portCombo->addItems(usbManager->availablePorts());
    connect(visualizerButton,
        &QPushButton::clicked,
        this,
        [this]()
    {
        FieldWindow *window = new FieldWindow();

        window->show();
    });
    connect(refreshUsbButton,
        &QPushButton::clicked,
        this,
        [this]()
    {
        portCombo->clear();
        portCombo->addItems(usbManager->availablePorts());
    });
    // Browse button
    connect(browseButton,
        &QPushButton::clicked,
        this,
        [this]()
    {
        QString file =
            QFileDialog::getOpenFileName(
                this,
                "Select Robot Source File",
                "",
                "Source Files (*.cpp *.h *.hpp)");

        if(file.isEmpty())
            return;

        projectPath->setText(file);
        loadAutonomousFunctions(file);

        robotStatus->setText("🟢 Source File Loaded");

        terminal->appendPlainText("Loaded:");
        terminal->appendPlainText(file);
    });
    
    
}

void MainWindow::createUi()
{
    resize(1200, 800);
    setWindowTitle("Hyper Robot Manager");

    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    auto *mainLayout = new QVBoxLayout(central);

    //--------------------------------------------------
    // Top Bar
    //--------------------------------------------------

    auto *topLayout = new QHBoxLayout();

    topLayout->addWidget(new QLabel("Project"));

    projectPath = new QLineEdit();
    projectPath->setPlaceholderText("Select Robot Source...");
    topLayout->addWidget(projectPath);

    browseButton = new QPushButton("Browse");
    topLayout->addWidget(browseButton);

    // USB Label
    topLayout->addWidget(new QLabel("USB"));

    // USB Port Combo Box
    portCombo = new QComboBox();
    topLayout->addWidget(portCombo);

    // Refresh Button
    refreshUsbButton = new QPushButton("Refresh");
    topLayout->addWidget(refreshUsbButton);

    topLayout->addStretch();

    robotStatus = new QLabel("🔴 No Robot");
    topLayout->addWidget(robotStatus);

    mainLayout->addLayout(topLayout);

    //--------------------------------------------------
    // Splitter
    //--------------------------------------------------

    auto *splitter = new QSplitter(Qt::Horizontal);

    mainLayout->addWidget(splitter);

    //--------------------------------------------------
    // LEFT PANEL
    //--------------------------------------------------

    QWidget *leftWidget = new QWidget();
    auto *leftLayout = new QVBoxLayout(leftWidget);

    //--------------------------------------------------
    // Driver Control
    //--------------------------------------------------

    auto *driverGroup = new QGroupBox("Driver Control");
    auto *driverLayout = new QVBoxLayout(driverGroup);

    mainRadio = new QRadioButton("Main");
    arcadeRadio = new QRadioButton("Arcade");
    atacRadio = new QRadioButton("ATAC");

    driverLayout->addWidget(mainRadio);
    driverLayout->addWidget(arcadeRadio);
    driverLayout->addWidget(atacRadio);

    leftLayout->addWidget(driverGroup);

    //--------------------------------------------------
    // Robot Mode
    //--------------------------------------------------

    auto *modeGroup = new QGroupBox("Robot Mode");
    auto *modeLayout = new QVBoxLayout(modeGroup);

    matchRadio = new QRadioButton("Match");
    skillsRadio = new QRadioButton("Skills");

    modeLayout->addWidget(matchRadio);
    modeLayout->addWidget(skillsRadio);

    leftLayout->addWidget(modeGroup);

    //--------------------------------------------------
    // Autonomous
    //--------------------------------------------------

    auto *autonGroup = new QGroupBox("Autonomous");

    auto *autonLayout = new QVBoxLayout(autonGroup);

    autonCombo = new QComboBox();

    autonCombo->addItem("None");

    autonLayout->addWidget(autonCombo);

    leftLayout->addWidget(autonGroup);

    //--------------------------------------------------
    // Features
    //--------------------------------------------------

    auto *featureGroup = new QGroupBox("Features");
    auto *featureLayout = new QVBoxLayout(featureGroup);

    skillsPrepCheck = new QCheckBox("Skills Prep");
    postAutonCheck = new QCheckBox("Post Auton");
    gpsCheck = new QCheckBox("GPS");
    visionCheck = new QCheckBox("Vision");

    featureLayout->addWidget(skillsPrepCheck);
    featureLayout->addWidget(postAutonCheck);
    featureLayout->addWidget(gpsCheck);
    featureLayout->addWidget(visionCheck);

    leftLayout->addWidget(featureGroup);

    leftLayout->addStretch();

    splitter->addWidget(leftWidget);

    //--------------------------------------------------
    // RIGHT PANEL
    //--------------------------------------------------

    QWidget *rightWidget = new QWidget();
    auto *rightLayout = new QVBoxLayout(rightWidget);

    terminal = new QPlainTextEdit();
    terminal->setReadOnly(true);
    terminal->appendPlainText("Hyper Robot Manager");
    terminal->appendPlainText("----------------------------");
    terminal->appendPlainText("Ready.");

    rightLayout->addWidget(terminal);

    auto *buttonLayout = new QHBoxLayout();

    cleanButton = new QPushButton("Clean");
    buildButton = new QPushButton("Build");
    uploadButton = new QPushButton("Upload");
    buildUploadButton = new QPushButton("Build && Upload");
    visualizerButton = new QPushButton("Auton Visualizer");

    buttonLayout->addWidget(cleanButton);
    buttonLayout->addWidget(buildButton);
    buttonLayout->addWidget(uploadButton);
    buttonLayout->addWidget(buildUploadButton);
    buttonLayout->addWidget(visualizerButton);

    rightLayout->addLayout(buttonLayout);

    splitter->addWidget(rightWidget);

    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 3);
}
void MainWindow::loadAutonomousFunctions(const QString &filename)
{
    autonCombo->clear();

    QFile file(filename);

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);

    QString text = in.readAll();

    file.close();

    // Find every "void functionName("
    QRegularExpression regex(
        R"(void\s+([A-Za-z_][A-Za-z0-9_]*)\s*\()");

    auto matches = regex.globalMatch(text);

    while(matches.hasNext())
    {
        auto match = matches.next();

    
        QString name = match.captured(1);
        QString lowerName = name.toLower();

        if (name.contains("left") ||
            name.contains("right") ||
            name.contains("auton") ||
            name.contains("sector") ||
            name.contains("test") ||
            name.contains("skills") ||
            name.contains("default"))
        {
            autonCombo->addItem(name);
        }
    }
    if (autonCombo->count() == 0)
    {
        autonCombo->addItem("No autonomous routines found");
    }
}