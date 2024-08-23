#ifndef _COMMONHEADER_H_
#define _COMMONHEADER_H_

enum ModuleDealInfo {
    moduleRegister    = 0x1, // 请求在系统中注册模块
    moduleTimeControl = 0x3, // 时间请求
    moduleGeoLocation = 0x5, // 模块地址请求
    moduleRouteMarker = 0x9, // 模块路线标记 

    moduleDiagram     = 0x20, // 模块图，只有发送该消息后才能被控制，发送模块组件及其参数
    moduleParaStatus  = 0x21, // 电路元素变化
    moduleControlledParaAndStatus = 0x22, // 受控参数和状态
    moduleControlledOrder = 0x23,         // 模块受控状态，每秒发送一次
    moduleStatus = 0x24,                  // 每秒发送模块状态
    moduleSendLog = 0x25,                 // 发送消息日志
    moduleSendMsg = 0x26,                 // 发送消息给操作员
    moduleExtendedOrder = 0x27,           // 发送扩展控制命令
    moduleCustomedPara = 0x28             // 发送电路元件自定义参数的值
};

enum ServerDealInfo {
    serverRegister    = 0x2, // 确定模块在系统中的注册
    serverTimeControl = 0x4, // 服务器的时间戳
    serverRouteMarker = 0x7, // 服务器路线标记

    serverStart = 0x40,   // 启动
    serverClose = 0x41,   // 关闭
    serverRestart = 0x42, // 重启
    serverReset = 0x43,   // 重设
    serverAlter = 0x44,   // 修改
    serverSendNote = 0x45,// 发送消息
};
// 通用头
struct GenericHeader
{
    uint32_t sender   : 24;  // 发送者ID
    uint32_t moduleId : 8;   // 模块ID
    uint32_t vMajor   : 8;   // 主协议版本
    uint32_t vMinor   : 8;   // 次协议版本
    uint32_t packIdx  : 16;  // 端对端包序列号
    uint32_t dataSize : 31;  // 数据块大小
    uint32_t isAsku   : 1;   // 应答标识
    uint32_t packType : 16;  // 包类型编号
    uint32_t checkSum : 16;  // 头校验和
};
// 0x01
struct ModuleRegister {
    uint32_t idManuf     : 8; // 制造商标识符
    uint32_t serialNum   : 24;// 产品序列号
    uint32_t versHardMaj : 8; // 产品修改版本(大)
    uint32_t versHardMin : 8; // 产品修改版本(小)
    uint32_t versProgMaj : 7; // 软件版本
    uint32_t isInfo      : 1; // 信息处理标志
    uint32_t versProgMin : 7; // 软件版本
    uint32_t isAsku      : 1; // 控制和管理标志
};

// 0x03
struct ModuleTimeControl {
    uint32_t timeRequest1     : 32; // 查询时间戳小字节
    uint32_t timeRequest2     : 32; // 请求时间戳高级字节
};

// 0x05
struct ModuleGeoLocation {
    uint32_t typeData     : 3;  // 定位数据类型
    uint32_t isValid      : 1;  // 数据可靠性状况
    uint32_t reserve      : 28; // 备用字段（未使用）
    float    xLat;              // 模块站点坐标
    float    yLong;
    float    zHeight;
};

// 0x02
struct ServerRegister
{
    uint32_t idxModule    : 8;  // 综合体中的模块号
    uint32_t errorConnect : 8;  // 注册错误状态
    uint32_t reserve      : 16; // 备用字段（未使用）
};

// 0x04
struct ServerTimeControl {
    uint32_t timeRequest1     : 32; // 查询时间戳小字节
    uint32_t timeRequest2     : 32; // 请求时间戳高级字节
    uint32_t timeAnswer1      : 32; // 响应时间戳小字节
    uint32_t timeAnswer2      : 32; // 响应时间戳高级字节
};


#endif // _COMMONHEADER_H_