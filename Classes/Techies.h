#ifndef TECHIES_TECHIES_H
#define TECHIES_TECHIES_H

#include "2d/CCSprite.h"

class Mine;
class Techies final : public cocos2d::Sprite {
public:
    static Techies* create(const cocos2d::Vec2& position);
    
    ~Techies() override;

    Techies(const Techies&) = delete;
    Techies& operator=(const Techies&) = delete;

    Techies(Techies&&) = delete;
    Techies& operator=(Techies&&) = delete;

    void plantMine(Mine* mine, const cocos2d::Vec2& location);

    bool isIdle() const;
    void setIdle();
private:
    enum class State {
        Idle,
        PlantingMine
    };
	
    explicit Techies(const cocos2d::Vec2& position);
    cocos2d::Animation* minePlantingAnimation;
    State state;
	
    void createMinePlantingAnimation();
};

#endif // TECHIES_TECHIES_H