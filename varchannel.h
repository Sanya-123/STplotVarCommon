#ifndef VARCHANNEL_H
#define VARCHANNEL_H

#include <QObject>
#include <QColor>
#include <QString>
#include <vector>
#include <chrono>
#include <QTime>

#include "varcommon.h"
//#include "qcustomplot.h"

//using time_stamp = std::chrono::time_point<std::chrono::system_clock,
//                                           std::chrono::microseconds>;
#define MAX_NUMBER_DOT_STYLE                16
#define MAX_NUMBER_LINE_STYLE               6

#define MAX_DEFAOULT_DOT_STYLE              1
#define MAX_DEFAOULT_LINE_STYLE             1

#define REPLEASE_DOT_VAR_NAME(name)         name = name.replace(".", "_").replace("[", "___").replace("]", "___") /*TODO make better replease*/

typedef struct {
    double value;
    double rawValue;//value without mult and offset
    QTime qtime;
}VarValue;

class VarChannel : public QObject
{
    Q_OBJECT
public:
    VarChannel(varloc_node_t* node, QColor lineColor = Qt::green, int dotStyle = MAX_DEFAOULT_DOT_STYLE);
    VarChannel(varloc_location_t location, QString name, QColor lineColor = Qt::green, int dotStyle = MAX_DEFAOULT_DOT_STYLE);
    VarChannel(QString script, QString name, QColor lineColor = Qt::green, int dotStyle = MAX_DEFAOULT_DOT_STYLE);
    ~VarChannel();
    QString getName();
    float getValue();
    varloc_location_t getLocation();
    static float decode_value(uint64_t value, varloc_location_t location);
    static uint64_t code_value(float value, varloc_location_t location);
    bool hasLocation(varloc_location_t loc);

    static QString getFullNmaeNode(varloc_node_t* node);

    QVector<bool> getPlotList();
    int getTotalSizePlot();
    void setTotalSizePlot(int size);
    void removePlot(int numberPlot);
    bool isEnableOnPlot(int numberPlot);
    void setEnableOnPlot(int numberPlot, bool en);

    VarValue getValue(int numberElement);
    QVector<VarValue> getBuffer();
    int getBufferSize();
    int dotStyle() const;
    void setDotStyle(int newDotStyle);
    void setLocation(varloc_location_t loc);
    void clearLocation();
    int lineStyle() const;
    void setLineStyle(int newLineStyle);

    QColor lineColor() const;
    void setLineColor(const QColor &newLineColor);

    void setLineWidth(unsigned int newWidth);
    unsigned int lineWidth() const;

    QString displayName() const;
    void setDisplayName(const QString &newDisplayName);

    QString script() const;
    void setScript(const QString &newScript);

    double getOffset() const;
    void setOffset(double newOffset);

    double getMult() const;
    void setMult(double newMult);

public slots:
    void pushValue(float value, QTime record_time);
    void pushValueRaw(uint64_t value);
    void pushValueRawWithTime(uint64_t value, QDateTime date_time);
    void selectCurentPlot();
    void writeValues(float value);
    void requestClearGraph();//just clear gruph and clear buffer for math chanale
    void requestClearBufer();//clear buffered data
    void reloadValues();

private:
    float                   m_rawValue;
    float                   m_value;
    QString                 m_name;
    QString                 m_displayName;
    varloc_location_t       m_location;
//    uint32_t                m_mask;
//    varloc_node_t*          m_var_node;
    QVector<VarValue>       m_buffer;
    //list for each plot gruph
    QVector<bool>           m_plotListl;

    //graph styles
    QColor                  m_lineColor;
    unsigned int            m_lineWidth;
//    QCPGraph::LineStyle     m_lineStyle;
//    QCPScatterStyle::ScatterShape m_dotStyle;
    int                     m_dotStyle;
    int                     m_lineStyle;

    //custom ckript
    bool                    m_isMathChanale;
    QString                 m_script;

    //offset and mult
    //value = (rawValue + offset)*mult
    double m_offset, m_mult;


signals:
    void changePlotColor();
    void changePlotLineWidth();
    void changePlotLineStyle();
    void changePlotDotStyle();
    void changeDisplayName();
    void updatePlot();
    void selectPlot();
    void clearGraph();

    void requestWriteData(uint64_t data, varloc_location_t location);
};

#endif // VARCHANNEL_H
