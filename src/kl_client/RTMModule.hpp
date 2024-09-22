#ifndef _RTMMODULE_H_
#define _RTMMODULE_H_

#include "ModuleBase.hpp"

namespace NEBULA {
class RTMModule : public ModuleBase {
 private:
    QSet<qint16> pkgsRTM;
    QTimer* m_pSettingTimer823;
    QTimer* m_pStateMachineTimer;
    QThread* m_pthread;
    bool m_isSendFunc825;
    bool m_isSendRI828;

 public:
    RTMModule(qint16 id);
    ~RTMModule();
    void startup() override;
    // 设置
    void recvSetting561(const QByteArray& buf);
    void sendSetting823();
    // 功能
    void sendFunc825();
    // 干扰频率
    void recvSettingIRI564(const QByteArray& buf);
    void sendIRI828();
    // 调用828
    void recvReqIRI563(const QByteArray& buf);

    OSetting0x823 m_oSetting0x823;
    QVector<FreqAndDFreq> m_freqs823;
    OFunc0x825 m_oFunc0x825;
    QVector<RTMFuncFreq> m_vecFunc825;
    OSetIRI0x828 m_oSetIRI0x828;
    QVector<FreqAndDFreq> m_freqs828;
 public slots:
    // 状态机线程执行函数
    void stateMachine();
    void onRecvData() override;
     // 发送目标,接受地图nebula发来的数据
    void sendTarget822(OTarget822& oTarget822);
};
}  // namespace NEBULA
#endif // _RTMMODULE_H_