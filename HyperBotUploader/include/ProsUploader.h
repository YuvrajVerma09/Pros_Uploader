#pragma once

#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>

class ProsUploader : public QObject
{
    Q_OBJECT

public:
    explicit ProsUploader(QObject *parent = nullptr);

    bool setSourceFile(
        const QString &sourceFile,
        QString *errorMessage = nullptr
    );

    void setPort(const QString &port);

    QString sourceFile() const;
    QString projectRoot() const;

    bool isBusy() const;

public slots:
    void clean();
    void build();
    void upload();
    void buildAndUpload();
    void cancel();

signals:
    void outputReady(const QString &text);
    void progressChanged(int percentage);
    void stageChanged(const QString &stage);
    void busyChanged(bool busy);

    void operationFinished(
        const QString &operation,
        bool success
    );

private:
    enum class Operation
    {
        None,
        Clean,
        Build,
        Upload
    };

    QString findProjectRoot(const QString &sourceFile) const;
    QString findProsExecutable() const;
    QString normalisePort(const QString &port) const;
    QString operationName(Operation operation) const;

    void startProcess(
        Operation operation,
        const QStringList &arguments
    );

    void startUploadProcess();
    void readProcessOutput();
    void processUploadProgress(const QString &text);

    void processFinished(
        int exitCode,
        QProcess::ExitStatus exitStatus
    );

    void processError(QProcess::ProcessError error);

    QProcess m_process;

    QString m_sourceFile;
    QString m_projectRoot;
    QString m_port;
    QString m_prosExecutable;

    QString m_progressBuffer;

    Operation m_operation = Operation::None;

    bool m_uploadAfterBuild = false;
};