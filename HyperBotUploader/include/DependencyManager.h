#pragma once

#include <QObject>
#include <QString>

class DependencyManager : public QObject
{
    Q_OBJECT

public:
    explicit DependencyManager(QObject *parent = nullptr);

    QString appDataRoot() const;
    QString toolsRoot() const;

    QString prosRoot() const;
    QString prosExecutable() const;

    QString toolchainRoot() const;
    QString toolchainBin() const;

    QString pythonRoot() const;

    QString tempRoot() const;
};