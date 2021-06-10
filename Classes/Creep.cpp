#include "Creep.h"

#include <utility>

#include "ccUTF8.h"
#include "2d/CCActionInterval.h"
#include "2d/CCActionInstant.h"

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

void Creep::createRunAnimation(int alternative) {
    auto runAnimation = Animation::create();
    for (int i = 0; i < animationsInfo.runCounts[alternative - 1]; ++i) {
        auto name = StringUtils::format("%s/run/%02d/%02d.png", folderName.c_str(), alternative, i);
        runAnimation->addSpriteFrameWithFile(name);
    }
    runAnimation->setDelayPerUnit(0.04f);
    runAnimation->setRestoreOriginalFrame(true);
    runAnimation->setLoops(-1);
    animateRun = Animate::create(runAnimation);
    animateRun->retain();
}

void Creep::createDieAnimation() {
    int alternative = 1 + rand() % animationsInfo.alternativesCount;
    auto deathAnimation = Animation::create();
    for (int i = 0; i < animationsInfo.dieCounts[alternative - 1]; ++i) {
        auto name = StringUtils::format("%s/die/%02d/%02d.png", folderName.c_str(), alternative, i);
        deathAnimation->addSpriteFrameWithFile(name);
    }
    deathAnimation->setDelayPerUnit(0.025f);
    deathAnimation->setRestoreOriginalFrame(false);
    deathAnimation->setLoops(1);
    animateDie = Animate::create(deathAnimation);
    animateDie->retain();
}

Creep::~Creep() {
    CC_SAFE_RELEASE(animateRun);
    CC_SAFE_RELEASE(animateDie);
}

void Creep::reset(const Size& screenSize, const std::function<void(Node*)>& func) {
    const float x = randomFloat(screenSize.width * 0.1f, screenSize.width * 0.9f);
    
    setPosition(Vec2(x, screenSize.height + getBoundingBox().size.height * 0.5f));
    setVisible(true);

    auto sequenceAction = Sequence::create(
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
