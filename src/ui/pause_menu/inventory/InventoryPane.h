// Copyright (c) 2018-2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VIGILANTE_INVENTORY_PANE_H_
#define VIGILANTE_INVENTORY_PANE_H_

#include <memory>
#include <string>
#include <vector>

#include <cocos2d.h>
#include <2d/CCLabel.h>
#include <ui/UIImageView.h>

#include "item/Equipment.h"
#include "ui/TabView.h"
#include "ui/pause_menu/AbstractPane.h"
#include "ui/pause_menu/inventory/ItemListView.h"

namespace vigilante {

class InventoryPane : public AbstractPane {
 public:
  InventoryPane(PauseMenu* pauseMenu);
  virtual ~InventoryPane() = default;

  virtual void update() override;
  virtual void handleInput() override;

  void selectEquipment(Equipment::Type equipmentType);

 private:
  cocos2d::ui::ImageView* _background;
  std::unique_ptr<TabView> _tabView;
  std::unique_ptr<ItemListView> _itemListView;

  // These two variables are necessary when
  // the inventory pane is to select a equipment.
  bool _isSelectingEquipment;
  Equipment::Type _selectingEquipmentType;
};

}  // namespace vigilante

#endif  // VIGILANTE_INVENTORY_PANE_H_
