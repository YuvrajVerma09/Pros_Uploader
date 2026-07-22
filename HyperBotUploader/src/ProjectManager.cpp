#include "ProjectManager.h"

#include <QDir>
#include <QFileInfo>

ProjectManager::ProjectManager(QObject *parent)
    : QObject(parent)
{
}

bool ProjectManager::openProject(const QString &folder)
{
    QDir dir(folder);

    QFileInfo prosFile(dir.filePath("project.pros"));

    if (!prosFile.exists())
        return false;

    m_projectPath = folder;

    emit projectLoaded(folder);

    return true;
}

bool ProjectManager::isProjectLoaded() const
{
    return !m_projectPath.isEmpty();
}

QString ProjectManager::projectPath() const
{
    return m_projectPath;
}

QString ProjectManager::projectName() const
{
    return QFileInfo(m_projectPath).fileName();
}