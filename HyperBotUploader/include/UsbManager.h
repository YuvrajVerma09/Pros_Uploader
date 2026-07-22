#pragma once

#include <QObject>
#include <QStringList>

class UsbManager : public QObject
{
    Q_OBJECT

public:
    explicit UsbManager(QObject *parent = nullptr);

    QStringList availablePorts();

signals:
    void portsChanged();
};