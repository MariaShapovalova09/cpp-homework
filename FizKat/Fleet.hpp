#pragma once

#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "Items.hpp"
#include "Registry.hpp"

const int cHistorySize = 128;

class Fleet {
 public:
  void CheckFreeId(const std::string& id) const;
  // парк забирает владение: unique_ptr -> shared_ptr
  void Add(std::unique_ptr<InventoryItem> item);
  void Remove(const std::string& id);

  void List(std::ostream& out) const;
  void Novice(std::ostream& out) const;
  EnergyReport Report() const;

  void Focus(const std::string& id);
  void ShowFocus(std::ostream& out) const;
  void History(std::ostream& out) const;

  void Ride(std::ostream& out, const std::string& id, int now) const;
  void Rush(std::ostream& out, const std::string& id,
            const std::string& building, int now, int start) const;
  void Lunch(std::ostream& out, const std::string& id, int now) const;

 private:
  std::shared_ptr<InventoryItem> Find(const std::string& id) const;
  std::shared_ptr<InventoryItem> Get(const std::string& id) const;

  std::vector<std::shared_ptr<InventoryItem>> items_;
  std::weak_ptr<InventoryItem> focus_;
  bool has_focus_ = false;
  Registry<std::weak_ptr<InventoryItem>, cHistorySize> history_;
};
