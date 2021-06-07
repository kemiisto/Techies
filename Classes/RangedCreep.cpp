#include "RangedCreep.h"

using namespace cocos2d;
using namespace techies;

RangedCreep::RangedCreep() :
        Creep("creep_ranged", {2, {20, 23}, {40, 40}}) {
}

int RangedCreep::value() const {
    return 2;
}

int RangedCreep::damage() const {
    return 20;
}
