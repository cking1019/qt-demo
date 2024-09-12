#ifndef _PRUEMODULE_H_
#define _PRUEMODULE_H_

#include "ModuleBase.hpp"

namespace NEBULA {
class PRUEModule : public ModuleBase {
 private:
   QTimer* m_pCurrentSettingTimerD21;
   QTimer* m_pStateMachineTimer;
   QSet<qint16> pkgsPRUE;

   bool m_isSendInstalledBanSectorD01;
   bool m_isSendPRUEFunctionD22;

 public:
    PRUEModule();
    ~PRUEModule();
    void startup() override;

    void sendInstalledBanSectorD01();
    void recvSettingBanSector201(const QByteArray& buf);
    // 发送配置信息
    void sendPRUESettingsD21();
    // 发送功能信息
    void sendPRUEFunctionD22();
    
    void recvBanRadiation202(const QByteArray& buf);
    // 接收更改设置
    void recvUpdatePRUESetting601(const QByteArray& buf);

    // 设置
    OTrapSettings0xD21 m_oTrapSettings0xD21;
    QVector<OTrapSettings0xD21_1> m_vecOTrapSettings0xD21_1;
    // 功能
    OTrapFunc0xD22 m_oTrapFunc0xD22;
    QVector<OTrapFunc0xD22_2> m_vecOTrapFunc0xD22_2;

    // 接收设置更改
    QVector<FreqAndDFreq> m_vecORecvSetting0x601;
    NavigationInfluence601 m_NavigationInfluence601;
    // 接收禁止扇区
    OTrapBanSectorD01 m_oTrapBanSectorD01;
    QVector<OTrapBanSectorD01_1> m_vecOTrapBanSectorD01_1;

    OTrapRadiationBan0x202 m_oTrapRadiationBan0x202;
    
    
 public slots:
   void stateMachine();
   void onRecvData();
};
}  // namespace NEBULA

#endif // _PRUEMODULE_H_
