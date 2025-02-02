// Copyright (c) 2018-2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VIGILANTE_COMMAND_HANDLER_H_
#define VIGILANTE_COMMAND_HANDLER_H_

#include <string>
#include <unordered_map>
#include <vector>

namespace vigilante {

class CommandHandler {
 public:
  CommandHandler();
  virtual ~CommandHandler() = default;

  bool handle(const std::string& cmd, bool showNotification);

 private:
  void setSuccess();
  void setError(const std::string& errMsg);

  // Command handlers.
  void startQuest(const std::vector<std::string>& args);
  void addItem(const std::vector<std::string>& args);
  void removeItem(const std::vector<std::string>& args);
  void updateDialogueTree(const std::vector<std::string>& args);
  void joinPlayerParty(const std::vector<std::string>& args);
  void leavePlayerParty(const std::vector<std::string>& args);
  void playerPartyMemberWait(const std::vector<std::string>& args);
  void playerPartyMemberFollow(const std::vector<std::string>& args);
  void tradeWithPlayer(const std::vector<std::string>& args);
  void killCurrentTarget(const std::vector<std::string>& args);
  void interact(const std::vector<std::string>& args);

  bool _success;
  std::string _errMsg;
};

}  // namespace vigilante

#endif  // VIGILANTE_COMMAND_HANDLER_H_
