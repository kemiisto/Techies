#ifndef TECHIES_GAME_SCENE_H
#define TECHIES_GAME_SCENE_H

#include "2d/CCScene.h"

#include "Creep.h"
#include "Creeps.h"
#include "Mine.h"
#include "Techies.h"

class Ui;

namespace cocos2d {
	namespace ui {
		class Button;
	}

	class Label;
}

enum class GameState {
    Launched,
    Running,
    Over
};

class GameScene final : public cocos2d::Scene {
public:
    GameScene();
    ~GameScene() override = default;
    
    GameScene(const GameScene&) = delete;
    GameScene& operator=(const GameScene&) = delete;

    GameScene(GameScene&&) = delete;
    GameScene& operator=(GameScene&&) = delete;
    
    bool init() override;
    CREATE_FUNC(GameScene)
private:
    GameState state;
    const cocos2d::Size screenSize;

    std::unique_ptr<Creeps> creeps;
    std::vector<Creep*> runningCreeps;

    Mine* remoteMine;
    Mine* proximityMine;
    std::vector<Mine*> mines;
    
    Techies* techies;

    std::map<Creep::Type, float> creepSpawnInterval;
    std::map<Creep::Type, float> creepSpawnTimer;
    float gameOverDelayTimer;
    
    int score;
    int health;
    
    void stop();
    
    void changeHealth(int value);

    cocos2d::Label* scoreLabel;
    cocos2d::Label* healthLabel;
    cocos2d::Label* gameOverLabel;
    cocos2d::Label* tryAgainLabel;

    cocos2d::ui::Button* playButton = nullptr;

    cocos2d::Sprite* topLeftHud = nullptr;
    cocos2d::Sprite* bottomLeftHudSprite = nullptr;
    cocos2d::Sprite* bottomRightHudSprite = nullptr;
    cocos2d::Sprite* topRightHud = nullptr;

    bool intersectsHud(cocos2d::Sprite* sprite) const;
    
    void createBackground();
    void createHud(const Ui& ui);
    void createLabels(const Ui& ui);

    void spawnCreep(const Creep::Type& creepType);
    
    void createRemoteMine(const Ui& ui);
    void createProximityMine(const Ui& ui);

    void createTechies();
    
    void updateHud();
    
    void checkCollisionsWithCrater(const cocos2d::Sprite* crater);
    void checkCollisionsWithProximityMine();
    
    void update(float dt) override;
    
    bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);
    void onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event);
    void onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event);
    
    void keepMineInsideScreen(Mine* mine, cocos2d::Point& nextPosition) const;
    
    void creepReachedTheEnd(Node* node);
};

#endif // TECHIES_GAME_SCENE_H
