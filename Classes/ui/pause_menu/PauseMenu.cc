// Copyright (c) 2019 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include "PauseMenu.h"

#include "AssetManager.h"
#include "input/InputManager.h"
#include "ui/pause_menu/inventory/InventoryPane.h"
#include "ui/pause_menu/equipment/EquipmentPane.h"
#include "ui/pause_menu/skill/SkillPane.h"

#define HEADER_PANE_POS {140, 280}
#define STATS_PANE_POS {50, 240}
#define DIALOG_POS {50, 30}
#define MAIN_PANE_POS {230, 240} // InventoryPane, EquipmentPane... etc

using std::array;
using std::string;
using std::unique_ptr;
using cocos2d::Director;
using cocos2d::Label;
using cocos2d::Layer;
using cocos2d::ui::ImageView;
using cocos2d::EventKeyboard;
using vigilante::asset_manager::kPauseMenuBg;

namespace vigilante {

const array<string, PauseMenu::Pane::SIZE> PauseMenu::_kPaneNames = {
  "INVENTORY",
  "EQUIPMENT",
  "SKILLS",
  "QUESTS",
  "OPTIONS"
};

PauseMenu::PauseMenu(Character* character)
    : _character(character),
      _layer(Layer::create()),
      _background(ImageView::create(kPauseMenuBg)),
      _headerPane(new HeaderPane(this)),
      _statsPane(new StatsPane(this)),
      _dialog(new PauseMenuDialog(this)) {
  // Scale the bg image to fill the entire visible area.
  auto visibleSize = Director::getInstance()->getVisibleSize();
  _background->setScaleX(visibleSize.width / _background->getContentSize().width);
  _background->setScaleY(visibleSize.height / _background->getContentSize().height);
  _background->setAnchorPoint({0, 0});
  _background->setPosition({0, 0});
  _layer->addChild(_background, 0);

  // Initialize header (Inventory/Equipment/Skills/Quest/Options).
  _headerPane->getLayout()->setPosition(HEADER_PANE_POS);
  _layer->addChild(_headerPane->getLayout());

  // Initialize StatsPane.
  _statsPane->getLayout()->setPosition(STATS_PANE_POS);
  _layer->addChild(_statsPane->getLayout());

  // Initialize PauseMenuDialog.
  _dialog->getLayout()->setPosition(DIALOG_POS);
  _dialog->setVisible(false);
  _layer->addChild(_dialog->getLayout());

  // Initialize InventoryPane.
  _panes[0] = unique_ptr<InventoryPane>(new InventoryPane(this));
  InventoryPane* inventoryPane = dynamic_cast<InventoryPane*>(_panes[0].get());
  inventoryPane->setPosition(MAIN_PANE_POS);
  inventoryPane->setVisible(false);
  _layer->addChild(inventoryPane->getLayout());

  // Initialize EquipmentPane.
  _panes[1] = unique_ptr<EquipmentPane>(new EquipmentPane(this));
  EquipmentPane* equipmentPane = dynamic_cast<EquipmentPane*>(_panes[1].get());
  equipmentPane->setPosition(MAIN_PANE_POS);
  equipmentPane->setVisible(false);
  _layer->addChild(equipmentPane->getLayout());

  // Initialize EquipmentPane.
  _panes[2] = unique_ptr<SkillPane>(new SkillPane(this));
  SkillPane* skillPane = dynamic_cast<SkillPane*>(_panes[2].get());
  skillPane->setPosition(MAIN_PANE_POS);
  skillPane->setVisible(false);
  _layer->addChild(skillPane->getLayout());

  // Show inventory pane by default.
  _panes.front()->setVisible(true);
}


void PauseMenu::update() {
  _statsPane->update();
  if (_panes[_headerPane->getCurrentIndex()]) {
    _panes[_headerPane->getCurrentIndex()]->update();
  }
}

void PauseMenu::handleInput() {
  InputManager* inputMgr = InputManager::getInstance();

  if (_dialog->isVisible()) {
    _dialog->handleInput();
    return;
  }
  
  // FIXME: when all panes are implemented, this section of code
  // should be cleaned up.
  if (inputMgr->isKeyJustPressed(EventKeyboard::KeyCode::KEY_Q)) {
    AbstractPane* oldPane = getCurrentPane();
    if (oldPane) {
      oldPane->setVisible(false);
    }
    _headerPane->selectPrev();
    AbstractPane* newPane = getCurrentPane();
    if (newPane) {
      newPane->update();
      newPane->setVisible(true);
    }
  } else if (inputMgr->isKeyJustPressed(EventKeyboard::KeyCode::KEY_E)) {
    AbstractPane* oldPane = getCurrentPane();
    if (oldPane) {
      oldPane->setVisible(false);
    }
    _headerPane->selectNext();
    AbstractPane* newPane = getCurrentPane();
    if (newPane) {
      newPane->update();
      newPane->setVisible(true);
    }
  }

  if (_panes[_headerPane->getCurrentIndex()]) {
    _panes[_headerPane->getCurrentIndex()]->handleInput();
  }
}

void PauseMenu::show(Pane pane) {
  AbstractPane* oldPane = getCurrentPane();
  if (oldPane) {
    oldPane->setVisible(false);
  }

  _headerPane->select(static_cast<int>(pane));

  AbstractPane* newPane = getCurrentPane();
  if (newPane) {
    newPane->setVisible(true);
  }

  update();
}


Character* PauseMenu::getCharacter() const {
  return _character;
}

void PauseMenu::setCharacter(Character* character) {
  _character = character;
}

AbstractPane* PauseMenu::getCurrentPane() const {
  return _panes[_headerPane->getCurrentIndex()].get();
}

Layer* PauseMenu::getLayer() const {
  return _layer;
}

PauseMenuDialog* PauseMenu::getDialog() const {
  return _dialog.get();
}

} // namespace vigilante
