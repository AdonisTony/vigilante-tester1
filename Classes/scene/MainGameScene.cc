// Copyright (c) 2019 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include "MainGameScene.h"

#include <string>

#include <SimpleAudioEngine.h>
#include "AssetManager.h"
#include "Constants.h"
#include "character/Player.h"
#include "gameplay/ExpPointTable.h"
#include "input/InputManager.h"
#include "map/GameMap.h"
#include "skill/Skill.h"
#include "quest/Quest.h"
#include "util/box2d/b2DebugRenderer.h"
#include "util/CameraUtil.h"
#include "util/CallbackUtil.h"
#include "util/KeyCodeUtil.h"
#include "util/RandUtil.h"

using std::string;
using std::unique_ptr;
using std::shared_ptr;
using cocos2d::Vec3;
using cocos2d::Camera;
using cocos2d::CameraFlag;
using cocos2d::Director;
using cocos2d::Layer;
using cocos2d::EventKeyboard;
using cocos2d::ui::ImageView;

namespace vigilante {

MainGameScene::~MainGameScene() {}

bool MainGameScene::init() {
  if (!Scene::init()) {
    return false;
  }

  // Camera note:
  // DEFAULT (orthographic): used to render tilemaps/game objects
  // USER1 (orthographic): used to render HUD
  // Initialize the default camera from "perspective" to "orthographic",
  // and use it as the game world camera.
  auto winSize = Director::getInstance()->getWinSize();
  cocos2d::log("winSize: w=%f h=%f", winSize.width, winSize.height);
  _gameCamera = getDefaultCamera();
  _gameCamera->initOrthographic(winSize.width, winSize.height, 1, 1000);
  _gameCamera->setPosition(0, 0);

  // Initialize HUD camera.
  _hudCamera = Camera::createOrthographic(winSize.width, winSize.height, 1, 1000);
  _hudCamera->setDepth(2);
  _hudCamera->setCameraFlag(CameraFlag::USER1);
  const Vec3& eyePosOld = _gameCamera->getPosition3D();
  Vec3 eyePos = {eyePosOld.x, eyePosOld.y, eyePosOld.z};
  _hudCamera->setPosition3D(eyePos);
  _hudCamera->lookAt(eyePos);
  _hudCamera->setPosition(0, 0);
  addChild(_hudCamera);
  
  // Initialize shade.
  _shade = unique_ptr<Shade>(Shade::getInstance());
  _shade->getImageView()->setCameraMask(static_cast<uint16_t>(CameraFlag::USER1));
  addChild(_shade->getImageView(), graphical_layers::kShade);

  // Initialize HUD.
  _hud = unique_ptr<Hud>(Hud::getInstance());
  _hud->getLayer()->setCameraMask(static_cast<uint16_t>(CameraFlag::USER1));
  _hud->getLayer()->setPosition(75, winSize.height - 40);
  addChild(_hud->getLayer(), graphical_layers::kHud);

  // Initialize notification manager.
  _notifications = unique_ptr<Notifications>(Notifications::getInstance());
  _notifications->getLayer()->setCameraMask(static_cast<uint16_t>(CameraFlag::USER1));
  addChild(_notifications->getLayer(), graphical_layers::kNotification);
  _notifications->show("Notification Manager initialized!");
  _notifications->show("Welcome to Vigilante 0.0.1 alpha");

  _questHints = unique_ptr<QuestHintManager>(QuestHintManager::getInstance());
  addChild(_questHints->getLayer(), graphical_layers::kQuestHint);

  // Initialize floating damage manager.
  _floatingDamages = unique_ptr<FloatingDamages>(FloatingDamages::getInstance());
  addChild(_floatingDamages->getLayer(), graphical_layers::kFloatingDamage);

  // Initialize dialog manager.
  _subtitles = unique_ptr<Subtitles>(Subtitles::getInstance());
  _subtitles->getLayer()->setCameraMask(static_cast<uint16_t>(CameraFlag::USER1));
  _subtitles->addDialog("???: Hey, you are finally awake.");
  _subtitles->addDialog("Aesophor: Wut?");
  _subtitles->beginDialog();
  addChild(_subtitles->getLayer(), graphical_layers::kDialog);

  // Initialize Vigilante's exp point table.
  exp_point_table::import(asset_manager::kExpPointTable);

  // Initialize Vigilante's CallbackUtil.
  vigilante::callback_util::init(this);

  // Initialize Vigilante's KeycodeUtil.
  vigilante::keycode_util::init();

  // Initialize Vigilante's RandUtil.
  vigilante::rand_util::init();
  
  // Initialize GameMapManager.
  // b2World is created when GameMapManager's ctor is called.
  _gameMapManager = unique_ptr<GameMapManager>(GameMapManager::getInstance());
  _gameMapManager->loadGameMap("Map/prison_cell1.tmx");
  addChild(static_cast<Layer*>(_gameMapManager->getLayer()));


  _gameMapManager->getPlayer()->addItem(Item::create("Resources/Database/item/equipment/short_sword.json"));
  _gameMapManager->getPlayer()->addItem(Item::create("Resources/Database/item/equipment/royal_cape.json"));

  // Initialize InputManager.
  // InputManager keep tracks of which keys are pressed.
  InputManager::getInstance()->activate(this);

  // Create b2DebugRenderer.
  _b2dr = b2DebugRenderer::create(getWorld());
  _b2dr->setVisible(false);
  addChild(_b2dr);

  _hud->setPlayer(_gameMapManager->getPlayer());

  auto player = _gameMapManager->getPlayer();
  player->getSkills().push_back(Skill::create("Resources/Database/skill/back_dash.json", player));
  player->getSkills().push_back(Skill::create("Resources/Database/skill/forward_slash.json", player));
  player->getSkills().push_back(Skill::create("Resources/Database/skill/ice_spike.json", player));

  player->getQuestBook().startQuest("Resources/Database/quest/main/main01.json");

  //player->getQuestBook().addQuest(new Quest("Resources/Database/quest/main/main01.json"));
  /*
  Quest quest("Resources/Database/quest/main/main01.json");
  cocos2d::log("quest title: %s", quest.getQuestProfile().title.c_str());
  cocos2d::log("quest desc: %s", quest.getQuestProfile().title.c_str());
  for (const auto& stage : quest.getQuestProfile().stages) {
    cocos2d::log("stage: %s | %s", stage.objective.c_str(), stage.questDesc.c_str());
  }
  */

  // Initialize Pause Menu.
  _pauseMenu = unique_ptr<PauseMenu>(new PauseMenu(_gameMapManager->getPlayer()));
  _pauseMenu->getLayer()->setCameraMask(static_cast<uint16_t>(CameraFlag::USER1));
  _pauseMenu->getLayer()->setVisible(false);
  addChild(_pauseMenu->getLayer(), graphical_layers::kPauseMenu);

  // Tick the box2d world.
  schedule(schedule_selector(MainGameScene::update));
  return true;
}

void MainGameScene::update(float delta) {
  // REVIEW this method
  handleInput();

  if (_pauseMenu->getLayer()->isVisible()) {
    return;
  }

  // If there are no ongoing GameMap transitions, then step the box2d world.
  if (_shade->getImageView()->getNumberOfRunningActions() == 0) {
    getWorld()->Step(1 / kFps, kVelocityIterations, kPositionIterations);
  }

  _gameMapManager->update(delta);
  _floatingDamages->update(delta);
  _notifications->update(delta);
  _questHints->update(delta);
  _subtitles->update(delta);

  vigilante::camera_util::lerpToTarget(_gameCamera, _gameMapManager->getPlayer()->getBody()->GetPosition());
  vigilante::camera_util::boundCamera(_gameCamera, _gameMapManager->getGameMap());
  vigilante::camera_util::updateShake(_gameCamera, delta);
}

void MainGameScene::handleInput() {
  auto inputMgr = InputManager::getInstance();

  // Toggle b2dr (b2DebugRenderer)
  if (inputMgr->isKeyJustPressed(EventKeyboard::KeyCode::KEY_0)) {
    bool isVisible = _b2dr->isVisible();
    _b2dr->setVisible(!isVisible);
    _notifications->show(string("Debug Mode: ") + ((isVisible) ? "on" : "off"));
  }

  // Toggle PauseMenu
  if (inputMgr->isKeyJustPressed(EventKeyboard::KeyCode::KEY_ESCAPE)) {
    bool isVisible = _pauseMenu->getLayer()->isVisible();
    _pauseMenu->getLayer()->setVisible(!isVisible);
    _pauseMenu->update();
    return;
  }


  if (_pauseMenu->getLayer()->isVisible()) {
    _pauseMenu->handleInput(); // paused
  } else if (_subtitles->getLayer()->isVisible()) {
    _subtitles->handleInput(); // dialog being shown
  } else {
    _gameMapManager->getPlayer()->handleInput(); // not paused
  }
}


b2World* MainGameScene::getWorld() const {
  return _gameMapManager->getWorld();
}

} // namespace vigilante
