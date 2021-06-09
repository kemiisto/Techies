#ifndef TECHIES_MINE_H
#define TECHIES_MINE_H

#include "cocos2d.h"

enum class MineType {
    Proximity,
    Remote
};

class GameScene;
class Mine final : public cocos2d::Sprite {
public:
    ~Mine() override = default;
    
    Mine(const Mine&) = delete;
    Mine& operator=(const Mine&) = delete;

    Mine(Mine&&) = delete;
    Mine& operator=(Mine&&) = delete;

    void setFlyingSpritePosition(const cocos2d::Vec2& v);
    void showFlyingSprite();
    void hideFlyingSprite();
    void flyTo(const cocos2d::Vec2& v);

    void returnToHUD();
    void detonate();
    const Sprite* getCrater() const;
    void setPosition(const cocos2d::Vec2& v) override;
    float radius() const;
    
    bool isPlanted() const;
    void plant();

    bool isFlying() const;
    void fly();
    
    const cocos2d::Vec2& getNextPosition() const;
    void setNextPosition(const cocos2d::Vec2& v);
    
    const cocos2d::Touch* getTouch() const;
    void setTouch(cocos2d::Touch* t);

    static Mine* create(GameScene* scene, const std::string& filename, const cocos2d::Vec2& position);
private:
    enum class State {
    	Created,
    	Flying,
    	Planted
    };
    explicit Mine(const cocos2d::Vec2& position);
    
    const cocos2d::Vec2 originalPosition;
    State state;
    cocos2d::Vec2 nextPosition;
    cocos2d::Touch* touch;
    cocos2d::ParticleSystemQuad* explosion;
    cocos2d::ParticleSystemQuad* smoke;
    Sprite* flyingSprite;
    Sprite* crater;
    void createCrater(GameScene* scene);
    void createFlyingSprite(GameScene* scene);
    void createExplosion(GameScene* scene);
    void createSmoke(GameScene* scene);
};

#endif // TECHIES_MINE_H
