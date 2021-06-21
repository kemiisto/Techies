#ifndef TECHIES_CREEP_H
#define TECHIES_CREEP_H

#include "2d/CCSprite.h"

namespace cocos2d {
	class Animate;
}

struct AnimationsInfo {
    int alternativesCount;
    std::vector<int> runCounts;
    std::vector<int> dieCounts;
};

class Creep : public cocos2d::Sprite {
public:
    friend class CreepFactory;
    enum class Type {
        Melee,
        Ranged,
        Siege
    };

    ~Creep() override;
    
    Creep(const Creep&) = delete;
    Creep& operator=(const Creep&) = delete;
    
    Creep(Creep&&) = delete;
    Creep& operator=(Creep&&) = delete;
    
    void spawn(const cocos2d::Size& screenSize, const std::function<void(Node*)>& func);
    void die();
    int getValue() const;
    int getDamage() const;
protected:
    Creep(std::string folderName, AnimationsInfo animationsInfo, int value, int damage);
private:
    void createRunAnimation(int alternative);
    void createDieAnimation();
    const std::string folderName;
    const AnimationsInfo animationsInfo;
    const int value;
    const int damage;
    cocos2d::Animate* animateRun = nullptr;
    cocos2d::Animate* animateDie = nullptr;
};

#endif // TECHIES_CREEP_H
