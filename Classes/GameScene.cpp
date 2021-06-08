#include "GameScene.h"
#include "cocos/editor-support/cocostudio/SimpleAudioEngine.h"

using namespace cocos2d;
using namespace CocosDenshion;
using namespace techies;

const Color4F GRAY(0.501f, 0.501f, 0.501f, 1.0f);

GameScene::GameScene() :
	state(GameState::Launched),
	screenSize(Director::getInstance()->getWinSize()),
	creeps(this),
	drawNode(nullptr),
	remoteMine(nullptr),
	proximityMine(nullptr),
	techies(nullptr),
	creepsSpawnIntervals{
		{Creep::Type::Melee, 3.0f},
		{Creep::Type::Ranged, 6.0f},
		{Creep::Type::Siege, 9.0f}
	}, creepsTimers{
		{Creep::Type::Melee, 2.0f},
		{Creep::Type::Ranged, 0.0f},
		{Creep::Type::Siege, 0.0f}
	},
	score(0),
	health(100),
	scoreLabel(nullptr),
	healthLabel(nullptr),
	playLabel(nullptr),
	gameOverLabel(nullptr) {
}

Scene* GameScene::createScene() {
    return create();
}

bool GameScene::init() {
    if (!Scene::init()) {
        return false;
    }
    
    createBackground();
    createLabels();
    createTechies();
    createRemoteMine();
    createProximityMine();
    createHUD();

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
    for (const auto creep : runningCreeps) {
        creep->stopAllActions();
    }
    proximityMine->stopAllActions();
    remoteMine->stopAllActions();
    unscheduleUpdate();
}

void GameScene::createBackground() {
    auto sprite = Sprite::create("Snow.jpg");
    sprite->setPosition(screenSize/2);
    addChild(sprite, 0);
}

void GameScene::createHUD() {
    scoreLabel = Label::createWithTTF("SCORE: 0", "fonts/Marker Felt.ttf", 60);
    scoreLabel->setAnchorPoint(Vec2(1.0, 0.0));
    scoreLabel->setPosition(Vec2(screenSize.width-10, 0));
    scoreLabel->enableOutline(Color4B::BLACK, 3);
    addChild(scoreLabel, 1);
    
    healthLabel = Label::createWithTTF("HEALTH: 100%", "fonts/Marker Felt.ttf", 60);
    healthLabel->setAnchorPoint(Vec2(0.0, 0.0));
    healthLabel->setPosition(Vec2(10, 0));
    healthLabel->enableOutline(Color4B::BLACK, 3);
    addChild(healthLabel, 1);

    drawNode = DrawNode::create();
    addChild(drawNode, 1);
    drawNode->drawSolidRect(proximityMineIconRect.origin, proximityMineIconRect.origin + proximityMineIconRect.size, GRAY);
    drawNode->drawRect(proximityMineIconRect.origin, proximityMineIconRect.origin + proximityMineIconRect.size,
        Color4F::WHITE);
    drawNode->drawSolidRect(remoteMineIconRect.origin, remoteMineIconRect.origin + remoteMineIconRect.size, GRAY);
    drawNode->drawRect(remoteMineIconRect.origin, remoteMineIconRect.origin + remoteMineIconRect.size, Color4F::WHITE);
}

void GameScene::createLabels() {
    playLabel = Label::createWithTTF("PLAY!", "fonts/Marker Felt.ttf", 180);
    playLabel->setPosition(screenSize/2);
    playLabel->enableOutline(Color4B::BLACK, 3);
    addChild(playLabel, 1);
    
    gameOverLabel = Label::createWithTTF("WASTED", "fonts/Marker Felt.ttf", 180);
    gameOverLabel->setPosition(screenSize/2);
    gameOverLabel->setColor(Color3B::RED);
    gameOverLabel->enableOutline(Color4B::BLACK, 3);
    gameOverLabel->setVisible(false);
    addChild(gameOverLabel, 1);
}

void GameScene::createRemoteMine() {
    remoteMine = Mine::create(this, "fx_techies_remotebomb.png", Vec2(screenSize.width/2 + 60, 60));
    addChild(remoteMine, 2);
    mines.push_back(remoteMine);
    remoteMineIconRect = Rect(screenSize.width/2, 0, 125, 125);
}

void GameScene::createProximityMine() {
    proximityMine = Mine::create(this, "fx_techiesfx_mine.png", Vec2(screenSize.width/2 - 60, 60));
    addChild(proximityMine, 2);
    mines.push_back(proximityMine);
    proximityMineIconRect = Rect(screenSize.width/2 - 125, 0, 125, 125);
}

void GameScene::createTechies() {
    techies = Techies::create(Vec2(screenSize.width / 2, 0));
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
    creep->reset(screenSize, std::bind(&GameScene::creepReachedTheEnd, this, creep));
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

    for (auto& [creepType, t] : creepsTimers) {
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
                for (const auto mine : mines) {
                    if (mine->getBoundingBox().containsPoint(touchLocation) && !mine->isPlanted()) {
                        mine->setTouch(touch);
                        mine->setOpacity(128);
                        mine->setScale(0.125f);
                    }
                }
                if (remoteMine->getBoundingBox().containsPoint(touchLocation) && remoteMine->isPlanted()) {
                    remoteMine->detonate();
                }
            }
            break;
        case GameState::Over:
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
    if (touch) {
        auto touchLocation = touch->getLocation();
        for (const auto mine : mines) {
            if (mine->getTouch() && mine->getTouch() == touch) {
                keepMineInsideScreen(proximityMine, touchLocation);
                mine->setNextPosition(touchLocation);
            }
        }
    }
}

void GameScene::onTouchEnded(Touch* touch, Event* event) {
    if (touch) {
	    const auto touchLocation = touch->getLocation();
        for (const auto mine : mines) {
            if (mine->getTouch() && mine->getTouch() == touch) {
                techies->plantMine(mine, touchLocation);
            }
        }
    }
}

void GameScene::creepReachedTheEnd(Node* node) {
	const auto creep = dynamic_cast<Creep*>(node);
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
        SimpleAudioEngine::getInstance()->playEffect("Wasted.mp3");
    }
    if (health > 100) {
        health = 100;
    }
    updateHUD();
}
