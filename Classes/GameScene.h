#ifndef TECHIES_GAME_SCENE_H
#define TECHIES_GAME_SCENE_H

#include "cocos2d.h"
#include "Creep.h"
#include "Creeps.h"
#include "Mine.h"
#include "Techies.h"

namespace techies {
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
    	
        static Scene* createScene();
        bool init() override;
        CREATE_FUNC(GameScene)
    private:
        GameState state;
        cocos2d::Size screenSize;

        Creeps creeps;
        std::vector<Creep*> runningCreeps;
        
        cocos2d::DrawNode* drawNode;

        Mine* remoteMine;
        Mine* proximityMine;
        std::vector<Mine*> mines;
        
        cocos2d::Rect remoteMineIconRect;
        cocos2d::Rect proximityMineIconRect;
        
        Techies* techies;
        cocos2d::Vec2 techiesMidTop;

        std::map<Creep::Type, float> creepsSpawnIntervals;
        std::map<Creep::Type, float> creepsTimers;
        
        int score;
        int health;
        
        void stop();
        
        void changeHealth(int value);
        
        cocos2d::Label* scoreLabel;
        cocos2d::Label* healthLabel;
        cocos2d::Label* playLabel;
        cocos2d::Label* gameOverLabel;
        
        void createBackground();
        void createHUD();
        void createLabels();

        void spawnCreep(const Creep::Type& creepType);
        
        void createRemoteMine();
        void createProximityMine();

        void createTechies();
        
        void createIcons();
        
        void updateHUD();
        
        void checkCollisionsWithCrater(const cocos2d::Sprite* crater);
        void checkCollisionsWithProximityMine();
        
        void update(float dt) override;
        
        bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);
        void onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event);
        void onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event);
        
        void keepMineInsideScreen(Mine* mine, cocos2d::Point& nextPosition) const;
        
        void creepReachedTheEnd(Node* node);
    };
}

#endif // TECHIES_GAME_SCENE_H
