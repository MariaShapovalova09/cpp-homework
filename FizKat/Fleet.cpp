#include "Fleet.hpp"

#include <algorithm>

#include "Errors.hpp"
#include "Items.hpp"

namespace {

const int cBuildings = 7;
const char* const cNames[cBuildings] = {"GK",   "LK",     "NK", "KPM",
                                        "Arktika", "Cifra", "KSP"};
const int cMeters[cBuildings] = {1100, 900, 700, 1300, 500, 800, 1500};

int DistanceTo(const std::string& building) {
  for (int i = 0; i < cBuildings; i++) {
    if (building == cNames[i]) {
      return cMeters[i];
    }
  }
  throw NotFoundError("building " + building);
}

bool ById(const std::shared_ptr<InventoryItem>& a,
          const std::shared_ptr<InventoryItem>& b) {
  return *a < *b;
}

}  // namespace

std::shared_ptr<InventoryItem> Fleet::Find(const std::string& id) const {
  for (const auto& item : items_) {
    if (item->InventoryId() == id) {
      return item;
    }
  }
  return nullptr;
}

std::shared_ptr<InventoryItem> Fleet::Get(const std::string& id) const {
  std::shared_ptr<InventoryItem> item = Find(id);
  if (item == nullptr) {
    throw NotFoundError(id);
  }
  return item;
}

void Fleet::CheckFreeId(const std::string& id) const {
  if (Find(id) != nullptr) {
    throw ArgumentError("id " + id + " is already taken");
  }
}

void Fleet::Add(std::unique_ptr<InventoryItem> item) {
  CheckFreeId(item->InventoryId());
  items_.push_back(std::shared_ptr<InventoryItem>(std::move(item)));
}

void Fleet::Remove(const std::string& id) {
  for (size_t i = 0; i < items_.size(); i++) {
    if (items_[i]->InventoryId() == id) {
      items_.erase(items_.begin() + i);
      return;
    }
  }
  throw NotFoundError(id);
}

void Fleet::List(std::ostream& out) const {
  // сортируем копию, чтобы не менять порядок в самом парке
  std::vector<std::shared_ptr<InventoryItem>> sorted = items_;
  std::sort(sorted.begin(), sorted.end(), ById);
  for (const auto& item : sorted) {
    out << *item << '\n';
  }
}

void Fleet::Novice(std::ostream& out) const {
  for (const auto& item : items_) {
    if (item->ForNovice()) {
      out << *item << '\n';
    }
  }
}

EnergyReport Fleet::Report() const {
  EnergyReport report;
  for (const auto& item : items_) {
    report += *item;
  }
  return report;
}

void Fleet::Focus(const std::string& id) {
  focus_ = Get(id);
  has_focus_ = true;
  history_.Add(focus_);
}

void Fleet::ShowFocus(std::ostream& out) const {
  if (!has_focus_) {
    out << "empty\n";
    return;
  }
  if (auto item = focus_.lock()) {
    out << item->Describe() << '\n';
  } else {
    out << "removed\n";
  }
}

void Fleet::History(std::ostream& out) const { out << history_; }

void Fleet::Ride(std::ostream& out, const std::string& id, int now) const {
  std::shared_ptr<InventoryItem> item = Get(id);
  item->CheckRide(now);
  out << *item << ": have a nice ride\n";
}

void Fleet::Rush(std::ostream& out, const std::string& id,
                 const std::string& building, int now, int start) const {
  std::shared_ptr<InventoryItem> item = Get(id);
  item->CheckRide(now);
  int meters = DistanceTo(building);
  int minutes_left = start - now;
  double speed = item->RushSpeed(meters, minutes_left);
  out << *item << " -> " << building << ": " << meters << " m in "
      << minutes_left << " min, speed " << FormatOne(speed) << " km/h\n";
}

void Fleet::Lunch(std::ostream& out, const std::string& id, int now) const {
  std::shared_ptr<InventoryItem> item = Get(id);
  item->CheckRide(now);
  double speed = item->CanteenSpeed(now);
  out << *item << " -> canteen: " << cCanteenClose - now
      << " min before closing, speed " << FormatOne(speed) << " km/h\n";
}
