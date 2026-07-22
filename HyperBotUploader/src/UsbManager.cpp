#include "UsbManager.h"

#include <QDir>

UsbManager::UsbManager(QObject *parent)
    : QObject(parent)
{
}

QStringList UsbManager::availablePorts()
{
    QDir dev("/dev");

    QStringList filters;
    filters << "cu.usb*" << "tty.usb*";

    return dev.entryList(filters, QDir::System);
}