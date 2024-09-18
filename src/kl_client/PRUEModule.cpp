#include "PRUEModule.hpp"

using namespace NEBULA;

PRUEModule::PRUEModule(qint16 id) {
    pkgsPRUE = {0x601, 0x201, 0x202};
    m_id = id;
    m_pStateMachineTimer = new QTimer(this);
    m_pSettingTimerD21   = new QTimer(this);
    m_isSendSISD01       = false;
    m_isSendFuncD22      = false;

    connect(m_pStateMachineTimer,  &QTimer::timeout, this, &PRUEModule::stateMachine);
    connect(m_pSettingTimerD21,    &QTimer::timeout, this, &PRUEModule::sendSettingD21);
    // 包头
    m_genericHeader.sender   = 0x524542;
    m_genericHeader.moduleId = 0xff;
    m_genericHeader.vMajor   = 2;
    m_genericHeader.vMinor   = 2;
    m_genericHeader.isAsku   = 1;

    moduleCfg20 = readJson(commCfgini->value(QString("Jammer%1/devconfig20").arg(m_id)).toString());
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
            item.n_val = 2;
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
    // 设置D21
    m_oTrapSettings0xD21.curAzREB  = 0;
    m_oTrapSettings0xD21.curEpsREB = 0;
    m_oTrapSettings0xD21.kGainREB  = 0;
    m_oTrapSettings0xD21.taskGeo   = 0;
    m_oTrapSettings0xD21.taskREB   = 0;
    m_vecOTrapSettings0xD21_1.append({100, 20, 1000});
    // 功能D22
    m_oTrapFunc0xD22.numDiap   = 0;
    m_oTrapFunc0xD22.isGeo     = 0;
    m_oTrapFunc0xD22.maxPowREB = 0;
    m_oTrapFunc0xD22.dAzREB    = 0;
    m_oTrapFunc0xD22.dElevREB  = 0;
    m_oTrapFunc0xD22.azMinREB  = 0;
    m_oTrapFunc0xD22.azMaxREB  = 0;
    m_oTrapFunc0xD22.dAzGeo    = 0;
    m_oTrapFunc0xD22.dElevGeo  = 0;
    m_oTrapFunc0xD22.azMinGeo  = 0;
    m_oTrapFunc0xD22.azMaxGeo  = 0;
    m_vecFuncD22 = {};
    QString freqStr = commCfgini->value(QString("Jammer%1/freqs").arg(m_id)).toString();
    QRegularExpression re("\\[(\\d+),\\s*(\\d+),\\s*(\\d+)\\]");
    QRegularExpressionMatchIterator matchIterator = re.globalMatch(freqStr);
    while (matchIterator.hasNext()) {
        QRegularExpressionMatch match = matchIterator.next();
        if (match.hasMatch()) {
            OTrapFunc0xD22_2 freq;
            freq.minFreqREB = match.captured(1).toFloat();
            freq.maxFreqREB = match.captured(2).toFloat();
            freq.maxDFreq = match.captured(3).toFloat();
            m_vecFuncD22.append(freq);
        }
    } 
    m_oTrapFunc0xD22.numDiap2 = m_vecFuncD22.size();
    // 禁止干扰扇区
    m_oTrapBanSectorD01.num = 0;
    m_vecOTrapBanSectorD01_1 = {};
}

PRUEModule::~PRUEModule() {
    if (m_pStateMachineTimer == nullptr) delete m_pStateMachineTimer;
    if (m_pSettingTimerD21 == nullptr)   delete m_pSettingTimerD21;
}

void PRUEModule::startup() {
    initTcp();
    m_pStateMachineTimer->start();
    qDebug() << QString("The PRUE number %1 is running.").arg(m_id);
}

void PRUEModule::onRecvData() {
    QByteArray buf;
    if(m_pTcpSocket->bytesAvailable() > 0) {
        buf = m_pTcpSocket->read(m_pTcpSocket->bytesAvailable());
    }
    quint16 pkgID = 0;
    memcpy(&pkgID, buf.data() + 12, 2);
    if (pkgsComm.contains(pkgID)) {
        onReadCommData(pkgID, buf);
    }
    if (pkgsPRUE.contains(pkgID)) {
        switch (pkgID) {
        case 0x201: recvSIS201(buf);      break;
        case 0x202: recvSISOrder202(buf); break;
        case 0x601: recvSetting601(buf);  break;
        default: break;
        }
    }
}


// 状态机
void PRUEModule::stateMachine() {
    switch (m_runStatus)
    {
    case RunStatus::unConnected:
        if (!m_pReconnTimer->isActive())   m_pReconnTimer->start(1000);
        if (m_pReqTimer03->isActive())     m_pReqTimer03->stop();
        if (m_isSendRegister01)            m_isSendRegister01 = false;

        if (m_runStatus == RunStatus::registered || m_runStatus == RunStatus::timed)   m_runStatus = RunStatus::unRegister;
        break;
    case RunStatus::connected:
        if (m_pReconnTimer->isActive())    m_pReconnTimer->stop();
        if (!m_isSendRegister01)           sendRegister01();
        break;
    case RunStatus::unRegister:
        if (m_pReqTimer03->isActive())       m_pReqTimer03->stop();
        if (m_isSendLocation05)              m_isSendLocation05  = false;
        if (m_isSendConf20)                  m_isSendConf20      = false;
        if (m_isSendSISD01)                  m_isSendSISD01      = false;
        if (m_isSendFuncD22)                 m_isSendFuncD22     = false;

        if (m_runStatus == RunStatus::timed) m_runStatus = RunStatus::unTime;
        break;
    case RunStatus::registered:
        if (!m_pReqTimer03->isActive()) m_pReqTimer03->start(1000);
        if (!m_isSendLocation05)        sendModuleLocation05();
        if (!m_isSendConf20)            sendModuleFigure20();
        if (!m_isSendSISD01)            sendSISD01();
        if (!m_isSendFuncD22)           sendFuncD22();
        break;
    case RunStatus::unTime: {
        if (m_pStatusTimer21->isActive())   m_pStatusTimer21->stop();
        if (m_pCPTimer22->isActive())       m_pCPTimer22->stop();
        if (m_pStatusTimer24->isActive())   m_pStatusTimer24->stop();
        if (m_pNPTimer28->isActive())       m_pNPTimer28->stop();
        if (m_pSettingTimerD21->isActive()) m_pSettingTimerD21->stop();
        break;
    }
    case RunStatus::timed:
    {
        if (!m_pStatusTimer21->isActive())    m_pStatusTimer21->start(5000);
        if (!m_pCPTimer22->isActive())        m_pCPTimer22->start(5000);
        if (!m_pStatusTimer24->isActive())    m_pStatusTimer24->start(1000);
        if (!m_pNPTimer28->isActive())        m_pNPTimer28->start(5000);
        if (!m_pSettingTimerD21->isActive())  m_pSettingTimerD21->start(1000);
        break;
    }
    default: break;
    }
}

// 0x601,接收更改设置 601-D21
void PRUEModule::recvSetting601(const QByteArray& buf) {
    qint8 len1 = HEADER_LEN;
    qint8 len2 = sizeof(ORecvSetting0x601);
    ORecvSetting0x601 m_oRecvTrapFixed0x601;
    memcpy(&m_oRecvTrapFixed0x601, buf.data() + len1, len2);
    qint16 offset = len1 + len2;
    for(uint32_t i = 0; i < m_oRecvTrapFixed0x601.N; i++) {
        FreqAndDFreq freqAndDFreq;
        memcpy(&freqAndDFreq, buf.data() + offset, sizeof(FreqAndDFreq));
        m_vecORecvSetting0x601.append(freqAndDFreq);
        offset += sizeof(FreqAndDFreq);
    }
    if(m_oRecvTrapFixed0x601.taskREB == 3) {
        memcpy(&m_NavigationInfluence601, buf.data() + offset, sizeof(NavigationInfluence601));
    }
    /* ------------------------------------------------------------------------ */
    quint16 pkgIdx = 0;
    memcpy(&pkgIdx, buf.data() + 6, 2);
    qDebug() << "recv 0x601:" << buf.toHex() << "pkgSize:"    << buf.length()
             << "num:"        << m_oTrapBanSectorD01.num << "pkgIdx:"     << pkgIdx;
    for(auto& item : m_vecORecvSetting0x601) {
        qDebug() << QString("lat=%1, long=%2").arg(item.freq).arg(item.dfreq);
    }
    qDebug() << QString("m_NavigationInfluence601,latitude=%1").arg(m_NavigationInfluence601.latitude).arg(m_NavigationInfluence601.longitude);
    sendControlledOrder23(0, pkgIdx);
    sendSettingD21();
}

// 0xD21,设置 D21-601
void PRUEModule::sendSettingD21() {
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OTrapSettings0xD21);
    /* ------------------------------------------------------------------------ */
    m_genericHeader.packType = 0xD21;
    m_genericHeader.dataSize = len2;
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    QByteArray buf;
    buf.resize(len1 + len2);
    memcpy(buf.data(), &m_genericHeader, len1);
    memcpy(buf.data() + len1, &m_oTrapSettings0xD21, len2);
    m_pTcpSocket->write(buf);
    m_pTcpSocket->flush();
    qDebug() << "send 0xD21:" << buf.toHex()
             << "pkgSize:"    << buf.length();
}

// 0xD01-0x201,发送已安装的辐射禁止扇区或接收设置辐射禁止扇区
void PRUEModule::recvSIS201(const QByteArray& buf) {
    qint8 len1 = HEADER_LEN;
    qint8 len2 = sizeof(OTrapBanSectorD01);
    memcpy(&m_oTrapBanSectorD01, buf.data() + len1, len2);
    qint16 offset = len1 + len2;
    m_vecOTrapBanSectorD01_1.clear();
    for(quint32 i = 0; i < m_oTrapBanSectorD01.num; i++) {
        OTrapBanSectorD01_1 oTrapBanSector201;
        memcpy(&oTrapBanSector201, buf.data() + offset, sizeof(OTrapBanSectorD01_1));
        m_vecOTrapBanSectorD01_1.append(oTrapBanSector201);
        offset += sizeof(OTrapBanSectorD01_1);
    } 
    /* ------------------------------------------------------------------------ */
    quint16 pkgIdx = 0;
    memcpy(&pkgIdx, buf.data() + 6, 2);
    qDebug() << "recv 0x201:" << buf.toHex()
             << "pkgSize:"    << buf.length()
             << "num:"        << m_oTrapBanSectorD01.num
             << "pkgIdx:"     << pkgIdx;
    for(auto& item : m_vecOTrapBanSectorD01_1) {
        qDebug() << QString("AzBegin=%1,AzEnd=%2,EpsBegin=%3,EpsEnd=%4,Freq=%5,delFreq=%6")
                    .arg(item.AzBegin).arg(item.AzEnd).arg(item.EpsBegin)
                    .arg(item.EpsEnd).arg(item.Freq).arg(item.delFreq);
    }
    sendControlledOrder23(0, pkgIdx);
    sendSISD01();
}

// 0xD01-0x201,发送辐射禁止扇区,其中num值是禁止扇区个数，其参数是OTrapBanSectorD01_1结构体
void PRUEModule::sendSISD01() {
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OTrapBanSectorD01);
    qint32 len3 = m_vecOTrapBanSectorD01_1.size() * sizeof(OTrapBanSectorD01_1);
    m_isSendSISD01 = true;
    /* ------------------------------------------------------------------------ */
    m_genericHeader.packType = 0xD01;
    m_genericHeader.dataSize = len2 + len3;
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    QByteArray buf;
    buf.resize(len1 + len2 + len3);
    memcpy(buf.data(), &m_genericHeader, len1);
    memcpy(buf.data() + len1, &m_oTrapBanSectorD01, len2);
    qint16 offset = len1 + len2;
    for(auto& item : m_vecOTrapBanSectorD01_1) {
        memcpy(buf.data() + offset, &item, sizeof(OTrapBanSectorD01_1));
        offset += sizeof(OTrapBanSectorD01_1);
    }
    m_pTcpSocket->write(buf);
    m_pTcpSocket->flush();
    qDebug() << "send 0xD01:" << buf.toHex()
             << "pkgSize:"    << buf.length();
}

// 0xD22,发送功能，其中numDiap2值是频率、频段、最大暴露范围结构体的数量
void PRUEModule::sendFuncD22() {
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OTrapFunc0xD22);
    quint32 len3 = m_vecFuncD22.size() * sizeof(OTrapFunc0xD22_2);
    m_isSendFuncD22 = true;
    /* ------------------------------------------------------------------------ */
    m_genericHeader.packType = 0xD22;
    m_genericHeader.dataSize = len2;
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    QByteArray buf;
    buf.resize(len1 + len2 + len3);
    memcpy(buf.data(), &m_genericHeader, len1);
    memcpy(buf.data() + len1, &m_oTrapFunc0xD22, len2);
    quint16 offset = len1 + len2;
    for(auto& item : m_vecFuncD22) {
        memcpy(buf.data() + offset, &item, sizeof(OTrapFunc0xD22_2));
        offset += sizeof(OTrapFunc0xD22_2);
    }
    m_pTcpSocket->write(buf);
    m_pTcpSocket->flush();
    /* ------------------------------------------------------------------------ */
    qDebug() << "send 0xD22:" << buf.toHex()
             << "pkgSize:"    << buf.length();
    for(auto& item : m_vecFuncD22) {
        qDebug() << QString("minFreqREB=%1, maxDFreq=%2, maxDFreq=%3").arg(item.minFreqREB).arg(item.maxFreqREB).arg(item.maxDFreq);
    }
}

// 0x202,接收设置辐射禁止
void PRUEModule::recvSISOrder202(const QByteArray& buf) {
    qint8 len1 = HEADER_LEN;
    qint8 len2 = sizeof(OTrapRadiationBan0x202);
    quint16 pkgIdx = 0;
    memcpy(&pkgIdx, buf.data() + 6, 2);
    memcpy(&m_oTrapRadiationBan0x202, buf.data() + len1, len2);
    qDebug() << "recv 0x202:" << buf.toHex()
             << "pkgSize:"    << buf.length()
             << "pkgIdx:"     << pkgIdx
             << "isOn:"       << m_oTrapRadiationBan0x202.isOn;
    sendControlledOrder23(0, pkgIdx);
}
