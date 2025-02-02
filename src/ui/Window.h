// Copyright (c) 2018-2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VIGILANTE_WINDOW_H_
#define VIGILANTE_WINDOW_H_

#include <string>

#include <cocos2d.h>
#include <2d/CCLabel.h>
#include <ui/UILayout.h>
#include <ui/UIImageView.h>

#include "Controllable.h"

namespace vigilante {

class Window : public Controllable {
 public:
  Window(float width=300, float height=150);
  virtual ~Window() = default;

  virtual void update(float delta) = 0;
  virtual void handleInput() = 0;  // Controllable

  virtual void move(const cocos2d::Vec2& position);
  virtual void move(float x, float y);
  virtual void resize(const cocos2d::Size& size);
  virtual void resize(float width, float height);

  inline cocos2d::Layer* getLayer() const { return _layer; }
  inline cocos2d::ui::Layout* getLayout() const { return _layout; }
  inline cocos2d::ui::Layout* getContentLayout() const { return _contentLayout; }
  inline std::string getTitle() const { return _titleLabel->getString(); }
  inline bool isVisible() const { return _isVisible; }
  inline const cocos2d::Vec2& getPosition() const { return _position; }
  inline const cocos2d::Size& getSize() const { return _size; }

  void setTitle(const std::string& title);
  void setVisible(bool visible);

 protected:
  // Place the window at the center, and place `_titleLabel` as well as
  // `_contentLayout` at the correct position.
  void normalize(bool init=false);

  inline void setPosition(const cocos2d::Vec2& position) { _position = position; }
  inline void setSize(const cocos2d::Size& size) { _size = size; }

  cocos2d::Layer* _layer;

  cocos2d::ui::Layout* _layout;
  cocos2d::ui::Layout* _contentLayout;
  cocos2d::Label* _titleLabel;

  cocos2d::ui::ImageView* _contentBg;
  cocos2d::ui::ImageView* _topLeftBg;
  cocos2d::ui::ImageView* _topRightBg;
  cocos2d::ui::ImageView* _bottomLeftBg;
  cocos2d::ui::ImageView* _bottomRightBg;
  cocos2d::ui::ImageView* _topBg;
  cocos2d::ui::ImageView* _leftBg;
  cocos2d::ui::ImageView* _rightBg;
  cocos2d::ui::ImageView* _bottomBg;
  
  bool _isVisible;
  cocos2d::Vec2 _position;
  cocos2d::Size _size;
};

}  // namespace vigilante

#endif  // VIGILANTE_WINDOW_H_
