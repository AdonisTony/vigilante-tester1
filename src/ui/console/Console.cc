// Copyright (c) 2018-2020 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include "Console.h"

#include "input/InputManager.h"
#include "util/Logger.h"

#define DEFAULT_HISTORY_SIZE 32

using std::string;
using cocos2d::Layer;
using cocos2d::EventKeyboard;

namespace vigilante {

Console* Console::getInstance() {
  static Console instance;
  return &instance;
}

Console::Console()
    : _layer(Layer::create()),
      _textField(),
      _cmdParser(),
      _cmdHistory() {
  _textField.setOnSubmit([this]() {
    executeCmd(_textField.getString(), /*showNotification=*/true);
    _textField.clear();
  });

  _layer->setVisible(false);
  _layer->addChild(_textField.getLayout());
}


void Console::update(float delta) {
  if (!_layer->isVisible()) {
    return;
  }
  _textField.update(delta);
}

void Console::handleInput() {
  if (IS_KEY_JUST_PRESSED(EventKeyboard::KeyCode::KEY_GRAVE)) {
    _layer->setVisible(!_layer->isVisible());
    _textField.setReceivingInput(false);
    InputManager::getInstance()->popEvLstnr();
    return;
  }

  if (!_layer->isVisible()) {
    return;
  }

  if (IS_KEY_JUST_PRESSED(EventKeyboard::KeyCode::KEY_UP_ARROW) && _cmdHistory.canGoBack()) {
    _cmdHistory.goBack();
    _textField.setString(_cmdHistory.getCurrentLine());
  } else if (IS_KEY_JUST_PRESSED(EventKeyboard::KeyCode::KEY_DOWN_ARROW) && _cmdHistory.canGoForward()) {
    _cmdHistory.goForward();
    _textField.setString(_cmdHistory.getCurrentLine());
  } else {
    _textField.handleInput();
  }
}

void Console::executeCmd(const string& cmd, bool showNotification) {
  VGLOG(LOG_INFO, "Executing: %s", cmd.c_str());
  _cmdParser.parse(cmd, showNotification);
  _cmdHistory.push(cmd);
  _cmdHistory._current = _cmdHistory._tail;
}


bool Console::isVisible() const {
  return _layer->isVisible();
}

void Console::setVisible(bool visible) {
  _layer->setVisible(visible);
  _textField.setReceivingInput(visible);

  if (visible) {

  } else {
    _textField.setReceivingInput(false);
    InputManager::getInstance()->popEvLstnr();
  }
}

Layer* Console::getLayer() const {
  return _layer;
}


Console::CommandHistory::CommandHistory()
    : CircularBuffer<string>(DEFAULT_HISTORY_SIZE), _current(_tail) {}

bool Console::CommandHistory::canGoBack() const {
  return _current != _head;
}

bool Console::CommandHistory::canGoForward() const {
  return _current != _tail;
}

void Console::CommandHistory::goBack() {
  _current--;
  if (_current < 0) {
    _current = capacity() - 1;
  }
}

void Console::CommandHistory::goForward() {
  _current++;
  if (_current >= (int) capacity()) {
    _current = 0;
  }
}

const string& Console::CommandHistory::getCurrentLine() const {
  return _data[_current];
}

} // namespace vigilante
