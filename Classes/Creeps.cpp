#include "Creeps.h"

#include <numeric>

#include "GameScene.h"

using namespace cocos2d;
using namespace techies;

const std::map<Creep::Type, std::size_t> maxCreepsOfType = {
	{Creep::Type::Melee,  20},
	{Creep::Type::Ranged, 10},
	{Creep::Type::Siege,   5}
};

const std::map<Creep::Type, float> creepScale = {
    {Creep::Type::Melee,  0.400f},
    {Creep::Type::Ranged, 0.375f},
    {Creep::Type::Siege,  0.300f}
};

Creeps::Creeps(GameScene* scene) :
	    creeps{
	        {Creep::Type::Melee,  {} },
	        {Creep::Type::Ranged, {} },
	        {Creep::Type::Siege,  {} }
		},
		idx{
	        {Creep::Type::Melee,  0 },
	        {Creep::Type::Ranged, 0 },
	        {Creep::Type::Siege,  0 }
		},
		maxCreeps(0) {
    createCreeps(scene, Creep::Type::Melee);
    createCreeps(scene, Creep::Type::Ranged);
    createCreeps(scene, Creep::Type::Siege);
    maxCreeps = std::accumulate(
        maxCreepsOfType.begin(),
        maxCreepsOfType.end(),
        0,
        [](const std::size_t previous, const auto& element) {
            return previous + element.second;
        }
    );
}

std::size_t Creeps::size() const {
    return maxCreeps;
}

Creep* Creeps::get(const Creep::Type creepType) {
    auto& i = idx.at(creepType);
    const auto creep = creeps.at(creepType).at(i);
    i++;
    if (i == creeps.at(creepType).size()) {
        i = 0;
    }
    return creep;
}

void Creeps::createCreeps(GameScene* scene, const Creep::Type& creepType) {
    for (std::size_t i = 0; i < maxCreepsOfType.at(creepType); ++i) {
        auto creep = Creep::create(creepType);
        creep->setVisible(false);
        creep->setScale(creepScale.at(creepType));
        creeps.at(creepType).push_back(creep);
        scene->addChild(creep, 1);
    }
}