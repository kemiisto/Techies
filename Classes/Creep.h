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
    
    void reset(const cocos2d::Size& screenSize, const std::function<void(Node*)>& func);
    void die();
    virtual int value() const = 0;
    virtual int damage() const = 0;
protected:
    Creep(std::string folderName, AnimationsInfo animationsInfo);
private:
    void createRunAnimation(int alternative);
    void createDieAnimation();
    const std::string folderName;
    const AnimationsInfo animationsInfo;
    cocos2d::Animate* animateRun;
    cocos2d::Animate* animateDie;
};

#endif // TECHIES_CREEP_H
