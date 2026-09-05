#include "DependencyManager.h"

#include <QDir>
#include <QStandardPaths>

DependencyManager::DependencyManager(QObject *parent)
    : QObject(parent)
{
}


QString DependencyManager::appDataRoot() const
{
    return QStandardPaths::writableLocation(
        QStandardPaths::AppLocalDataLocation
    );
}


QString DependencyManager::toolsRoot() const
{
    return QDir(
        appDataRoot()
    ).filePath("tools");
}


QString DependencyManager::prosRoot() const
{
    return QDir(
        toolsRoot()
    ).filePath("pros");
}


QString DependencyManager::prosExecutable() const
{
    return QDir(
        prosRoot()
    ).filePath("pros");
}


QString DependencyManager::toolchainRoot() const
{
    return QDir(
        toolsRoot()
    ).filePath("toolchain");
}


QString DependencyManager::toolchainBin() const
{
    return QDir(
        toolchainRoot()
    ).filePath("bin");
}


QString DependencyManager::pythonRoot() const
{
    return QDir(
        toolsRoot()
    ).filePath("python");
}


QString DependencyManager::tempRoot() const
{
    return QDir(
        toolsRoot()
    ).filePath("tmp");
}