#include "UsbManager.h"

#include <QSettings>
#include <QStringList>

UsbManager::UsbManager(QObject *parent)
    : QObject(parent)
{
}

QStringList UsbManager::availablePorts()
{
    QStringList ports;

#ifdef Q_OS_WIN

    QSettings registry(
        "HKEY_LOCAL_MACHINE\\HARDWARE\\DEVICEMAP\\SERIALCOMM",
        QSettings::NativeFormat
    );

    const QStringList keys = registry.allKeys();

    for (const QString &key : keys)
    {
        const QString port =
            registry.value(key).toString();

        if (!port.isEmpty() &&
            !ports.contains(port))
        {
            ports.append(port);
        }
    }

#else

    // Existing macOS/Linux behaviour can remain here if wanted.

#endif

    ports.sort();

    return ports;
}