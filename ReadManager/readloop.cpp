#include "readloop.h"
#include <QDebug>
#include <QThread>
#include <QElapsedTimer>
#include <QScriptEngine>

ReadLoop::ReadLoop(QObject *parent)
    : QObject{parent}, readDevicec(nullptr), saveDeviceces(nullptr)/*, channels(nullptr)*/
{
}

void ReadLoop::readLoop()
{
    stopSignal = false;
    if(readDevicec == nullptr/* || channels == nullptr*/)
    {
        emit stopedLoop();
        return;
    }

    saveSequence.clear();
    requestedWriteData.clear();

    bool isFileDev = readDevicec->isFileDevice();

//    readDevicec->moveToThread(this->thread());
//    for(int i = 0; i < saveDeviceces->size(); i++)
//        saveDeviceces->at(i)->moveToThread(this->thread());

    connect(readDevicec, SIGNAL(addressesReedWithTime(uint32_t,QVector<uint8_t>,QDateTime)), this, SIGNAL(addressesReedWithTime(uint32_t,QVector<uint8_t>,QDateTime)));
    connect(readDevicec, SIGNAL(addressesReedWithTime(uint32_t,QVector<uint8_t>,QDateTime)), this, SLOT(saveReedSequence(uint32_t,QVector<uint8_t>,QDateTime)));

    try {

        //try init dev
//        int res = readDevicec->initDevise(channels);
        int res = readDevicec->initDevise(readSequence);
        if(res != 0)//error read device
            throw res;

        if(!isFileDev && saveDeviceces != nullptr)
        {
            for(int i = 0; i < saveDeviceces->size(); i++)
                saveDeviceces->at(i)->initDevise(readSequence);
        }

//        QElapsedTimer timer;
//        timer.start();

        //main loop of read data
        do{
            //read value
            int resRead = readDevicec->execReadDevice();
            if(resRead != 0)//error read device
                throw resRead;


            //savde data in file
            if(!isFileDev && saveDeviceces != nullptr)
            {
                for(int i = 0; i < saveDeviceces->size(); i++)
                    saveDeviceces->at(i)->execSaveDevice(saveSequence);
            }

            if(decodeList.size() != 0)
            {
                QDateTime _t = QDateTime::currentDateTime();
                QVector<float> listDecoded = decodSavedSequence();
                emit decodedDataWithTime(listDecoded, _t);

                //calc math chanales
                QVector<float> listMathValues = calcMathChanales(chanaleNames, listDecoded, &listMathChanales);
                emit mathDataWithTime(listMathValues, _t);

            }

            //decode saved sequence

            saveSequence.clear();

            //write data
            while(requestedWriteData.size())
            {
                readDevicec->writeDataDevice(requestedWriteData[0].first, requestedWriteData[0].second);
                requestedWriteData.removeFirst();
            }
            QThread::msleep(2);

        }while(stopSignal == false);

//        qDebug () << "spendTime:" << timer.elapsed();

    } catch (int err) {
        Q_UNUSED(err);
        qDebug() << err;
    }
    //stop
    //stop read
    readDevicec->stopDev();
    //stao all dev if it nesasary
    if(!isFileDev && saveDeviceces != nullptr)
    {
        for(int i = 0; i < saveDeviceces->size(); i++)
            saveDeviceces->at(i)->stopDev();
    }

    disconnect(readDevicec, SIGNAL(addressesReedWithTime(uint32_t,QVector<uint8_t>,QDateTime)), this, SIGNAL(addressesReedWithTime(uint32_t,QVector<uint8_t>,QDateTime)));
    disconnect(readDevicec, SIGNAL(addressesReedWithTime(uint32_t,QVector<uint8_t>,QDateTime)), this, SLOT(saveReedSequence(uint32_t,QVector<uint8_t>,QDateTime)));
    saveSequence.clear();
    emit stopedLoop();
}

void ReadLoop::setSaveDeviceces(QVector<SaveDeviceObject *> *newSaveDeviceces)
{
    saveDeviceces = newSaveDeviceces;
}
void ReadLoop::setReadDevicec(ReadDeviceObject *newReadDevicec)
{
    readDevicec = newReadDevicec;
}

void ReadLoop::stopLoop()
{
    stopSignal = true;
}

void ReadLoop::requestWriteData(uint32_t data, varloc_location_t location)
{
    requestedWriteData.append(qMakePair(data, location));
}

void ReadLoop::saveReedSequence(uint32_t addres, QVector<uint8_t> data, QDateTime time)
{
    Q_UNUSED(time);
    saveSequence.append(qMakePair(addres, data));
}

void ReadLoop::setReadSequence(const QVector<ReadDeviceObject::ReadAddres> &newReadSequence, QVector<VarChannel *> *channels)
{
    readSequence = newReadSequence;
    calcDecodList(channels);
}

void ReadLoop::updateMathChanales(QVector<VarChannel *> *mathChanales)
{
    listMathChanales.clear();

    if(mathChanales != nullptr)
    {
        for(int i = 0; i < mathChanales->size(); i++)
        {
            QString name = mathChanales->at(i)->displayName();
            QString script = mathChanales->at(i)->script();
//            script = script.replace(".", "_");
            listMathChanales.append(qMakePair(name, script));
        }
    }
}

QMap<QString, float> ReadLoop::fillMapValues(QStringList chanaleNames, QVector<float> values)
{
    QMap<QString, float> res;

    for(int i = 0; i < chanaleNames.size(); i++)
    {
        if(i >= values.size())
            break;

        res[chanaleNames[i]] = values[i];
    }

    return res;
}

QVector<float> ReadLoop::calcMathChanales(QMap<QString, float> mapChanales, QVector<QPair<QString, QString>> *listMathChanales)
{
//    QList<QString> chanaleNames
//    QScriptEngine myEngine;
//    for(mapChanales.)
//    myEngine.globalObject().setProperty("foo", 123);

    QList<QString> listChanalesName = mapChanales.keys();
    QVector<float> listChanalesValues;

    for(int i = 0; i < listChanalesName.size(); i++)
    {
        listChanalesValues.append(mapChanales[listChanalesName[i]]);
    }

    return calcMathChanales(listChanalesName, listChanalesValues, listMathChanales);
}

QVector<float> ReadLoop::calcMathChanales(QList<QString> listChanalesName, QVector<float> listChanalesValues, QVector<QPair<QString,QString>> *listMathChanales)
{
    //https://doc.qt.io/qt-5/qtscript-index.html

    QVector<float> res;

    if(listMathChanales == nullptr)
        return res;

    if(listMathChanales->size() == 0)
        return res;

    QScriptEngine myEngine;
    for(int i = 0; i < listChanalesName.size(); i++)
    {
        if(i >= listChanalesValues.size())
            break;

        listChanalesName[i] = listChanalesName[i].replace(".", "_");

        myEngine.globalObject().setProperty(listChanalesName[i], listChanalesValues[i]);
    }

    for(int i = 0; i < listMathChanales->size(); i++)
    {
        QString script = listMathChanales->at(i).second;
//        script = script.replace(".", "_");
        res.append(myEngine.evaluate(script).toNumber());
    }

    return res;
}

void ReadLoop::calcDecodList(QVector<VarChannel *> *channels)
{
    decodeList.clear();
    chanaleNames.clear();

    if(channels != nullptr)
    {
        for(int i = 0; i < channels->size(); i++)
        {
            chanaleNames << channels->at(i)->displayName();
            varloc_location_t loc = channels->at(i)->getLocation();
            uint32_t mask = channels->at(i)->getMask();
            decodeList.append(qMakePair(mask, loc));
        }
    }
}

QVector<float> ReadLoop::decodSavedSequence()
{
    int numbegChanale = 0;
    QVector<float> res;
    union __attribute__((packed)){
        uint8_t _8[4];
        uint32_t _32;
    }combiner;

    for(int i = 0; i < readSequence.size(); i++)
    {
        if(i >= saveSequence.size())
            break;

        if(numbegChanale >= decodeList.size())
            break;

        ReadDeviceObject::ReadAddres addresSequence = readSequence[i];

        for(int j = 0 ; j < saveSequence[i].second.size()/4; j++)
        {
            combiner._32 = 0;
            memcpy(combiner._8, saveSequence[i].second.data() + addresSequence.vectorChanales[j].offset, /*addresSequence.vectorChanales[j].varSize*/4);


            float valuesFloat = VarChannel::decode_value(combiner._32, decodeList[numbegChanale].first, decodeList[numbegChanale].second);

            res.append(valuesFloat);

            numbegChanale++;
        }
    }

    return res;
}
//void ReadLoop::setChannels(QVector<VarChannel *> *newChannels)
//{
//    channels = newChannels;
//}
