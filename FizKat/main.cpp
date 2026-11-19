#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "Errors.hpp"
#include "Fleet.hpp"
#include "Items.hpp"
#include "ServiceCenter.hpp"

template <typename T>
T ReadArg(std::istringstream& in) {
  T value{};
  if (!(in >> value)) {
    throw ArgumentError("not enough arguments or wrong format");
  }
  return value;
}

void CheckEnd(std::istringstream& in) {
  std::string extra;
  if (in >> extra) {
    throw ArgumentError("too many arguments");
  }
}

int ParseTime(const std::string& text) {
  std::istringstream in(text);
  int hours = -1;
  char colon = 0;
  int minutes = -1;
  in >> hours >> colon >> minutes;
  bool ok = !in.fail() && colon == ':' && hours >= 0 && hours <= 23 &&
            minutes >= 0 && minutes <= 59;
  std::string rest;
  if (!ok || in >> rest) {
    throw ArgumentError("time must look like 9:45");
  }
  return hours * 60 + minutes;
}

void AddCommand(std::istringstream& in, Fleet& fleet, ServiceCenter& center) {
  std::string kind = ReadArg<std::string>(in);
  std::string id = ReadArg<std::string>(in);
  std::string name = ReadArg<std::string>(in);

  std::unique_ptr<InventoryItem> item;
  bool is_transport = true;
  if (kind == "scooter") {
    double energy = ReadArg<double>(in);
    int score = ReadArg<int>(in);
    CheckEnd(in);
    item = std::make_unique<ElectricScooter>(id, name, energy, score);
  } else if (kind == "bike") {
    int score = ReadArg<int>(in);
    CheckEnd(in);
    item = std::make_unique<Bike>(id, name, score);
  } else if (kind == "ebike") {
    double energy = ReadArg<double>(in);
    int score = ReadArg<int>(in);
    CheckEnd(in);
    item = std::make_unique<ElectricBike>(id, name, energy, score);
  } else if (kind == "rush") {
    double energy = ReadArg<double>(in);
    int score = ReadArg<int>(in);
    double max_speed = ReadArg<double>(in);
    CheckEnd(in);
    item = std::make_unique<RushScooter>(id, name, energy, score, max_speed);
  } else if (kind == "sober") {
    double energy = ReadArg<double>(in);
    int score = ReadArg<int>(in);
    CheckEnd(in);
    item = std::make_unique<SoberScooter>(id, name, energy, score);
  } else if (kind == "canteen") {
    double energy = ReadArg<double>(in);
    int score = ReadArg<int>(in);
    double max_speed = ReadArg<double>(in);
    CheckEnd(in);
    item =
        std::make_unique<CanteenScooter>(id, name, energy, score, max_speed);
  } else if (kind == "helmet") {
    CheckEnd(in);
    item = std::make_unique<Helmet>(id, name);
    is_transport = false;
  } else if (kind == "dock") {
    double energy = ReadArg<double>(in);
    CheckEnd(in);
    item = std::make_unique<Dock>(id, name, energy);
    is_transport = false;
  } else {
    throw ArgumentError("unknown kind: " + kind);
  }

  // сначала все проверки, и только потом парк меняется
  fleet.CheckFreeId(id);
  if (is_transport) {
    item = center.Inspect(std::move(item));
  }
  fleet.Add(std::move(item));
}

void RunCommand(const std::string& command, std::istringstream& in,
                Fleet& fleet, ServiceCenter& center) {
  if (command == "add") {
    AddCommand(in, fleet, center);
  } else if (command == "energy") {
    CheckEnd(in);
    std::cout << FormatOne(fleet.Report().total) << '\n';
  } else if (command == "report") {
    CheckEnd(in);
    EnergyReport report = fleet.Report();
    std::cout << "items: " << report.count << '\n';
    std::cout << "energy: " << FormatOne(report.total) << '\n';
  } else if (command == "novice") {
    CheckEnd(in);
    fleet.Novice(std::cout);
  } else if (command == "list") {
    CheckEnd(in);
    fleet.List(std::cout);
  } else if (command == "focus") {
    std::string id = ReadArg<std::string>(in);
    CheckEnd(in);
    fleet.Focus(id);
  } else if (command == "show_focus") {
    CheckEnd(in);
    fleet.ShowFocus(std::cout);
  } else if (command == "history") {
    CheckEnd(in);
    fleet.History(std::cout);
  } else if (command == "remove") {
    std::string id = ReadArg<std::string>(in);
    CheckEnd(in);
    fleet.Remove(id);
  } else if (command == "journal") {
    CheckEnd(in);
    std::cout << center.Journal();
  } else if (command == "late") {
    std::string id = ReadArg<std::string>(in);
    std::string building = ReadArg<std::string>(in);
    int now = ParseTime(ReadArg<std::string>(in));
    int start = ParseTime(ReadArg<std::string>(in));
    CheckEnd(in);
    fleet.Rush(std::cout, id, building, now, start);
  } else if (command == "ride") {
    std::string id = ReadArg<std::string>(in);
    int now = ParseTime(ReadArg<std::string>(in));
    CheckEnd(in);
    fleet.Ride(std::cout, id, now);
  } else if (command == "lunch") {
    std::string id = ReadArg<std::string>(in);
    int now = ParseTime(ReadArg<std::string>(in));
    CheckEnd(in);
    fleet.Lunch(std::cout, id, now);
  } else {
    throw ArgumentError("unknown command: " + command);
  }
}

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  Fleet fleet;
  ServiceCenter center;
  std::string line;
  while (std::getline(std::cin, line)) {
    std::istringstream in(line);
    std::string command;
    if (!(in >> command)) {
      continue;
    }
    if (command == "quit") {
      break;
    }
    try {
      RunCommand(command, in, fleet, center);
    } catch (const FleetError& e) {
      std::cout << "error: " << e.what() << '\n';
    } catch (const std::exception& e) {
      std::cout << "error: " << e.what() << '\n';
    }
  }
  return 0;
}
