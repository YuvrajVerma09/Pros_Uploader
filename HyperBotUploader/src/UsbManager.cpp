#include "UsbManager.h"

#include <QStringList>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

UsbManager::UsbManager(QObject *parent)
    : QObject(parent)
{
}

QStringList UsbManager::availablePorts()
{
    QStringList ports;

#ifdef Q_OS_WIN

    /*
     * Check COM1 -> COM256.
     *
     * QueryDosDevice tells us whether Windows currently
     * has that COM device registered.
     */
    wchar_t targetPath[4096];

    for (int i = 1; i <= 256; ++i)
    {
        const QString portName =
            QString("COM%1").arg(i);

        const std::wstring widePort =
            portName.toStdWString();

        const DWORD result =
            QueryDosDeviceW(
                widePort.c_str(),
                targetPath,
                4096
            );

        if (result != 0)
        {
            ports.append(portName);
        }
    }

#else

    // macOS/Linux fallback if you still want cross-platform builds.
    QDir dev("/dev");

    QStringList filters;
    filters
        << "cu.usb*"
        << "tty.usb*"
        << "ttyACM*"
        << "ttyUSB*";

    const QStringList entries =
        dev.entryList(filters, QDir::System);

    for (const QString &entry : entries)
    {
        ports.append("/dev/" + entry);
    }

#endif

    return ports;
}