#ifndef _RTMMODULE_H_
#define _RTMMODULE_H_

#include "ModuleCommon.hpp"

namespace NEBULA {
class RTMModule : public CommonModule {
 private:
    QTimer* m_pCurrentSettingTimer823;
    QTimer* m_pStateMachineTimer;
    QThread* m_pSTateMachinethread;
    QSet<qint16> pkgsRTM;
    bool m_isSendRTMFunction825;

 public:
    RTMModule();
    ~RTMModule();
    void onRecvData();

    void sendBearingMarker822();
    void sendRTMSettings823();
    void sendRTMFunction825();
    void sendForbiddenIRIList828();

    void recvChangingRTMSettings561(const QByteArray& buf);
    void recvRequestForbiddenIRIList563(const QByteArray& buf);
    void recvSettingForbiddenIRIList564(const QByteArray& buf);

    // RTMCustomizedCfg m_rtmCustomizedCfg;
    OBearingMark0x822 m_oBearingMark0x822;
    OSubRezhRTR0x823 m_oSubRezhRTR0x823;
    OSubPosobilRTR0x825 m_oSubPosobilRTR0x825;
    OSetBanIRIlist0x828 m_oSetBanIRIlist0x828;
    QVector<FreqAndDFreq> m_freqs823;
    QVector<FreqAndDFreq> m_freqs828;

 public slots:
    // 状态机线程执行函数
    void stateMachine();
};
}  // namespace NEBULA
#endif // _RTMMODULE_H_