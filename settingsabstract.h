#ifndef SETTINGSABSTRACT_G
#define SETTINGSABSTRACT_G
#include <QObject>
#include <QString>
#include <QSettings>
#include <QMap>
/**
 * @brief The SettingsAbstract class - abstract setting class,
 */
class SettingsAbstract : public QObject
{
    Q_OBJECT
public:
    //int constructor I should fill mapSettings
    SettingsAbstract (QObject *parent = nullptr) : QObject(parent){mapSettings = mapSettingsDefauold;}
    QMap<QString, QVariant> getSettingsMap() {return mapSettings;}

public slots:
    /**
     * @brief setSettings - metod copy form setting to mapSettings
     * @param settings
     */
    virtual void setSettings(SettingsAbstract *settings) {setSettings(settings->getSettingsMap());}
    virtual void setSettings(QMap<QString, QVariant> nemMap)
    {
        QList<QString> names = nemMap.keys();
        foreach (QString name, names) {
            setValues(name, nemMap[name]);
        }
    }
    /**
     * @brief setValues - set value for setings
     * @param name - name setting
     * @param val - value setting
     */
    virtual void setValues(QString name, QVariant val)
    {
        if(updateValue(name, val))
        {
            emit settingsUpdated();
            emit settingsUpdated(name, val);
        }
    }

    /**
     * @brief updateValue - set values with out emit update settings
     * @param name - name setting
     * @param val - value setting
     * @return - true is map contain name uand type is corrent
     */
    virtual bool updateValue(QString name, QVariant val)
    {
        if(mapSettings.contains(name))
        {
            if(mapSettings[name].type() == val.type())
            {
                mapSettings[name] = val;
                return true;
            }
        }
        return false;
    }

    virtual void restoreDefoultSetings() {mapSettings = mapSettingsDefauold; emit settingsUpdated();}

signals:
    void settingsUpdated();
    void settingsUpdated(QString name, QVariant val);

protected:
    //map settings mname and values
    QMap<QString, QVariant> mapSettingsDefauold;
    //    friend class PlotWidgetAbstract;
    //    friend class PlotWidgetInterfacePlugin;
    QMap<QString, QVariant> mapSettings;
};

#endif // SETTINGSABSTRACT_G
