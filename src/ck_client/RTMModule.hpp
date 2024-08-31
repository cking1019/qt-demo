#ifndef _RTMMODULE_H_
#define _RTMMODULE_H_

#include "CommonModule.hpp"

namespace NEBULA {
class RTMModule : public CommonModule {
 private:
    QTimer* pCurrentSettingTimer823;
    QTimer* pCurrentFunctionTimer825;
    QTimer* pStateMachineTimer;
    QSet<qint16> pkgsRTM;
 public:
    RTMModule();
    ~RTMModule();
    void onRecvData();
    void stateMachine();

    void sendBearingMarker822();
    void sendRTMSettings823();
    void sendRTMFunction825();
    void sendBearingAndRoute827();
    void sendForbiddenIRIList828();
    void sendWirelessEnvInfo829();

    void recvChangingRTMSettings561(const QByteArray& buff);
    void recvRequestForbiddenIRIList563(const QByteArray& buff);
    void recvSettingForbiddenIRIList564(const QByteArray& buff);

 public slots:
    void onReadRTMData(QByteArray& buff);
};
}  // namespace NEBULA
#endif // _RTMMODULE_H_