#ifndef _RTMMODULE_H_
#define _RTMMODULE_H_

#include "ModuleBase.hpp"

namespace NEBULA {
class RTMModule : public ModuleBase {
 private:
    QTimer* m_pSettingTimer823;
    QTimer* m_pStateMachineTimer;
    QThread* m_pSTateMachinethread;
    QSet<qint16> pkgsRTM;
    // RTM功能需要定期发送
    bool m_isSendRTMFunction825;
    bool m_isSendForbiddenIRIList828;

 public:
    RTMModule(qint16 id);
    ~RTMModule();
    void startup() override;
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
    void processTask();
     // 发送目标,接受地图nebula发来的数据
    void sendTargetMarker822(OTargetMark0x822 oTargetMark0x822);
};
}  // namespace NEBULA
#endif // _RTMMODULE_H_