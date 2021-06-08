#include "MeleeCreep.h"

using namespace cocos2d;

MeleeCreep::MeleeCreep() :
        Creep("creep_melee", {3, {24, 48, 45}, {52, 52, 45}}) {
}

int MeleeCreep::value() const {
    return 1;
}

int MeleeCreep::damage() const {
    return 10;
}
