#include "SiegeCreep.h"

using namespace cocos2d;

SiegeCreep::SiegeCreep(int value, int damage) :
        Creep("creep_siege", {1, {49}, {51}}, value, damage) {
}

