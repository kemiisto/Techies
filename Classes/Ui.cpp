#include <fstream>

#include "CCFileUtils.h"
#include "json/istreamwrapper.h"
#include "2d/CCLabel.h"

#include "Ui.h"

using namespace cocos2d;
using namespace rapidjson;

void setAnchorPoint(const GenericValue<UTF8<>>& elem, cocos2d::Node* element) {
    const auto& anchorPoint = elem["anchorPoint"].GetArray();
    element->setAnchorPoint(Vec2(anchorPoint[0].GetFloat(), anchorPoint[1].GetFloat()));
}

void setPosition(const GenericValue<UTF8<>>& elem, cocos2d::Node* element, const cocos2d::Size& screenSize) {
    const auto& position = elem["position"].GetArray();
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

void setColor(const GenericValue<UTF8<>>& elem, cocos2d::Node* element) {
    const auto& color = elem["color"].GetArray();
    const auto r = static_cast<uint8_t>(color[0].GetInt());
    const auto g = static_cast<uint8_t>(color[1].GetInt());
    const auto b = static_cast<uint8_t>(color[2].GetInt());
    element->setColor(Color3B(r, g, b));
}

void enableOutline(const GenericValue<UTF8<>>& elem, cocos2d::Node* element) {
    const auto& outline = elem["outline"].GetObject();
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

Label* Ui::createLabel(const std::string& elementName, const cocos2d::Size& screenSize) const {
    const auto& label = doc[elementName];
    const auto& text = label["text"].GetString();
    const auto& fontFilePath = label["fontFilePath"].GetString();
    const auto& fontSize = label["fontSize"].GetFloat();

    auto element = Label::createWithTTF(text, fontFilePath, fontSize);
    setAnchorPoint(label, element);
    setPosition(label, element, screenSize);
    setColor(label, element);
    enableOutline(label, element);
    return element;
}
