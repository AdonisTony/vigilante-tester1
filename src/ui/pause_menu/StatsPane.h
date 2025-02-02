// Copyright (c) 2018-2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VIGILANTE_STATS_PANE_H_
#define VIGILANTE_STATS_PANE_H_

#include <string>

#include <cocos2d.h>
#include <2d/CCLabel.h>
#include <ui/UILayout.h>
#include <ui/UIImageView.h>

#include "AbstractPane.h"
#include "ui/TableLayout.h"

namespace vigilante {

class StatsPane : public AbstractPane {
 public:
  explicit StatsPane(PauseMenu* pauseMenu);
  virtual ~StatsPane() = default;
  virtual void update() override;
  virtual void handleInput() override;

 private:
  void addEntry(const std::string& title, cocos2d::Label* label) const;

  static inline constexpr float _kPadLeft = 10.0f;
  static inline constexpr float _kPadRight = 10.0f;
  static inline constexpr float _kRowHeight = 16.0f;
  static inline constexpr float _kSectionHeight = 8.0f;

  cocos2d::ui::ImageView* _background;

  cocos2d::Label* _name;
  cocos2d::Label* _level;
  cocos2d::Label* _health;
  cocos2d::Label* _magicka;
  cocos2d::Label* _stamina;

  cocos2d::Label* _attackRange;
  cocos2d::Label* _attackSpeed;
  cocos2d::Label* _moveSpeed;
  cocos2d::Label* _jumpHeight;

  cocos2d::Label* _str;
  cocos2d::Label* _dex;
  cocos2d::Label* _int;
  cocos2d::Label* _luk;
};

} // namespace vigilante

#endif // VIGILANTE_STATS_PANE_H_
