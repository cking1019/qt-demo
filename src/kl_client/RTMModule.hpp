#ifndef _RTMMODULE_H_
#define _RTMMODULE_H_

#include "ModuleCommon.hpp"

namespace NEBULA {
class RTMModule : public CommonModule {
 private:
    QTimer* pCurrentTargetTimer822;
    QTimer* pCurrentSettingTimer823;
    QTimer* pCurrentFunctionTimer825;
    QTimer* pStateMachineTimer;
    QThread* pSTateMachinethread;
    QSet<qint16> pkgsRTM;
    

 public:
    RTMModule();
    ~RTMModule();
    void onRecvData();
    

    void sendBearingMarker822();
    void sendRTMSettings823();
    void sendRTMFunction825();
    void sendBearingAndRoute827();
    void sendForbiddenIRIList828();
    void sendWirelessEnvInfo829();

    void recvChangingRTMSettings561(const QByteArray& buf);
    void recvRequestForbiddenIRIList563(const QByteArray& buf);
    void recvSettingForbiddenIRIList564(const QByteArray& buf);

    // RTMCustomizedCfg m_rtmCustomizedCfg;
    OBearingMark0x822 m_oBearingMark0x822;
    OSubRezhRTR0x823 m_oSubRezhRTR0x823;
    OSubPosobilRTR0x825 m_oSubPosobilRTR0x825;
    OSetBanIRIlist0x828 m_oSetBanIRIlist0x828;
    OSubRadioTime0x829 m_oSubRadioTime0x829;
    QVector<QVector<float>> m_freqs823;
    QVector<QVector<float>> m_freqs828;

 public slots:
    void onReadRTMData(qint16 pkgID, QByteArray& buf);
    // 状态机线程执行函数
    void stateMachine();
};
}  // namespace NEBULA
#endif // _RTMMODULE_H_