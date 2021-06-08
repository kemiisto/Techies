#include "Creep.h"

#include <utility>
#include "MeleeCreep.h"
#include "RangedCreep.h"
#include "SiegeCreep.h"

using namespace cocos2d;

float randomFloat(const float min, const float max) {
    static std::default_random_engine e;
    static std::uniform_real_distribution dis(min, max);
    return dis(e);
}

Creep::Creep(std::string folderName, AnimationsInfo animationsInfo) :
        Sprite(),
        folderName(std::move(folderName)),
        animationsInfo(std::move(animationsInfo)),
        animateRun(nullptr),
        animateDie(nullptr) {
}

Creep::~Creep() {
    CC_SAFE_RELEASE(animateRun);
    CC_SAFE_RELEASE(animateDie);
}

void Creep::reset(const Size& screenSize, const std::function<void(Node*)>& func) {
    const float x = randomFloat(screenSize.width * 0.1f, screenSize.width * 0.9f);
    
    setPosition(Vec2(x, screenSize.height + getBoundingBox().size.height * 0.5f));
    setVisible(true);

    const auto sequenceAction = Sequence::create(
        MoveTo::create(15.0f, Vec2(x, 0.0f)),
        CallFuncN::create(func),
        nullptr
    );
    runAction(animateRun);
    runAction(sequenceAction);
}

void Creep::die() {
    stopAllActions();
    runAction(
	    Sequence::create(
            animateDie,
            FadeOut::create(0.5f),
            CallFunc::create([this] { setVisible(false); }),
            nullptr
        )
    );
}

Creep* Creep::create(const Type creepType) {
    Creep* creep = nullptr;
	
    switch (creepType) {
        case Type::Melee:
            creep = new MeleeCreep();
            break;
        case Type::Ranged:
            creep = new RangedCreep();
            break;
        case Type::Siege:
            creep = new SiegeCreep();
            break;
    }

    if (creep) {
        int idx = 1 + rand() % creep->animationsInfo.alternativesCount;
        if (creep->initWithFile(StringUtils::format("%s/run/%02d/%02d.png", creep->folderName.c_str(), idx, 0))) {
            auto runAnimation = Animation::create();
            for (int i = 0; i < creep->animationsInfo.runCounts[idx - 1]; ++i) {
                auto name = StringUtils::format("%s/run/%02d/%02d.png", creep->folderName.c_str(), idx, i);
                runAnimation->addSpriteFrameWithFile(name);
            }
            runAnimation->setDelayPerUnit(0.04f);
            runAnimation->setRestoreOriginalFrame(true);
            runAnimation->setLoops(-1);
            creep->animateRun = Animate::create(runAnimation);
            creep->animateRun->retain();

            idx = 1 + rand() % creep->animationsInfo.alternativesCount;
            auto deathAnimation = Animation::create();
            for (int i = 0; i < creep->animationsInfo.dieCounts[idx - 1]; ++i) {
                auto name = StringUtils::format("%s/die/%02d/%02d.png", creep->folderName.c_str(), idx, i);
                deathAnimation->addSpriteFrameWithFile(name);
            }
            deathAnimation->setDelayPerUnit(0.025f);
            deathAnimation->setRestoreOriginalFrame(false);
            deathAnimation->setLoops(1);
            creep->animateDie = Animate::create(deathAnimation);
            creep->animateDie->retain();

            creep->autorelease();
            return creep;
        }
    }
	
    CC_SAFE_DELETE(creep);
    return nullptr;
}
