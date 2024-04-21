#ifndef VAR_READER_INTERFACE_H
#define VAR_READER_INTERFACE_H

#include <QObject>
#include "varcommon.h"
#include <QIODevice>
#include "stplotpluginloader.h"

#define VAR_READER_INTERFACE_HEADER_VERSION              0x00000000

/************************************************************************/
/* BEGIN init plugins */
/************************************************************************/
/**
 * @brief The VarReadInterfacePlugin class - plugin for read and write tree varivle
 */
class VarReadInterfacePlugin
{
public:
    /**
     * @brief getVersion - get version builded library(SHOULDN'T be implemented)
     * @return - ersion builded library
     */
    virtual uint32_t getVersion() {return VAR_READER_INTERFACE_HEADER_VERSION;}

    /**
     * @brief getName - get plugin name
     * @return - plugin name
     */
    virtual QString getName() { return QString(""); }

    /**
     * @brief getFileExtensions - get file format that support this plugin
     * @return - file extensions ex(*.elf)
     */
    virtual QString getFileExtensions() = 0;

    /**
     * @brief allowMode - allow mode device
     * @return
     */
    virtual QIODevice::OpenModeFlag allowMode() {return QIODevice::NotOpen;}

    /**
     * @brief readFile - read tree from file
     * @param fileName - name of file
     * @return - tree varibles; nullptr if coudn't ope file
     */
    virtual varloc_node_t* readTree(QString fileName) = 0;

    /**
     * @brief saveTree - save tree intofile
     * @param tree - tree varibles
     * @param fileName - name of file
     * @return - negative error; positive OK
     */
    virtual int saveTree(varloc_node_t* tree, QString fileName) = 0;

    /**
     * @brief getInfo - get plugin info
     * @return - plugin info
     */
    virtual QString getInfo() { return getName() + ":" + versionUnionToString(getVersion()); }
};


// registrate plugin
QT_BEGIN_NAMESPACE
#define VarReadInterfacePlugin_iid "org.qt-project.S.debuger.VarReadInterfacePlugin_iid"
Q_DECLARE_INTERFACE(VarReadInterfacePlugin, VarReadInterfacePlugin_iid)
QT_END_NAMESPACE

/************************************************************************/
/* END init plugins */
/************************************************************************/


#endif // VAR_READER_INTERFACE_H
