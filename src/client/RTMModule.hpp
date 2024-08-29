#ifndef _RTMModule_H_
#define _RTMModule_H_
#include "CommonModule.hpp"

namespace NEBULA
{
class RTMModule : public CommonModule
{
private:
    // 发送RTM配置定时器
    QTimer* pCurrentSettingTimer;
    // 发送RTM功能定时器
    QTimer* pCurrentFunctionTimer;
    // 查看RTM状态定时器
    QTimer* pCurrentStatusTimer;
    // RTM包类型集合
    QSet<qint16> pkgsRTM;
public:
    RTMModule(/* args */);
    ~RTMModule();
    // 接收数据统一接口
    void onRecvData();
    // 查看当前设备状态
    void checkStatus();

    // 0x822,发送方位标记
    void sendBearingMarker();
    // 0x823,发送当前RTM设置，定时器发送
    void sendRTMSettings();
    // 0x825,发送当前RTM功能，注册后发送
    void sendRTMFunction();
    // 0x827,发送方位路线信息，发送json字符串
    void sendBearingAndRoute();
    // 0x828,发送禁止IRI列表，允许定期发送
    void sendForbiddenIRIList();
    // 0x829,发送无线电环境信息，收到无线电环境新消息发送
    void sendWirelessEnvInfo();

    // 0x561,收到更改RTM设置，必须用0x23去响应
    void recvChangingRTMSettings(QByteArray buff);
    // 0x563,请求禁止IRI列表，必须用0x23去响应，如果没有错，还得加上0x828响应
    void recvRequestForbiddenIRIList(QByteArray buff);
    // 0x564,设置禁止IRI列表，必须用0x23去响应
    void recvSettingForbiddenIRIList(QByteArray buff);

public slots:
    // 接收RTM专有协议数据
    void onReadRTMData(QByteArray& buff);
};
}
#endif // _RTMModule_H_