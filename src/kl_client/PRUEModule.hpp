#ifndef _PRUEMODULE_H_
#define _PRUEMODULE_H_

#include "../ModuleCommon.hpp"

namespace NEBULA {
class PRUEModule : public CommonModule {
 private:
    QTimer* pCurrentSettingTimerD21;
    QTimer* pCurrentFunctionTimerD22;
    QTimer* pStateMachineTimer;
    QSet<qint16> pkgsPRUE;

 public:
    PRUEModule();
    ~PRUEModule();
    void onRecvData();
    void stateMachine();

    void sendInstalledBanSectorD01();
    void sendPRUESettingsD21();
    void sendPRUEFunctionD22();
    
    void recvSettingBanSector201(const QByteArray& buff);
    void recvBanRadiation202(const QByteArray& buff);
    void recvUpdatePRUESetting601(const QByteArray& buff);
    
 public slots:
    void onReadPRUEData(qint16 pkgID, const QByteArray& buff);
};
}  // namespace NEBULA

#endif // _PRUEMODULE_H_
