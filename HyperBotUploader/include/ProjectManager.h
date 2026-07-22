#pragma once

#include <QObject>
#include <QString>

class ProjectManager : public QObject
{
    Q_OBJECT

public:
    explicit ProjectManager(QObject *parent = nullptr);

    bool openProject(const QString &folder);

    bool isProjectLoaded() const;
    QString projectPath() const;
    QString projectName() const;

signals:
    void projectLoaded(QString path);
    void projectClosed();

private:
    QString m_projectPath;
};