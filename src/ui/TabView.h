// Copyright (c) 2018-2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VIGILANTE_TABVIEW_H_
#define VIGILANTE_TABVIEW_H_

#include <vector>
#include <string>
#include <memory>

#include <cocos2d.h>
#include <2d/CCLabel.h>
#include <ui/UILayout.h>
#include <ui/UIImageView.h>

namespace vigilante {

class TabView {
 public:
  TabView(const std::string& regularBg, const std::string& highlightedBg);
  virtual ~TabView() = default;

  class Tab {
   public:
    friend class TabView;
    Tab(TabView* parent, const std::string& text);
    virtual ~Tab() = default;

    inline int getIndex() const { return _index; }
    inline bool isSelected() const { return _isSelected; }
    void setIsSelected(bool isSelected);

   private:
    TabView* _parent;
    cocos2d::ui::ImageView* _background;
    cocos2d::Label* _label;
    int _index;
    bool _isSelected;
  };

  virtual void addTab(const std::string& text);
  virtual void selectTab(int index);
  virtual void selectPrev();
  virtual void selectNext();

  TabView::Tab* getSelectedTab() const;
  inline cocos2d::ui::Layout* getLayout() const { return _layout; }

 protected:
  cocos2d::ui::Layout* _layout;
  std::vector<std::unique_ptr<TabView::Tab>> _tabs;
  int _current;
  cocos2d::Vec2 _nextTabPos;

  std::string _regularBg;
  std::string _highlightedBg;
};

} // namespace vigilante

#endif // VIGILANTE_TABVIEW_H_
