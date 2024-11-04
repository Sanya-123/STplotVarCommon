// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef PLOT_WIDGET_INTERFACE_H
#define PLOT_WIDGET_INTERFACE_H

#include <QObject>
#include <QString>
#include <QWidget>
#include <QSettings>
#include <QMap>
#include <QHash>
#include <QtPlugin>
#include <chrono>
#include "varchannel.h"
#include "settingsabstract.h"
#include "stplotpluginloader.h"

#define PLOT_INTERFACE_HEADER_VERSION           0x00000000

/**
 * @brief The PlotWidgetAbstract class - abstrac widget clas for plot data
 */
class PlotWidgetAbstract : public QWidget
{
    Q_OBJECT
public:
    PlotWidgetAbstract(SettingsAbstract* settings = nullptr, QWidget *parent = nullptr) :QWidget(parent) {}
    PlotWidgetAbstract(QWidget *parent = nullptr) :QWidget(parent) {}
    /**
     * @brief addPlot - redraw plot graphics
     */
    virtual void redraw() = 0;
    /**
     * @brief addPlot - add new plot(graph)
     * @param plotName - plot name
     */
    virtual void addPlot(VarChannel *varChanale) = 0;

    /**
     * @brief deletePlot - delete plot
     * @param plotName - plot name
     */
    virtual void deletePlot(VarChannel *varChanale) = 0;

    /**
     * @brief gedSettings - get curent gruph settings
     * @return - point to settings
     */
    virtual SettingsAbstract* gedSettings() = 0;

public slots:
    /**
     * @brief setViewProps - update view with new properties
     * @param props - properties to update
     */
    virtual void setViewProps(QHash<QString, QVariant> props) {}
signals:
    /**
     * @brief viewPropsUpdated - view properties updated
     * @param props - changed properties
     */
    void viewPropsUpdated(QHash<QString, QVariant> props);
};

/**
 * @brief The PlotWidgetInterfacePlugin class - external plugin with widgets
 */
class PlotWidgetInterfacePlugin
{
public:

    /**
     * @brief getVersion - get version builded library(SHOULDN'T be implemented)
     * @return - ersion builded library
     */
    virtual uint32_t getVersion() {return PLOT_INTERFACE_HEADER_VERSION;}
    /**
     * @brief createWidgetPlot - make new plot widget
     * @return - plot widget (alocate new QWidget)
     */
    virtual PlotWidgetAbstract* createWidgetPlot(QWidget *parent = nullptr) = 0;

    /**
     * @brief gedDefauoldSettings - get defouoult settings for gruph
     * @return - reyrn point to defoult settings
     */
    virtual SettingsAbstract* gedDefauoldSettings() = 0;

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
#define PlotWidgetInterfacePlugin_iid "org.qt-project.S.debuger.PlotWidgetInterfacePlugin"
Q_DECLARE_INTERFACE(PlotWidgetInterfacePlugin, PlotWidgetInterfacePlugin_iid)
QT_END_NAMESPACE


#endif //PLOT_WIDGET_INTERFACE_H
