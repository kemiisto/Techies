#ifndef TECHIES_UI_READER_H
#define TECHIES_UI_READER_H

#include <string>
#define RAPIDJSON_HAS_STDSTRING 1
#include "json/document.h"

class Ui {
public:
	Ui(const std::string& uiFileName);
	cocos2d::Label* createLabel(const std::string& elementName, const cocos2d::Size& screenSize) const;
private:
	rapidjson::Document doc;
};

#endif // TECHIES_UI_READER_H