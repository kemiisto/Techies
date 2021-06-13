#include "CreepFactory.h"

#include "ccUTF8.h"
#include "MeleeCreep.h"
#include "RangedCreep.h"
#include "SiegeCreep.h"

using namespace cocos2d;

Creep* CreepFactory::create(Creep::Type creepType) {
    Creep* creep = nullptr;

    switch (creepType) {
    case Creep::Type::Melee:
        creep = new (std::nothrow) MeleeCreep();
        break;
    case Creep::Type::Ranged:
        creep = new (std::nothrow) RangedCreep();
        break;
    case Creep::Type::Siege:
        creep = new (std::nothrow) SiegeCreep();
        break;
    }

    if (creep) {
        int alternative = 1 + rand() % creep->animationsInfo.alternativesCount;
        if (creep->initWithFile(StringUtils::format("%s/run/%02d/%02d.png", creep->folderName.c_str(), alternative, 0))) {
            creep->createRunAnimation(alternative);
            creep->createDieAnimation();
            creep->autorelease();
            return creep;
        }
    }

    CC_SAFE_DELETE(creep);
    return nullptr;
}
