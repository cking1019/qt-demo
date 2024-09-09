#include "ModuleController.hpp"

namespace NEBULA {

ModuleController::ModuleController() {
}

ModuleController::~ModuleController() {
}

void ModuleController::init() {
    auto commCfg = new QSettings(COMM_CFG, QSettings::IniFormat);
    auto rtmCfg = new QSettings(RTM_CFG, QSettings::IniFormat);
    auto prueCfg = new QSettings(PRUE_CFG, QSettings::IniFormat);
    commCfg->setIniCodec(QTextCodec::codecForName("utf-8"));

    auto serverAddress = commCfg->value("common/serverIP").toString();
    auto serverPort = commCfg->value("common/serverPort").toInt();
    auto isDebugout = commCfg->value("common/debugOut").toBool();
    qDebug() << "server address is " << serverAddress;
    qDebug() << "server port is " << serverPort;

    // 侦测设备配置
    auto iDetectNum = rtmCfg->value("DetectDevNum/num").toInt();
    for (int i = 1; i <= iDetectNum; i++) {
        auto p = new RTMModule();
        p->m_commCfg.serverAddress  = serverAddress;
        p->m_commCfg.serverPort     = serverPort;
        p->m_commCfg.moduleAddress  = rtmCfg->value("common/clientIP").toString();
        p->m_commCfg.modulePort     = rtmCfg->value("common/clientPort").toInt();
        p->m_commCfg.moduleCfg20    = readJson(rtmCfg->value("common/devconfig20").toString());
        p->m_isDebugOut             = isDebugout;

        p->m_genericHeader.sender   = rtmCfg->value("common/sender").toInt();
        p->m_genericHeader.moduleId = rtmCfg->value("common/moduleId").toInt();
        p->m_genericHeader.vMajor   = rtmCfg->value("common/vMajor").toInt();
        p->m_genericHeader.vMinor   = rtmCfg->value("common/vMinor").toInt();
        p->m_genericHeader.isAsku   = rtmCfg->value("common/isAsku").toInt();

        p->m_oBearingMark0x822.idxCeilVOI  = rtmCfg->value("822/idxCeilVOI").toInt();
        p->m_oBearingMark0x822.idxCeilSPP  = rtmCfg->value("822/idxCeilSPP").toInt();
        p->m_oBearingMark0x822.idxPoint    = rtmCfg->value("822/idxPoint").toInt();
        p->m_oBearingMark0x822.typeCeilSPP = rtmCfg->value("822/typeCeilSPP").toInt();
        p->m_oBearingMark0x822.typeChannel = rtmCfg->value("822/typeChannel").toInt();
        p->m_oBearingMark0x822.typeSignal  = rtmCfg->value("822/typeSignal").toInt();
        p->m_oBearingMark0x822.azim        = rtmCfg->value("822/azim").toInt();
        p->m_oBearingMark0x822.elev        = rtmCfg->value("822/elev").toFloat();
        p->m_oBearingMark0x822.range       = rtmCfg->value("822/range").toInt();
        p->m_oBearingMark0x822.freqMhz     = rtmCfg->value("822/freqMhz").toInt();
        p->m_oBearingMark0x822.dFreqMhz    = rtmCfg->value("822/dFreqMhz").toInt();
        p->m_oBearingMark0x822.Pow_dBm     = rtmCfg->value("822/Pow_dBm").toInt();
        p->m_oBearingMark0x822.SNR_dB      = rtmCfg->value("822/SNR_dB").toInt();

        p->m_oSubRezhRTR0x823.N = rtmCfg->value("823/N").toInt();
        p->m_oSubRezhRTR0x823.curAz = rtmCfg->value("823/curAz").toInt();
        float freqMhz = rtmCfg->value("823/freqMhz").toFloat();
        float dFreqMhz = rtmCfg->value("823/dFreqMhz").toFloat();
        p->m_freqs823 = {{freqMhz, dFreqMhz}};

        p->m_oSubPosobilRTR0x825.isRotate   = rtmCfg->value("825/isRotate").toInt();
        p->m_oSubPosobilRTR0x825.maxTasks   = rtmCfg->value("825/maxTasks").toInt();
        p->m_oSubPosobilRTR0x825.numDiap    = rtmCfg->value("825/numDiap").toInt();
        p->m_oSubPosobilRTR0x825.dAz        = rtmCfg->value("825/dAz").toInt();
        p->m_oSubPosobilRTR0x825.dElev      = rtmCfg->value("825/dElev").toInt();
        p->m_oSubPosobilRTR0x825.minFreqRTR = rtmCfg->value("825/minFreqRTR").toInt();
        p->m_oSubPosobilRTR0x825.maxFreqRTR = rtmCfg->value("825/maxFreqRTR").toInt();

        p->m_oSetBanIRIlist0x828.NIRI = rtmCfg->value("828/NIRI").toInt();
        float freqMhz2  = rtmCfg->value("828/freqMhz").toFloat();
        float dFreqMhz2 = rtmCfg->value("828/dFreqMhz").toFloat();
        p->m_freqs828   = {{freqMhz2, dFreqMhz2}};

        rtmVec.append(p);
    }

    // 诱骗设备配置
    auto iTrapNum = prueCfg->value("JamDevNum/num").toInt();
    for (int i = 1; i <= iTrapNum; i++) {
        auto p = new PRUEModule();
        p->m_commCfg.serverAddress  = serverAddress;
        p->m_commCfg.serverPort     = serverPort;
        p->m_commCfg.moduleAddress  = prueCfg->value("common/clientIP").toString();
        p->m_commCfg.modulePort     = prueCfg->value("common/clientPort").toInt();
        p->m_commCfg.moduleCfg20    = readJson(prueCfg->value("common/devconfig20").toString());
        p->m_isDebugOut             = isDebugout;

        p->m_oSendTrapFixed0xD21.taskREB   = prueCfg->value("D21/taskREB").toInt();
        p->m_oSendTrapFixed0xD21.taskGeo   = prueCfg->value("D21/taskGeo").toInt();
        p->m_oSendTrapFixed0xD21.curAzREB  = prueCfg->value("D21/curAzREB").toFloat();
        p->m_oSendTrapFixed0xD21.curEpsREB = prueCfg->value("D21/curEpsREB").toFloat();
        p->m_oSendTrapFixed0xD21.kGainREB  = prueCfg->value("D21/kGainREB").toFloat();

        p->m_oTrapFunc0xD22.numDiap      = prueCfg->value("D22/numDiap").toInt();
        p->m_oTrapFunc0xD22.isGeo        = prueCfg->value("D22/isGeo").toInt();
        p->m_oTrapFunc0xD22.numDiap2     = prueCfg->value("D22/numDiap2").toInt();
        p->m_oTrapFunc0xD22.dTgeo        = prueCfg->value("D22/dTgeo").toInt();
        p->m_oTrapFunc0xD22.maxPowREB    = prueCfg->value("D22/maxPowREB").toFloat();
        p->m_oTrapFunc0xD22.dAzREB       = prueCfg->value("D22/dAzREB").toFloat();
        p->m_oTrapFunc0xD22.dElevREB     = prueCfg->value("D22/dElevREB").toFloat();
        p->m_oTrapFunc0xD22.azMinREB     = prueCfg->value("D22/azMinREB").toFloat();
        p->m_oTrapFunc0xD22.azMaxREB     = prueCfg->value("D22/azMaxREB").toFloat();
        p->m_oTrapFunc0xD22.dAzGeo       = prueCfg->value("D22/dAzGeo").toFloat();
        p->m_oTrapFunc0xD22.dElevGeo     = prueCfg->value("D22/dElevGeo").toFloat();
        p->m_oTrapFunc0xD22.azMinGeo     = prueCfg->value("D22/azMinGeo").toFloat();
        p->m_oTrapFunc0xD22.azMaxGeo     = prueCfg->value("D22/azMaxGeo").toFloat();
        
        prueVec.append(p);
    }

    // 启动所有设备
    for (auto& item : rtmVec) {
        item->startup();
    }
    for (auto& item : prueVec) {
        // item->startup();
    }
}

}