// Copyright (c) 2018-2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VIGILANTE_CHEST_H_
#define VIGILANTE_CHEST_H_

#include <string>
#include <vector>

#include <cocos2d.h>

#include "DynamicActor.h"
#include "Interactable.h"

namespace vigilante {

class Chest : public DynamicActor, public Interactable {
 public:
  Chest(const std::string& tmxMapFileName,
        const int chestId,
        const std::string& itemJsons);
  virtual ~Chest() override = default;

  virtual bool showOnMap(float x, float y) override;  // DynamicActor

  virtual void onInteract(Character* user) override;  // Interactable
  virtual bool willInteractOnContact() const override { return false; }  // Interactable
  virtual void showHintUI() override;  // Interactable
  virtual void hideHintUI() override;  // Interactable

 protected:
  virtual void createHintBubbleFx() override;  // Interactable
  virtual void removeHintBubbleFx() override;  // Interactable

  void defineBody(b2BodyType bodyType,
                  float x,
                  float y,
                  short categoryBits,
                  short maskBits);

  const std::string _tmxMapFileName;
  const int _chestId;
  const std::vector<std::string> _itemJsons;
  bool _isOpened;
  cocos2d::Sprite* _hintBubbleFxSprite;
};

}  // namespace vigilante

#endif  // VIGILANTE_CHEST_H_
