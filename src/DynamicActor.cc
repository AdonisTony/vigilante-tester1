// Copyright (c) 2018-2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include "DynamicActor.h"

#include "Constants.h"
#include "map/GameMapManager.h"

using namespace std;

namespace vigilante {

DynamicActor::DynamicActor(size_t numAnimations, size_t numFixtures)
    : StaticActor(numAnimations),
      _body(),
      _fixtures(numFixtures) {}

bool DynamicActor::removeFromMap() {
  if (!StaticActor::removeFromMap()) {
    return false;
  }
  destroyBody();
  return true;
}

void DynamicActor::setPosition(float x, float y) {
  _body->SetTransform({x, y}, 0);
}

void DynamicActor::update(float) {
  // Sync the body sprite with its b2body.
  b2Vec2 b2bodyPos = _body->GetPosition();
  _bodySprite->setPosition(b2bodyPos.x * kPpm, b2bodyPos.y * kPpm);
}

void DynamicActor::destroyBody() {
  if (!_body) {
    return;
  }
  _body->GetWorld()->DestroyBody(_body);
  _body = nullptr;
}

void DynamicActor::setCategoryBits(b2Fixture* fixture, const short categoryBits) {
  b2Filter filter = fixture->GetFilterData();
  filter.categoryBits = categoryBits;
  fixture->SetFilterData(filter);
}

void DynamicActor::setMaskBits(b2Fixture* fixture, const short maskBits) {
  b2Filter filter = fixture->GetFilterData();
  filter.maskBits = maskBits;
  fixture->SetFilterData(filter);
}

}  // namespace vigilante
