#ifndef _COMMONHEADER_H_
#define _COMMONHEADER_H_

#pragma pack(push, 4)

enum ModuleDealInfo {
    moduleRegister    = 0x1, // 请求在系统中注册模块
    moduleTimeControl = 0x3, // 时间请求
    moduleGeoLocation = 0x5, // 模块地址请求

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

// 0x1
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

// 0x2
struct ServerRegister
{
    uint32_t idxModule    : 8;  // 综合体中的模块号
    uint32_t errorConnect : 8;  // 注册错误状态
    uint32_t reserve      : 16; // 备用字段（未使用）
};

// 0x3
struct ModuleTimeControl {
    uint32_t timeRequest1     : 32; // 查询时间戳小字节
    uint32_t timeRequest2     : 32; // 请求时间戳高级字节
};

// 0x4
struct ServerTimeControl {
    uint64_t timeRequest1     : 64; // 查询时间戳小字节
    // uint32_t timeRequest2     : 32; // 请求时间戳高级字节
    uint64_t timeAnswer1      : 64; // 响应时间戳小字节
    // uint32_t timeAnswer2      : 32; // 响应时间戳高级字节
};

// 0x5
struct ModuleGeoLocation {
    uint32_t typeData     : 3;  // 定位数据类型
    uint32_t isValid      : 1;  // 数据可靠性状况
    uint32_t reserve      : 28; // 备用字段（未使用）

    float    xLat;              // 模块站点坐标
    float    yLong;
    float    zHeight;
};

// 0x21,设备np状态
struct ONPStatus
{
    quint32 IDElem  : 16;
    quint32 status  : 4;
    quint32 workF1  : 4;
    quint32 local   : 1;
    quint32 isImit  : 1;
    quint32 reserve : 6;
};

// 0x22,设备cp状态
struct OCPStatus
{
    quint32 IDParam : 32;

    quint32 status  : 8;
    quint32 size    : 8;
    quint32 isNewStatus : 1;
    quint32 isNewValue  : 1;
    quint32 reserve     : 14;
};

// 0x23,控制指令
struct OReqCtl{
    GenericHeader o_Header;
    quint32 n_TimeReq1;
    quint32 n_TimeReq2;
    quint32 n_id_Com:16;
    quint32 n_code:16;
};

// 0x24,设备状态
struct OModuleStatus
{
    quint32 time1;
    quint32 time2;

    quint32 status : 3;
    quint32 work   : 3;
    quint32 isRGDV : 1;
    quint32 isRAF  : 1;
    quint32 isLocal : 1;
    quint32 isImit  : 1;
    quint32 hasTP : 1;
    quint32 isTP  : 1;
    quint32 isWP  : 1;
    quint32 isTPValid : 1;
    quint32 isWpValid : 1;
    quint32 statusTwp : 1;
    quint32 mode : 16;

    quint32 reserve;
};

// 0x25,消息日志
struct LogMsg
{
    quint16 IDParam : 16;
    quint8 type : 8;
    quint8 reserve : 8;
};

// 消息体的头
struct OTimeReq{
    GenericHeader o_Header;
    quint32 n_TimeReq1;
    quint32 n_TimeReq2;
};


// 0x40
struct ServerStart {
    uint32_t flag;
};

// 0x41
struct ServerClose {
    uint32_t flag;
};

// 0x42
struct ServerRestart {
    uint32_t flag;
};

// 0x43
struct ServerReset {
    uint32_t flag;
};

// 0x44
struct ServerUpdate {
    uint32_t flag;
};

// 0x822，方位标记
struct OBearingMark
{
    GenericHeader o_Header;
    quint32 idxCeilVOI:16;
    quint32 iReserve:16;
    quint32 idxCeilSPP;
    quint32 idxPoint:8;
    quint32 typeCeilSPP:8;
    quint32 typeChannel:8;
    quint32 typeSignal:8;
    quint32 timePel1;
    quint32 timePel2;
    float azim;
    float elev;
    float range;
    float freqMhz;
    float dFreqMhz;
    float Pow_dBm;
    float SNR_dB;
};

// 0x823，RTM设置
struct OSubRezhRTR20 {
    // 1
    quint32 n_Cnt:8;
    quint32 n_Reserv:24;
    // 2
    float f_CurAz;
};

// 0x825,RTM功能
struct OSubPosobilRTR22 {
   quint32 n_IsRotate:1;
   quint32 n_MaxTask:7;
   quint32 n_MaxSubDiap:8;
   quint32 n_Rezerv:16;
   float f_AzSize;
   float f_EpsSize;
};

// 0x828，禁止IRI列表
struct OBanIRI {
  float f_Freq;
  float f_DelFreq;
  uint n_Numb;
  OBanIRI() {
    f_Freq = f_DelFreq = 0.f;
    n_Numb = 0;
  }
  OBanIRI(uint nNumb, float fFreq, float fDelFreq) {
      f_Freq = fFreq;
      f_DelFreq = fDelFreq;
      n_Numb = nNumb;
  }
};

// 0x829，当前无线电环境信息
struct OSubRadioTime {
    qint64 n_Time;
    quint32 n_Num:24;
    quint32 n_Type:8;
    float f_FrBegin;
    float f_FrStep;
    uint n_Cnt;
};



#pragma pack(pop)
#endif // _COMMONHEADER_H_