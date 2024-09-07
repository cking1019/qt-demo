#include "ModuleController.hpp"

namespace NEBULA {

ModuleController::ModuleController() {
}

ModuleController::~ModuleController() {
}

void ModuleController::init() {
    auto relayConfig = new QSettings(RELAY_PATH, QSettings::IniFormat);
    relayConfig->setIniCodec(QTextCodec::codecForName("utf-8"));

    auto serverAddress = relayConfig->value("common/serverIP").toString();
    auto serverPort = relayConfig->value("common/serverPort").toInt();
    qDebug() << "server address is " << serverAddress;
    qDebug() << "server port is " << serverPort;

    // 侦测设备配置
    auto iDetectNum = relayConfig->value("DetectDevNum/num").toInt();
    for (int i = 1; i <= iDetectNum; i++) {
        auto p = new RTMModule();
        p->m_commCfg.serverAddress  = relayConfig->value("common/serverIP").toString();
        p->m_commCfg.serverPort     = relayConfig->value("common/serverPort").toInt();
        p->m_commCfg.moduleAddress  = relayConfig->value("common/clientIP").toString();
        p->m_commCfg.modulePort     = relayConfig->value("common/clientPort").toInt();
        p->m_commCfg.moduleCfg20    = readJson(relayConfig->value("common/devconfig20").toString());
        p->m_isDebugOut             = relayConfig->value("common/debugOut").toBool();

        p->m_genericHeader.sender   = relayConfig->value("common/sender").toInt();
        p->m_genericHeader.moduleId = relayConfig->value("common/moduleId").toInt();
        p->m_genericHeader.vMajor   = relayConfig->value("common/vMajor").toInt();
        p->m_genericHeader.vMinor   = relayConfig->value("common/vMinor").toInt();
        p->m_genericHeader.isAsku   = relayConfig->value("common/isAsku").toInt();

        p->m_oBearingMark0x822.idxCeilVOI  = relayConfig->value("822/idxCeilVOI").toInt();
        p->m_oBearingMark0x822.idxCeilSPP  = relayConfig->value("822/idxCeilSPP").toInt();
        p->m_oBearingMark0x822.idxPoint    = relayConfig->value("822/idxPoint").toInt();
        p->m_oBearingMark0x822.typeCeilSPP = relayConfig->value("822/typeCeilSPP").toInt();
        p->m_oBearingMark0x822.typeChannel = relayConfig->value("822/typeChannel").toInt();
        p->m_oBearingMark0x822.typeSignal  = relayConfig->value("822/typeSignal").toInt();
        p->m_oBearingMark0x822.azim        = relayConfig->value("822/azim").toInt();
        p->m_oBearingMark0x822.elev        = relayConfig->value("822/elev").toFloat();
        p->m_oBearingMark0x822.range       = relayConfig->value("822/range").toInt();
        p->m_oBearingMark0x822.freqMhz     = relayConfig->value("822/freqMhz").toInt();
        p->m_oBearingMark0x822.dFreqMhz    = relayConfig->value("822/dFreqMhz").toInt();
        p->m_oBearingMark0x822.Pow_dBm     = relayConfig->value("822/Pow_dBm").toInt();
        p->m_oBearingMark0x822.SNR_dB      = relayConfig->value("822/SNR_dB").toInt();

        p->m_oSubRezhRTR0x823.N = relayConfig->value("823/N").toInt();
        p->m_oSubRezhRTR0x823.curAz = relayConfig->value("823/curAz").toInt();
        float freqMhz = relayConfig->value("823/freqMhz").toFloat();
        float dFreqMhz = relayConfig->value("823/dFreqMhz").toFloat();
        p->m_freqs823 = {{freqMhz, dFreqMhz}};

        p->m_oSubPosobilRTR0x825.isRotate   = relayConfig->value("825/isRotate").toInt();
        p->m_oSubPosobilRTR0x825.maxTasks   = relayConfig->value("825/maxTasks").toInt();
        p->m_oSubPosobilRTR0x825.numDiap    = relayConfig->value("825/numDiap").toInt();
        p->m_oSubPosobilRTR0x825.dAz        = relayConfig->value("825/dAz").toInt();
        p->m_oSubPosobilRTR0x825.dElev      = relayConfig->value("825/dElev").toInt();
        p->m_oSubPosobilRTR0x825.minFreqRTR = relayConfig->value("825/minFreqRTR").toInt();
        p->m_oSubPosobilRTR0x825.maxFreqRTR = relayConfig->value("825/maxFreqRTR").toInt();

        p->m_oSetBanIRIlist0x828.NIRI = relayConfig->value("828/NIRI").toInt();
        float freqMhz2  = relayConfig->value("828/freqMhz").toFloat();
        float dFreqMhz2 = relayConfig->value("828/dFreqMhz").toFloat();
        p->m_freqs828   = {{freqMhz2, dFreqMhz2}};

        p->m_oSubRadioTime0x829.taskNum   = relayConfig->value("829/taskNum").toInt();
        p->m_oSubRadioTime0x829.powType   = relayConfig->value("829/powType").toInt();
        p->m_oSubRadioTime0x829.freqBegin = relayConfig->value("829/freqBegin").toInt();
        p->m_oSubRadioTime0x829.freqStep  = relayConfig->value("829/freqStep").toInt();
        p->m_oSubRadioTime0x829.N         = relayConfig->value("829/N").toInt();
        p->m_oSubRadioTime0x829.pow1      = relayConfig->value("829/pow1").toInt();

        rtmVec.append(p);
    }

    // 诱骗设备配置
    auto iTrapNum = relayConfig->value("TrapDevNum/DevNum").toInt();
    for (int i = 1; i <= iTrapNum; i++) {
        auto p = new PRUEModule();
        prueVec.append(p);
    }

    // 启动所有设备
    for (auto& item : rtmVec) {
        item->startup();
    }
}

}