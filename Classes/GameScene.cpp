#include "GameScene.h"

#include <fstream>

#include "CCDirector.h"
#include "CCEventDispatcher.h"
#include "CCEventListenerTouch.h"
#include "CCFileUtils.h"
#include "ccUTF8.h"
#include "2d/CCLabel.h"
#include "cocostudio/SimpleAudioEngine.h"
#include "json/document.h"
#include "json/istreamwrapper.h"

using namespace cocos2d;
using namespace CocosDenshion;
using namespace rapidjson;

const Color4F gray(0.501f, 0.501f, 0.501f, 1.0f);
const Color4F transparentRed(1.0f, 0.0f, 0.0f, 0.25f);

const std::string fontFilePath = "fonts/Pricedown.ttf";
const float smallFontSize = 60;
const float bigFontSize = smallFontSize * 3;

const float iconSize = 125;

Creep::Type creepTypeFromString(const std::string& s) {
	if (s == "Melee") {
        return Creep::Type::Melee;
	}
	if (s == "Ranged") {
		return Creep::Type::Ranged;
	}
	if (s == "Siege") {
		return Creep::Type::Siege;
	}
	throw std::invalid_argument("Unknown creep type!");
}

GameScene::GameScene() :
		state(GameState::Launched),
		screenSize(Director::getInstance()->getWinSize()),
		creeps(this),
		hudDrawNode(nullptr),
		forbiddenRegionDrawNode(nullptr),
		remoteMine(nullptr),
		proximityMine(nullptr),
		techies(nullptr),
		creepsSpawnTimers{
			{Creep::Type::Melee, 2.0f},
			{Creep::Type::Ranged, 0.0f},
			{Creep::Type::Siege, 0.0f}
		},
		score(0),
		health(100),
		scoreLabel(nullptr),
		healthLabel(nullptr),
		playLabel(nullptr),
		gameOverLabel(nullptr),
		tryAgainLabel(nullptr) {
}

Scene* GameScene::createScene() {
    return create();
}

bool GameScene::init() {
    if (!Scene::init()) {
        return false;
    }

    readConfig();
    
    createBackground();
    createLabels();
    createTechies();
    createRemoteMine();
    createProximityMine();
    createHUD();
    createForbiddenRegionDrawNode();

    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = CC_CALLBACK_2(GameScene::onTouchBegan, this);
    listener->onTouchMoved = CC_CALLBACK_2(GameScene::onTouchMoved, this);
    listener->onTouchEnded = CC_CALLBACK_2(GameScene::onTouchEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
    
    scheduleUpdate();
    
    return true;
}

void GameScene::stop() {
    state = GameState::Over;
    for (auto creep : runningCreeps) {
        creep->stopAllActions();
    }
    proximityMine->stopAllActions();
    remoteMine->stopAllActions();
    techies->stopAllActions();
    unscheduleUpdate();
}

void GameScene::createBackground() {
    auto sprite = Sprite::create("Snow.jpg");
    sprite->setPosition(screenSize/2);
    addChild(sprite, 0);
}

void GameScene::createHUD() {
    scoreLabel = Label::createWithTTF("SCORE: 0", fontFilePath, smallFontSize);
    scoreLabel->setAnchorPoint(Vec2(1.0, 0.0));
    scoreLabel->setPosition(Vec2(screenSize.width-10, 0));
    scoreLabel->enableOutline(Color4B::BLACK, 3);
    addChild(scoreLabel, 1);
    
    healthLabel = Label::createWithTTF("HEALTH: 100%", fontFilePath, smallFontSize);
    healthLabel->setAnchorPoint(Vec2(0.0, 0.0));
    healthLabel->setPosition(Vec2(10, 0));
    healthLabel->enableOutline(Color4B::BLACK, 3);
    addChild(healthLabel, 1);

    const auto remoteMineIconRect = Rect(screenSize.width / 2, 0, iconSize, iconSize);
    const auto proximityMineIconRect = Rect(screenSize.width / 2 - iconSize, 0, iconSize, iconSize);

    hudDrawNode = DrawNode::create();
    hudDrawNode->drawSolidRect(proximityMineIconRect.origin, proximityMineIconRect.origin + proximityMineIconRect.size, gray);
    hudDrawNode->drawRect(proximityMineIconRect.origin, proximityMineIconRect.origin + proximityMineIconRect.size, Color4F::WHITE);
    hudDrawNode->drawSolidRect(remoteMineIconRect.origin, remoteMineIconRect.origin + remoteMineIconRect.size, gray);
    hudDrawNode->drawRect(remoteMineIconRect.origin, remoteMineIconRect.origin + remoteMineIconRect.size, Color4F::WHITE);
    addChild(hudDrawNode, 1);
}

void GameScene::createForbiddenRegionDrawNode() {
    forbiddenRegionDrawNode = DrawNode::create();
    const auto radius = techies->getBoundingBox().getMaxY();
    forbiddenRegionDrawNode->drawSolidCircle(Vec2(screenSize.width / 2, 0), radius, 0, 30, 1, 1, transparentRed);
    forbiddenRegionDrawNode->setVisible(false);
    addChild(forbiddenRegionDrawNode, 0);
}

void GameScene::createLabels() {
    playLabel = Label::createWithTTF("PLAY!", fontFilePath, bigFontSize);
    playLabel->setPosition(screenSize/2);
    playLabel->enableOutline(Color4B::BLACK, 3);
    addChild(playLabel, 2);
    
    gameOverLabel = Label::createWithTTF("WASTED", fontFilePath, bigFontSize);
    gameOverLabel->setPosition(Vec2(screenSize/2) + Vec2(0, bigFontSize));
    gameOverLabel->setColor(Color3B::RED);
    gameOverLabel->enableOutline(Color4B::BLACK, 3);
    gameOverLabel->setVisible(false);
    addChild(gameOverLabel, 2);

    tryAgainLabel = Label::createWithTTF("TRY AGAIN!", fontFilePath, bigFontSize);
    tryAgainLabel->setPosition(Vec2(screenSize / 2) - Vec2(0, bigFontSize));
    tryAgainLabel->enableOutline(Color4B::BLACK, 3);
    tryAgainLabel->setVisible(false);
    addChild(tryAgainLabel, 2);
}

void GameScene::readConfig() {
    std::ifstream ifs(FileUtils::getInstance()->fullPathForFilename("config.json"));
    IStreamWrapper isw(ifs);
    Document d;
    d.ParseStream(isw);

    for (const auto& [name, value] : d["creepsSpawnIntervals"].GetObject()) {
        creepsSpawnIntervals[creepTypeFromString(name.GetString())] = value.GetFloat();
    }
}

void GameScene::createRemoteMine() {
    remoteMine = Mine::create(this, "fx_techies_remotebomb.png", Vec2(screenSize.width/2 + 60, 60));
    addChild(remoteMine, 2);
    mines.push_back(remoteMine);
}

void GameScene::createProximityMine() {
    proximityMine = Mine::create(this, "fx_techiesfx_mine.png", Vec2(screenSize.width/2 - 60, 60));
    addChild(proximityMine, 2);
    mines.push_back(proximityMine);
}

void GameScene::createTechies() {
    techies = Techies::create();
	techies->setAnchorPoint(Vec2(0.5, 0));
	techies->setPosition(Vec2(screenSize.width / 2, 0));
    addChild(techies, 1);
}

void GameScene::updateHUD() {
    scoreLabel->setString(StringUtils::format("SCORE: %d", score));
    healthLabel->setString(StringUtils::format("HEALTH: %d%%", health));
}

void GameScene::spawnCreep(const Creep::Type& creepType) {
    if (runningCreeps.size() >= creeps.size()) {
        return;
    }
    auto creep = creeps.get(creepType);
    creep->spawn(screenSize, std::bind(&GameScene::creepReachedTheEnd, this, creep));
    runningCreeps.push_back(creep);
}

void GameScene::checkCollisionsWithCrater(const Sprite* const crater) {
    for (auto it = runningCreeps.begin(); it != runningCreeps.end();) {
        auto object = *it;
        if (crater->getBoundingBox().intersectsRect(object->getBoundingBox())) {
            object->die();
            score += object->value();
            it = runningCreeps.erase(it);
        } else {
            ++it;
        }
    }
}

void GameScene::checkCollisionsWithProximityMine() {
    for (auto it = runningCreeps.begin(); it != runningCreeps.end();) {
        auto object = *it;
        if (proximityMine->getBoundingBox().intersectsRect(object->getBoundingBox())) {
            proximityMine->detonate();
            break;
        }
        ++it;
    }
}

void GameScene::update(float dt) {
    if (state != GameState::Running) {
        return;
    }

    for (auto& [creepType, t] : creepsSpawnTimers) {
        t += dt;
    	if (t > creepsSpawnIntervals[creepType]) {
            t = 0;
            spawnCreep(creepType);
    	}
    }
    
    if (proximityMine->isPlanted()) {
        checkCollisionsWithProximityMine();
    }
    
    if (proximityMine->getCrater()->isVisible()) {
        checkCollisionsWithCrater(proximityMine->getCrater());
        updateHUD();
    }
    
    if (remoteMine->getCrater()->isVisible()) {
        checkCollisionsWithCrater(remoteMine->getCrater());
        updateHUD();
    }
    
    proximityMine->setPosition(proximityMine->getNextPosition());
    remoteMine->setPosition(remoteMine->getNextPosition());
}

bool GameScene::onTouchBegan(Touch* touch, Event* event) {
    switch (state) {
        case GameState::Launched:
            playLabel->setVisible(false);
            state = GameState::Running;
            break;
        case GameState::Running:
            if (touch) {
                if (!techies->isIdle() || proximityMine->isFlying() || remoteMine->isFlying()) {
                    return false;
                }
                const auto touchLocation = touch->getLocation();
                for (auto mine : mines) {
                    if (mine->getBoundingBox().containsPoint(touchLocation) && !mine->isPlanted()) {
                        forbiddenRegionDrawNode->setVisible(true);
                        mine->setTouch(touch);
                        mine->setOpacity(128);
                        mine->setScale(0.25f);
                    }
                }
                if (remoteMine->getBoundingBox().containsPoint(touchLocation) && remoteMine->isPlanted()) {
                    remoteMine->detonate();
                }
            }
            break;
        case GameState::Over:
            auto scene = GameScene::createScene();
            Director::getInstance()->replaceScene(scene);
            break;
    }
    return true;
}

void GameScene::keepMineInsideScreen(Mine* mine, Point& nextPosition) const {
    const float radius = mine->radius();
    if (nextPosition.x < radius) {
        nextPosition.x = radius;
    }
    if (nextPosition.x > screenSize.width - radius) {
        nextPosition.x = screenSize.width - radius;
    }
    if (nextPosition.y < radius) {
        nextPosition.y = radius;
    }
    if (nextPosition.y > screenSize.height - radius) {
        nextPosition.y = screenSize.height - radius;
    }
}

void GameScene::onTouchMoved(Touch* touch, Event* event) {
    auto touchLocation = touch->getLocation();
    for (auto mine : mines) {
        if (mine->getTouch() && mine->getTouch() == touch) {
            keepMineInsideScreen(proximityMine, touchLocation);
            mine->setNextPosition(touchLocation);
            const auto radius = techies->getBoundingBox().getMaxY();
            const auto v = touchLocation - Vec2(screenSize.width / 2, 0);
            if (v.length() <= radius) {
                mine->setColor(Color3B::BLACK);
            } else {
                mine->setColor(Color3B::WHITE);
            }
        }
    }
}

void GameScene::onTouchEnded(Touch* touch, Event* event) {
    auto touchLocation = touch->getLocation();
    for (auto mine : mines) {
        if (mine->getTouch() && mine->getTouch() == touch) {
            keepMineInsideScreen(proximityMine, touchLocation);
            const auto radius = techies->getBoundingBox().getMaxY();
            const auto v = touchLocation - Vec2(screenSize.width / 2, 0);
            if (v.length() <= radius) {
                mine->setColor(Color3B::WHITE);
                mine->returnToHUD();
                SimpleAudioEngine::getInstance()->playEffect("Mine_Error.mp3");
            } else {
                techies->plantMine(mine, touchLocation);
                SimpleAudioEngine::getInstance()->playEffect("Mine_Spawn.mp3");
            }
            mine->setTouch(nullptr);
            forbiddenRegionDrawNode->setVisible(false);
        }
    }
}

void GameScene::creepReachedTheEnd(Node* node) {
	auto creep = dynamic_cast<Creep*>(node);
    runningCreeps.erase(std::remove(runningCreeps.begin(), runningCreeps.end(), creep));
    node->stopAllActions();
    changeHealth(-creep->damage());
    node->setVisible(false);
}

void GameScene::changeHealth(const int value) {
    health += value;
    if (health <= 0) {
        health = 0;
        stop();
        gameOverLabel->setVisible(true);
        tryAgainLabel->setVisible(true);
        SimpleAudioEngine::getInstance()->playEffect("Wasted.mp3");
    }
    if (health > 100) {
        health = 100;
    }
    updateHUD();
}
