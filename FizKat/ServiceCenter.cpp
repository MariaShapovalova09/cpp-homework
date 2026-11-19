#include "ServiceCenter.hpp"

#include <cctype>

#include "Errors.hpp"

std::unique_ptr<InventoryItem> ServiceCenter::Inspect(
    std::unique_ptr<InventoryItem> item) {
  std::string name = item->Name();
  for (char& c : name) {
    c = std::tolower(static_cast<unsigned char>(c));
  }
  std::string record = item->InventoryId() + " " + item->Name();
  if (name.find("broken") != std::string::npos) {
    journal_.Add(record + ": rejected");
    throw InspectionError();
  }
  journal_.Add(record + ": accepted");
  return item;
}

const Registry<std::string, cJournalSize>& ServiceCenter::Journal() const {
  return journal_;
}
