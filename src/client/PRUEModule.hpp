#ifndef _PRUEMODULE_H_
#define _PRUEMODULE_H_

#include "CommonModule.hpp"

namespace NEBULA {
class PRUEModule : public CommonModule {
 private:
    // 发送PRUE配置定时器
    QTimer* pCurrentSettingTimer;
    // 发送PRUE功能定时器
    QTimer* pCurrentFunctionTimer;
    // 查看PRUE状态定时器
    QTimer* pCurrentStatusTimer;
    // PRUE包类型集合
    QSet<qint16> pkgsPRUE;

 public:
    PRUEModule(/* args */);
    ~PRUEModule();
    // 接收数据统一接口
    void onRecvData();
    // 查看当前设备状态
    void checkStatus();

    // 发送当前PRUE设置,0xD21
    void sendPRUESettings();
    // 发送当前PRUE功能,0xD22
    void sendPRUEFunction();
    // 发送已安装的辐射禁止扇区,0xD01
    void sendInstalledBanSector();

    // 收到更改PRUE设置,0x601
    void recvUpdatePRUESetting(const QByteArray& buff);
    // 收到设置辐射禁止扇区,0x201
    void recvSettingBanSector(const QByteArray& buff);
    // 收到设置辐射禁止,0x202
    void recvBanRadiation(const QByteArray& buff);
 public slots:
    // 接收RTM专有协议数据
    void onReadPRUEData(const QByteArray& buff);
};
}  // namespace NEBULA

#endif // _PRUEMODULE_H_
