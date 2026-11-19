#include "Items.hpp"

#include <iomanip>
#include <sstream>

#include "Errors.hpp"

void InventoryItem::CheckRide(int /*now*/) const {
  throw RideError(InventoryId() + " is not a vehicle");
}

double InventoryItem::RushSpeed(int /*meters*/, int /*minutes_left*/) const {
  throw RideError(InventoryId() + " is not a rush scooter");
}

double InventoryItem::CanteenSpeed(int /*now*/) const {
  throw RideError(InventoryId() + " is not a canteen scooter");
}

EnergyConsumer::EnergyConsumer(double energy) : energy_(energy) {
  if (energy < 0) {
    throw ArgumentError("energy must be >= 0");
  }
}

std::ostream& operator<<(std::ostream& out, const InventoryItem& item) {
  return out << item.InventoryId() << ' ' << item.Name();
}

bool operator<(const InventoryItem& a, const InventoryItem& b) {
  return a.InventoryId() < b.InventoryId();
}

EnergyReport& EnergyReport::operator+=(const InventoryItem& item) {
  count++;
  total += item.DailyEnergy();
  return *this;
}

std::string FormatOne(double x) {
  std::ostringstream out;
  out << std::fixed << std::setprecision(1) << x;
  return out.str();
}

Transport::Transport(const std::string& id, const std::string& name, int score)
    : id_(id), name_(name), score_(score) {
  if (id.empty() || name.empty()) {
    throw ArgumentError("id and name must not be empty");
  }
  if (score < 1 || score > 10) {
    throw ArgumentError("score must be in 1..10");
  }
}

std::string Transport::InventoryId() const { return id_; }

std::string Transport::Name() const { return name_; }

bool Transport::ForNovice() const { return score_ >= cNoviceMinScore; }

void Transport::CheckRide(int /*now*/) const {}

Bike::Bike(const std::string& id, const std::string& name, int score)
    : Transport(id, name, score) {}

std::string Bike::Describe() const {
  return "bike " + id_ + " " + name_ + ", score " + std::to_string(score_);
}

ElectricScooter::ElectricScooter(const std::string& id,
                                 const std::string& name, double energy,
                                 int score)
    : Transport(id, name, score), EnergyConsumer(energy) {}

double ElectricScooter::DailyEnergy() const { return energy_; }

std::string ElectricScooter::Describe() const {
  return "scooter " + id_ + " " + name_ + ", " + FormatOne(energy_) +
         " kWh/day, score " + std::to_string(score_);
}

ElectricBike::ElectricBike(const std::string& id, const std::string& name,
                           double energy, int score)
    : Transport(id, name, score), EnergyConsumer(energy) {}

double ElectricBike::DailyEnergy() const { return energy_; }

std::string ElectricBike::Describe() const {
  return "ebike " + id_ + " " + name_ + ", " + FormatOne(energy_) +
         " kWh/day, score " + std::to_string(score_);
}

RushScooter::RushScooter(const std::string& id, const std::string& name,
                         double energy, int score, double max_speed)
    : ElectricScooter(id, name, energy, score), max_speed_(max_speed) {
  if (max_speed <= 0) {
    throw ArgumentError("max speed must be > 0");
  }
}

std::string RushScooter::Describe() const {
  return "rush " + id_ + " " + name_ + ", " + FormatOne(energy_) +
         " kWh/day, up to " + FormatOne(max_speed_) +
         " km/h, only for those who are late";
}

// слишком быстрый, новичкам не даём, даже если score большой
bool RushScooter::ForNovice() const { return false; }

double RushScooter::RushSpeed(int meters, int minutes_left) const {
  if (minutes_left <= 0) {
    throw RideError("the class has already started, nobody can help you");
  }
  double km = meters / 1000.0;
  double walk_minutes = km / cWalkSpeed * 60.0;
  if (walk_minutes <= minutes_left) {
    throw RideError("you are not late, just walk");
  }
  double speed = km / (minutes_left / 60.0);
  if (speed > max_speed_) {
    throw RideError("too late even for " + name_);
  }
  return speed;
}

SoberScooter::SoberScooter(const std::string& id, const std::string& name,
                           double energy, int score)
    : ElectricScooter(id, name, energy, score) {}

std::string SoberScooter::Describe() const {
  return "sober " + id_ + " " + name_ + ", " + FormatOne(energy_) +
         " kWh/day, score " + std::to_string(score_) +
         ", sleeps from 2:00 to 6:00";
}

void SoberScooter::CheckRide(int now) const {
  if (now >= cNightStart && now < cNightEnd) {
    throw RideError(name_ + " is sleeping, no rides from 2:00 to 6:00");
  }
}

CanteenScooter::CanteenScooter(const std::string& id, const std::string& name,
                               double energy, int score, double max_speed)
    : ElectricScooter(id, name, energy, score), max_speed_(max_speed) {
  if (max_speed <= 0) {
    throw ArgumentError("max speed must be > 0");
  }
}

std::string CanteenScooter::Describe() const {
  return "canteen " + id_ + " " + name_ + ", " + FormatOne(energy_) +
         " kWh/day, up to " + FormatOne(max_speed_) +
         " km/h when the canteen is closing";
}

// пока до закрытия больше часа - едем на половине скорости,
// в последний час скорость линейно растёт до максимума
double CanteenScooter::CanteenSpeed(int now) const {
  int left = cCanteenClose - now;
  if (left <= 0) {
    throw RideError("the canteen is already closed");
  }
  double half = max_speed_ / 2;
  if (left >= cCanteenRush) {
    return half;
  }
  return half + half * (cCanteenRush - left) / cCanteenRush;
}

Thing::Thing(const std::string& id, const std::string& name)
    : id_(id), name_(name) {
  if (id.empty() || name.empty()) {
    throw ArgumentError("id and name must not be empty");
  }
}

std::string Thing::InventoryId() const { return id_; }

std::string Thing::Name() const { return name_; }

Helmet::Helmet(const std::string& id, const std::string& name)
    : Thing(id, name) {}

std::string Helmet::Describe() const { return "helmet " + id_ + " " + name_; }

Dock::Dock(const std::string& id, const std::string& name, double energy)
    : Thing(id, name), EnergyConsumer(energy) {}

double Dock::DailyEnergy() const { return energy_; }

std::string Dock::Describe() const {
  return "dock " + id_ + " " + name_ + ", " + FormatOne(energy_) + " kWh/day";
}
