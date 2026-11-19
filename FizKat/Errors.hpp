#pragma once

#include <stdexcept>
#include <string>

// общий базис для всех ошибок парка, в main ловим именно его
class FleetError : public std::runtime_error {
 public:
  explicit FleetError(const std::string& message)
      : std::runtime_error(message) {}
};

class InspectionError : public FleetError {
 public:
  InspectionError() : FleetError("inspection rejected") {}
};

class ArgumentError : public FleetError {
 public:
  explicit ArgumentError(const std::string& message) : FleetError(message) {}
};

class NotFoundError : public FleetError {
 public:
  explicit NotFoundError(const std::string& name)
      : FleetError("not found: " + name) {}
};

class RideError : public FleetError {
 public:
  explicit RideError(const std::string& message) : FleetError(message) {}
};
