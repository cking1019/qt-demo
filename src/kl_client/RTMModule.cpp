#include "RTMModule.hpp"

using namespace NEBULA;

RTMModule::RTMModule(qint16 id) {
    pkgsRTM = {0x561, 0x563, 0x564};
    m_id = id;
    m_pStateMachineTimer = new QTimer(this);
    m_pSettingTimer823   = new QTimer(this);
    m_isSendFunc825      = false;
    m_isSendRI828        = false;

    connect(m_pStateMachineTimer, &QTimer::timeout, this, &RTMModule::stateMachine);
    connect(m_pSettingTimer823,   &QTimer::timeout, this, &RTMModule::sendSetting823);
    // 包头
    m_genericHeader.sender   = 0x50454C;
    m_genericHeader.moduleId = 0xff;
    m_genericHeader.vMajor   = 2;
    m_genericHeader.vMinor   = 2;
    m_genericHeader.isAsku   = 1;

    moduleCfg20 = readJson(commCfgini->value(QString("Detector%1/devconfig20").arg(m_id)).toString());
    // 读取0x20配置信息
    QJsonDocument jsonDocument = QJsonDocument::fromJson(moduleCfg20.toUtf8());
    QVariantList list = jsonDocument.toVariant().toList();
    for(int i = 0; i < list.count(); i++) {
        // 读取0x20中的元素配置
        OElemStatus0x21 item;
        QVariantMap map = list[i].toMap();
        item.IDElem = map["IDElem"].toInt();
        m_vecOElemStatus0x21.append(item);
        // 解析0x22cp参数
        QVariantList cpList = map["Params"].toList();
        for(int j = 0; j < cpList.count(); j++) {
            QVariantMap map2 = cpList[j].toMap();
            OCPStatus0x22 item;
            item.IDParam = map2["IDParam"].toInt();
            item.n_val  = 2;
            item.status = 4;
            m_vecOCPStatus0x22.append(item);
        }
        // 解析0x28np参数
        QVariantList npList = map["ConfigParam"].toList();
        for(int j = 0; j < npList.count(); j++) {
            QVariantMap map2 = npList[j].toMap();
            CustomisedNP0x28 item;
            item.IDParam = map2["IDConfigParam"].toInt();
            item.np_v = 1;
            item.size = 4;
            m_vecCustomisedNP0x28.append(item);
        }
    }
    // 设置823
    m_oSetting0x823.N     =  0;
    m_oSetting0x823.curAz = -1;
    m_freqs823 = {};
    // 功能825
    m_oFunc0x825.isRotate =  0;
    m_oFunc0x825.maxTasks =  6;
    m_oFunc0x825.dAz      = -1;
    m_oFunc0x825.dElev    = -1;
    m_vecFunc825 = {};
    QString freqStr = commCfgini->value(QString("Detector%1/freqs").arg(m_id)).toString();
    QRegularExpression re("\\[(\\d+),\\s*(\\d+)\\]");
    QRegularExpressionMatchIterator matchIterator = re.globalMatch(freqStr);
    while (matchIterator.hasNext()) {
        QRegularExpressionMatch match = matchIterator.next();
        if (match.hasMatch()) {
            RTMFuncFreq freq;
            freq.minFreqRTR = match.captured(1).toFloat();
            freq.maxFreqRTR = match.captured(2).toFloat();
            m_vecFunc825.append(freq);
        }
    } 
    m_oFunc0x825.numDiap = m_vecFunc825.size();
    // 禁止扫描频率828
    m_oSetIRI0x828.NIRI = 0;
    m_freqs828 = {};
}

RTMModule::~RTMModule() {
    delete m_pStateMachineTimer;
    delete m_pSettingTimer823;
}

// 设备启动，开始与服务器建立连接
void RTMModule::startup() {
    initTcp();
    m_pStateMachineTimer->start();
    qDebug() << QString("The RTM  number %1 is running.").arg(m_id);
}

// 接收数据
void RTMModule::onRecvData() {
    QByteArray buf;
    // if(m_pTcpSocket->waitForReadyRead()) {
        // buf = m_pTcpSocket->readAll();
        // qDebug() << buf.toHex();
    // }
    // 非阻塞
    if(m_pTcpSocket->bytesAvailable() > 0) {
        buf = m_pTcpSocket->readAll();
        // buf = m_pTcpSocket->read(m_pTcpSocket->bytesAvailable());
    }
    quint16 pkgID = 0;
    memcpy(&pkgID, buf.data() + 12, 2);
    if (pkgsComm.contains(pkgID)) {
        onReadCommData(pkgID, buf);
    }
    if (pkgsRTM.contains(pkgID)) {
        switch (pkgID) {
        case 0x561: recvSetting561(buf);    break;
        case 0x563: recvReqIRI563(buf);     break;
        case 0x564: recvSettingIRI564(buf); break;
        default: break;
        }
    }
}

// 状态机,连接->(注册|对时) 注册->对时
void RTMModule::stateMachine() {
    switch (m_runStatus)
    {
    case RunStatus::unConnected:
        if (!m_pReconnTimer->isActive())    m_pReconnTimer->start(1000);
        if (m_pReqTimer03->isActive())      m_pReqTimer03->stop();
        if (m_isSendRegister01)             m_isSendRegister01 = false;

        if (m_runStatus == RunStatus::registered || m_runStatus == RunStatus::timed) m_runStatus = RunStatus::unRegister;
        break;
    case RunStatus::connected:
        if (m_pReconnTimer->isActive())        m_pReconnTimer->stop();
        if (!m_isSendRegister01)               sendRegister01();
        break;
    case RunStatus::unRegister:
        if (m_pReqTimer03->isActive())  m_pReqTimer03->stop();
        if (m_isSendLocation05)         m_isSendLocation05  = false;
        if (m_isSendConf20)             m_isSendConf20      = false;
        if (m_isSendFunc825)            m_isSendFunc825     = false;
        if (m_isSendRI828)              m_isSendRI828       = false;

        if (m_runStatus == RunStatus::timed)      m_runStatus = RunStatus::unTime;
        break;
    case RunStatus::registered:
        if (!m_pReqTimer03->isActive()) m_pReqTimer03->start(1000);
        if (!m_isSendLocation05)        sendModuleLocation05();
        if (!m_isSendConf20)            sendModuleFigure20();
        if (!m_isSendFunc825)           sendFunc825();
        if (!m_isSendRI828)             sendIRI828();
        break;
    case RunStatus::unTime: {
        if (m_pStatusTimer21->isActive())       m_pStatusTimer21->stop();
        if (m_pCPTimer22->isActive())           m_pCPTimer22->stop();
        if (m_pStatusTimer24->isActive())       m_pStatusTimer24->stop();
        if (m_pNPTimer28->isActive())           m_pNPTimer28->stop();
        if (m_pSettingTimer823->isActive())     m_pSettingTimer823->stop();
        break;
    }
    case RunStatus::timed:
    {
        if (!m_pStatusTimer21->isActive())       m_pStatusTimer21->start(5000);
        if (!m_pCPTimer22->isActive())           m_pCPTimer22->start(5000);
        if (!m_pStatusTimer24->isActive())       m_pStatusTimer24->start(1000);
        if (!m_pNPTimer28->isActive())           m_pNPTimer28->start(5000);
        if (!m_pSettingTimer823->isActive())     m_pSettingTimer823->start(1000);
        break;
    }
    default: break;
    }
}

// 0x561,修改频率、频段 561-823
void RTMModule::recvSetting561(const QByteArray& buf) {
    quint32 len1 = HEADER_LEN;
    quint32 len2 = 4;
    memcpy(&m_oSetting0x823, buf.data() + len1, len2);
    // 4 + 8 * n
    m_freqs823.clear();
    QVector<FreqAndDFreq> goodFreq;
    QVector<FreqAndDFreq> badFreq;
    quint16 offset = len1 + len2;
    for(uint32_t i = 0; i < m_oSetting0x823.N; i++) {
        FreqAndDFreq item;
        memcpy(&item, buf.data() + offset, sizeof(FreqAndDFreq));
        goodFreq.append(item);
        offset += sizeof(FreqAndDFreq);
    }
    // 判断收到的频率是否满足要求
    for(auto& item : goodFreq) {
        bool flag = false;
        for(auto& itemFunc : m_vecFunc825) {
            if(item.freq - item.dfreq * 0.5 >= itemFunc.minFreqRTR && 
               item.freq + item.dfreq * 0.5 <= itemFunc.maxFreqRTR) {
                flag = true;
                break;
            }
        }
        // 接收满足功能要求的频率
        if(flag) m_freqs823.append(item);
        else     badFreq.append(item);
    }
    /* ------------------------------------------------------------------------ */
    quint16 pkgIdx = 0;
    memcpy(&pkgIdx, buf.data() + 6, 2);
    qDebug() << "recv 0x561:" << buf.toHex()
             << "pkgSize:"    << buf.length()
             << "N:"          << m_oSetting0x823.N
             << "pkgIdx:"     << pkgIdx;
    quint8 code;
    if(badFreq.isEmpty()) code = 0;
    else                  code = 2;
    sendControlledOrder23(code, pkgIdx);
    sendSetting823();
}

// 发送设置，823-561
void RTMModule::sendSetting823() {
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OSetting0x823);
    qint32 len3 = m_freqs823.size() * sizeof(FreqAndDFreq);
    // 设置,默认初始化
    m_oSetting0x823.curAz = -1;
    /* ------------------------------------------------------------------------ */
    m_genericHeader.packType = 0x823;
    m_genericHeader.dataSize = len2 + len3;
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    QByteArray buf;
    buf.resize(len1 + len2 + len3);
    memcpy(buf.data(), &m_genericHeader, len1);
    memcpy(buf.data() + len1, &m_oSetting0x823, len2);
    int offset = len1 + len2;
    for (auto& item : m_freqs823) {
        memcpy(buf.data() + offset, &item, sizeof(FreqAndDFreq));
        offset += sizeof(FreqAndDFreq);
    }
    m_pTcpSocket->write(buf);
    m_pTcpSocket->flush();
    qDebug() << "send 0x823:" << buf.toHex()
             << "pkgSize:"    << buf.length()
             << "N:"          << m_oSetting0x823.N;
    for(auto &item : m_freqs823) {
        qDebug() << QString("freq=%1, DFreq=%2").arg(item.freq).arg(item.dfreq);
    }
}

// 564修改IRI的中心评率 564-828
void RTMModule::recvSettingIRI564(const QByteArray& buf) {
    quint32 len1 = HEADER_LEN;
    quint32 len2 = sizeof(OSetIRI0x828);
    memcpy(&m_oSetIRI0x828, buf.data() + len1, len2);
    // 清空原来的中心频率设置
    m_freqs828.clear();
    QVector<FreqAndDFreq> m_freqs823Temp;
    qint16 offset = len1 + len2;
    for(uint32_t i = 0; i < m_oSetIRI0x828.NIRI; i++) {
        FreqAndDFreq item;
        memcpy(&item, buf.data() + offset, sizeof(FreqAndDFreq));
        m_freqs828.append(item);
        offset += sizeof(FreqAndDFreq);
    }
    // 判断收到的频率是否满足要求
    for(auto& item : m_freqs823Temp) {
        bool flag = false;
        for(auto& itemFunc : m_vecFunc825) {
            if(item.freq - item.dfreq * 0.5 >= itemFunc.minFreqRTR && 
               item.freq + item.dfreq * 0.5 <= itemFunc.maxFreqRTR) {
                flag = true;
                break;
            }
        }
        // 接收满足功能要求的频率
        if(flag) m_freqs828.append(item);
    }
    /* ------------------------------------------------------------------------ */
    quint16 pkgIdx = 0;
    memcpy(&pkgIdx, buf.data() + 6, 2);
    qDebug() << "recv 0x564:" << buf.toHex() << "pkgSize:"    << buf.length()
             << "NIRI:"       << m_oSetIRI0x828.NIRI << "pkgIdx:" << pkgIdx;
    for(auto &item : m_freqs828) {
        qDebug() << QString("freq=%1, DFreq=%2").arg(item.freq).arg(item.dfreq);
    }
    sendControlledOrder23(0, pkgIdx);
}

// 修改频率、频段 828-564
void RTMModule::sendIRI828() {
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OSetIRI0x828);
    qint32 len3 = m_freqs828.size() * sizeof(FreqAndDFreq);
    m_isSendRI828 = true;
    /* ------------------------------------------------------------------------ */
    m_genericHeader.packType = 0x828;
    m_genericHeader.dataSize = len2 + len3;
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    QByteArray buf;
    buf.resize(len1 + len2 + len3);
    memcpy(buf.data(), &m_genericHeader, len1);
    memcpy(buf.data() + len1, &m_oSetIRI0x828, len2);
    int offset = len1 + len2;
    for (auto& item : m_freqs828) {
        memcpy(buf.data() + offset, &item, sizeof(FreqAndDFreq));
        offset += sizeof(FreqAndDFreq);
    }
    m_pTcpSocket->write(buf);
    m_pTcpSocket->flush();
    qDebug() << "send 0x828:" << buf.toHex() << "pkgSize:"    << buf.length()
             << "NIRI:"       << m_oSetIRI0x828.NIRI;
    for(auto &item : m_freqs828) {
        qDebug() << QString("freq=%1, DFreq=%2").arg(item.freq).arg(item.dfreq);
    }
}

// 563->828
void RTMModule::recvReqIRI563(const QByteArray& buf) {
    quint16 pkgIdx = 0;
    memcpy(&pkgIdx, buf.data() + 6, 2);
    qDebug() << "recv 0x563:" << buf.toHex() << "pkgSize:"    << buf.length()
             << "pkgIdx:"     << pkgIdx;
    sendControlledOrder23(0, pkgIdx);
    sendIRI828();
}

void RTMModule::sendTarget822(OTarget822& m_oTargetMark0x822) {
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OTarget822);
    qint64 reqTimestamp = QDateTime::currentMSecsSinceEpoch();
    // 目标
    // OTarget822 m_oTargetMark0x822;
    // m_oTargetMark0x822.timePel1 = reqTimestamp & 0xFFFFFFFF;
    // m_oTargetMark0x822.timePel2 = (reqTimestamp >> 32) & 0xFFFFFFFF;
    // m_oTargetMark0x822.idxPoint++;
    // m_oTargetMark0x822.idxCeilVOI  = 0xffff;
    // m_oTargetMark0x822.idxCeilSPP  = 1;
    // m_oTargetMark0x822.idxPoint    = 0;
    // m_oTargetMark0x822.typeCeilSPP = 4;
    // m_oTargetMark0x822.typeChannel = 0;
    // m_oTargetMark0x822.typeSignal  = 0;
    // m_oTargetMark0x822.azim        = 180;
    // m_oTargetMark0x822.elev        = 10.5;
    // m_oTargetMark0x822.range       = 1060;
    // m_oTargetMark0x822.freqMhz     = 5840;
    // m_oTargetMark0x822.dFreqMhz    = 10;
    // m_oTargetMark0x822.Pow_dBm     = 80;
    // m_oTargetMark0x822.SNR_dB      = 15;
    /* ------------------------------------------------------------------------ */
    m_genericHeader.packType = 0x822;
    m_genericHeader.dataSize = len2;
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    QByteArray buf;
    buf.resize(len1 + len2);
    memcpy(buf.data(), &m_genericHeader, len1);
    memcpy(buf.data() + len1, &m_oTargetMark0x822, len2);
    m_pTcpSocket->write(buf);
    m_pTcpSocket->flush();
    /* ------------------------------------------------------------------------ */
    if(m_isDebugOut) {
        sendLogMsg25("a target has being detected");
        sendNote2Operator26("a target has being detected");
    }
    qDebug() << "send 0x822:" << buf.toHex() << "pkgSize:"    << buf.length()
             << "checksum:"   << m_genericHeader.checkSum << "freqMhz:"    << m_oTargetMark0x822.freqMhz;
}

void RTMModule::sendFunc825() {
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OFunc0x825);
    quint32 len3 = m_vecFunc825.size() * sizeof(RTMFuncFreq);
    m_isSendFunc825 = true;
    /* ------------------------------------------------------------------------ */
    m_genericHeader.packType = 0x825;
    m_genericHeader.dataSize = len2;
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    QByteArray buf;
    buf.resize(len1 + len2 + len3);
    memcpy(buf.data(), &m_genericHeader, len1);
    memcpy(buf.data() + len1, &m_oFunc0x825, len2);
    quint16 offset = len1 + len2;
    for(auto& item : m_vecFunc825) {
        memcpy(buf.data() + offset, &item, sizeof(RTMFuncFreq));
        offset += sizeof(RTMFuncFreq);
    }

    m_pTcpSocket->write(buf);
    m_pTcpSocket->flush();
    qDebug() << "send 0x825:"   << buf.toHex()
             << "pkgSize:"      << buf.length()
             << "numDiap:"      << m_oFunc0x825.numDiap;
    for(auto &item : m_vecFunc825) {
        qDebug() << QString("minFreq=%1, maxFreq=%2").arg(item.minFreqRTR).arg(item.maxFreqRTR);
    }
}
