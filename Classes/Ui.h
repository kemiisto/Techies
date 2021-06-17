#ifndef TECHIES_UI_READER_H
#define TECHIES_UI_READER_H

#include <string>
#define RAPIDJSON_HAS_STDSTRING 1
#include "json/document.h"

namespace cocos2d {
	namespace ui {
		class Button;
	}
}

class Ui {
public:
	Ui(const std::string& uiFileName);
	cocos2d::Sprite* createSprite(const std::string& name, const cocos2d::Size& screenSize) const;
	cocos2d::Label* createLabel(const std::string& name, const cocos2d::Size& screenSize) const;
	cocos2d::ui::Button* createButton(const std::string& name, const cocos2d::Size& screenSize) const;
private:
	rapidjson::Document doc;
};

#endif // TECHIES_UI_READER_H