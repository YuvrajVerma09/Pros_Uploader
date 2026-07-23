#include "UsbManager.h"
#include <QSerialPortInfo>
#include <QDir>

UsbManager::UsbManager(QObject *parent)
    : QObject(parent)
{
}

QStringList UsbManager::availablePorts()
{
    QStringList ports;

    const auto serialPorts = QSerialPortInfo::availablePorts();

    for (const QSerialPortInfo &port : serialPorts)
    {
        QString info =
            port.portName() + " - " +
            port.description() + " - " +
            port.manufacturer();

        ports << info;
    }

    return ports;
}