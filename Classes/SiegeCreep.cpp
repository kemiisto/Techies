#include "SiegeCreep.h"

using namespace cocos2d;

SiegeCreep::SiegeCreep() :
        Creep("creep_siege", {1, {49}, {51}}) {
}

int SiegeCreep::value() const {
    return 3;
}

int SiegeCreep::damage() const {
    return 30;
}
