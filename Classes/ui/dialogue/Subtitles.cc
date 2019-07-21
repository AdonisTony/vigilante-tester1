// Copyright (c) 2019 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include "Subtitles.h"

#include "AssetManager.h"
#include "input/InputManager.h"
#include "ui/hud/Hud.h"

#define SHOW_CHAR_INTERVAL .05f
#define LETTERBOX_HEIGHT 50
#define SUBTITLES_Y 38

using std::string;
using std::queue;
using cocos2d::Director;
using cocos2d::Layer;
using cocos2d::Label;
using cocos2d::ui::ImageView;
using cocos2d::EventKeyboard;
using cocos2d::MoveBy;
using cocos2d::Sequence;
using cocos2d::CallFunc;
using vigilante::asset_manager::kShade;
using vigilante::asset_manager::kDialogueTriangle;
using vigilante::asset_manager::kRegularFont;
using vigilante::asset_manager::kRegularFontSize;

namespace vigilante {

Subtitles* Subtitles::_instance = nullptr;

Subtitles* Subtitles::getInstance() {
  if (!_instance) {
    _instance = new Subtitles();
  }
  return _instance;
}

Subtitles::Subtitles()
    : _layer(Layer::create()),
      _label(Label::createWithTTF("", kRegularFont, kRegularFontSize)),
      _nextSubtitleIcon(ImageView::create(kDialogueTriangle)),
      _upperLetterbox(ImageView::create(kShade)),
      _lowerLetterbox(ImageView::create(kShade)),
      _currentSubtitle(""),
      _timer() {
  auto winSize = Director::getInstance()->getWinSize();
  _label->setPosition(winSize.width / 2, SUBTITLES_Y);
  _label->getFontAtlas()->setAliasTexParameters();

  _upperLetterbox->setAnchorPoint({0, 0});
  _upperLetterbox->setPositionY(winSize.height);
  _upperLetterbox->setScaleX(winSize.width);
  _upperLetterbox->setScaleY(LETTERBOX_HEIGHT);

  _lowerLetterbox->setAnchorPoint({0, 0});
  _lowerLetterbox->setPositionY(-LETTERBOX_HEIGHT);
  _lowerLetterbox->setScaleX(winSize.width);
  _lowerLetterbox->setScaleY(LETTERBOX_HEIGHT);

  _nextSubtitleIcon->setVisible(false);

  _layer->addChild(_upperLetterbox);
  _layer->addChild(_lowerLetterbox);
  _layer->addChild(_label);
  _layer->addChild(_nextSubtitleIcon);
  _layer->setVisible(false);
}


void Subtitles::update(float delta) {
  if (!_layer->isVisible() || _label->getString().size()  == _currentSubtitle.text.size()) {
    return;
  }

  if (_timer >= SHOW_CHAR_INTERVAL) {
    int nextCharIdx = _label->getString().size();
    _label->setString(_label->getString() + _currentSubtitle.text.at(nextCharIdx));
    _timer = 0;
  }
  if (_label->getString().size() == _currentSubtitle.text.size()) {
    float x = _label->getPositionX() + _label->getContentSize().width / 2;
    float y = _label->getPositionY();
    _nextSubtitleIcon->setPosition({x + 25, y});
  }
  _timer += delta;
}

void Subtitles::handleInput() {
  auto inputMgr = InputManager::getInstance();

  if (inputMgr->isKeyJustPressed(EventKeyboard::KeyCode::KEY_ENTER)) {
    showNextSubtitle();
  }
}

void Subtitles::addSubtitle(const string& s) {
  _subtitleQueue.push(Subtitle(s));
}

void Subtitles::beginSubtitles() {
  if (_subtitleQueue.empty()) {
    return;
  }
  Hud::getInstance()->getLayer()->setVisible(false);
  _layer->setVisible(true);
  _upperLetterbox->runAction(MoveBy::create(2.0f, {0, -LETTERBOX_HEIGHT}));
  _lowerLetterbox->runAction(Sequence::createWithTwoActions(
    MoveBy::create(2.0f, {0, LETTERBOX_HEIGHT}),
    CallFunc::create([=]() {
      showNextSubtitle();
    })
  ));
}

void Subtitles::endSubtitles() {
  _upperLetterbox->runAction(MoveBy::create(2.0f, {0, LETTERBOX_HEIGHT}));
  _lowerLetterbox->runAction(Sequence::createWithTwoActions(
    MoveBy::create(2.0f, {0, -LETTERBOX_HEIGHT}),
    CallFunc::create([=]() {
      Hud::getInstance()->getLayer()->setVisible(true);
      _layer->setVisible(false);
    })
  ));
}

void Subtitles::showNextSubtitle() {
  _nextSubtitleIcon->setVisible(false);

  if (_subtitleQueue.empty()) {
    _currentSubtitle.text.clear();
    _label->setString("");
    endSubtitles();
    return;
  }

  _currentSubtitle = _subtitleQueue.front();
  _subtitleQueue.pop();
  _label->setString("");
  _timer = 0;
}

Layer* Subtitles::getLayer() const {
  return _layer;
}


Subtitles::Subtitle::Subtitle(const string& text) : text(text) {}

} // namespace vigilante
