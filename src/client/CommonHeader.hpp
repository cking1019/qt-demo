#ifndef _COMMONHEADER_H_
#define _COMMONHEADER_H_

#pragma pack(push, 4)

enum ModuleDealInfo {
    moduleRegister    = 0x1,  // 请求在系统中注册模块
    moduleTimeControl = 0x3,  // 时间请求
    moduleGeoLocation = 0x5,  // 模块地址请求

    moduleDiagram     = 0x20,  // 模块图，只有发送该消息后才能被控制，发送模块组件及其参数
    moduleParaStatus  = 0x21,  // 电路元素变化
    moduleControlledParaAndStatus = 0x22,  // 受控参数和状态
    moduleControlledOrder = 0x23,          // 模块受控状态，每秒发送一次
    moduleStatus = 0x24,                   // 每秒发送模块状态
    moduleSendLog = 0x25,                  // 发送消息日志
    moduleSendMsg = 0x26,                  // 发送消息给操作员
    moduleExtendedOrder = 0x27,            // 发送扩展控制命令
    moduleCustomedPara = 0x28              // 发送电路元件自定义参数的值
};

enum ServerDealInfo {
    serverRegister    = 0x2,  // 确定模块在系统中的注册
    serverTimeControl = 0x4,  // 服务器的时间戳

    serverStart = 0x40,     // 启动
    serverClose = 0x41,     // 关闭
    serverRestart = 0x42,   // 重启
    serverReset = 0x43,     // 重设
    serverAlter = 0x44,     // 修改
    serverSendNote = 0x45,  // 发送消息
};
// 通用头,424552ff 00000000 00000000 00000000
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
    uint32_t idManuf     : 8;   // 制造商标识符
    uint32_t serialNum   : 24;  // 产品序列号

    uint32_t versHardMaj : 8;  // 产品修改版本(大)
    uint32_t versHardMin : 8;  // 产品修改版本(小)
    uint32_t versProgMaj : 7;  // 软件版本
    uint32_t isInfo      : 1;  // 信息处理标志
    uint32_t versProgMin : 7;  // 软件版本
    uint32_t isAsku      : 1;  // 控制和管理标志
};

// 0x2
struct ServerRegister
{
    uint32_t idxModule    : 8;   // 综合体中的模块号
    uint32_t errorConnect : 8;   // 注册错误状态
    uint32_t reserve      : 16;  // 备用字段（未使用）
};

// 0x3
struct ModuleTimeControl {
    uint32_t timeRequest1     : 32;  // 查询时间戳小字节
    uint32_t timeRequest2     : 32;  // 请求时间戳高级字节
};

// 0x4
struct ServerTimeControl {
    uint64_t timeRequest1     : 64;  // 查询时间戳小字节
    // uint32_t timeRequest2     : 32; // 请求时间戳高级字节
    uint64_t timeAnswer1      : 64;  // 响应时间戳小字节
    // uint32_t timeAnswer2      : 32; // 响应时间戳高级字节
};

// 0x5
struct ModuleGeoLocation {
    uint32_t typeData     : 3;   // 定位数据类型
    uint32_t isValid      : 1;   // 数据可靠性状况
    uint32_t reserve      : 28;  // 备用字段（未使用）

    float    xLat;               // 模块站点坐标
    float    yLong;
    float    zHeight;
};

// 0x21,设备np状态
struct ONPStatus {
    uint32_t time1;
    uint32_t time2;

    uint32_t IDElem  : 16;
    uint32_t status  : 4;
    uint32_t workF1  : 4;
    uint32_t local   : 1;
    uint32_t isImit  : 1;
    uint32_t reserve : 6;
};

// 0x22,设备cp状态
struct OCPStatus {
    uint32_t time1;
    uint32_t time2;

    uint32_t IDParam : 32;

    uint32_t status  : 8;
    uint32_t size    : 8;
    uint32_t isNewStatus : 1;
    uint32_t isNewValue  : 1;
    uint32_t reserve     : 14;
};

// 0x23,控制指令;0x27,扩展指令
struct OReqCtl {
    uint32_t n_TimeReq1;
    uint32_t n_TimeReq2;

    uint32_t n_id_Com:16;
    uint32_t n_code:16;
};

// 0x24,设备状态
struct OModuleStatus {
    uint32_t time1;
    uint32_t time2;

    uint32_t status : 3;
    uint32_t work   : 3;
    uint32_t isRGDV : 1;
    uint32_t isRAF  : 1;
    uint32_t isLocal : 1;
    uint32_t isImit  : 1;
    uint32_t hasTP : 1;
    uint32_t isTP  : 1;
    uint32_t isWP  : 1;
    uint32_t isTPValid : 1;
    uint32_t isWpValid : 1;
    uint32_t statusTwp : 1;
    uint32_t mode : 16;

    uint32_t reserve;
};

// 0x25,消息日志
struct LogMsg {
    uint32_t time1;
    uint32_t time2;

    quint16 IDParam : 16;
    quint8 type : 8;
    quint8 reserve : 8;
};


// 时间头
struct OTimeReq {
    uint32_t time1;
    uint32_t time2;
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


// ==================RTM===========================================
// 0x822，方位标记
struct OBearingMark
{
    GenericHeader header;
    uint32_t idxCeilVOI:16;
    uint32_t iReserve:16;
    uint32_t idxCeilSPP;
    uint32_t idxPoint:8;
    uint32_t typeCeilSPP:8;
    uint32_t typeChannel:8;
    uint32_t typeSignal:8;
    uint32_t timePel1;
    uint32_t timePel2;
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
    uint32_t n_Cnt:8;
    uint32_t n_Reserv:24;
    // 2
    float f_CurAz;
};

// 0x825,RTM功能 52454201 02022f00 40000000 25087a01 00000000 00000000 00000000 f87f0000 00000000
struct OSubPosobilRTR22 {
    uint32_t n_IsRotate:1;
    uint32_t n_MaxTask:7;
    uint32_t n_MaxSubDiap:8;
    uint32_t n_Rezerv:16;
    float f_AzSize;
    float f_EpsSize;
    float f_maxBand;
    float f_minBand;
};

// 0x561,更改RTM设置
struct OUpdateRTMSetting {
    uint32_t n_range : 8;
    uint32_t rezerv  : 24;
    float f_Freq;
    float f_DelFreq;
};

// 0x564,设置禁用IRI列表
struct OSetBanIRIlist {
    uint32_t n_trans : 8;
    uint32_t rezerv  : 24;
    float f_Freq;
    float f_DelFreq;
};

// 0x828，禁止IRI列表
struct OBanIRI {
    uint32_t n_Numb : 16;
    uint32_t rezerv : 16;

    float f_Freq;
    float f_DelFreq;
};

// 0x829，当前无线电环境信息
struct OSubRadioTime {
    uint64_t n_Time;

    uint32_t n_Num:24;
    uint32_t n_Type:8;

    float f_FrBegin;
    float f_FrStep;

    uint8_t n_Cnt;
};



// ==================PRUE===========================================
// 0xD21,发送当前PRUE设置
struct OSendTrapFixed {
    uint32_t taskREB:8;
    uint32_t taskGeo:8;
    uint32_t reserve:16;

    float curAzREB;
    float curEpsREB;
    float kGainREB;
};

// 0xD22,发送当前PRUE功能
struct OTrapFunc {
    uint32_t numDiap : 8;
    uint32_t isGeo : 1;
    uint32_t numDiap2 : 8;
    uint32_t reserve : 7;
    uint32_t dTgeo : 8;

    float maxPowREB;
    float dAzREB;
    float dElevREB;
    float azMinREB;
    float azMaxREB;
    float dAzGeo;
    float dElevGeo;
    float azMinGeo;
    float azMaxGeo;
    float minFreqREB;
    float maxFreqREB;
    float maxDFreq;
};

// 0xD01 & 0x201,发送已安装的辐射禁止扇区或接收设置辐射禁止扇区
struct OTrapBanSector {
    uint32_t time1;
    uint32_t time2;

    uint32_t num     : 8;
    uint32_t reserve : 24;

    uint32_t type      : 1;
    uint32_t isUse     : 1;
    uint32_t isUseEps  : 1;
    uint32_t isUseFrep : 1;
    uint32_t reserve2   : 28;

    float AzBegin;
    float AzEnd;
    float EpsBegin;
    float EpsEnd;
    float Freq;
    float delFreq;
};

// 0x202,接收设置辐射禁止
struct OTrapRadiationBan {
    uint32_t time1;
    uint32_t time2;
    uint32_t isOn : 1;
    uint32_t reserve : 31;
};

// 0x601,接收更改当前PRUE设置
struct ORecvTrapFixed
{
    uint32_t i_Num:8;
    uint32_t taskREB:8;
    uint32_t reserve:16;
    float azREB;
    float elevREB;
    float kGainREB;
};

#pragma pack(pop)
#endif  // _COMMONHEADER_H_