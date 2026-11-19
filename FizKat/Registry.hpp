#pragma once

#include <memory>
#include <ostream>

#include "Errors.hpp"
#include "Items.hpp"

template <typename T>
void PrintEntry(std::ostream& out, const T& entry) {
  out << entry;
}

// weak_ptr через << не напечатать: сначала lock(), а объекта может уже не быть
template <>
inline void PrintEntry<std::weak_ptr<InventoryItem>>(
    std::ostream& out, const std::weak_ptr<InventoryItem>& entry) {
  if (auto item = entry.lock()) {
    out << *item;
  } else {
    out << "removed";
  }
}

template <typename T, int Cap>
class Registry {
 public:
  bool Add(const T& x) {
    if (size_ == Cap) {
      return false;
    }
    data_[size_] = x;
    size_++;
    return true;
  }

  int Size() const { return size_; }

  const T& operator[](int i) const {
    if (i < 0 || i >= size_) {
      throw FleetError("registry index out of range");
    }
    return data_[i];
  }

 private:
  T data_[Cap];
  int size_ = 0;
};

template <typename T, int Cap>
std::ostream& operator<<(std::ostream& out, const Registry<T, Cap>& registry) {
  for (int i = 0; i < registry.Size(); i++) {
    PrintEntry(out, registry[i]);
    out << '\n';
  }
  return out;
}
