// Copyright (c) 2018-2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include "CommandHandler.h"

#include <memory>

#include "character/Player.h"
#include "character/Npc.h"
#include "gameplay/DialogueTree.h"
#include "item/Item.h"
#include "scene/GameScene.h"
#include "scene/SceneManager.h"
#include "util/StringUtil.h"
#include "util/Logger.h"

#define DEFAULT_ERR_MSG "unable to parse this line"

using namespace std;

using CmdTable = std::unordered_map<std::string,
      void (vigilante::CommandHandler::*)(const std::vector<std::string>&)>;

namespace vigilante {

CommandHandler::CommandHandler() : _success(), _errMsg() {}

bool CommandHandler::handle(const string& cmd, bool showNotification) {
  vector<string> args;
  if (cmd.empty() || (args = string_util::split(cmd)).empty()) {
    return false;
  }

  _success = false;
  _errMsg = DEFAULT_ERR_MSG;

  // Command handler table.
  static const CmdTable cmdTable = {
    {"startQuest",              &CommandHandler::startQuest             },
    {"addItem",                 &CommandHandler::addItem                },
    {"removeItem",              &CommandHandler::removeItem             },
    {"updateDialogueTree",      &CommandHandler::updateDialogueTree     },
    {"joinPlayerParty",         &CommandHandler::joinPlayerParty        },
    {"leavePlayerParty",        &CommandHandler::leavePlayerParty       },
    {"playerPartyMemberWait",   &CommandHandler::playerPartyMemberWait  },
    {"playerPartyMemberFollow", &CommandHandler::playerPartyMemberFollow},
    {"tradeWithPlayer",         &CommandHandler::tradeWithPlayer        },
    {"killCurrentTarget",       &CommandHandler::killCurrentTarget      },
    {"interact",                &CommandHandler::interact               },
  };

  // Execute the corresponding command handler from _cmdTable.
  // The obtained value from _cmdTable is a class member function pointer.
  CmdTable::const_iterator it = cmdTable.find(args[0]);
  if (it != cmdTable.end()) {
    VGLOG(LOG_INFO, "Executing cmd: [%s].", cmd.c_str());
    (this->*((*it).second))(args);
  }

  if (!_success) {
    _errMsg = args[0] + ": " + _errMsg;
    VGLOG(LOG_ERR, "%s", _errMsg.c_str());
  }

  if (showNotification) {
    auto notifications = SceneManager::the().getCurrentScene<GameScene>()->getNotifications();
    notifications->show((_success) ? cmd : _errMsg);
  }

  return _success;
}

void CommandHandler::setSuccess() {
  _success = true;
}

void CommandHandler::setError(const string& errMsg) {
  _success = false;
  _errMsg = errMsg;
}

void CommandHandler::startQuest(const vector<string>& args) {
  if (args.size() < 2) {
    setError("usage: startQuest <quest>");
    return;
  }

  auto gmMgr = SceneManager::the().getCurrentScene<GameScene>()->getGameMapManager();
  gmMgr->getPlayer()->getQuestBook().startQuest(args[1]);
  setSuccess();
}

void CommandHandler::addItem(const vector<string>& args) {
  if (args.size() < 2) {
    setError("usage: addItem <itemName> [amount]");
    return;
  }

  int amount = 1;
  if (args.size() >= 3) {
    try {
      amount = std::stoi(args[2]);
    } catch (const invalid_argument& ex) {
      setError("invalid argument `amount`");
      return;
    } catch (const out_of_range& ex) {
      setError("`amount` is too large");
      return;
    } catch (...) {
      setError("unknown error");
      return;
    }
  }

  if (amount <= 0) {
    setError("`amount` has to be at least 1");
    return;
  }

  unique_ptr<Item> item = Item::create(args[1]);
  auto gmMgr = SceneManager::the().getCurrentScene<GameScene>()->getGameMapManager();
  gmMgr->getPlayer()->addItem(std::move(item), amount);
  setSuccess();
}

void CommandHandler::removeItem(const vector<string>& args) {
  if (args.size() < 2) {
    setError("usage: removeItem <itemName> [amount]");
    return;
  }

  int amount = 1;
  if (args.size() >= 3) {
    try {
      amount = std::stoi(args[2]);
    } catch (const invalid_argument& ex) {
      setError("invalid argument `amount`");
      return;
    } catch (const out_of_range& ex) {
      setError("`amount` is too large");
      return;
    } catch (...) {
      setError("unknown error");
      return;
    }
  }

  if (amount <= 0) {
    setError("`amount` has to be at least 1");
    return;
  }

  unique_ptr<Item> item = Item::create(args[1]);
  auto gmMgr = SceneManager::the().getCurrentScene<GameScene>()->getGameMapManager();
  gmMgr->getPlayer()->removeItem(item.get(), amount);
  setSuccess();
}

void CommandHandler::updateDialogueTree(const vector<string>& args) {
  if (args.size() < 3) {
    setError("usage: updateDialogueTree <npcJson> <dialogueTreeJson>");
    return;
  }

  // TODO: Maybe add some argument check here?

  auto dialogueMgr = SceneManager::the().getCurrentScene<GameScene>()->getDialogueManager();
  dialogueMgr->setLatestNpcDialogueTree(args[1], args[2]);
  setSuccess();
}

void CommandHandler::joinPlayerParty(const vector<string>&) {
  auto gmMgr = SceneManager::the().getCurrentScene<GameScene>()->getGameMapManager();
  auto dialogueMgr = SceneManager::the().getCurrentScene<GameScene>()->getDialogueManager();

  Player* player = gmMgr->getPlayer();
  Npc* targetNpc = dialogueMgr->getTargetNpc();
  assert(player != nullptr && targetNpc != nullptr);

  if (targetNpc->isPlayerLeaderOfParty()) {
    setError("This Npc is already in player's party.");
    return;
  }

  player->getParty()->recruit(targetNpc);
  setSuccess();
}

void CommandHandler::leavePlayerParty(const vector<string>&) {
  auto gmMgr = SceneManager::the().getCurrentScene<GameScene>()->getGameMapManager();
  auto dialogueMgr = SceneManager::the().getCurrentScene<GameScene>()->getDialogueManager();

  Player* player = gmMgr->getPlayer();
  Npc* targetNpc = dialogueMgr->getTargetNpc();
  assert(player != nullptr && targetNpc != nullptr);

  if (!targetNpc->isPlayerLeaderOfParty()) {
    setError("This Npc has not joined player's party yet.");
    return;
  }

  player->getParty()->dismiss(targetNpc);
  setSuccess();
}

void CommandHandler::playerPartyMemberWait(const vector<string>&) {
  auto gmMgr = SceneManager::the().getCurrentScene<GameScene>()->getGameMapManager();
  auto dialogueMgr = SceneManager::the().getCurrentScene<GameScene>()->getDialogueManager();

  Player* player = gmMgr->getPlayer();
  Npc* targetNpc = dialogueMgr->getTargetNpc();
  assert(player != nullptr && targetNpc != nullptr);

  if (!targetNpc->isPlayerLeaderOfParty()) {
    setError("This Npc has not joined player's party yet.");
    return;
  }

  if (player->getParty()->getWaitingMemberLocationInfo(targetNpc->getCharacterProfile().jsonFileName)) {
    setError("This Npc is already waiting for player.");
    return;
  }

  player->getParty()->askMemberToWait(targetNpc);
  setSuccess();
}

void CommandHandler::playerPartyMemberFollow(const vector<string>&) {
  auto gmMgr = SceneManager::the().getCurrentScene<GameScene>()->getGameMapManager();
  auto dialogueMgr = SceneManager::the().getCurrentScene<GameScene>()->getDialogueManager();

  Player* player = gmMgr->getPlayer();
  Npc* targetNpc = dialogueMgr->getTargetNpc();
  assert(player != nullptr && targetNpc != nullptr);

  if (!targetNpc->isPlayerLeaderOfParty()) {
    setError("This Npc has not joined player's party yet.");
    return;
  }

  if (!player->getParty()->getWaitingMemberLocationInfo(targetNpc->getCharacterProfile().jsonFileName)) {
    setError("This Npc is not waiting for player yet.");
    return;
  }

  player->getParty()->askMemberToFollow(targetNpc);
  setSuccess();
}

void CommandHandler::tradeWithPlayer(const vector<string>&) {
  auto gmMgr = SceneManager::the().getCurrentScene<GameScene>()->getGameMapManager();
  auto dialogueMgr = SceneManager::the().getCurrentScene<GameScene>()->getDialogueManager();

  Player* player = gmMgr->getPlayer();
  Npc* targetNpc = dialogueMgr->getTargetNpc();
  assert(player != nullptr && targetNpc != nullptr);

  if (!targetNpc->getNpcProfile().isTradable) {
    setError("This Npc is not tradable.");
    return;
  }

  targetNpc->beginTrade();
  setSuccess();
}

void CommandHandler::killCurrentTarget(const vector<string>&) {
  auto gmMgr = SceneManager::the().getCurrentScene<GameScene>()->getGameMapManager();
  auto dialogueMgr = SceneManager::the().getCurrentScene<GameScene>()->getDialogueManager();

  Player* player = gmMgr->getPlayer();
  Npc* targetNpc = dialogueMgr->getTargetNpc();
  assert(player != nullptr && targetNpc != nullptr);

  targetNpc->receiveDamage(player, 999);
  setSuccess();
}

void CommandHandler::interact(const vector<string>&) {
  auto gmMgr = SceneManager::the().getCurrentScene<GameScene>()->getGameMapManager();

  Player* player = gmMgr->getPlayer();
  if (!player) {
    setError("No player.");
    return;
  }

  const auto& interactables = player->getInRangeInteractables();
  if (interactables.empty()) {
    setError("No nearby interactable objects.");
    return;
  }

  player->interact(*interactables.begin());
  setSuccess();
}

}  // namespace vigilante
