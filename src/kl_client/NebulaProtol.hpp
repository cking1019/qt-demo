#ifndef __NEBULAPROTOL_H__
#define __NEBULAPROTOL_H__
#include <QTime>
#include <iostream>
#include <Vector>

namespace NEBULA
{

enum DeviceType
{
    radioDevice = 0,
    radarDevice,
};
#pragma pack(1)

struct MyTimeStamp
{
    uint16_t wyear;
    uint8_t wMonth;
    uint8_t wDay;
    uint8_t wHour;
    uint8_t wMin;
    uint8_t wSec;
    uint16_t wMillsec;
};



//侦测设备状态帧
//帧头+数据部分+帧尾
struct DetectHead
{
    int uiStart;              //开始标志
    short uiVersion;          //协议版本
    int uiLen;                //数据帧长度
    MyTimeStamp stcTimeStamp; //时间
    short    sType;           //包类别
    uint64_t uiPackNum;       //包编号
};

struct DetectTail
{
    char uiSum;//校验
    int uiStop;//结束标志
};

struct DetectStatus
{
    int iDevID;        //设备ID
    char devName[20];  //设备名称
    float flon;        //设备经度
    float flat;        //设备纬度
    int   iHeight;     //设备海拔
    short sDevWorkStat;    //设备状态
    int   iAmith;      //方位角
    char  cDevType[4]; //设备类别
    char  cDevStatusInfo[4]; //设备工作状态信息
    int   iRang;//天线覆盖范围
    int   iRcvID;//接收设备ID
    int   iDevTem;//设备温度
    int   iWet;//设备湿度
    int   iPara;//处理参数
    int   iStatID;
};

//侦测设备状态帧


//雷达设备状态帧

struct RadarDeviceStateYF
{
    int head;
    int radarID;                  ///< 雷达ID
    uint8_t radarConState;        ///< 雷达连接状态：连接和断开
    uint8_t radarSwitchState;     ///< 雷达激励状态: 0待机/关闭灰 1搜索/开启绿 2异常红 3告警黄
    uint8_t radarFpgaState;       ///< 雷达一体机在线状态/数处 0断开灰 1正常绿 2异常红 3告警黄
    uint8_t radarFreqsyntState;   ///< 雷达前端在线状态/信处 0断开灰 1正常绿 2异常红 3告警黄
    uint8_t connMsg;              ///< 连接告警与异常信息 conn_msg
    uint8_t dataMsg;              ///< 数处告警与异常信息 data_msg
    uint8_t signalMsg;            ///< 信处告警与异常信息 signal_msg
    double radarLon;              ///
    double radarLat;              ///< 纬度
    float radarHeight;            ///< 高度
    float radarAzimuth;           ///< 方位
    float radarPitch;             ///< 俯仰
    float radarAmendAngel;        ///< 偏北补偿角
    QTime mRecordTime;            ///< 收到数据时间
    uint16_t radarSpeedMin;       ///< 速度下限
    uint16_t radarHeightMin;      ///< 高度下限
    uint16_t radarStartAngle;     ///< 起始角度
    uint16_t radarEndAngle;       ///< 终止角度
};                ///< 雷达设备状态




//雷达设备状态帧

//全向干扰状态帧
struct OmnidirectionalJammingDevicesStatus
{
    uint32_t uiHead;            //帧头0x22222222
    uint8_t  uiOperateType;     //操作类型：0连接 1状态回复 2指令控制
    uint8_t  uiIp[4];              //设备IP
    uint8_t  uiChannelStatus[16];   //对应8个通道
};


struct OmnidirectionalJammingDevicesCtl
{
    uint32_t uiHead;            //帧头0x22222222
    uint8_t  uiOperateType;     //操作类型：0连接 1状态回复 2指令控制
    uint8_t  uiIp[4];              //设备IP
    uint8_t  uiChannelStatus[16];   //对应8个通道
    uint32_t uiOpenTime[16];
};

//全向干扰状态帧


//定向干扰状态帧
struct DirectionalJPackHead
{
    uint32_t head;
    uint16_t jamVersion;             ///< 版本
    uint8_t jamID;                   ///< 设备ID
    uint8_t jamServeType;            ///< 服务类型
    uint32_t jamLen;                 ///< 数据长度
    uint32_t jamCheckSum;            ///< 校验码
    uint64_t jamToken;               ///< 令牌
    uint64_t jamTimeStamp;           ///< 时间戳
};  //32字节


struct DirectionalJammerSigSourceState
{
    DirectionalJPackHead packHead;
    uint8_t jamSigSourState;         ///< 信号源状态
    uint8_t jamCurrentSigType;       ///< 当前信号类型
    uint8_t jamBandNum;              ///< 频段个数
    uint32_t jamFre[8];              ///< 频率
    uint32_t jamBandWidth[8];        ///< 带宽
    uint16_t jamPowerFade;           ///< 功率衰减
    uint8_t jamFreLock;              ///< 频率锁定
    uint8_t jamChannelNum;           ///< 通道号
    uint32_t jamRemainedDuration;    ///< 剩余开启时长
    uint8_t jamReserve;              ///< 保留字
    uint8_t jamPowerAmpState;        ///< 功放状态
    uint8_t jamPowerAmpWarn;         ///< 功放告警
    uint8_t jamTemperatueWarn;       ///< 温度告警
    int32_t jamPowerAmpTemp;         ///< 功放温度
};

struct DirectionalJammerTurnableState
{
    DirectionalJPackHead packHead;
    uint8_t jamTurnableState;        ///< 状态
    int32_t jamAzimuth;              ///< 方位角
    int32_t jamPitch;                ///< 俯仰角
    uint8_t jamSpeed;                ///< 速度
};

struct DirectionalJammerTurnTableControl
{
    DirectionalJPackHead packHead;
    int8_t mCtrlType;       ///< 控制类型
    int8_t mAzimuthFlag;    ///< 方位角有效
    int8_t mPitchFlag;      ///< 俯仰角有效
    int32_t mAzimuth;       ///< 方位角
    int32_t mPitchAngel;    ///< 俯仰角
    int8_t mSpeed;          ///< 速度
};

struct DirectionalJammerControlCmd
{
    DirectionalJPackHead packHead;
    uint8_t jamOperateType;          ///< 操作类型
    uint8_t jamSignalType;           ///< 信号类型
    uint8_t jamBandNum;              ///< 频段个数
    uint32_t jamFre[8];              ///< 频率
    uint32_t jamBandWidth[8];        ///< 带宽
    short jamPowerFade;              ///< 功率衰减
    uint8_t jamVcoNum;               ///< 调制类型
    uint8_t jamChannelNum;           ///< 通道号
    uint32_t jamDuration;            ///< 开启时长
};

//定向干扰状态帧



//光电状态帧
struct PhotoElec 
{
    uint32_t uiHead;
    float fAmith;
    float fPitch;
};


//光电状态帧

//协议破解状态
struct ProtolDecodeHead
{
    uint32_t head;
    int16_t version;
    uint32_t len;
    uint16_t wYear;
    uint8_t wMonth;
    uint8_t wDay;
    uint8_t wHour;
    uint8_t wMinute;
    uint8_t wSecond;
    uint16_t wMilliseconds;
    uint16_t dataType;
    uint64_t packNum;
};  //29字节

struct ProtolDecodeTail
{
    uint8_t checksum;
    uint32_t tail;
};
struct ProtolDecodeDev
{
    int32_t sId;                             ///< 监测设备ID
    char sName[20];                          ///< 设备名称   utf8 encode[20]
    double sLongitude;                       ///< 经度 (东经)
    double sLatitude;                        ///< 纬度 (北纬)
    int32_t sAltitude;                       ///< 高度 (海拔)
    int16_t sWorkingCondition;               ///< 设备工作状态  0: idle 1: work
    int32_t sAzimuth;                        ///< 系统方位角
    char sType[4];                           ///< 设备类型  char[4]
    int8_t sEngine1;                         ///< 引擎1工作状态:0正常,1受限,2故障
    int8_t sEngine2;                         ///< 引擎2工作状态
    int8_t sSensor1;                         ///< 传感器1工作状态
    int8_t sSensor2;                         ///< 传感器2工作状态
    int8_t sSensor3;                         ///< 传感器3工作状态
    int8_t sJammer;                          ///< 干扰工作状态
    int32_t sCoverage;                       ///< 天线覆盖范围
    int32_t sReceiverId;                     ///< 接收设备ID
    int32_t sDetectTemp;                     ///< 设备温度
    int32_t sDetectHumid;                    ///< 设备湿度
    int32_t sProcessingParameters;       
    ///< 信号处理参数3
    int32_t sSiteNumber;                     ///< 站点编号
};

struct ProtolDecodeDevInfo
{
    ProtolDecodeHead pHead;
    ProtolDecodeDev stateData;
    ProtolDecodeTail pTail;
};


struct TargetDataAfter
{
    int32_t tStationId;                      ///< 监测站ID
    int32_t tAzimuth;                        ///< 监测站识别的目标方向角
    int32_t tRange;                          ///< 目标距离
    float tLongitude;                        ///< 经度 (东经)
    float tLatitude;                         ///< 纬度 (北纬)
    int32_t tAltitude;                       ///< 高度 (海拔)
    double tFrequency;                       ///< 目标使用频率
    double tBandwidth;                       ///< 目标带宽
    double tSignalStrength;                  ///< 目标信号强度
    int8_t tDuration[18];                    ///< 目标信号持续时间
    int8_t tConfidence;                      ///< 置信度
    int8_t tReserve;                         ///< 保留字节//1干扰0不干扰2不转转台
    int8_t tHour;                            ///< 发现时间: 时
    int8_t tMinute;                          ///< 发现时间: 分
    int8_t tSecond;                          ///< 发现时间: 秒
    int16_t tMillisecond;                    ///< 发现时间: 毫秒
    int8_t tType;                            ///< 数据类型
    int8_t tMrm;                             ///< 调制方式
};

struct DetectTargetData
{
    int32_t tId;                             ///< 目标ID
    char tUniqueId[32];                    ///< 唯一Id
    int32_t tInfoLen;                        ///< 目标信息部分长度
    char tInfo[64];                        ///< 目标信息
    TargetDataAfter tDataAfter;
};
//协议破解状态


//诱骗设备状态
struct TrapDeviceState
{
    int head;
    int ctrlType;
    double lon;
    double lat;
    float height;
    uint8_t strategy;
    uint8_t trapState;             ///< 诱骗状态 0断开 1正常 2告警 3异常
    uint8_t trapMsg;               ///< 诱骗告警与异常信息 对应枚举trap_msg
    uint8_t turntableState;        ///< 转台状态 0断开 1正常 2告警 3异常
    uint8_t signalState[6]; ///< 信号单元 0接收发送关 1正常 2告警 3异常 4工作中
    uint8_t antennaState;//1:定向 0：全向
    float azimuth;
    float pitch;
};

struct TrapStrategyCtrl
{
    int head;
    int ctrlType;
    int8_t mode;          //0:诱导 1:驱离 2:其他
    double inducedLon;    //诱导经度
    double inducedLat;    //诱导纬度
    double inducedHeight; //诱导高度
    uint32_t inducedRadius; //诱导半径
    int expellSpeed;      //驱离速度
    int expellDuration;   //驱离时间
    float expellAzimuth;  //驱离方位
    double circleLon; //圆周运动中心经度
    double circleLat; //圆周运动中心纬度
    double circleHeight; //圆周运动高度
    int circleRadius; //圆周运动半径
    int circleSpeed; //圆周运动角速度
};
struct TrapPowerCtrl
{
    int head;
    int ctrlType;
    int8_t powerSwitch;    //接收机开关;信号开关;衰减值
};
struct TrapTarget
{
    int head;
    int ctrlType;
    double targetLon;       // 目标经度
    double targetLat;       // 目标纬度
    float targetHeight;     // 目标高度
    float targetSpeed;      // 目标速度
    double targetDistance;  // 目标距离
    float targetAzimuth;   // 目标方位
    float targetCourse;     // 目标航向角
};

struct TrapSrcConfig
{
    int head;
    int ctrlType;
    int8_t gpsSwitch;
    int8_t gloSwitch;
    int8_t bdSwitch;
    int8_t galSwitch;
    int8_t qzssSwitch;
    int8_t irnssSwitch;
};
//诱骗设备状态



//融合目标信息
typedef struct AssociDevState
{
    int32_t mId;                            ///< 监测设备ID
    DeviceType mType;                       ///< 设备类型
    int targetID;                           ///< 当前站点目标ID 
    float targetAzimuth;                    ///< 目标相对于此设备的方位
}AssociDevState;

typedef struct AssociTarget
{
    int AssociTargetID;                     ///< 融合目标ID
    float mLongitude;                       ///< 经度 (东经)
    float mLatitude;                        ///< 纬度 (北纬)
    int32_t mStationId;                     ///< 监测站ID
    int32_t mAzimuth;                       ///< 监测站识别的目标方向角
    int32_t mRange;                         ///< 目标距离
    float mPitch;                           ///< 俯仰
    float mAltitude;                        ///< 高度

    //侦测、察打信息
    char mUniqueId[32];                   ///< 唯一Id       char[32] fixed bytes in tcp datagram
    char   mInfo[64];                       ///< 目标名称
    double mFrequency;                      ///< 目标使用频率
    double mBandwidth;                      ///< 目标带宽
    double mSignalStrength;                 ///< 目标信号强度
    int8_t mConfidence;                     ///< 置信度
    int8_t mType;                           ///< 目标类型
    int8_t mMrm;                            ///< 调制方式
    time_t firstTime;//第一次发现时间
    time_t mTime;
    //雷达信息
    int dangerLevel;          ///< 危险等级
    int historyTracks;        ///< 历史点迹个数
    float speed;              ///< 速度
    float course;             ///< 航向
    uint8_t trackLineFlag;    ///< 航迹标志位
    uint8_t selectedFlag;     ///< 跟踪标志位
    //融合目标各站点信息 
    //std::vector<AssociDevState> deviceInfo;
}AssociTarget;                ///< 融合目标
//融合目标信息


#pragma pack()
} // namespace NEBULA


#endif