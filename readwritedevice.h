#ifndef READDEVICE_H
#define READDEVICE_H

#include <QObject>
#include "varchannel.h"
#include <QPair>
#include "stplotpluginloader.h"
#include <QDialog>
#include <QSettings>

#define READ_WRITE_DEVICE_INTERFACE_HEADER_VERSION              0x00020000

#define MAX_PROCENT_FILE_DIALOG                                 100.0

class AbstractFileProgress : public QObject
{
    Q_OBJECT
public:
    AbstractFileProgress(QObject *parent = nullptr) : QObject{parent} {}
    //by defaoult is is initid becouse in some of
    //case I would like to use this class as empty progress
    virtual bool isCanceled() {return canceFlag;}

public slots:
    virtual void startProgress() {canceFlag = false;}
    virtual void stopProgress() {canceFlag = true;}
    virtual void setProgress(float procent) {}
    virtual void setMessadge(QString msg) {}

protected:
    bool canceFlag;
};

class ReadDeviceObject : public QObject
{
    Q_OBJECT
public:
    //addres and waribels with curent addres
    struct ReadChanale {
        VarChannel* chanale;
        uint32_t offset;
    };
    struct ReadAddres {
        QVector<struct ReadChanale> vectorChanales;//varivels with offset for this addres
        uint32_t addres;//begin addres read
        uint32_t readSize;//read addresses
    };

    /**
     * @brief isFileDevice - is read from file (if true soft will not save data in file)
     * @return
     */
    virtual bool isFileDevice() = 0;

    /**
     * @brief initDevise - init device (exec when push connect button) read var adresses and prepear read data
     * @param channels - list of varibels
     * @return - 0 if OK else return error code
     */
//    virtual int initDevise(QVector<VarChannel*> *channels) = 0;
    virtual int initDevise(QVector<struct ReadDeviceObject::ReadAddres> readSeuqence) = 0;

    /**
     * @brief stopDev - disconnect from device
     */
    virtual void stopDev() = 0;

    /**
     * @brief execReadDevice - do read from instance
     * @return - 0 if OK else return error code
     */
    virtual int execReadDevice() = 0;

    /**
     * @brief writeDataDevice - write raw data into memory
     * @param data - raw data values
     * @param location - addres located varibles
     * @return - 0 if OK else return error code
     */
    virtual int writeDataDevice(uint64_t data, varloc_location_t location) {return -1;}

    /**
     * @brief getReadDevConfigDialog - get configure widget
     * @return - configure widget for curent device
     */
    virtual QDialog* getReadDevConfigDialog() = 0;

    /**
     * @brief readFileDevice - function for read file device (directly read chanales)
     * @param chanales - list of chanales
     * @param fileProgress - point to progress bar object
     * @param readTimes - point to vector of times for math chanale
     * @return - -1 this fuction is note support; -2... error
     */
    virtual int readFileDevice(QVector<VarChannel *> chanales, AbstractFileProgress *fileProgress, QVector<QTime> *readTimes = nullptr) {return -1;}

    /**
     * @brief saveSettings - function save RW confir
     * @param settings - point to setting
     */
    virtual void saveSettings(QSettings *settings) {}

    /**
     * @brief saveSettings - function restore RW confir
     * @param settings - point to setting
     */
    virtual void restoreSettings(QSettings *settings) {}

signals:
    void addressesReed(uint32_t addres, QVector<uint8_t> data);
    void addressesReedWithTime(uint32_t addres, QVector<uint8_t> data, QDateTime time);
};

class SaveDeviceObject : public ReadDeviceObject
{
    Q_OBJECT
public:
    /**
     * @brief initDevise - init device (exec when push connect button) read var adresses and prepear save data
     * @param channels - list of varibels
     * @return - 0 if OK else return error code
     */
//    virtual int initDevise(QVector<VarChannel*> *channels) = 0;
    virtual int initDevise(QVector<struct ReadDeviceObject::ReadAddres> readSeuqence) = 0;

    /**
     * @brief stopDev - disconnect from device
     */
    virtual void stopDev() = 0;

    /**
     * @brief execSaveDevice - do save varibels should exec after read waribels and append just last value
     * @return - 0 if OK else return error code
     * NOTE remo it when test with decod list will be finished
     */
    virtual int execSaveDevice(QVector<QPair<uint32_t,QVector<uint8_t>>> saveSequence, QDateTime time) = 0;

    /**
     * @brief execSaveDevice - second metod save into file
     * @param chanaleNames - chanales name
     * @param listDecoded - vector values
     * @param time - save time
     * @return - 0 if OK else return error code
     */
    virtual int execSaveDevice(QList<QString> chanaleNames, QVector<float> listDecoded, QDateTime time) = 0;

    /**
     * @brief getSaveDevConfigDialog - get configure widget
     * @return - configure widget for curent device
     */
    virtual QDialog* getSaveDevConfigDialog() = 0;

    //some overwrite from read device
    bool isFileDevice() {return true;}
    int execReadDevice() {return -1;}
};


/************************************************************************/
/* BEGIN init plugins */
/************************************************************************/
/**
 * @brief The ReadDeviceInterfacePlugin class - plugin for read data from device
 */
class ReadDeviceInterfacePlugin
{
public:
    /**
     * @brief getVersion - get version builded library(SHOULDN'T be implemented)
     * @return - ersion builded library
     */
    virtual uint32_t getVersion() {return READ_WRITE_DEVICE_INTERFACE_HEADER_VERSION;}

    /**
     * @brief createDeviceObject - make new device object
     * @return - plot widget (alocate new QWidget)
     */
    virtual ReadDeviceObject* createReadDeviceObject() = 0;

    /**
     * @brief getName - get plugin name
     * @return - plugin name
     */
    virtual QString getName() { return QString(""); }

    /**
     * @brief getInfo - get plugin info
     * @return - plugin info
     */
    virtual QString getInfo() { return getName() + ":" + versionUnionToString(getVersion()); }
};

/**
 * @brief The ReadDeviceInterfacePlugin class - plugin for save data
 */
class SaveDeviceInterfacePlugin
{
public:
    /**
     * @brief getVersion - get version builded library(SHOULDN'T be implemented)
     * @return - ersion builded library
     */
    virtual uint32_t getVersion() {return READ_WRITE_DEVICE_INTERFACE_HEADER_VERSION;}

    /**
     * @brief createDeviceObject - make new device object
     * @return - plot widget (alocate new QWidget)
     */
    virtual SaveDeviceObject* createWriteDeviceObject() = 0;

    /**
     * @brief getName - get plugin name
     * @return - plugin name
     */
    virtual QString getName() { return QString(""); }

    /**
     * @brief getInfo - get plugin info
     * @return - plugin info
     */
    virtual QString getInfo() { return getName() + ":" + versionUnionToString(getVersion()); }
};

// registrate plugin
QT_BEGIN_NAMESPACE
#define ReadDeviceInterfacePlugin_iid "org.qt-project.S.debuger.ReadDeviceInterfacePlugin"
Q_DECLARE_INTERFACE(ReadDeviceInterfacePlugin, ReadDeviceInterfacePlugin_iid)
#define SaveDeviceInterfacePlugin_iid "org.qt-project.S.debuger.SaveDeviceInterfacePlugin"
Q_DECLARE_INTERFACE(SaveDeviceInterfacePlugin, SaveDeviceInterfacePlugin_iid)
QT_END_NAMESPACE


#endif // READDEVICE_H
