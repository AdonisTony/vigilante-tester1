// Copyright (c) 2018-2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include "GameScene.h"

#include <SimpleAudioEngine.h>

#include "Assets.h"
#include "CallbackManager.h"
#include "Constants.h"
#include "character/Player.h"
#include "gameplay/ExpPointTable.h"
#include "gameplay/GameState.h"
#include "gameplay/ItemPriceTable.h"
#include "scene/SceneManager.h"
#include "skill/Skill.h"
#include "quest/Quest.h"
#include "util/box2d/b2DebugRenderer.h"
#include "util/CameraUtil.h"
#include "util/KeyCodeUtil.h"
#include "util/RandUtil.h"
#include "util/Logger.h"

using namespace std;
using namespace vigilante::assets;
USING_NS_CC;

namespace vigilante {

bool GameScene::init() {
  if (!Scene::init()) {
    return false;
  }

  _isRunning = true;
  _isTerminating = false;

  // Initialize vigilante's exp point table.
  exp_point_table::import(kExpPointTable);

  // Initialize vigilante's item price table.
  item_price_table::import(kItemPriceTable);

  // Initialize InputManager.
  // InputManager keep tracks of which keys are pressed.
  InputManager::the().activate(this);

  // Initialize HotkeyManager.
  _hotkeyManager = std::make_unique<HotkeyManager>();

  // Camera note:
  // DEFAULT (orthographic): used to render tilemaps/game objects
  // USER1 (orthographic): used to render HUD
  // Initialize the default camera from "perspective" to "orthographic",
  // and use it as the game world camera.
  const auto& winSize = Director::getInstance()->getWinSize();
  VGLOG(LOG_INFO, "winSize: w=%f h=%f", winSize.width, winSize.height);
  _gameCamera = getDefaultCamera();
  _gameCamera->initOrthographic(winSize.width, winSize.height, 1, 1000);
  _gameCamera->setPosition(0, 0);

  // Initialize CallbackManager.
  CallbackManager::the().setScene(this);

  // Initialize Vigilante's utils.
  vigilante::keycode_util::init();
  vigilante::rand_util::init();

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
  _shade = std::make_unique<Shade>();
  _shade->getImageView()->setCameraMask(static_cast<uint16_t>(CameraFlag::USER1));
  addChild(_shade->getImageView(), graphical_layers::kShade);

  // Initialize HUD.
  _hud = std::make_unique<Hud>();
  _hud->getLayer()->setCameraMask(static_cast<uint16_t>(CameraFlag::USER1));
  addChild(_hud->getLayer(), graphical_layers::kHud);

  // Initialize console.
  _console = std::make_unique<Console>();
  _console->getLayer()->setCameraMask(static_cast<uint16_t>(CameraFlag::USER1));
  addChild(_console->getLayer(), graphical_layers::kConsole);

  // Initialize notifications.
  _notifications = std::make_unique<Notifications>();
  _notifications->getLayer()->setCameraMask(static_cast<uint16_t>(CameraFlag::USER1));
  addChild(_notifications->getLayer(), graphical_layers::kNotification);

  // Initialize quest hints.
  _questHints = std::make_unique<QuestHints>();
  _questHints->getLayer()->setCameraMask(static_cast<uint16_t>(CameraFlag::USER1));
  addChild(_questHints->getLayer(), graphical_layers::kQuestHint);

  // Initialize floating damages.
  _floatingDamages = std::make_unique<FloatingDamages>();
  addChild(_floatingDamages->getLayer(), graphical_layers::kFloatingDamage);

  // Initialize control hints.
  _controlHints = std::make_unique<ControlHints>();
  _controlHints->getLayer()->setCameraMask(static_cast<uint16_t>(CameraFlag::USER1));
  addChild(_controlHints->getLayer(), graphical_layers::kControlHints);

  // Initialize dialogue manager.
  _dialogueManager = std::make_unique<DialogueManager>();
  _dialogueManager->getLayer()->setCameraMask(static_cast<uint16_t>(CameraFlag::USER1));
  addChild(_dialogueManager->getLayer(), graphical_layers::kDialogue);

  // Initialize window manager.
  _windowManager = std::make_unique<WindowManager>();
  _windowManager->setDefaultCameraMask(static_cast<uint16_t>(CameraFlag::USER1));
  _windowManager->setScene(this);

  // Initialize GameMapManager.
  // b2World is created when GameMapManager's ctor is called.
  _gameMapManager = std::make_unique<GameMapManager>(b2Vec2{0, kGravity});
  addChild(_gameMapManager->getLayer());

  // Initialize FxManager.
  _fxManager = std::make_unique<FxManager>();

  // Initialize Pause Menu.
  _pauseMenu = std::make_unique<PauseMenu>();
  _pauseMenu->getLayer()->setCameraMask(static_cast<uint16_t>(CameraFlag::USER1));
  addChild(_pauseMenu->getLayer(), graphical_layers::kPauseMenu);

  // Create b2DebugRenderer.
  _b2dr = b2DebugRenderer::create(_gameMapManager->getWorld());
  _b2dr->setVisible(false);
  addChild(_b2dr);

  // Tick the box2d world.
  schedule(schedule_selector(GameScene::update));

  return true;
}

void GameScene::update(float delta) {
  if (!_isRunning) {
    if (_isTerminating) {
      return;
    }
    _shade->getImageView()->runAction(Sequence::createWithTwoActions(
        FadeIn::create(Shade::kFadeInTime * 5),
        CallFunc::create([]() { SceneManager::the().popScene(); })
    ));
    _isTerminating = true;
    return;
  }

  if (!_gameMapManager->getGameMap()) {
    return;
  }

  handleInput();

  if (_pauseMenu->isVisible()) {
    return;
  }

  // If there are no ongoing GameMap transitions, then step the box2d world.
  if (_shade->getImageView()->getNumberOfRunningActions() == 0) {
    _gameMapManager->getWorld()->Step(1 / kFps, kVelocityIterations, kPositionIterations);
  }

  _gameMapManager->update(delta);
  _floatingDamages->update(delta);
  _notifications->update(delta);
  _questHints->update(delta);
  _dialogueManager->update(delta);
  _console->update(delta);
  _windowManager->update(delta);

  vigilante::camera_util::lerpToTarget(_gameCamera, _gameMapManager->getPlayer()->getBody()->GetPosition());
  vigilante::camera_util::boundCamera(_gameCamera, _gameMapManager->getGameMap());
  vigilante::camera_util::updateShake(_gameCamera, delta);
}

void GameScene::handleInput() {
  // First thing first:
  // If there is a specialOnKeyPressed Event Listener,
  // then we should simply let it do its job,
  // and return immediately so that we won't interfere with it.
  if (InputManager::the().hasSpecialOnKeyPressed()) {
    return;
  }

  // Toggle b2dr (b2DebugRenderer)
  if (IS_KEY_JUST_PRESSED(EventKeyboard::KeyCode::KEY_0)) {
    bool isVisible = !_b2dr->isVisible();
    _b2dr->setVisible(isVisible);
    _notifications->show(string("Debug Mode: ") + ((isVisible) ? "on" : "off"));
    return;
  }

  // Exit window or toggle PauseMenu
  if (IS_KEY_JUST_PRESSED(EventKeyboard::KeyCode::KEY_ESCAPE)) {
    if (!_windowManager->isEmpty()) {
      _windowManager->pop();
      return;
    }

    bool isVisible = !_pauseMenu->isVisible();
    _pauseMenu->setVisible(isVisible);
    _pauseMenu->update();
    return;
  }

  // Toggle Console
  if (IS_KEY_JUST_PRESSED(EventKeyboard::KeyCode::KEY_GRAVE)) {
    _console->setVisible(true);
    return;
  }

  if (!_windowManager->isEmpty()) {
    _windowManager->top()->handleInput();
    return;
  }

  if (_pauseMenu->isVisible()) {
    _pauseMenu->handleInput();
    return;
  }

  if (_dialogueManager->getDialogueMenu()->getLayer()->isVisible() ||
      _dialogueManager->getSubtitles()->getLayer()->isVisible()) {
    _dialogueManager->handleInput();
    return;
  }

  if (_gameMapManager->getPlayer()) {
    _gameMapManager->getPlayer()->handleInput();
  }
}

void GameScene::startNewGame() {
  _gameMapManager->loadGameMap(kNewGameInitialMap);
}

void GameScene::loadGame(const string& gameSaveFilePath) {
  _gameMapManager->loadGameMap(kNewGameInitialMap, [gameSaveFilePath]() {
    GameState(gameSaveFilePath).load();
  });
}

void GameScene::quit() {
  _gameMapManager->destroyGameMap();
}

}  // namespace vigilante
