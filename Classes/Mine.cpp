#include "Mine.h"

#include "2d/CCActionInstant.h"
#include "2d/CCActionInterval.h"
#include "2d/CCParticleSystemQuad.h"
#include "cocostudio/SimpleAudioEngine.h"

#include "GameScene.h"

using namespace cocos2d;
using namespace CocosDenshion;

Mine::Mine(const Vec2& position) : Sprite(),
        originalPosition(position) {
}

Mine* Mine::create(GameScene* scene, const std::string& filename, const Vec2& position) {
    auto object = new (std::nothrow) Mine(position);
    if (object && object->initWithFile(filename)) {
        object->setPosition(position);
        object->createCrater(scene);
        object->createFlyingSprite(scene);
        object->createExplosion(scene);
        object->createSmoke(scene);
        object->autorelease();
        return object;
    }
    CC_SAFE_DELETE(object);
    return nullptr;
}

void Mine::setFlyingSpritePosition(const Vec2& v) {
    flyingSprite->setPosition(v);
}

void Mine::showFlyingSprite() {
    flyingSprite->setVisible(true);
}

void Mine::hideFlyingSprite() {
    flyingSprite->setVisible(false);
}

void Mine::flyTo(const Vec2& v) {
	flyingSprite->runAction(
		Sequence::create(
			DelayTime::create(1.5f),
			CallFunc::create([this] { fly(); }),
			MoveTo::create(0.5f, v),
			CallFunc::create([this] { flyingSprite->setVisible(false); }),
			CallFunc::create([this] { plant(); }),
            nullptr
        )
    );
}

void Mine::returnToHUD() {
    setPosition(originalPosition);
    state = State::Created;
    setOpacity(255);
}

void Mine::detonate() {
    SimpleAudioEngine::getInstance()->playEffect("Mine_Detonate.mp3");
    
    explosion->setPosition(getPosition());
    explosion->resetSystem();
    
    smoke->setPosition(getPosition());
    smoke->resetSystem();
    
    crater->setPosition(getPosition());
    crater->runAction(
        Sequence::create(
            CallFunc::create([this] { crater->setVisible(true); }),
            DelayTime::create(0.5f),
            CallFunc::create([this] { crater->setVisible(false); }),
            nullptr
        )
    );
    returnToHUD();
}

const Sprite* Mine::getCrater() const {
    return crater;
}

void Mine::createCrater(GameScene* scene) {
    crater = Sprite::create("Crater.png");
    crater->setScale(2.0f);
    crater->setOpacity(0);
    crater->setVisible(false);
    scene->addChild(crater, 1);
}

void Mine::createFlyingSprite(GameScene* scene) {
    flyingSprite = createWithTexture(getTexture());
    flyingSprite->setVisible(false);
    scene->addChild(flyingSprite);
}

void Mine::createExplosion(GameScene* scene) {
    explosion = ParticleSystemQuad::create("boom.plist");
    explosion->setBlendFunc(BlendFunc::ADDITIVE);
    explosion->stopSystem();
    scene->addChild(explosion, 1);
}

void Mine::createSmoke(GameScene* scene) {
    smoke = ParticleSystemQuad::create("smoke.plist");
    smoke->setBlendFunc(BlendFunc::ADDITIVE);
    smoke->stopSystem();
    scene->addChild(smoke, 1);
}

void Mine::setPosition(const Vec2& v) {
    Sprite::setPosition(v);
    if (!nextPosition.equals(v)) {
        nextPosition = v;
    }
}

float Mine::radius() const {
    return getBoundingBox().size.width * 0.5f;
}

bool Mine::isPlanted() const { 
    return state == State::Planted;
}

void Mine::plant() {
    assert(state == State::Flying);
    assert(getOpacity() == 128);
	
    state = State::Planted;
    setOpacity(255);
}

bool Mine::isFlying() const {
    return state == State::Flying;
}

void Mine::fly() {
    assert(state == State::Created);
    state = State::Flying;
}

const Touch* Mine::getTouch() const { 
    return touch;
}

void Mine::setTouch(Touch* t) {
    touch = t;
}

void Mine::stop() {
    stopAllActions();
    explosion->stopSystem();
    smoke->stopSystem();
    flyingSprite->stopAllActions();
    crater->stopAllActions();
}

const Vec2& Mine::getNextPosition() const {
    return nextPosition;
}

void Mine::setNextPosition(const Vec2& v) {
    nextPosition = v;
}
