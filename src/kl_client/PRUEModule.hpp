#ifndef _PRUEMODULE_H_
#define _PRUEMODULE_H_

#include "../ModuleCommon.hpp"

namespace NEBULA {
class PRUEModule : public CommonModule {
 private:
   QTimer* m_pCurrentSettingTimerD21;
   QTimer* m_pStateMachineTimer;
   QSet<qint16> pkgsPRUE;

   bool m_isSendInstalledBanSectorD01;
   bool m_isSendPRUEFunctionD22;

 public:
    PRUEModule();
    ~PRUEModule();
    void onRecvData();

    void sendInstalledBanSectorD01();
    void sendPRUESettingsD21();
    void sendPRUEFunctionD22();

    void recvSettingBanSector201(const QByteArray& buf);
    void recvBanRadiation202(const QByteArray& buf);
    void recvUpdatePRUESetting601(const QByteArray& buf);

    OTrapSettings0xD21 m_oTrapSettings0xD21;
    QVector<OTrapSettings0xD21_1> m_vecOTrapSettings0xD21_1;

    OTrapFunc0xD22 m_oTrapFunc0xD22;
    QVector<OTrapFunc0xD22_2> m_vecOTrapFunc0xD22_2;

    ORecvTrapFixed0x601 m_oRecvTrapFixed0x601;
    QVector<FreqAndDFreq> m_vecORecvTrapFixed0x601;
    NavigationInfluence601 m_NavigationInfluence601;

    OTrapBanSectorD01 m_oTrapBanSectorD01;
    OTrapRadiationBan0x202 m_oTrapRadiationBan0x202;
    QVector<OTrapBanSector201> m_vecOTrapBanSector201;
   
    
    
 public slots:
   void stateMachine();
};
}  // namespace NEBULA

#endif // _PRUEMODULE_H_
