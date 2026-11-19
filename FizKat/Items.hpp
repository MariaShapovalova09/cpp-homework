#pragma once

#include <ostream>
#include <string>

class InventoryItem {
 public:
  virtual ~InventoryItem() = default;

  virtual std::string InventoryId() const = 0;
  virtual std::string Name() const = 0;
  virtual std::string Describe() const = 0;

  // по умолчанию позиция энергию не тратит и новичкам не выдаётся,
  // кому надо - переопределяют
  virtual double DailyEnergy() const { return 0.0; }
  virtual bool ForNovice() const { return false; }

  // можно ли сейчас (now - минуты от полуночи) взять позицию покататься.
  // если нельзя - RideError. на шлеме не уедешь, поэтому по умолчанию нельзя
  virtual void CheckRide(int now) const;

  // с какой скоростью надо ехать, чтобы успеть на пару.
  // умеет только RushScooter, остальные бросают RideError
  virtual double RushSpeed(int meters, int minutes_left) const;

  // с какой скоростью ехать в столовую. умеет только CanteenScooter
  virtual double CanteenSpeed(int now) const;
};

class EnergyConsumer {
 public:
  explicit EnergyConsumer(double energy);
  virtual ~EnergyConsumer() = default;

  virtual double DailyEnergy() const = 0;

 protected:
  double energy_;
};

std::ostream& operator<<(std::ostream& out, const InventoryItem& item);
bool operator<(const InventoryItem& a, const InventoryItem& b);

struct EnergyReport {
  int count = 0;
  double total = 0.0;

  EnergyReport& operator+=(const InventoryItem& item);
};

std::string FormatOne(double x);

const int cNoviceMinScore = 6;
const double cWalkSpeed = 5.0;
const int cNightStart = 2 * 60;
const int cNightEnd = 6 * 60;
const int cCanteenClose = 17 * 60;
const int cCanteenRush = 60;

// Describe() не переопределён, поэтому Transport тоже абстрактный
class Transport : public InventoryItem {
 public:
  Transport(const std::string& id, const std::string& name, int score);

  std::string InventoryId() const override;
  std::string Name() const override;
  bool ForNovice() const override;
  void CheckRide(int now) const override;

 protected:
  std::string id_;
  std::string name_;
  int score_;
};

class Bike : public Transport {
 public:
  Bike(const std::string& id, const std::string& name, int score);

  std::string Describe() const override;
};

class ElectricScooter : public Transport, public EnergyConsumer {
 public:
  ElectricScooter(const std::string& id, const std::string& name,
                  double energy, int score);

  // перекрывает DailyEnergy сразу из обеих баз
  double DailyEnergy() const override;
  std::string Describe() const override;
};

class ElectricBike : public Transport, public EnergyConsumer {
 public:
  ElectricBike(const std::string& id, const std::string& name, double energy,
               int score);

  double DailyEnergy() const override;
  std::string Describe() const override;
};

// креативная часть: самокат, который возит только опаздывающих на пару
class RushScooter : public ElectricScooter {
 public:
  RushScooter(const std::string& id, const std::string& name, double energy,
              int score, double max_speed);

  std::string Describe() const override;
  bool ForNovice() const override;
  double RushSpeed(int meters, int minutes_left) const override;

 private:
  double max_speed_;
};

// самокат-трезвенник: с 2 до 6 ночи не выдаётся
class SoberScooter : public ElectricScooter {
 public:
  SoberScooter(const std::string& id, const std::string& name, double energy,
               int score);

  std::string Describe() const override;
  void CheckRide(int now) const override;
};

// самокат до столовой: чем ближе закрытие, тем быстрее едет
class CanteenScooter : public ElectricScooter {
 public:
  CanteenScooter(const std::string& id, const std::string& name,
                 double energy, int score, double max_speed);

  std::string Describe() const override;
  double CanteenSpeed(int now) const override;

 private:
  double max_speed_;
};

class Thing : public InventoryItem {
 public:
  Thing(const std::string& id, const std::string& name);

  std::string InventoryId() const override;
  std::string Name() const override;

 protected:
  std::string id_;
  std::string name_;
};

class Helmet : public Thing {
 public:
  Helmet(const std::string& id, const std::string& name);

  std::string Describe() const override;
};

class Dock : public Thing, public EnergyConsumer {
 public:
  Dock(const std::string& id, const std::string& name, double energy);

  double DailyEnergy() const override;
  std::string Describe() const override;
};
