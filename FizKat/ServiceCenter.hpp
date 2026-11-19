#pragma once

#include <memory>
#include <string>

#include "Items.hpp"
#include "Registry.hpp"

const int cJournalSize = 128;

class ServiceCenter {
 public:
  // забирает технику, при отказе бросает InspectionError (объект при этом
  // удаляется вместе с unique_ptr), при успехе отдаёт владение обратно
  std::unique_ptr<InventoryItem> Inspect(std::unique_ptr<InventoryItem> item);

  const Registry<std::string, cJournalSize>& Journal() const;

 private:
  Registry<std::string, cJournalSize> journal_;
};
