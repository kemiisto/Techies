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

#include "Ui.h"

using namespace cocos2d;
using namespace CocosDenshion;
using namespace rapidjson;

#define GET_VARIABLE_NAME(Variable) (#Variable)

const auto gameOverDelay = 2.0f;

const Color4F gray(0.501f, 0.501f, 0.501f, 1.0f);
const Color4F transparentRed(1.0f, 0.0f, 0.0f, 0.25f);

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

void readConfig(
		std::map<Creep::Type, float>& creepSpawnInterval,
		std::map<Creep::Type, int>& creepValue,
		std::map<Creep::Type, float>& creepDamage) {
    std::ifstream ifs(FileUtils::getInstance()->fullPathForFilename("config.json"));
    IStreamWrapper isw(ifs);
    Document d;
    d.ParseStream(isw);

    for (const auto& [name, value] : d["creepSpawnInterval"].GetObject()) {
        creepSpawnInterval[creepTypeFromString(name.GetString())] = value.GetFloat();
    }

    for (const auto& [name, value] : d["creepValue"].GetObject()) {
        creepValue[creepTypeFromString(name.GetString())] = value.GetInt();
    }


    for (const auto& [name, value] : d["creepDamage"].GetObject()) {
        creepDamage[creepTypeFromString(name.GetString())] = value.GetFloat();
    }
}

GameScene::GameScene() :
		state(GameState::Launched),
		screenSize(Director::getInstance()->getWinSize()),
		hudDrawNode(nullptr),
		forbiddenRegionDrawNode(nullptr),
		remoteMine(nullptr),
		proximityMine(nullptr),
		techies(nullptr),
		creepSpawnTimer{
			{Creep::Type::Melee, 2.0f},
			{Creep::Type::Ranged, 0.0f},
			{Creep::Type::Siege, 0.0f}
		},
		gameOverDelayTimer(0.0f),
		score(0),
		health(100),
		scoreLabel(nullptr),
		healthLabel(nullptr),
		playLabel(nullptr),
		gameOverLabel(nullptr),
		tryAgainLabel(nullptr) {
}

bool GameScene::init() {
    if (!Scene::init()) {
        return false;
    }

    std::map<Creep::Type, int> creepValue;
    std::map<Creep::Type, float> creepDamage;
    readConfig(creepSpawnInterval, creepValue, creepDamage);
    creeps = std::make_unique<Creeps>(this, creepValue, creepDamage);
	
    const auto ui = Ui("ui.json");
    createBackground();
    createLabels(ui);
    createTechies();
    createRemoteMine();
    createProximityMine();
    createHUD(ui);
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
    proximityMine->stop();
    remoteMine->stop();
    techies->stopAllActions();
    //unscheduleUpdate();
}

void GameScene::createBackground() {
    auto sprite = Sprite::create("Snow.jpg");
    sprite->setPosition(screenSize/2);
    addChild(sprite, 0);
}

void GameScene::createHUD(const Ui& ui) {
    scoreLabel = ui.createLabel(GET_VARIABLE_NAME(scoreLabel), screenSize);
    addChild(scoreLabel, 1);

    healthLabel = ui.createLabel(GET_VARIABLE_NAME(healthLabel), screenSize);
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

void GameScene::createLabels(const Ui& ui) {
    playLabel = ui.createLabel(GET_VARIABLE_NAME(playLabel), screenSize);
    addChild(playLabel, 2);
    
    gameOverLabel = ui.createLabel(GET_VARIABLE_NAME(gameOverLabel), screenSize);
    gameOverLabel->setVisible(false);
    addChild(gameOverLabel, 2);

    tryAgainLabel = ui.createLabel(GET_VARIABLE_NAME(tryAgainLabel), screenSize);
    tryAgainLabel->setVisible(false);
    addChild(tryAgainLabel, 2);
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
    if (runningCreeps.size() >= creeps->size()) {
        return;
    }
    auto creep = creeps->get(creepType);
    creep->spawn(screenSize, std::bind(&GameScene::creepReachedTheEnd, this, creep));
    runningCreeps.push_back(creep);
}

void GameScene::checkCollisionsWithCrater(const Sprite* const crater) {
    for (auto it = runningCreeps.begin(); it != runningCreeps.end();) {
        auto creep = *it;
        if (crater->getBoundingBox().intersectsRect(creep->getBoundingBox())) {
            creep->die();
            score += creep->getValue();
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
	if (state == GameState::Over) {
        gameOverDelayTimer += dt;
	}
	
    if (state != GameState::Running) {
        return;
    }

    for (auto& [creepType, t] : creepSpawnTimer) {
        t += dt;
    	if (t > creepSpawnInterval[creepType]) {
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
                    }
                }
                if (remoteMine->getBoundingBox().containsPoint(touchLocation) && remoteMine->isPlanted()) {
                    remoteMine->detonate();
                }
            }
            break;
        case GameState::Over:
            if (gameOverDelayTimer >= gameOverDelay) {
                auto scene = GameScene::create();
                Director::getInstance()->replaceScene(scene);
            }
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
    changeHealth(-creep->getDamage());
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
