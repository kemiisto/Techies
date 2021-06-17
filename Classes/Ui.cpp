#include <fstream>

#include "CCFileUtils.h"
#include "json/istreamwrapper.h"
#include "2d/CCLabel.h"
#include "ui/CocosGUI.h"

#include "Ui.h"

using namespace cocos2d;
using namespace rapidjson;

void setAnchorPoint(const GenericValue<UTF8<>>& desc, cocos2d::Node* element) {
    const auto& anchorPoint = desc["anchorPoint"].GetArray();
    element->setAnchorPoint(Vec2(anchorPoint[0].GetFloat(), anchorPoint[1].GetFloat()));
}

void setPosition(const GenericValue<UTF8<>>& desc, cocos2d::Node* element, const cocos2d::Size& screenSize) {
    const auto& position = desc["position"].GetArray();
    auto x = position[0].GetFloat();
    auto y = position[1].GetFloat();
    if (x < 0) {
        x = 1.0f + x;
    }
    if (y < 0) {
        y = 1.0f + y;
    }
    element->setPosition(screenSize.width * x, screenSize.height * y);
}

void setColor(const GenericValue<UTF8<>>& desc, cocos2d::Node* element) {
    const auto& color = desc["color"].GetArray();
    const auto r = static_cast<uint8_t>(color[0].GetInt());
    const auto g = static_cast<uint8_t>(color[1].GetInt());
    const auto b = static_cast<uint8_t>(color[2].GetInt());
    element->setColor(Color3B(r, g, b));
}

void enableOutline(const GenericValue<UTF8<>>& desc, cocos2d::Node* element) {
    const auto& outline = desc["outline"].GetObject();
    const auto& color = outline["color"].GetArray();
    const auto r = static_cast<uint8_t>(color[0].GetInt());
    const auto g = static_cast<uint8_t>(color[1].GetInt());
    const auto b = static_cast<uint8_t>(color[2].GetInt());
    const auto a = static_cast<uint8_t>(color[3].GetInt());
    const auto& size = outline["size"].GetInt();
    auto label = dynamic_cast<Label*>(element);
    label->enableOutline(Color4B(r, g, b, a), size);
}

Ui::Ui(const std::string& uiFileName) {
    std::ifstream ifs(cocos2d::FileUtils::getInstance()->fullPathForFilename("ui.json"));
    rapidjson::IStreamWrapper isw(ifs);
    doc.ParseStream(isw);
}

cocos2d::Sprite* Ui::createSprite(const std::string& name, const cocos2d::Size& screenSize) const {
    const auto& desc = doc[name];
    const auto& filename = desc["filename"].GetString();

    auto sprite = Sprite::create(filename);
    setAnchorPoint(desc, sprite);
    setPosition(desc, sprite, screenSize);
	
    return sprite;
}

Label* Ui::createLabel(const std::string& name, const cocos2d::Size& screenSize) const {
    const auto& desc = doc[name];
    const auto& text = desc["text"].GetString();
    const auto& fontFilePath = desc["fontFilePath"].GetString();
    const auto& fontSize = desc["fontSize"].GetFloat();

    auto label = Label::createWithTTF(text, fontFilePath, fontSize);
    setAnchorPoint(desc, label);
    setPosition(desc, label, screenSize);
    setColor(desc, label);
    enableOutline(desc, label);
	
    return label;
}

cocos2d::ui::Button* Ui::createButton(const std::string& name, const cocos2d::Size& screenSize) const {
    const auto& desc = doc[name];
    const auto& normalImage = desc["normalImage"].GetString();
	
    auto button = ui::Button::create(normalImage);
    setAnchorPoint(desc, button);
    setPosition(desc, button, screenSize);
	
    return button;
}
