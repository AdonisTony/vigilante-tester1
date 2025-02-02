// Copyright (c) 2018-2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VIGILANTE_PARTY_H_
#define VIGILANTE_PARTY_H_

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace vigilante {

// Forward declaration
class Character;

class Party final {
 public:
  struct WaitingLocationInfo {
    std::string tmxMapFileName;
    float x;
    float y;
  };

  explicit Party(Character* leader);

  Character* getMember(const std::string& characterJsonFileName) const;
  bool hasMember(const std::string& characterJsonFileName) const;
  void recruit(Character* targetCharacter);
  void dismiss(Character* targetCharacter, bool addToMap=true);
  void dismissAll(bool addToMap=true);

  void askMemberToWait(Character* targetCharacter);  // wait in a specific map
  void askMemberToFollow(Character* targetCharacter);  // resume following

  void addWaitingMember(const std::string& characterJsonFileName,
                        const std::string& currentTmxMapFileName,
                        float x,
                        float y);
  void removeWaitingMember(const std::string& characterJsonFileName);

  std::optional<Party::WaitingLocationInfo>
  getWaitingMemberLocationInfo(const std::string& characterJsonFileName) const;

  inline const std::unordered_set<std::shared_ptr<Character>>& getMembers() const { return _members; }
  inline Character* getLeader() const { return _leader; }
  std::unordered_set<Character*> getLeaderAndMembers() const;

  inline const std::unordered_map<std::string, Party::WaitingLocationInfo>&
  getWaitingMembersLocationInfos() const {
    return _waitingMembersLocationInfos;
  }

  void dump();

 protected:
  void addMember(std::shared_ptr<Character> character);
  std::shared_ptr<Character> removeMember(Character* character);

  // `_leader` will NOT be in `_members`.
  Character* _leader;
  std::unordered_set<std::shared_ptr<Character>> _members;
  std::unordered_map<std::string, Party::WaitingLocationInfo> _waitingMembersLocationInfos;

  friend class GameState;
};

}  // namespace vigilante

#endif  // VIGILANTE_PARTY_H_
