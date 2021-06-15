#include "RangedCreep.h"

using namespace cocos2d;

RangedCreep::RangedCreep(int value, int damage) :
        Creep("creep_ranged", {2, {20, 23}, {40, 40}}, value, damage) {
}
