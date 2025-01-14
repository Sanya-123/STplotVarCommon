#include "varchannel.h"
#include <QtMath>

static_assert(CHAR_BIT * sizeof (float) == 32, "Error float size");

using namespace std::chrono;

VarChannel::VarChannel(varloc_node_t* node, QColor lineColor, int dotStyle) :
    m_lineWidth(1), m_lineColor(lineColor), m_isMathChanale(false), m_value(0),
    m_mult(1.0), m_offset(0)
{
    if (node == NULL){
        return;
    }
    setLocation(var_node_get_load_location(node));
    m_name = getFullNmaeNode(node);

    m_dotStyle = MAX_DEFAOULT_DOT_STYLE;
    m_lineStyle = MAX_DEFAOULT_LINE_STYLE;
    m_displayName = m_name;

    setDotStyle(dotStyle);
}

VarChannel::VarChannel(varloc_location_t location, QString name, QColor lineColor, int dotStyle) :
    m_lineWidth(1), m_lineColor(lineColor), m_isMathChanale(false), m_value(0),
    m_mult(1.0), m_offset(0)
{
    setLocation(location);
    m_name = name;
    m_displayName = m_name;

    m_dotStyle = MAX_DEFAOULT_DOT_STYLE;
    m_lineStyle = MAX_DEFAOULT_LINE_STYLE;

    setDotStyle(dotStyle);
}

VarChannel::VarChannel(QString script, QString name, QColor lineColor, int dotStyle) :
    m_lineWidth(1), m_lineColor(lineColor), m_isMathChanale(true), m_script(script), m_value(0),
    m_mult(0), m_offset(0)
{
    varloc_location_t loc;
    loc.address.base = 0x00000000;
    loc.address.offset_bits = 0;
    loc.address.size_bits = 32;
    loc.type = VARLOC_FLOAT;

    setLocation(loc);
    m_name = name;
    m_displayName = m_name;

    m_dotStyle = MAX_DEFAOULT_DOT_STYLE;
    m_lineStyle = MAX_DEFAOULT_LINE_STYLE;

    setDotStyle(dotStyle);
}

VarChannel::~VarChannel(){

}

void VarChannel::pushValue(float value, QTime record_time){
    m_rawValue = value;
    if(m_isMathChanale)
        m_value = value;
    else
        m_value = (m_rawValue + m_offset)*m_mult;
    VarValue var = {
        .value = m_value,
        .rawValue = m_rawValue,
        .qtime = record_time, //QTime::currentTime(),
    };
//    m_buffer.push_back(var);
    m_buffer.append(var);
    emit updatePlot();
}
float VarChannel::decode_value(uint64_t value, varloc_location_t location)
{
    float ret = 0;
    union {
        float       _f;
        uint8_t     _u8;
        int8_t      _i8;
        uint16_t    _u16;
        int16_t     _i16;
        uint32_t    _u32;
        int32_t     _i32;
        uint64_t    _u64;
    }combiner;
    combiner._u64 = (value & location.mask) >> location.address.offset_bits;
    if(location.address.size_bits <= 8){
        if (location.type == VARLOC_SIGNED){
            ret = combiner._i8;
        } else{
            ret = combiner._u8;
        }
    }
    else if(location.address.size_bits <= 16){
        if (location.type == VARLOC_SIGNED){
            ret = combiner._i16;
        } else{
            ret = combiner._u16;
        }
    }
    else {
        if (location.type == VARLOC_SIGNED){
            ret = combiner._i32;
        }
        else if (location.type == VARLOC_FLOAT){
            ret = combiner._f;
        } else{
            ret = combiner._u32;
        }
    }
    return ret;
}

uint64_t VarChannel::code_value(float value, varloc_location_t location)
{
    uint64_t ret = 0;

    if (location.type == VARLOC_FLOAT)
    {
        union {
            float       _f;
            uint32_t    _u32;
        }combiner;
        combiner._f = value;
        ret = combiner._u32;
    }
    else
    {
        if (location.type == VARLOC_SIGNED)
            ret = (int32_t)value;
        else
            ret = (uint32_t)value;
    }

    ret = ret << location.address.offset_bits;
    ret = ret & location.mask;

    return ret;
}

void VarChannel::pushValueRawWithTime(uint64_t value, QDateTime date_time){
    pushValue(decode_value(value, m_location), date_time.time());
}

void VarChannel::pushValueRaw(uint64_t value){

    QTime time = QTime::currentTime();
    pushValue(decode_value(value, m_location), time);
}

void VarChannel::selectCurentPlot()
{
    emit selectPlot();
}

void VarChannel::writeValues(float value)
{
    if(!m_isMathChanale)
    {
        emit requestWriteData(code_value(value, m_location), m_location);
    }
    else
    {
        pushValue(value, QTime::currentTime());
    }
}

void VarChannel::requestClearGraph()
{
    if(m_isMathChanale)
    {
        m_buffer.clear();
    }
    emit clearGraph();
}

void VarChannel::requestClearBufer()
{
    m_buffer.clear();
    emit clearGraph();
}

void VarChannel::reloadValues()
{
    for(int i = 0; i < m_buffer.size(); i++)
        m_buffer[i].value = (m_buffer[i].rawValue + m_offset)*m_mult;

    emit updatePlot();
}

double VarChannel::getMult() const
{
    return m_mult;
}

void VarChannel::setMult(double newMult)
{
    m_mult = newMult;
}

double VarChannel::getOffset() const
{
    return m_offset;
}

void VarChannel::setOffset(double newOffset)
{
    m_offset = newOffset;
}

QString VarChannel::script() const
{
    return m_script;
}

void VarChannel::setScript(const QString &newScript)
{
    m_script = newScript;
}

QString VarChannel::displayName() const
{
    return m_displayName;
}

void VarChannel::setDisplayName(const QString &newDisplayName)
{
    m_displayName = newDisplayName;
    emit changeDisplayName();
}

varloc_location_t VarChannel::getLocation()
{
    return m_location;
}

void VarChannel::clearLocation()
{
    m_location.address.base = 0;
    m_location.address.offset_bits = 0;
    m_location.address.size_bits = 0;;
}

bool VarChannel::hasLocation(varloc_location_t loc){
    if ((loc.address.base == this->m_location.address.base)
    && (loc.address.offset_bits == this->m_location.address.offset_bits)
    && (loc.address.size_bits == this->m_location.address.size_bits)
    ){
        return true;
    }
    return false;

}

void VarChannel::setLocation(varloc_location_t loc){
    m_location = loc;
    if(m_location.mask == 0)
    {
        uint32_t m_mask = /*pow(2, m_location.address.size_bits)*/(1 << m_location.address.size_bits) - 1;
        if(m_location.address.size_bits == 32)
            m_mask = 0xFFFFFFFF;

        m_mask = m_mask << m_location.address.offset_bits;

        m_location.mask = m_mask;
    }
}

QString VarChannel::getFullNmaeNode(varloc_node_t *node)
{
    if(node == nullptr)
        return QString();


    QString name = QString(node->name);
    varloc_node_t * parent = var_node_get_parent(node);
    while (parent != NULL){
        if (parent->var_type != ARRAY){
            name.prepend(".");
            name.prepend(parent->name);
        }
        parent = var_node_get_parent(parent);
    }

    return name;
}

QString VarChannel::getName(){
    return m_name;
}

float VarChannel::getValue(){
    return m_value;
}

float VarChannel::getRawValue(){
    return m_rawValue;
}

QVector<bool> VarChannel::getPlotList()
{
    return m_plotListl;
}

int VarChannel::getTotalSizePlot()
{
    return m_plotListl.size();
}

void VarChannel::setTotalSizePlot(int size)
{
    m_plotListl.resize(size);
}

void VarChannel::removePlot(int numberPlot)
{
    if(m_plotListl.size() > numberPlot)
    {
        m_plotListl.removeAt(numberPlot);
    }
}

bool VarChannel::isEnableOnPlot(int numberPlot)
{
    if(m_plotListl.size() > numberPlot)
    {
        return m_plotListl[numberPlot];
    }
    return false;
}

void VarChannel::setEnableOnPlot(int numberPlot, bool en)
{
    if(m_plotListl.size() > numberPlot)
    {
        m_plotListl[numberPlot] = en;
    }
}

VarValue VarChannel::getValue(int numberElement)
{
    if(numberElement < m_buffer.size() && numberElement >= 0)
        return m_buffer[numberElement];


    return (VarValue){0.0, 0.0, QTime::currentTime()};//NOTE could be paroblem in save files
}

QVector<VarValue> VarChannel::getBuffer()
{
    return m_buffer;
}

int VarChannel::getBufferSize()
{
    return m_buffer.size();
}

int VarChannel::dotStyle() const
{
    return m_dotStyle;
}

void VarChannel::setDotStyle(int newDotStyle)
{
    if(newDotStyle < 0)
        newDotStyle = 0;
    if(newDotStyle >= MAX_NUMBER_DOT_STYLE)
        newDotStyle = MAX_NUMBER_DOT_STYLE - 1;

    m_dotStyle = newDotStyle;

    emit changePlotDotStyle();
}

int VarChannel::lineStyle() const
{
    return m_lineStyle;
}

void VarChannel::setLineStyle(int newLineStyle)
{
    if(newLineStyle < 0)
        newLineStyle = 0;
    if(newLineStyle >= MAX_NUMBER_LINE_STYLE)
        newLineStyle = MAX_NUMBER_LINE_STYLE - 1;

    m_lineStyle = newLineStyle;

    emit changePlotLineStyle();
}

QColor VarChannel::lineColor() const
{
    return m_lineColor;
}

void VarChannel::setLineColor(const QColor &newLineColor)
{
    m_lineColor = newLineColor;
    emit changePlotColor();
}

unsigned int VarChannel::lineWidth() const
{
    return m_lineWidth;
}

void VarChannel::setLineWidth(unsigned int newWidth)
{
    m_lineWidth = newWidth;
    emit changePlotLineWidth();
}
