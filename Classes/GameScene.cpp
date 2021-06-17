#include "GameScene.h"

#include <fstream>

#include "CCDirector.h"
#include "CCEventDispatcher.h"
#include "CCEventListenerTouch.h"
#include "CCFileUtils.h"
#include "ccUTF8.h"
#include "2d/CCLabel.h"
#include "cocostudio/SimpleAudioEngine.h"
#include "ui/CocosGUI.h"
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
    createHud(ui);
    createRemoteMine(ui);
    createProximityMine(ui);

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

bool GameScene::intersectsHud(cocos2d::Sprite* sprite) const {
    const auto& mineBoundingBox = sprite->getBoundingBox();
    return
		mineBoundingBox.intersectsRect(topLeftHud->getBoundingBox()) ||
			mineBoundingBox.intersectsRect(bottomLeftHudSprite->getBoundingBox()) ||
				mineBoundingBox.intersectsRect(bottomRightHudSprite->getBoundingBox()) ||
					mineBoundingBox.intersectsRect(topRightHud->getBoundingBox());
}

void GameScene::createBackground() {
    auto sprite = Sprite::create("Snow.jpg");
    sprite->setPosition(screenSize/2);
    addChild(sprite, 0);
}

void GameScene::createHud(const Ui& ui) {
    bottomLeftHudSprite = ui.createSprite(GET_VARIABLE_NAME(bottomLeftHudSprite), screenSize);
    addChild(bottomLeftHudSprite, 1);

    bottomRightHudSprite = ui.createSprite(GET_VARIABLE_NAME(bottomRightHudSprite), screenSize);
    addChild(bottomRightHudSprite, 1);

	topLeftHud = Sprite::create("left_hud.png");
    topLeftHud->setAnchorPoint(Vec2(0.35, 0.5));
    topLeftHud->setPosition(Vec2(0, screenSize.height));
    addChild(topLeftHud, 1);

    auto heart = Sprite::create("heart.png");
    heart->setPosition(50, screenSize.height - 50);
    addChild(heart, 1);
    
    healthLabel = ui.createLabel(GET_VARIABLE_NAME(healthLabel), screenSize);
    addChild(healthLabel, 1);

    topRightHud = Sprite::create("right_hud.png");
    topRightHud->setAnchorPoint(Vec2(0.65, 0.5));
    topRightHud->setPosition(Vec2(screenSize.width, screenSize.height));
    addChild(topRightHud, 1);

    auto coin = Sprite::create("coin.png");
    coin->setPosition(screenSize.width - 50, screenSize.height - 50);
    addChild(coin, 1);

    scoreLabel = ui.createLabel(GET_VARIABLE_NAME(scoreLabel), screenSize);
    addChild(scoreLabel, 1);
}

void GameScene::createLabels(const Ui& ui) {
    playButton = ui.createButton(GET_VARIABLE_NAME(playButton), screenSize);
    playButton->addTouchEventListener([this](Ref* sender, ui::Widget::TouchEventType type) {
	    if (type == ui::Widget::TouchEventType::ENDED) {
            playButton->setVisible(false);
            state = GameState::Running;
	    }
    });
    addChild(playButton, 2);
    
    gameOverLabel = ui.createLabel(GET_VARIABLE_NAME(gameOverLabel), screenSize);
    gameOverLabel->setVisible(false);
    addChild(gameOverLabel, 2);

    tryAgainLabel = ui.createLabel(GET_VARIABLE_NAME(tryAgainLabel), screenSize);
    tryAgainLabel->setVisible(false);
    addChild(tryAgainLabel, 2);
}

void GameScene::createRemoteMine(const Ui& ui) {
    remoteMine = Mine::create(this, "fx_techies_remotebomb.png", bottomRightHudSprite->getPosition());
    addChild(remoteMine, 2);
    mines.push_back(remoteMine);
}

void GameScene::createProximityMine(const Ui& ui) {
    proximityMine = Mine::create(this, "fx_techiesfx_mine.png", bottomLeftHudSprite->getPosition());
    addChild(proximityMine, 2);
    mines.push_back(proximityMine);
}

void GameScene::createTechies() {
    techies = Techies::create();
	techies->setAnchorPoint(Vec2(0.5, 0));
	techies->setPosition(Vec2(screenSize.width / 2, 0));
    addChild(techies, 1);
}

void GameScene::updateHud() {
    scoreLabel->setString(StringUtils::format("%d", score));
    healthLabel->setString(StringUtils::format("%d", health));
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
        updateHud();
    }
    
    if (remoteMine->getCrater()->isVisible()) {
        checkCollisionsWithCrater(remoteMine->getCrater());
        updateHud();
    }
    
    proximityMine->setPosition(proximityMine->getNextPosition());
    remoteMine->setPosition(remoteMine->getNextPosition());
}

bool GameScene::onTouchBegan(Touch* touch, Event* event) {
    assert(touch);

    const auto touchLocation = touch->getLocation();
    switch (state) {
        case GameState::Running:
            if (!techies->isIdle() || proximityMine->isFlying() || remoteMine->isFlying()) {
                return false;
            }
            for (auto mine : mines) {
                if (mine->getBoundingBox().containsPoint(touchLocation) && !mine->isPlanted()) {
                    mine->setTouch(touch);
                    mine->setOpacity(128);
                }
            }
            if (remoteMine->getBoundingBox().containsPoint(touchLocation) && remoteMine->isPlanted()) {
                remoteMine->detonate();
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
            if (intersectsHud(mine)) {
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
            if (intersectsHud(mine)) {
                mine->setColor(Color3B::WHITE);
                mine->returnToHUD();
                SimpleAudioEngine::getInstance()->playEffect("Mine_Error.mp3");
            } else {
                techies->plantMine(mine, touchLocation);
                SimpleAudioEngine::getInstance()->playEffect("Mine_Spawn.mp3");
            }
            mine->setTouch(nullptr);
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
    updateHud();
}
