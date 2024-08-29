#ifndef CELLTYPES_H
#define CELLTYPES_H

#include <QObject>
#include <QVector>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>


#pragma pack(push, 4)

// идентификаторы абонентов
enum eIdUser {ID_ASKU = 0x415343, ID_OTS = 0x4F5453, ID_PEL = 0x50454C,
              ID_POI = 0x504f49, ID_VOI = 0x564f49, ID_MKC = 0x534b43, ID_BLA = 0x424c41,
              ID_VIP = 0x564950, ID_RLS = 0x524c53, ID_REB = 0x524542, ID_UNKNOWN = 0x0};
enum Commands {eCommandOn = 0, eCommandOff = 1, eCommandReboot = 2, eMaxCommand = 3};
enum ParamTypes {FLOAT = 0, STRING, INT32};
enum LogTypes {eIn = 1, eOut = 2, eErr = 3, eInfo = 4, eTime = 5, eMax};
enum ProtCellTypes {
    eNoRecord = 0, eProtNoRecogn = 0, eProtBird  = 1, eProtBirds   = 2, eProtUVAPlane = 3, eProtUVACopter = 4,
    eProtRocket   = 5, eProtHelic    = 6, eProtPlane = 7, eProtUnknown = 255, eProtMaxType = 8};


QString IDToStr(int nID);
QString ParamTypeToStr(int);
const float fLatFalse = 100.f;
const int nMaxCam = 8;
//=== версия ==============================================
struct OVersion {
  quint8 n_Maj, n_Min;
  OVersion(quint8 nMaj, quint8 nMin) {n_Maj = nMaj, n_Min = nMin;}
  OVersion() {n_Maj = 0, n_Min = 0;}
  bool operator< (OVersion& ver) {
    if (n_Maj < ver.n_Maj) return true;
    if (n_Maj > ver.n_Maj) return false;
    if (n_Min < ver.n_Min) return true;
    return false;
   }
  bool operator != (OVersion ver) {
    if (n_Maj != ver.n_Maj) return true;
    if (n_Min != ver.n_Min) return true;
    return false;
   }
  bool operator == (OVersion ver) {
    return !(*this != ver);
   }
  bool IsEmpty() const {
    if (n_Maj == 0 && n_Min == 0) return true;
    else return false;
    }
  void Clear() { n_Min = n_Maj = 0;}
  };
//---------------------------------------------------------
struct OHeader { // общий заголовок всех сообщений
    // 1
    quint32 n_IDSender:24;  // Идентификатор отправителя данных
    quint32 n_IdxModul:8;   // Номер модуля в комплексе
    //quint32 n_IDSender;  // Идентификатор отправителя данных
    // 2
    quint8 v_Major;     // Версия протокола мажор
    quint8 v_Minor;     // Версия протокола минор
    quint16 n_IdxPack;  // Сквозной порядковый номер пакета
    // 3
    quint32 n_SizeData:31; // Размер блока данных в байтах
    quint32 n_IsAsku:1;    // Признак данных АСКУ (используется только для чтения файла)
    // 4
    quint16 n_TypePack; // № типа передаваемого пакета
    quint16 n_CheckSum; // Контрольная сумма заголовка
};
// 0x1
//==================== общие кодограммы ===================
struct ORequest { // кодограмма для запроса на подключение
    // 1
    quint32 n_IdManuf:8; // идентификатор производителя
    quint32 n_SerNumb:24;    // серийный номер модуля
    // 2
    quint32 n_VerMajMod:8;       // версия модификации мажор
    quint32 n_VerMinMod:8;       // версия модификации минор
    quint32 n_VerMajPO:7;        // версия мажор ПО
    quint32 n_IsInfo:1;          // флаг информационного ПО
    quint32 n_VerMinPO:7;        // версия минор ПО
    quint32 n_IsAsku:1;          // флаг ПО АСКУ

    ORequest() { Clear(); }
    void Clear() {
      n_IdManuf = 0; // идентификатор производителя
      n_SerNumb = 0;    // серийный номер модуля
      n_VerMajMod = 0;       // версия мождификации мажор
      n_VerMinMod = 0;       // версия модификации минор
      n_VerMajPO = 0;        // версия мажор ПО
      n_IsInfo = 0;          // флаг информационного ПО
      n_VerMinPO = 0;        // версия минор ПО
      n_IsAsku = 0;          // флаг ПО АСКУ
      }

    bool operator ==(ORequest &oReq) const;    
    void CopyAsku(ORequest& oReq) {
      n_IdManuf = oReq.n_IdManuf; // идентификатор производителя
      n_SerNumb = oReq.n_SerNumb;    // серийный номер модуля
      n_VerMajMod = oReq.n_VerMajMod;       // версия мождификации мажор
      n_VerMinMod = oReq.n_VerMinMod;       // версия модификации минор
      }

};

// 0x2
//---------------------------------------------------------
struct ORequestAns { // кодограмма для ответа на запрос на подключение
    OHeader o_Header;
    quint32 n_IdxModul:8;
    quint32 n_Err:8;
    quint32 n_Reserv:16;
};

// 0x23
struct OReqCtl{
    OHeader o_Header;
    quint32 n_TimeReq1;
    quint32 n_TimeReq2;
    quint32 n_id_Com:16;
    quint32 n_code:16;
};


// 0x3
//---------------------------------------------------------
struct OTimeAns { // кодограмма для ответа на запрос времени
    OHeader o_Header;
    quint64 n_TimeReq;
    quint64 n_TimeAns;
};

struct ODirectTurnCtl{
    OHeader o_Header;
    quint32 flagAZ;
    quint32 flagPitch;
    quint32 Azimuth;
    quint32 Pitch;
};
struct ODirectSrcCtl
{
    OHeader o_Header;
    quint32 Channel:8;
    quint32 reserve:24;
    quint32 freq;
    quint32 bandWidth;
    quint32 reserve1;
    quint32 reserve2;
};
struct OJammerSwitchCtl
{
    OHeader o_Header;
    char Channel[32];
    quint32 OpenTime;
};

struct OTurnTableStatus
{
    OHeader o_Header;
    qint32 Azimuth;
    qint32 Pitch;
};

struct OJammerSrcStatus
{
    OHeader o_Header;
    char Channel[32];
    quint32 Reserve;
};
// 0x601
struct OTrapFixed
{
    quint32 i_Num:8;
    quint32 taskREB:8;
    quint32 reserve:16;
    float azREB;
    float elevREB;
    float kGainREB;
};
struct OTrapExone
{
    float freq;
    float dFreq;
};
struct OTrapExTwo
{
    quint32 flags;
    float latitude;
    float longitude;
    float altitude;
    float speed;
    float course;
};
struct OTrapProhibit
{
    OHeader o_Header;
    quint32 n_TimeReq1;
    quint32 n_TimeReq2;
    quint32 isOn:1;
    quint32 reserve:31;
};
// 0x822
struct OBearingMark
{
    OHeader o_Header;
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

// 0x4
struct OTimeAns1
{
    OHeader o_Header;
    quint32 timeRequest1;
    quint32 timeRequest2;
    quint32 timeAnswer1;
    quint32 timeAnswer2;
};

struct OTimeReq{
    OHeader o_Header;
    quint32 n_TimeReq1;
    quint32 n_TimeReq2;
};

// 0x21
struct ONPStatus
{
    quint32 IDElem : 16;
    quint32 status : 4;
    quint32 workF1 : 4;
    quint32 local : 1;
    quint32 isImit : 1;
    quint32 reserve : 6;
};

// 0x22
struct OCPStatus
{
    quint32 IDParam;
    quint32 status : 8;
    quint32 size : 8;
    quint32 isNewStatus : 1;
    quint32 isNewValue : 1;
    quint32 reserve : 14;
};

// 0x24
struct OModuleStatus
{
    quint32 time1;
    quint32 time2;
    quint32 status : 3;
    quint32 work : 3;
    quint32 isRGDV : 1;
    quint32 isRAF : 1;
    quint32 isLocal : 1;
    quint32 isImit : 1;
    quint32 hasTP : 1;
    quint32 isTP : 1;
    quint32 isWP : 1;
    quint32 isTPValid : 1;
    quint32 isWpValid : 1;
    quint32 statusTwp : 1;
    quint32 mode : 16;
    quint32 reserve;
};

// 0x5,模块位置
//---------------------------------------------------------
struct OModulPos { // кодограмма геопозиция модуля
    // 1
    quint32 n_TypeData:3;  // тип данных
    quint32 n_IsValid:1;  // статус достоверности данных
    quint32 n_Reserv:28; // резерв
    // 2
    float f_X_Lat; // широта в градусах или Х в метрах
    // 3
    float f_Y_Lon; // долгота в градусах или Y в метрах
    // 4
    float f_Z_H; // высота над уровнем моря или Z в метрах
};
//---------------------------------------------------------
struct ORLSPos { // кодограмма о координатах РЛС
    OHeader o_Header;
    // 1
    float f_Lat; // широта, град
    // 2
    float f_Lon; //долгота, град
    // 3
    float f_H; // высота над уровнем моря, град
};
//---------------------------------------------------------
struct OTrackOut { // кодограмма о трассах
    OHeader o_Header;
    // 1 // 2
    qint64 n_Time; // время
    // 3
    quint16 n_NumVOI; // номер трассы ВОИ
    quint16 n_NumKOIR; // номер трассы КОИР
    // 4
    quint8 n_KtPrizn; // признаки обнаружения
    quint8 n_TrPrizn; // признаки сопровождения
    quint8 n_TrType;  // тип цели

    quint8 n_Status:2; // статус информации о трассе
    quint8 n_Test:1; // признак тестовой цели
    quint8 n_KursMan:1; // признак маневра по курсу
    quint8 n_HMan:2; // признак изменения высоты
    quint8 n_IsH:1; // достоверность высоты
    quint8 n_IsVr:1; // достоверность радиальной скорости
    // 5
    float f_X;
    // 6
    float f_Y;
    // 7
    float f_Z;
    // 8
    float f_Lat; // широта
    // 9
    float f_Lon;  // долгота
    // 10
    float f_H; // высота
    // 11
    float f_AzSize; // размер строба по азимуту
    // 12
    float f_DSize; // размер строба по дальности
    // 13
    float f_Vx; // скорость по X
    // 14
    float f_Vy; // с5корость по Y
    // 15
    float f_Vz; // скорость по Z
    // 16
    float f_Vr; // радиальная скорость от ПОИ
    // 17
    quint32 n_NumSpp; // номер трассы СПП
    // 18
    quint32 n_Addr:24; // адрес цели от АЗН
    quint32 n_CellCat:8; // категория цели от АЗН
    // 19
    quint32 n_Rezerv; // резерв
};
//---------------------------------------------------------
struct OTrackFromModul { // кодограмма о трассах
    // 1 // 2
    qint64 n_Time; // время
    // 3
    quint16 n_NumVOI; // номер трассы РЛС
    quint16 n_NumKOIR; // номер трассы КОИР
    // 4
    quint8 n_KtPrizn; // признаки обнаружения
    quint8 n_TrPrizn; // признаки сопровождения
    quint8 n_TrType;  // тип цели

    quint8 n_Status:2; // статус информации о трассе
    quint8 n_Test:1; // признак тестовой цели
    quint8 n_KursMan:1; // признак маневра по курсу
    quint8 n_HMan:2; // признак изменения высоты
    quint8 n_IsVr:1; // достоверность радиальной скорости
    quint8 n_Rezerv1:1;
    // 5
    float f_Az;
    // 6
    float f_D;
    // 7
    float f_HPrl;
    // 8
    float f_HWrl; //
    // 9
    float f_HNrz;  //
    // 10
    quint32 n_NbUvd:30;
    quint32 n_NbUvdNew:1; // обновление номера борта УВД
    quint32 n_IsUvd:1; // наличие номера борта в формате УВД
    // 11
    quint32 n_NbRbs:12;
    quint32 n_NbRbsNew:1; // обновление номера борта УВД
    quint32 n_IsRbs:1; // наличие номера борта в формате УВД
    quint32 n_ResNb:18;
    // 12
    float f_AzSize; // размер строба по азимуту
    // 13
    float f_DSize; // размер строба по дальности
    // 14
    float f_Vx; // скорость по X
    // 15
    float f_Vy; // скорость по Y
    // 16
    float f_Vz; // скорость по Z
    // 17
    float f_Vr; // радиальная скорость от ПОИ
    // 18
    quint32 n_NumSpp; // номер трассы СПП
    // 19
    quint32 n_Addr:24; // адрес цели от АЗН
    quint32 n_CellCat:8; // категория цели от АЗН
    // 20
    quint32 n_Rezerv; // резерв
};
//---------------------------------------------------------
struct OSetModulCoord {
    OHeader o_Header;
    float f_Lat;
    float f_Lon;
    float f_H;
    float f_X;
    float f_Y;
    float f_Z;
};
//---------------------------------------------------------
struct ORLSOrient {
    OHeader o_Header;
    float f_Roll;
    float f_Pitch;
    float f_Course;
};
//---------------------------------------------------------
struct ODiap {
  float f_Freq;
  float f_DelFreq;
  ODiap() {f_Freq = f_DelFreq = 0.f;}
  ODiap(float fFreq, float fDelFreq) {f_Freq = fFreq; f_DelFreq = fDelFreq;}
  };
//---------------------------------------------------------
struct OToKoirSopr // кодограмма для КОИР для сопровождения цели
{
    OHeader o_Header;
    // 1 // 2
    quint64 n_Time;     // время
    // 3
    quint16 n_IdxCell;  // номер трассы ВОИ
    quint16 n_IdxKoir;   // номер  трассы КОИР
    // 4
    float f_X;         // Х
    // 5
    float f_Y;         // У
    // 6
    float f_Z;         // Z
    // 7
    float f_Vr;        // рад. скорость от ПОИ
    // 8
    float f_Vx;        // скорость по Х
    // 9
    float f_Vy;        // скорость по У
    //10
    quint32 n_CamMask:8;  // флаги камер, для которых предназначена команда
    quint32 n_Reserv:24;   // резерв
};
//---------------------------------------------------------
struct ToKoirTrackSopr23 // кодограмма для КОИР для сопровождения цели
{
    // 1 // 2
    quint64 m_time;     // время
    // 3
    quint16 m_idxTrackVoi;  // номер трассы ВОИ
    quint16 m_idxTrackKoir;   // номер  трассы КОИР
    // 4
    float m_x1;         // Х
    // 5
    float m_y1;         // У
    // 6
    float m_z1;         // Z
    // 7
    float m_epsSize1;
    // 8
    float m_x2;         // Х
    // 9
    float m_y2;         // У
    // 10
    float m_z2;         // Z
    // 11
    float m_epsSize2;
    // 12
    float m_azSize;
    // 13
    float m_vr;        // рад. скорость от ПОИ
    // 14
    float m_vx;        // скорость по Х
    // 15
    float m_vy;        // скорость по У
    // 16
    float m_vz;        // скорость по Z
    // 17
    quint32 m_camNumb:8;  // номер камеры, для которой предназначена команда
    quint32 m_reserve:24;   // резерв
};
static_assert (sizeof (ToKoirTrackSopr23) == 68, "Размер структуры ToKoirTrackSopr23 не совпадает с заданным");
//---------------------------------------------------------
struct OKoirManual // кодограмма для КОИР для ручного режима
{
    // 1
    quint32 b_IsCellFind:1;     // поиск целей при сканировании
    quint32 n_Reserv1:7;     // резерв
    quint32 n_Maska:8;       // маска номеров, для которых предназначена команда
    quint32 n_Reserv2:16;     // резерв
    // 2
    float f_Az; // азимут
    // 3
    float f_Eps; // ум
    // 4
    float f_VAz;         //
    // 5
    float f_VEps;         //
    OKoirManual() {
        f_Az = f_Eps = 0.f;
        f_VAz = f_VEps = 0.f;
        b_IsCellFind = 0;
        n_Reserv1 = 0;
        n_Reserv2 = 0;
        n_Maska = 0;
    }
};
//---------------------------------------------------------
struct ToKoirManualMode23 // кодограмма для КОИР для ручного режима
{
    // 1
    quint32 m_camNumb:8;       // маска номеров, для которых предназначена команда
    quint32 m_reserve:24;     // резерв
    // 2
    float m_az; // азимут
    // 3
    float m_eps; // ум

    ToKoirManualMode23()
    {
        m_az = m_eps = 0.f;
        m_reserve = 0;
        m_camNumb = 0;
    }
};
static_assert (sizeof (ToKoirManualMode23) == 12, "Размер структуры ToKoirManualMode23 не совпадает с заданным");
//---------------------------------------------------------
struct OToKoirManual // кодограмма для КОИР для ручного режима
{
    OHeader o_Header;
    OKoirManual o_Manual;
};
//---------------------------------------------------------
struct OHandZahKoir // кодограмма для КОИР для ручного захвата
{
    // 1
    float f_X; // относительные координаты для Х
    // 2
    float f_Y; // относительные координаты для У
    // 3
    quint32 n_Maska:8;       // маска номеров, для которых предназначена команда
    quint32 n_Reserv:24;     // резерв
  };
 //---------------------------------------------------------
 struct OToKoirHandZah // кодограмма для КОИР для ручного захвата
    {
      OHeader o_Header;
      OHandZahKoir o_HandZah;
};
//---------------------------------------------------------
struct OTakeoffAngle {
  float f_az;
  float f_elev;
};
//---------------------------------------------------------
struct OKoirSubImage1 { // начало сообщения о изображении цели 1 (0x727) от КОИР версия 2.0 и выше
     // 1 // 2
    quint64 n_Time;
    // 3
    quint32 n_CamMask:8;  // номера камер
    quint32 n_Format:8;  // формат изображения
    quint32 n_Rezerv:16;  // номера камер
    // 4
    quint16 n_ColumnCnt;  // количество передаваемых столбцов
    quint16 n_RowCnt;  // количество передаваемых строк
    // 5
    quint16 n_PosX;  // позиция кадра по горизонтали
    quint16 n_PosY;  // позиция кадра по вертикали
    // 6
    quint16 n_IdxVOI;  // номер трассы ВОИ
    quint16 n_IdxKoir;  // номер трассы КОИР
};
//---------------------------------------------------------
struct OSetModulPopr {
    OHeader o_Header;
    quint32 n_Save:1;
    quint32 n_Reserv:31;

    float f_DelAz;
    float f_DelEps;
    float f_DelD;
};
//---------------------------------------------------------
struct OOnOffRebootLang {
  OHeader o_Header;
  quint32 n_Flag;
};
//---------------------------------------------------------
struct OSbrosAvar {
  OHeader o_Header;
  quint32 n_IdElem:16;
  quint32 n_Reserv:16;
};
//---------------------------------------------------------
struct OChatMess {
  OHeader o_Header;
  qint64 llTime;
};
//---------------------------------------------------------
struct OElecWarf {
  OHeader o_Header;
  quint32 bIsREB:1;
  quint32 bIsGEO:1;
  quint32:30;
};
//---------------------------------------------------------
struct OTuneParam {
  quint32 n_IDConfigParam;
  quint32 n_Size:8;
  quint32 n_IsSave:1;
  quint32 resv:23;
};
constexpr inline bool operator<(const OTuneParam& p1, const OTuneParam& p2) noexcept
{ return p1.n_IDConfigParam < p2.n_IDConfigParam; }
//===================== кодограммы КОИР-ВОИ ================
struct ORezhKoir10 { // режим КОИР версия 1.0, 1.1, 1.2
    // 1 //2
    quint64 n_Time;
    // 3
    quint8 n_Rezh; // текущий режим
    quint8 n_Maska; // маска номеров трасс
    quint16 n_IdxCell;  // номер цели ВОИ
    // 4
    float f_Az; // текущее положение по азимуту
    // 5
    float f_Eps; // текущее положение по УМ
    // 6
    float f_AzSize; // ширина поля зрения по азимуту
    // 7
    float f_EpsSize; // ширина поля зрения по УМ
    // 8
    float f_Focus;  // фокусное расстояние в мм
    ORezhKoir10() {}
    bool IsChanged(ORezhKoir10 oRezh) {
      if (f_Az != oRezh.f_Az) return true;
      if (f_Eps != oRezh.f_Eps) return true;
      if (f_AzSize != oRezh.f_AzSize) return true;
      if (f_EpsSize != oRezh.f_EpsSize) return true;
      return false;
    }
};
struct ModeKoir23
{ // режим КОИР версия 2.3
    // 1 //2
    qint64 m_time;
    // 3
    quint32 m_camNumb:8; //номер камеры
    quint32 m_remoteMode:8; // признак внешнего управления
    quint32 m_isAutoFocus:1; // признак автофркусировки
    quint32 m_reserve:15; // резерв
    // 4
    float m_az; // текущее положение по азимуту
    // 5
    float m_eps; // текущее положение по УМ
    // 6
    float m_azSize; // ширина поля зрения по азимуту
    // 7
    float m_epsSize; // ширина поля зрения по УМ
    // 8
    float m_focus;  // фокусное расстояние в мм
    ModeKoir23() {m_time = -1;}
    ModeKoir23(ORezhKoir10 mode)
    {
        m_time = mode.n_Time;
        m_camNumb = (0x1) << mode.n_Maska; //номер камеры
        m_remoteMode = 0; // признак внешнего управления
        m_isAutoFocus = 0; // признак автофркусировки
        m_reserve = 0; // резерв
        m_az = mode.f_Az; // текущее положение по азимуту
        m_eps = mode.f_Eps; // текущее положение по УМ
        m_azSize = mode.f_AzSize; // ширина поля зрения по азимуту
        m_epsSize = mode.f_EpsSize; // ширина поля зрения по УМ
        m_focus = mode.f_Focus;  // фокусное расстояние в мм
    }
};
struct ModeKoir24
{ // режим КОИР версия 2.3
    // 1 //2
    qint64 m_time;
    // 3
    quint32 m_camNumb:8; //номер камеры
    quint32 m_remoteMode:8; // признак внешнего управления
    quint32 m_isAutoFocus:1; // признак автофркусировки
    quint32 m_reserve:7; // резерв
    quint32 m_stateScan : 8; // состояние сканирования
    // 4
    float m_az; // текущее положение по азимуту
    // 5
    float m_eps; // текущее положение по УМ
    // 6
    float m_azSize; // ширина поля зрения по азимуту
    // 7
    float m_epsSize; // ширина поля зрения по УМ
    // 8
    float m_focus;  // фокусное расстояние в мм
    // 9
    float m_zoom;
    ModeKoir24() {m_time = -1;}
    ModeKoir24(ORezhKoir10 mode)
    {
        m_time = mode.n_Time;
        m_camNumb = (0x1) << mode.n_Maska; //номер камеры
        m_remoteMode = 0; // признак внешнего управления
        m_isAutoFocus = 0; // признак автофркусировки
        m_reserve = 0; // резерв
        m_az = mode.f_Az; // текущее положение по азимуту
        m_eps = mode.f_Eps; // текущее положение по УМ
        m_azSize = mode.f_AzSize; // ширина поля зрения по азимуту
        m_epsSize = mode.f_EpsSize; // ширина поля зрения по УМ
        m_focus = mode.f_Focus;  // фокусное расстояние в мм
        m_zoom = 0;
    }
};
//---------------------------------------------------------
struct OPosobilKoir { // возможности КОИР версия 1.2
     // возможности КОИР версия 1.2, 2.0
        quint32 m_isPossibil:1; // признак того, что возможности уже приходили
        quint32 m_isRotate:1; // признак поворотной камеры
        quint32 m_isFocus:1; // возможность управления фокусом
        quint32 m_isZoom:1; // возможность управления зумом
        quint32 m_isMaxZoom:1; // задан максимальный зум, а не набор полей зрения
        quint32 m_isCanScan : 1;
        quint32 m_reserve:26;

        int m_portRtsp;  // порт для потока с камеры
        QString m_name; // имя камеры
        QString m_rtspName; // название видеоканала с камеры
        QString m_fullRtspName; // полный путь к видеопотоку
        float f_AzMin;  // минимальная граница возможной зоны обзора по азимуту
        float f_AzMax; // максимальная граница возможной зоны обзора по азимуту
        float f_EpsMin;  // минимальная граница возможной зоны обзора по УМ
        float f_EpsMax; // максимальная граница возможной зоны обзора по УМ

        float m_azSizeZoom; // размеры по азимуту при зуме 1 крат
        float m_epsSizeZoom; // размеры по углу места при зуме 1 крат
        float m_maxZoom; // максимальный зум в кратах
        int m_mainCamNumb; // номер главной камеры для ведомой
        QVector<float> m_possibleZooms; // набор возможных значений полей зрения по азимуту


    OPosobilKoir() {
      // int n_Num = -1; // условный номер камеры в составе КОИР
      m_isRotate = false; // признак поворотной камеры
      m_portRtsp = 0;  // порт для потока с камеры
      f_AzMin = -1;  // минимальная граница возможной зоны обзора по азимуту
      f_AzMax = 0.f; // максимальная граница возможной зоны обзора по азимуту
      f_EpsMin = 0.f;  // минимальная граница возможной зоны обзора по УМ
      f_EpsMax = 0.f; // максимальная граница возможной зоны обзора по УМ
      }

    void Imit(int nPort, QString sName, QString sRTSPName) {
      m_isRotate = true;
      m_portRtsp = nPort;
      m_name = sName;
      m_rtspName = sRTSPName;
      f_AzMin = 0.f;
      f_AzMax = 360.f;
      f_EpsMin = -90.f;
      f_EpsMax = 90.f;
    }
};
//---------------------------------------------------------
struct OKoirSubImage { // начало сообщения о изображении цели от КОИР версия 1.2 и выше
     // 1 // 2
    quint64 n_Time;
    // 3
    quint8 n_CamMask;  // номера камер
    quint8 n_ColumnCnt;  // количество передаваемых столбцов
    quint8 n_RowCnt;  // количество передаваемых строк
    quint8 n_Rezerv;  // резерв
    // 4
    quint16 n_PosX;  // позиция кадра по горизонтали
    quint16 n_PosY;  // позиция кадра по вертикали
    // 5
    quint16 n_IdxVOI;  // номер трассы ВОИ
    quint16 n_IdxKoir;  // номер трассы КОИР

};
//---------------------------------------------------------
struct OKoirImage { // изображение цели от КОИР
  int n_Idx; // номер модуля
  OKoirSubImage o_SubIm;
  QImage *p_Image;
  OKoirImage() {n_Idx = -1; p_Image = 0;}
};
struct OKoirClass {
    quint32 n_IdxVoi:16; // номер ВОИ
    quint32 n_IdxKoir:16; // номер КОИР
    quint32 n_Flags; //флаги
};

struct OToKoirClass {
    OHeader o_Header;
    OKoirClass o_KoirClass;
};

//---------------------------------------------------------
const unsigned short rFlFindKoir = 0x1;  // уточнить координаты
const unsigned short rFlSoprKoir = 0x2; // взять на сопровождение
const unsigned short rFlOperKoir = 0x4;  // управление от опреатора
const unsigned short rFlAvtomKoir = 0x8; // управление от ВОИ, автоматический обзор всех целей
enum EKoirSoprErr {eNoKoirErr = 0, eNoInZone = 1, eNoFreeMod = 2, eMaxKoirErr = 3};

//===================== кодограммы трассовый выход РЛС-ВОИ ================
//---------------------------------------------------------
struct ORecvSM { // кодограмма для СМ
    // 1
    quint32 n_NS:7; // номер сектора
    quint32 n_IsChangeSM:1;    // признак смены сектора
    quint32 n_IsPCh:1;    // признак включения ПЧ
    quint32 n_IsSKR:1;    // признак включения СКР
    quint32 n_Reserv:22; // резерв
    // 2
    float f_Az;          // азимут в градусах
    // 3 // 4
    quint64 n_Time;      // время
};
//----------------------------------------------------------
struct OPoiRezh { // информация о режиме работы РЛС от ПОИ
  // 1
  float f_MaxRange;     // текущая рабочая дальность
  // 2
  float f_MaxVRad;     // текущий модуль максимальной радиальной скорости м/с (+-)
   // 3
  float f_StepRange;  // дискрет по дальности в м
  // 4
  float f_Lambda;    // длина волны в м
  // 5
  float f_AzSize;   // ШДН антенны, град
  // 6
  float f_SpeedAnt; // текущая установленная скорость вращения антенны в град./сек.
  OPoiRezh() { f_MaxRange = -1; }

  };

struct ORlsRezh {
    // 1 // 2
    quint64 n_Time;      // время
    // 3
    quint32 n_IdxVoi:16; // номер ВОИ
    quint32 n_Type:16; //тип
};

struct OToRlsRezh {
    OHeader o_Header;
    ORlsRezh o_RlsRezh;
};

//---------------------------------------------------------
enum {e_MaxType = 7};
struct OMkcTypeCell { // распознавание от МКЦ
    // 1
    quint16 n_IdxCell;  // номер цели ВОИ
    quint16 n_IdxKT;  // номер отметки в трассе
    // 2
    quint32 n_TypeCell:8;  // тип цели
    quint32 n_Rezerv:24;  // резерв
    // 3 // 4 // 5 // 6 // 7 // 8 // 9
    float f_Probab[e_MaxType]; // вероятости для каждого типа
    void Init() {
      n_TypeCell = 0;
      for(int i = 0; i < e_MaxType; i++) f_Probab[i] = -1.f;
    }
};
//---------------------------------------------------------
struct OMkcRezh20 { // режим и статус МКЦ
    // 1
    quint8 n_VerMajPO;  // версия мажор ПО модуля
    quint8 n_VerMinPO;  // версия минор ПО модуля
    quint8 n_VerMajCore;  // версия мажор ядра модуля
    quint8 n_VerMinCore;  // версия минор ядра модуля
    // 2
    quint8 n_VerMajKoef;  // версия мажор весовых коэффициентов
    quint8 n_VerMinKoef;  // версия минор весовых коэффициентов
    quint8 n_VerMajClas;  // версия мажор классификатора
    quint8 n_VerMinClas;  // версия минор классификатора
    // 3
    quint32 n_Type:8;    // тип модуля распознавания
    quint32 n_Reserv:24;   // резерв

    OMkcRezh20() { n_Type = 0; }
};
//---------------------------------------------------------
struct OSendKt {
  uint n_Type;
  uint n_NumCell;
  uint n_NumKT;
  bool operator ==(OSendKt &oReq) const;
  };
//---------------------------------------------------------
struct OMkcStat {
  static qint64 n_TimeBeginZapros;
  qint64 n_TimeConnect;
  int n_RecvErr[eProtMaxType]; // количество ответов по типам
  int n_Answer[eProtMaxType][eProtMaxType]; // таблица соответствия тип запроса - тип ответа
  QVector<OSendKt> v_SendedKt; // вектр отправленных запросов
  OMkcStat();
  void Clear() {
    v_SendedKt.clear();
    for(int i = 0; i < eProtMaxType; i++) {
      n_RecvErr[i] = 0;
      for(int j = 0; j < eProtMaxType; j++) n_Answer[i][j] = 0;
      }
    }
  };
//---------------------------------------------------------
struct ORTRDiap { // диапазон сканирования
    float f_Freq; // центральная частота, МГц
    float f_DelFreq; // полоса, МГц
    bool b_On;      // вкл./выкл.
    ORTRDiap() {
      f_Freq = f_DelFreq = 0.f;
      b_On = false;
      }
    ORTRDiap(float fFreq, float fDelFreq, bool bOn) {
      f_Freq = fFreq;
      f_DelFreq = fDelFreq;
      b_On = bOn;
     }
    bool operator ==(const ORTRDiap oOth) {
        return ((f_Freq == oOth.f_Freq) && (f_DelFreq == oOth.f_DelFreq));
    }
};
// 0x823
//---------------------------------------------------------
struct OSubRezhRTR20 { // часть кодограммы текущих настроек РТР
    // 1
    quint32 n_Cnt:8; // количество текущих диапазонов сканирования
    quint32 n_Reserv:24; // резерв
    // 2
    float f_CurAz;  // азимутальное направление антенны РТР при ее наличии
};
//---------------------------------------------------------
struct ORezhRTR20 { // текущие настройки РТР
    OSubRezhRTR20 o_SubRezh;
    QVector<ORTRDiap> o_Scan; // текущие диапазоны сканирования
};
//---------------------------------------------------------
struct OSubRezhREB20 { // фиксированная часть кодограммы текущих настроек РЭП
    // 1
    quint32 n_Cnt:8;   // количество включенных диапазонов подавления
    quint32 n_TaskGeo:8; // состояние выполнения задачи подавления навигации
    quint32 n_Reserv:16; // резерв
    // 2
    float f_CurAz;  // азимутальное направление антенны РЭП
    // 3
    float f_CurEps;  // азимутальное направление антенны РЭП
    // 4
    float f_PGain; // коэффициент усиления мощности в процентах???

};
// 0x825
//---------------------------------------------------------
struct OSubPosobilRTR22 { // возможности РТР
   // 1
   quint32 n_IsRotate:1; // признак сканирующего РТР
   quint32 n_MaxTask:7; // максимальное количество заданий для РТР
   quint32 n_MaxSubDiap:8; // количество поддиапазонов частот
   quint32 n_Rezerv:16; // резерв
   // 2
   float f_AzSize; // ШДН антенны РТР по азимуту
   // 3
   float f_EpsSize; // ШДН антенны РТР по углу места
  };
// 0x828
//---------------------------------------------------------
struct OBanIRI { // запрещенный ИРИ
  // 1
  float f_Freq; // центральная частота, мГц
  // 2
  float f_DelFreq; // полоса, МГц
  // 3
  uint n_Numb; // номер
  OBanIRI() {f_Freq = f_DelFreq = 0.f;
              n_Numb = 0;}
  OBanIRI(uint nNumb, float fFreq, float fDelFreq) {
      f_Freq = fFreq;
      f_DelFreq = fDelFreq;
      n_Numb = nNumb;
    }
  };
// 0x829
//---------------------------------------------------------
struct OSubRadioTime {
    // 1 - 2
    qint64 n_Time;
    // 3
    quint32 n_Num:24;
    quint32 n_Type:8;
    // 4
    float f_FrBegin;
    // 5
    float f_FrStep;
    // 6
    uint n_Cnt;
};

//----------------------------------------------------------
struct OSZIReb {
    quint32 n_Type:1; // тип СЗИ (0 - оператор, 1- внешний потребитель)
    quint32 n_IsUse:1; // признак использования СЗИ (0 - в резерве, 1- используются)
    quint32 n_IsEps:1; // признак использования УМ
    quint32 n_IsFreq:1; // признак использования частоты
    quint32 n_Reserv:28; //
    float f_Az1;
    float f_Az2;
    float f_Eps1;
    float f_Eps2;
    float f_Freq;
    float f_DelFreq;
    /*
    OSZIReb() {}
    OSZIReb(float fAz1, float fAz2, bool bType, bool bUse) {
        f_Az1 = fAz1;
        f_Az2 = fAz2;
        n_Type = bType;
        n_IsUse = bUse;
    }*/
};
//---------------------------------------------------------
struct OGeoSpoof20 {
    // 1 - // 2
    qint64 n_TimeSpoof; // время имитируемых координат
    // 3 - 4 - 5
    float f_LatV, f_LonCourse, f_H; // имитируемые координаты
    OGeoSpoof20() { n_TimeSpoof = -1; }
};
//---------------------------------------------------------
struct OGeoSpoof22 {
    // 1 - // 2
    qint64 n_TimeSpoof; // время имитируемых координат
    // 3 - 4 - 5
    float f_Lat, f_Lon, f_H; // имитируемые координаты
    float f_V, f_Course;
    uint n_Flags;
    OGeoSpoof22() { n_TimeSpoof = -1; }
};
//---------------------------------------------------------
struct ORezhREB20 {
   OSubRezhREB20 o_SubRezh;
   QVector<ODiap> o_Diap;
   //OGeoSpoof20 o_Spoof;
   OGeoSpoof22 o_Spoof;
};

//================= кодограммы Анти БЛА-ВОИ ===============
struct OToBlaTarget
{
    // 1
    quint32 n_IdxBla;
    // 2
    quint8 n_TaskBla;   //
    quint8 n_TaskBch;   //
    quint16 n_Reserv;    //
    // 3
    float f_Lat;       //
    // 4
    float f_Lon;       //
    // 5
    float f_Alt;      //
    // 6 // 7
    quint64 n_Time;    // время
};
struct OToBlaCellSend {
    OHeader o_Header;
    OToBlaTarget o_Cell;
};
//---------------------------------------------------------
struct OToBlaTargetEx
{
    // 1
    quint32 n_IdxBla;  // серийный номер БЛА
    // 2
    quint8 n_TaskBla;   // команда для БЛА (1-новая, 2- оновление, 3 - отмена)
    quint8 n_TaskBch;   // команда для БЧ
    quint16 n_TypeCoordCeil:1; // тип передаваемых координат цели (0 - гео, 1 - локальные)
    quint16 n_TypeSpeed:2; // тип передаваемой скорости (0 -нет, 1 - скорость-курс, 2 - Vx, Vy, Vz)
    quint16 n_TypeAccel:2; // тип передаваемого ускорения (0 -нет, 1 - Аx, Аy, Аz)
    quint16 n_Prec:3; // условная точность определения координат
    quint16 n_Reserv:8;    //
    // 3 // 4
    quint64 n_Time;    // время
    // координаты
    // 5
    float f_LatX;       // широта или Х
    // 6
    float f_LonY;       // долгота или У
    // 7
    float f_AltZ;      // высота или Z
    // скорости
    // 8
    float f_VVx;
    // 9
    float f_CourseVy;
    // 10
    float f_VhVz;
    // ускорения
    // 11
    float f_Ax;
    // 12
    float f_Ay;
    // 13
    float f_Az;
};
//---------------------------------------------------------
struct OToBlaCellSendEx {
    OHeader o_Header;
    OToBlaTargetEx o_Cell;
};
//---------------------------------------------------------
struct ToBlaBlaPos
{
    // 1
    quint32 n_IdxBla;  // серийный номер БЛА
    // 2
    quint32 n_TypeCoordCeil:1; // тип передаваемых координат цели (0 - гео, 1 - локальные)
    quint32 n_TypeSpeed:2; // тип передаваемой скорости (0 -нет, 1 - скорость-курс, 2 - Vx, Vy, Vz)
    quint32 n_TypeAccel:2; // тип передаваемого ускорения (0 -нет, 1 - Аx, Аy, Аz)
    quint32 n_Reserv:27;    //
    // 3 // 4
    quint64 n_Time;    // время
    // координаты
    // 5
    float f_LatX;       // широта или Х
    // 6
    float f_LonY;       // долгота или У
    // 7
    float f_AltZ;      // высота или Z
    // скорости
    // 8
    float f_VVx;
    // 9
    float f_CourseVy;
    // 10
    float f_VhVz;
    // ускорения
    // 11
    float f_Ax;
    // 12
    float f_Ay;
    // 13
    float f_Az;
};
//---------------------------------------------------------
struct ToBlaCorrCoord
{
    // 1
    quint32 n_IdxBla;  // серийный номер БЛА
    // 2
    quint32 n_TaskBla : 8;   // команда для БЛА (1-новая, 2- оновление, 3 - отмена)
    quint32 n_TaskBch : 8;   // команда для БЧ
    quint32 n_TypeCorrCoord:1; // тип передаваемых координат цели (0 - гео, 1 - локальные)
    quint32 n_Reserv:15;    //
    // 3 // 4
    quint64 n_Time;    // время
    // координаты
    // 5
    float f_DelXAz;       // широта или Х
    // 6
    float f_DelYEps;       // долгота или У
    // 7
    float f_DelZ;      // высота или Z
};
//---------------------------------------------------------
struct OBlaRezh { // режим
    // 1
    quint8 n_BlaCnt;     // кол-во бла на связи
    quint8 n_BlaMax;   // максимальное кол-во БЛА
    quint8 n_State;  // режим работы СУ
    quint8 n_Err;       // статус ошибок
};
//---------------------------------------------------------
struct OBlaState { // сообщение о собственных БЛА
    // 1
    quint32 n_SerNumBla;  // серийный номер БЛА
    // 2
    quint16 n_Err;      // статус ошибок
    quint8 n_BlaType;   // тип БЛА
    quint8 n_BChType;   // тип БЧ
    // 3
    quint8 n_BlaState;  // режим работы БЛА
    quint8 n_BChState;   // сосотояние БЧ
    quint16 n_PortVideo{0};  // номер порта для выдачи видео
    // 4
    float f_LatTarget;  // широта заданной цели, град
    // 5
    float f_LonTarget;  // долгота заданной цели, град
    // 6
    float f_HTarget; // высота заданной цели, м
    // 7
    float f_LatBla;  // текущая широта БЛА, град
    // 8
    float f_LonBla; // текущая долгота БЛА, град
    // 9
    float f_HBla;  // текущая высота БЛА, м
    // 10 //11
    quint64 n_Time;  // время БЛА
    // 12
    float f_Freq;    // частота канала связи с БЛА
    // 13
    float f_SNR;  // ОСШ сигнала от БЛА, дБ
};
//---------------------------------------------------------
struct OBlaStateEx { // сообщение о собственных БЛА
    // 1
    quint32 n_SerNumBla;  // серийный номер БЛА
    // 2
    quint16 n_Err;      // статус ошибок
    quint8 n_BlaType;   // тип БЛА
    quint8 n_BChType;   // тип БЧ
    // 3
    quint8 n_BlaState;  // режим работы БЛА
    quint8 n_BChState;   // сосотояние БЧ
    quint16 n_PortVideo{0};  // номер порта для выдачи видео
    // 4
    float f_Freq;    // частота канала связи с БЛА
    // 5
    float f_SNR;  // ОСШ сигнала от БЛА, дБ
    // 6
    quint32 n_TypeCoordCeil:1; // тип координат цели
    quint32 n_TypeCoordBLA:1;  // тип координат текущего положения бла
    quint32 n_TypeSpeed:2;     // тип передаваемой скорости
    quint32 n_Reserv:28; // резерв
    // 7
    float f_LatXTarget;  // широта заданной цели, град
    // 8
    float f_LonYTarget;  // долгота заданной цели, град
    // 9
    float f_HZTarget; // высота заданной цели, м
    // 10
    float f_LatXBla;  // текущая широта БЛА, град
    // 11
    float f_LonYBla; // текущая долгота БЛА, град
    // 12
    float f_HZBla;  // текущая высота БЛА, м
    // 13 //14
    quint64 n_Time;  // время БЛА
    // 15
    float f_VVx; // модуль скорости - скорость по Х
    // 16
    float f_CourceVy; // курс - скорость по у
    // 17
    float f_VhVz; // скорость по высоте - скорость по z
};
//---------------------------------------------------------
struct OPosobilBla { // возможности одного БЛА
    // 1
    quint32 n_SerNumBla;  // серийный номер БЛА
    // 2
    float f_DMax; // максимальная дальность полета от точки старта
    // 3
    float f_VMax; // максимальная скорость полета
    // 4
    float f_HMax; // максимальная высота полета
 };
//---------------------------------------------------------
struct OPosobilBla23 { // возможности одного БЛА
    // 1
    quint32 n_SerNumBla;  // серийный номер БЛА
    // 2
    float f_DMax; // максимальная дальность полета от точки старта
    // 3
    float f_VMax; // максимальная скорость полета
    // 4
    float f_HMax; // максимальная высота полета
    // 5
    uint32_t m_isGround : 1;
    uint32_t m_isAerial : 1;
    uint32_t m_reserve : 30;
 };
//---------------------------------------------------------
struct OPosobilBla24 { // возможности одного БЛА
    // 1
    quint32 n_SerNumBla;  // серийный номер БЛА
    // 2
    float f_DMax; // максимальная дальность полета от точки старта
    // 3
    float f_VMax; // максимальная скорость полета
    // 4
    float f_HMax; // максимальная высота полета
    // 5
    uint32_t m_isGround : 1;
    uint32_t m_isAerial : 1;
    uint32_t m_isScout : 1;
    uint32_t m_isRetrans : 1;
    uint32_t m_reserve : 28;
 };
//---------------------------------------------------------
struct OBlaRazv
{// данные разведки оператора БпЛА
    // 1
    quint32 n_SerNumBla;            // серийный номер БпЛа
    // 2
    quint32 n_TypeCoord : 1;        // тип передаваемых координат цели
    quint32 n_ScreenEnable : 1;     // признак наличия снимка по цели
    quint32 n_ImageFormat : 6;      // формат изображения
    quint32 n_TypeTarg : 8;         // тип цели : 1-подвижная, 2-неподвижная
    quint32 n_ConditionTarg : 8;    // состояние цели : 0-неизвестно,1-актуальная,
                                    // 2-под огневым воздействием,3-уничтожена
    quint32 n_Reserve : 8;          // резерв
    // 3
    float f_LatXTarget;             // широта заданной цели, град
    // 4
    float f_LonYTarget;             // долгота заданной цели, град
    // 5
    float f_AltZTarget;             // высота заданной цели, м
    // 6 // 7
    quint64 n_TimeBla;             // метка времени координат
    // 8
    quint16 n_SizeStolb;            // количество передаваемых столбцов снимка
    quint16 n_SizeStrok;            // количество передаваемых строк снимка
};
//---------------------------------------------------------
struct OBlaPad
{   // данные о точке падения снаряда и рассчитанных корректур
    // 1
    quint32 n_SerNumBla;            // серийный номер БпЛА
    // 2
    quint32 n_TypeCoord : 1;         // тип передаваемых координат
    quint32 n_Reserve :31;          // резерв
    // 3
    float f_LatXPoint;              // широта или координаты точки падения снаряда Х
    // 4
    float f_LonYPoint;              // долгота или координаты точки падения снаряда У
    // 5
    float f_AltZPoint;              // высота или координаты точки падения снаряда Z
    // 6 // 7
    quint64 n_TimeBla;             // метка времени координат
    // 8
    int i_DeltaX : 16;              // Рассчитанные отклонения по оси Х
    int i_DeltaY : 16;              // Рассчитанные отклонения по оси У
};
//---------------------------------------------------------
struct PulseModulation
{
    QString m_modName; // Название
    QString m_modType; // Типы модуляции

    // параметры ЛЧМ
    QString m_freq; // Центральная частота ЛЧМ
    QString m_deviation; // Девиация ЛЧМ
    QString m_directionLfm; // Направление ЛЧМ

    // параметры ФКМ
    QString m_chipWidth; // Длительность, мкс, допустимые значения [0; 10000]
    QString m_chipCnt; // Количество, допусимые значения [0; 1000]
    QString m_directionPsk; // Направление ФКМ
    QString m_typePsk; // Тип ФКМ
};
//---------------------------------------------------------
struct Bearings
{
    QString m_lat; // Широта, град
    QString m_lon; // Долгота, град
    QString m_alt; // Высота, град
    QString m_bearing; // Пеленг, азимут, град
    QString m_timeStamp; // Время определения пеленга
    QString m_quality; // Качество, диапазон [0; 1]
    QString m_std; // СКО
    QString m_droneNumber; // Номер полезной нагрузки, которая обнаружила источник
};
//---------------------------------------------------------
struct SourceLocations
{
    double m_lat = -1000; // Широта, град
    double m_lon = -1000; // Долгота, град
    double m_alt = -1000; // Высота, град
    QString m_heading; // Курс, град
    QString m_speed; // Скорость, км/ч
    QString m_timeStamp; // Время определения пеленга
    QString m_a; // Большая полуось, м
    QString m_b; // Малая полуось, м
    QString m_phi; // Угол поворота (по часовой стрелке)
};
//---------------------------------------------------------
struct RtrData
{
    QString m_id; // идентификатор
    QStringList m_frequencies; // Частоты, МГц
    QStringList m_bands; // Полосы, МГц
    QStringList m_pulsePeriods; // Периоды повторения импульсов, мкс, допустимые значения [0; 30000]
    QStringList m_pulseWidths; // Длительности импульсов, мкс [0; 10000]
    QVector<PulseModulation> m_pulseMod; // Типы модуляций
    QString m_rotationPeriod; // Период вращения, допустимые значения, с [0; 120]
    QString m_rlsMode; // Режим работы РЛС
    QVector<Bearings> m_bearings; // Массив пеленгов
    QString m_localityName; // Населенный пункт
    QString m_areaName; // Зона
    QString m_beginTime; // Время начала, с
    QString m_endTime; // Время окончания, с
    QVector<SourceLocations> m_sourceLoc; // Местоположения источников

    void unpack(QJsonObject obj) {

        QString str;
        if (obj.contains("ID")) m_id = obj["ID"].toString();
        if (obj.contains("Frequencies")) {
            QJsonArray arr = obj["Frequencies"].toArray();
            for(int i = 0; i < arr.size(); i++) {
                m_frequencies.append(str.setNum(arr[i].toDouble()));
            }
        }
        if (obj.contains("Bands")) {
            QJsonArray arr = obj["Bands"].toArray();
            for(int i = 0; i < arr.size(); i++) {
                m_bands.append(str.setNum(arr[i].toDouble()));
            }
        }
        if (obj.contains("PulsePeriods")) {
            QJsonArray arr = obj["PulsePeriods"].toArray();
            for(int i = 0; i < arr.size(); i++) {
                m_pulsePeriods.append(str.setNum(arr[i].toInt()));
            }
        }
        if (obj.contains("PulseWidths")) {
            QJsonArray arr = obj["PulseWidths"].toArray();
            for(int i = 0; i < arr.size(); i++) {
                m_pulseWidths.append(str.setNum(arr[i].toDouble()));
            }
        }
        if (obj.contains("PulseModulations")) {
            QJsonArray arr = obj["PulseModulations"].toArray();
            for(int i = 0; i < arr.size(); i++) {
                QJsonObject objectPulseMod = arr[i].toObject();
                PulseModulation pulse_modulation;
                if (objectPulseMod.contains("ModulationName")) pulse_modulation.m_modName = objectPulseMod["ModulationName"].toString();
                if (objectPulseMod.contains("ModulationType")) pulse_modulation.m_modType.setNum(objectPulseMod["ModulationType"].toInt());
                if (objectPulseMod.contains("LfmParams")) {
                    QJsonObject objectLfm = objectPulseMod["LfmParams"].toObject();
                    if (objectLfm.contains("Freq")) pulse_modulation.m_freq.setNum(objectLfm["Freq"].toDouble());
                    if (objectLfm.contains("Deviation")) pulse_modulation.m_deviation.setNum(objectLfm["Deviation"].toDouble());
                    if (objectLfm.contains("Direction")) pulse_modulation.m_directionLfm.setNum(objectLfm["Direction"].toInt());
                }
                if (objectPulseMod.contains("PskParams")) {
                    QJsonObject objectPsk = objectPulseMod["PskParams"].toObject();
                    if (objectPsk.contains("ChipWidth")) pulse_modulation.m_chipWidth.setNum(objectPsk["ChipWidth"].toDouble());
                    if (objectPsk.contains("ChipCount")) pulse_modulation.m_chipCnt.setNum(objectPsk["ChipCount"].toInt());
                    if (objectPsk.contains("Direction")) pulse_modulation.m_directionPsk.setNum(objectPsk["Direction"].toInt());
                    if (objectPsk.contains("Type")) pulse_modulation.m_typePsk = objectPsk["Type"].toString();
                }
                m_pulseMod.append(pulse_modulation);
            }
        }
        if (obj.contains("RotationPeriod")) m_rotationPeriod.setNum(obj["RotationPeriod"].toDouble());
        if (obj.contains("RlsMode")) m_rlsMode = obj["RlsMode"].toString();
        if (obj.contains("Bearings")) {
            QJsonArray arr = obj["Bearings"].toArray();
            for(int i = 0; i < arr.size(); i++) {
                QJsonObject objectBearings = arr[i].toObject();
                Bearings bearings;
                if (objectBearings.contains("Lat")) bearings.m_lat.setNum(objectBearings["Lat"].toDouble());
                if (objectBearings.contains("Lon")) bearings.m_lon.setNum(objectBearings["Lon"].toDouble());
                if (objectBearings.contains("Alt")) bearings.m_alt.setNum(objectBearings["Alt"].toDouble());
                if (objectBearings.contains("Bearing")) bearings.m_bearing.setNum(objectBearings["Bearing"].toDouble());
                if (objectBearings.contains("Timestamp")) bearings.m_timeStamp =
                        QDateTime::fromMSecsSinceEpoch(static_cast<int64_t>(objectBearings["Timestamp"].toDouble())).
                                                          toUTC().toString("yyyy-MM-dd hh:mm:ss.zzz");
                if (objectBearings.contains("Quality")) bearings.m_quality.setNum(objectBearings["Quality"].toDouble());
                if (objectBearings.contains("Std")) bearings.m_std.setNum(objectBearings["Std"].toDouble());
                if (objectBearings.contains("DroneNumber")) bearings.m_droneNumber = objectBearings["DroneNumber"].toString();
                m_bearings.append(bearings);
            }
        }
        if (obj.contains("LocalityName")) m_localityName = obj["LocalityName"].toString();
        if (obj.contains("AreaName")) m_areaName = obj["AreaName"].toString();
        if (obj.contains("BeginTime")) m_beginTime =
                QDateTime::fromMSecsSinceEpoch(static_cast<int64_t>(obj["BeginTime"].toDouble())).
                                                  toUTC().toString("yyyy-MM-dd hh:mm:ss.zzz");
        if (obj.contains("EndTime")) m_endTime =
                QDateTime::fromMSecsSinceEpoch(static_cast<int64_t>(obj["EndTime"].toDouble())).
                                                  toUTC().toString("yyyy-MM-dd hh:mm:ss.zzz");
        if (obj.contains("SourceLocations")) {
            QJsonArray arr = obj["SourceLocations"].toArray();
            for(int i = 0; i < arr.size(); i++) {
                QJsonObject objectSourceLoc = arr[i].toObject();
                SourceLocations source_locations;
                if (objectSourceLoc.contains("Lat"))
                    source_locations.m_lat = objectSourceLoc["Lat"].toDouble();
                if (objectSourceLoc.contains("Lon"))
                    source_locations.m_lon = objectSourceLoc["Lon"].toDouble();
                if (objectSourceLoc.contains("Alt")) source_locations.m_alt = objectSourceLoc["Alt"].toDouble();
                if (objectSourceLoc.contains("Heading")) source_locations.m_heading.setNum(objectSourceLoc["Heading"].toInt());
                if (objectSourceLoc.contains("Speed")) source_locations.m_speed.setNum(objectSourceLoc["Speed"].toInt());
                if (objectSourceLoc.contains("Timestamp")) source_locations.m_timeStamp =
                        QDateTime::fromMSecsSinceEpoch(static_cast<int64_t>(objectSourceLoc["Timestamp"].toDouble())).
                                                          toUTC().toString("yyyy-MM-dd hh:mm:ss.zzz");
                if (objectSourceLoc.contains("StdEllipse")) {
                    QJsonObject objectPsk = objectSourceLoc["StdEllipse"].toObject();
                    if (objectPsk.contains("a")) source_locations.m_a.setNum(objectPsk["a"].toDouble());
                    if (objectPsk.contains("b")) source_locations.m_b.setNum(objectPsk["b"].toDouble());
                    if (objectPsk.contains("phi")) source_locations.m_phi.setNum(objectPsk["phi"].toDouble());
                }
                m_sourceLoc.append(source_locations);
            }
        }
    }

};
//---------------------------------------------------------
struct ZoneHeaderKoir
{ // заголовок сообщения о зоне закрытия для статических камер (0x728) от КОИР версия 2.3 и выше
    // 1
    quint32 m_camNumb:8;  // номера камер
    quint32 m_format:8;  // формат изображения
    quint32 m_reserve:16;
};
static_assert (sizeof (ZoneHeaderKoir) == 4, "Размер структуры ZoneHeaderKoir не совпадает с заданным");
#pragma pack(pop)

#endif // CELLTYPES_H
