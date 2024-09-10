#ifndef _RTMMODULE_H_
#define _RTMMODULE_H_

#include "ModuleCommon.hpp"

namespace NEBULA {
class RTMModule : public CommonModule {
 private:
    QTimer* m_pSettingTimer823;
    QTimer* m_pStateMachineTimer;
    QThread* m_pSTateMachinethread;
    QSet<qint16> pkgsRTM;
    bool m_isSendRTMFunction825;

 public:
    RTMModule();
    ~RTMModule();
    void init();
    void onRecvData();
    // 发送目标
    void sendTargetMarker822();
    // 设置
    void recvRTMSettings561(const QByteArray& buf);
    void sendRTMSettings823();
    // IRI设置
    void recvSettingIRI564(const QByteArray& buf);
    void sendIRI828();
    // 功能
    void sendRTMFunction825();
    // 调用828
    void recvRequestIRI563(const QByteArray& buf);

    // 设置
    OSetting0x823 m_oSetting0x823;
    QVector<FreqAndDFreq> m_freqs823;
    // 功能
    OFunc0x825 m_oFunc0x825;
    OSetBanIRIlist0x828 m_oSetBanIRIlist0x828;
    QVector<FreqAndDFreq> m_freqs828;

    OTargetMark0x822 m_oTargetMark0x822;

 public slots:
    // 状态机线程执行函数
    void stateMachine();
};
}  // namespace NEBULA
#endif // _RTMMODULE_H_