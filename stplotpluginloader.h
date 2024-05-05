#ifndef STPLOTPLUGINLOADER_H
#define STPLOTPLUGINLOADER_H

#include <QDir>
#include <QPluginLoader>
#include <QList>
#include <QDebug>

union __attribute__((packed)) VersionUnion {
    uint32_t version32;
    struct __attribute__((packed)){
        uint16_t build;
        uint8_t minor;
        uint8_t major;
    }versionS;
};

static QString versionUnionToString(VersionUnion version)
{
    return QString::asprintf("%d.%d.%d", version.versionS.major, version.versionS.minor, version.versionS.build);
}

static QString versionUnionToString(uint32_t version)
{
    VersionUnion _version;
    _version.version32 = version;
    return QString::asprintf("%d.%d.%d", _version.versionS.major, _version.versionS.minor, _version.versionS.build);
}

template <typename T>
QList<T*> loadPlugin(uint32_t minHeaderVersion, uint32_t maxHeaderVersion)
{
    QList<T*> plugins;
    T *_interface;
    QDir pluginsDir(QDir::currentPath());
#if defined(Q_OS_WIN)
    if (pluginsDir.dirName().toLower() == "debug" || pluginsDir.dirName().toLower() == "release")
        pluginsDir.cdUp();
#elif defined(Q_OS_MAC)
    if (pluginsDir.dirName() == "MacOS") {
        pluginsDir.cdUp();
        pluginsDir.cdUp();
        pluginsDir.cdUp();
    }
#endif

    if(!pluginsDir.cd("./plugins"))
        pluginsDir.cd("../plugins");

    qDebug() << "Plugin path:" << pluginsDir.absolutePath();
    qDebug() << "Plugin type:" << typeid(T).name();

    const QStringList entries = pluginsDir.entryList(QDir::Files);//get list plugins files

    for (const QString &fileName : entries) {
        //skip file none library
#if defined(Q_OS_WIN)
        if(QFileInfo(fileName).suffix() != "dll")
            continue;
#else
        if(QFileInfo(fileName).suffix() != "so")
            continue;
#endif
        qDebug() << "Try load plugin:" << fileName;
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));//load file as plugin
        QObject *plugin = pluginLoader.instance();//get object plugin
        if (plugin) {
            _interface = qobject_cast<T *>(plugin);//check type plugin
            if (_interface)//!0 if type correct
            {
                VersionUnion _version;
                _version.version32 = _interface->getVersion();
                qDebug() << "OK plugin:" << _interface->getName() <<
                            QString::asprintf("v%d.%d.%d", _version.versionS.major, _version.versionS.minor, _version.versionS.build) << _interface->getInfo();

                _version.versionS.build = 0;//do note abalize build version just major and minor

                //check version
                if(_version.version32 >= minHeaderVersion)
                {
                    if(_version.version32 <= maxHeaderVersion)//maximum avaleble version
                        plugins.append(_interface);
                    else
                    {
                        qDebug() << "App is too old for curent plugin please rebuild it";
                        pluginLoader.unload();
                    }
                }
                else
                {
                    qDebug() << "Plugin is too old please rebuild it";
                    pluginLoader.unload();
                }
            }
            else
            {
                qDebug() << "unload plugin:" << fileName;
                pluginLoader.unload();
            }
        }
        else{
            qDebug() << "Failed loading plugin";
        }
    }

    return plugins;
}

#endif // STPLOTPLUGINLOADER_H
