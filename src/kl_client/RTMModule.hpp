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
    // RTM功能需要定期发送
    bool m_isSendRTMFunction825;

 public:
    RTMModule();
    ~RTMModule();
    void startup() override;
    // 发送目标,由地图nebula发数据过来
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
    // IRI
    OSetBanIRIlist0x828 m_oSetBanIRIlist0x828;
    QVector<FreqAndDFreq> m_freqs828;

 public slots:
    // 状态机线程执行函数
    void stateMachine();
    void onRecvData();
};
}  // namespace NEBULA
#endif // _RTMMODULE_H_