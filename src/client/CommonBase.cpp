#include "CommonBase.hpp"

CommonBase::CommonBase() {

}

CommonBase::~CommonBase() {

}

qint16 CommonBase::calcChcekSum(const char* sMess,int nCnt) {
    quint16 nSum = 0;
    quint8 *pb = (quint8*)sMess;
    for(int i = 0; i < nCnt; i++) nSum += *pb++;
    return nSum;
}