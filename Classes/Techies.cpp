#include "Techies.h"

#include "Mine.h"
#include "cocostudio/SimpleAudioEngine.h"

using namespace cocos2d;
using namespace CocosDenshion;

Techies::Techies(const cocos2d::Vec2& position) : Sprite(),
		minePlantingAnimation(nullptr),
		state(State::Idle) {
}

Techies::~Techies() {
    minePlantingAnimation->release();
}

Techies* Techies::create(const cocos2d::Vec2& position) {
    auto object = new Techies(position);
    if (object->initWithFile("techies/00.png")) {
        object->setAnchorPoint(Vec2(0.5, 0));
        object->setPosition(position);
        object->createMinePlantingAnimation();
        object->autorelease();
        return object;
    }
    CC_SAFE_DELETE(object);
    return nullptr;
}



void Techies::plantMine(Mine* mine, const cocos2d::Vec2& location) {
    assert(state == State::Idle);
	
    const auto v = location - getPosition();
    const float angle = 90.0f - CC_RADIANS_TO_DEGREES(v.getAngle());

    const auto size = getBoundingBox().size.height * 0.85f;
    const auto v1 = getPosition();
    const auto v2 = location;
    const auto distance = v1.distance(v2);
    const float x = v1.x + size / distance * (v2.x - v1.x);
    const float y = v1.y + size / distance * (v2.y - v1.y);

    mine->setFlyingSpritePosition(Vec2(x, y));
    state = State::PlantingMine;
    runAction(
        Sequence::create(
            RotateTo::create(0.15f, angle),
            Animate::create(minePlantingAnimation),
            CallFunc::create([this] { setIdle(); }),
            CallFunc::create([mine] { mine->showFlyingSprite(); }),
            nullptr
        )
    );

    mine->flyTo(v2);

    SimpleAudioEngine::getInstance()->playEffect("Mine_Spawn.mp3");

    mine->setTouch(nullptr);
}

bool Techies::isIdle() const {
    return state == State::Idle;
}

void Techies::setIdle() {
    state = State::Idle;
}

void Techies::createMinePlantingAnimation() {
    minePlantingAnimation = Animation::create();
    for (int i = 0; i < 44; ++i) {
        auto name = StringUtils::format("techies/%02d.png", i);
        minePlantingAnimation->addSpriteFrameWithFile(name);
    }
    minePlantingAnimation->setDelayPerUnit(0.03f);
    minePlantingAnimation->setRestoreOriginalFrame(true);
    minePlantingAnimation->setLoops(1);
    minePlantingAnimation->retain();
}
