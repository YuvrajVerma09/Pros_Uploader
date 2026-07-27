#include "ProsUploader.h"

#include <QDir>
#include <QFileInfo>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTimer>

ProsUploader::ProsUploader(QObject *parent)
    : QObject(parent)
{
    m_process.setProcessChannelMode(QProcess::MergedChannels);

    connect(
        &m_process,
        &QProcess::readyRead,
        this,
        &ProsUploader::readProcessOutput
    );

    connect(
        &m_process,
        &QProcess::finished,
        this,
        &ProsUploader::processFinished
    );

    connect(
        &m_process,
        &QProcess::errorOccurred,
        this,
        &ProsUploader::processError
    );
}

bool ProsUploader::setSourceFile(
    const QString &sourceFile,
    QString *errorMessage
)
{
    QFileInfo sourceInfo(sourceFile);

    if (!sourceInfo.exists() || !sourceInfo.isFile())
    {
        if (errorMessage)
        {
            *errorMessage =
                "The selected source file does not exist.";
        }

        return false;
    }

    m_sourceFile = sourceInfo.absoluteFilePath();
    m_projectRoot = findProjectRoot(m_sourceFile);

    if (m_projectRoot.isEmpty())
    {
        if (errorMessage)
        {
            *errorMessage =
                "No project.pros file was found in this file's "
                "folder or any of its parent folders.\n\n"
                "The C++ file must be inside a PROS project so "
                "the PROS compiler can build it.";
        }

        return false;
    }

    return true;
}

void ProsUploader::setPort(const QString &port)
{
    m_port = normalisePort(port);
}

QString ProsUploader::sourceFile() const
{
    return m_sourceFile;
}

QString ProsUploader::projectRoot() const
{
    return m_projectRoot;
}

bool ProsUploader::isBusy() const
{
    return m_process.state() != QProcess::NotRunning;
}

QString ProsUploader::findProjectRoot(
    const QString &sourceFile
) const
{
    QFileInfo sourceInfo(sourceFile);
    QDir directory = sourceInfo.absoluteDir();

    while (true)
    {
        const QString projectFile =
            directory.filePath("project.pros");

        if (QFileInfo::exists(projectFile))
        {
            return directory.absolutePath();
        }

        const QString previousPath =
            directory.absolutePath();

        if (!directory.cdUp())
        {
            break;
        }

        if (directory.absolutePath() == previousPath)
        {
            break;
        }
    }

    return {};
}

QString ProsUploader::findProsExecutable() const
{
    QProcessEnvironment environment =
        QProcessEnvironment::systemEnvironment();

    QStringList searchPaths =
        environment.value("PATH").split(
            QDir::listSeparator(),
            Qt::SkipEmptyParts
        );

#ifdef Q_OS_MACOS
    searchPaths.prepend("/usr/local/bin");
    searchPaths.prepend("/opt/homebrew/bin");
#endif

    searchPaths.prepend(
        QDir::home().filePath(".local/bin")
    );

    QString executable =
        QStandardPaths::findExecutable(
            "pros",
            searchPaths
        );

#ifdef Q_OS_WIN
    if (executable.isEmpty())
    {
        executable =
            QStandardPaths::findExecutable(
                "pros.exe",
                searchPaths
            );
    }
#endif

    return executable;
}

QString ProsUploader::normalisePort(
    const QString &port
) const
{
    QString result = port.trimmed();

    if (result.isEmpty() ||
        result.startsWith("Automatic", Qt::CaseInsensitive) ||
        result.startsWith("No ", Qt::CaseInsensitive))
    {
        return {};
    }

    // Remove a friendly description if the combo box contains:
    // "/dev/cu.usbmodem123 - VEX Robotics"
    const qsizetype separator =
        result.indexOf(" - ");

    if (separator > 0)
    {
        result = result.left(separator);
    }

#if defined(Q_OS_MACOS) || defined(Q_OS_LINUX)
    if (!result.startsWith("/dev/") &&
        (result.startsWith("cu.") ||
         result.startsWith("tty.") ||
         result.startsWith("ttyACM") ||
         result.startsWith("ttyUSB")))
    {
        result.prepend("/dev/");
    }
#endif

    return result;
}

QString ProsUploader::operationName(
    Operation operation
) const
{
    switch (operation)
    {
    case Operation::Clean:
        return "Clean";

    case Operation::Build:
        return "Build";

    case Operation::Upload:
        return "Upload";

    case Operation::None:
    default:
        return "Operation";
    }
}

void ProsUploader::clean()
{
    if (isBusy())
    {
        return;
    }

    m_uploadAfterBuild = false;

    startProcess(
        Operation::Clean,
        {"make", "clean"}
    );
}

void ProsUploader::build()
{
    if (isBusy())
    {
        return;
    }

    m_uploadAfterBuild = false;

    startProcess(
        Operation::Build,
        {"make"}
    );
}

void ProsUploader::upload()
{
    if (isBusy())
    {
        return;
    }

    m_uploadAfterBuild = false;
    startUploadProcess();
}

void ProsUploader::buildAndUpload()
{
    if (isBusy())
    {
        return;
    }

    m_uploadAfterBuild = true;

    startProcess(
        Operation::Build,
        {"make"}
    );
}

void ProsUploader::startUploadProcess()
{
    QStringList arguments{"upload"};

    /*
     * The PROS upload syntax is:
     *
     * pros upload [PATH] [PORT]
     *
     * If a port is specified, "." is passed as PATH so that
     * the following argument is interpreted as PORT.
     */
    if (!m_port.isEmpty())
    {
        arguments << "." << m_port;
    }

    startProcess(
        Operation::Upload,
        arguments
    );
}

void ProsUploader::startProcess(
    Operation operation,
    const QStringList &arguments
)
{
    if (isBusy())
    {
        emit outputReady(
            "\nAnother PROS operation is already running.\n"
        );

        return;
    }

    if (m_projectRoot.isEmpty())
    {
        emit outputReady(
            "\nNo PROS project has been located.\n"
        );

        emit operationFinished(
            operationName(operation),
            false
        );

        return;
    }

    m_prosExecutable = findProsExecutable();

    if (m_prosExecutable.isEmpty())
    {
        emit outputReady(
            "\nCould not find the PROS CLI executable.\n"
            "Confirm that `pros --version` works in Terminal.\n"
        );

        emit operationFinished(
            operationName(operation),
            false
        );

        return;
    }

    m_operation = operation;
    m_progressBuffer.clear();

    QProcessEnvironment environment =
        QProcessEnvironment::systemEnvironment();

#ifdef Q_OS_MACOS
    QString path = environment.value("PATH");

    const QString additionalPaths =
        "/opt/homebrew/bin:/usr/local/bin:" +
        QDir::home().filePath(".local/bin");

    if (!path.isEmpty())
    {
        path = additionalPaths + ":" + path;
    }
    else
    {
        path = additionalPaths;
    }

    environment.insert("PATH", path);
#endif

    m_process.setProcessEnvironment(environment);
    m_process.setWorkingDirectory(m_projectRoot);
    m_process.setProgram(m_prosExecutable);
    m_process.setArguments(arguments);

    QString commandText =
        m_prosExecutable + " " + arguments.join(' ');

    emit outputReady(
        "\n\n$ " + commandText + "\n"
    );

    emit stageChanged(operationName(operation));
    emit busyChanged(true);

    if (operation == Operation::Upload)
    {
        emit progressChanged(0);
    }

    m_process.start();
}

void ProsUploader::readProcessOutput()
{
    const QByteArray outputBytes = m_process.readAll();

    if (outputBytes.isEmpty())
    {
        return;
    }

    QString output =
        QString::fromLocal8Bit(outputBytes);

    // Remove ANSI terminal colour/control sequences.
    static const QRegularExpression ansiExpression(
        QStringLiteral(
            "\x1B\\[[0-?]*[ -/]*[@-~]"
        )
    );

    output.remove(ansiExpression);

    emit outputReady(output);

    if (m_operation == Operation::Upload)
    {
        processUploadProgress(output);
    }
}

void ProsUploader::processUploadProgress(
    const QString &text
)
{
    QString normalised = text;
    normalised.replace('\r', '\n');

    m_progressBuffer += normalised;

    /*
     * PROS prints progress values such as 34% while
     * compressing and uploading files. Use the most recent
     * percentage visible in the output.
     */
    static const QRegularExpression percentageExpression(
        R"((\d{1,3})\s*%)"
    );

    QRegularExpressionMatchIterator matches =
        percentageExpression.globalMatch(
            m_progressBuffer
        );

    int latestPercentage = -1;

    while (matches.hasNext())
    {
        const QRegularExpressionMatch match =
            matches.next();

        bool conversionWorked = false;

        const int percentage =
            match.captured(1).toInt(
                &conversionWorked
            );

        if (conversionWorked &&
            percentage >= 0 &&
            percentage <= 100)
        {
            latestPercentage = percentage;
        }
    }

    if (latestPercentage >= 0)
    {
        emit progressChanged(latestPercentage);
    }

    // Prevent the buffer from growing indefinitely.
    if (m_progressBuffer.size() > 8192)
    {
        m_progressBuffer =
            m_progressBuffer.right(4096);
    }
}

void ProsUploader::processFinished(
    int exitCode,
    QProcess::ExitStatus exitStatus
)
{
    const Operation completedOperation = m_operation;

    const bool success =
        exitStatus == QProcess::NormalExit &&
        exitCode == 0;

    emit outputReady(
        success
            ? "\nOperation completed successfully.\n"
            : QString(
                  "\nOperation failed with exit code %1.\n"
              ).arg(exitCode)
    );

    m_operation = Operation::None;

    if (completedOperation == Operation::Upload &&
        success)
    {
        emit progressChanged(100);
    }

    if (completedOperation == Operation::Build &&
        success &&
        m_uploadAfterBuild)
    {
        m_uploadAfterBuild = false;

        emit operationFinished("Build", true);

        // Wait until QProcess has fully entered NotRunning.
        QTimer::singleShot(
            0,
            this,
            [this]()
            {
                startUploadProcess();
            }
        );

        return;
    }

    m_uploadAfterBuild = false;

    emit busyChanged(false);

    emit operationFinished(
        operationName(completedOperation),
        success
    );
}

void ProsUploader::processError(
    QProcess::ProcessError error
)
{
    if (error != QProcess::FailedToStart)
    {
        return;
    }

    const QString operation =
        operationName(m_operation);

    emit outputReady(
        "\nPROS failed to start.\n"
        "Executable: " + m_prosExecutable + "\n"
        "Error: " + m_process.errorString() + "\n"
    );

    m_operation = Operation::None;
    m_uploadAfterBuild = false;

    emit busyChanged(false);
    emit operationFinished(operation, false);
}

void ProsUploader::cancel()
{
    if (!isBusy())
    {
        return;
    }

    emit outputReady("\nCancelling operation...\n");

    m_uploadAfterBuild = false;
    m_process.kill();
}