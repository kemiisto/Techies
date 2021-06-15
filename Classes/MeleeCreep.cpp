#include "MeleeCreep.h"

using namespace cocos2d;

MeleeCreep::MeleeCreep(int value, int damage) :
        Creep("creep_melee", {3, {24, 48, 45}, {52, 52, 45}}, value, damage) {
}
