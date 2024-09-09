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

   OSendTrapFixed0xD21 m_oSendTrapFixed0xD21;
   OTrapFunc0xD22 m_oTrapFunc0xD22;
   OTrapBanSectorD01 m_oTrapBanSectorD01;
   OTrapRadiationBan0x202 m_oTrapRadiationBan0x202;
   ORecvTrapFixed0x601 m_oRecvTrapFixed0x601;

   QVector<OTrapBanSector201> m_vecOTrapBanSector201;
   QVector<FreqAndDFreq> m_vecORecvTrapFixed0x601;
   QVector<OTrapFunc0xD22_2> m_vecOTrapFunc0xD22_2;
   
   NavigationInfluence m_NavigationInfluence;
    
 public slots:
   void onReadPRUEData(qint16 pkgID, const QByteArray& buf);
   void stateMachine();
};
}  // namespace NEBULA

#endif // _PRUEMODULE_H_
