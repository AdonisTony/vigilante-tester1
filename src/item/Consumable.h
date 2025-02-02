// Copyright (c) 2018-2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VIGILANTE_CONSUMABLE_H_
#define VIGILANTE_CONSUMABLE_H_

#include <string>

#include <cocos2d.h>

#include "item/Item.h"
#include "input/Keybindable.h"

namespace vigilante {

class Consumable : public Item, public Keybindable {
 public:
  struct Profile final {
    explicit Profile(const std::string& jsonFileName);

    float duration; // sec

    int restoreHealth;
    int restoreMagicka;
    int restoreStamina;

    int bonusPhysicalDamage;
    int bonusMagicalDamage;

    int bonusStr;
    int bonusDex;
    int bonusInt;
    int bonusLuk;

    int bonusMoveSpeed;
    int bonusJumpHeight;

    cocos2d::EventKeyboard::KeyCode hotkey;
  };

  explicit Consumable(const std::string& jsonFileName);
  virtual ~Consumable() override = default;

  virtual void import (const std::string& jsonFileName) override;  // Importable

  virtual cocos2d::EventKeyboard::KeyCode getHotkey() const override;  // Keybindable
  virtual void setHotkey(cocos2d::EventKeyboard::KeyCode hotkey) override;  // Keybindable

  Consumable::Profile& getConsumableProfile();

 protected:
  Consumable::Profile _consumableProfile;
};

}  // namespace vigilante

#endif  // VIGILANTE_CONSUMABLE_H_
