#ifndef ORBIT_ORBITQT_EXPECTED_H
#define ORBIT_ORBITQT_EXPECTED_H


#include <variant>

class ExpectedErrorTag_t {};
inline const ExpectedErrorTag_t ExpectedErrorTag{};

template <typename Value, typename Error>
class Expected {
  // This class is just a little bit of scaffolding around
  // std::variant. There are proper implementations of an "expected" type
  // out there. So we should either move to a different kind of error handling
  // or replace this with one of the high quality libraries out there.

  std::variant<Value, Error> data;

 public:
  template <typename... Args>
  /* implicit */ Expected(Args&&... args)
      : data(std::in_place_type<Value>, std::forward<Args>(args)...) {}

  template <typename... Args>
  /* implicit */ Expected(ExpectedErrorTag_t, Args&&... args)
      : data(std::in_place_type<Error>, std::forward<Args>(args)...) {}

  bool IsError() const { return data.index() == 1; }
  bool IsValue() const { return data.index() == 0; }

  const Error& GetError() const { return std::get<1>(data); }
  Error& GetError() { return std::get<1>(data); }

  const Value& GetValue() const { return std::get<0>(data); }
  Value& GetValue() { return std::get<0>(data); }

  [[nodiscard]] operator bool() const { return IsValue(); }

  [[nodiscard]] const Value& operator*() const {
    assert(IsValue());
    return GetValue();
  }

  [[nodiscard]] Value& operator*() {
    assert(IsValue());
    return GetValue();
  }

  [[nodiscard]] const Value* operator->() const {
    assert(IsValue());
    return &GetValue();
  }

  [[nodiscard]] Value* operator->() {
    assert(IsValue());
    return &GetValue();
  }
};

#endif  // ORBIT_ORBITQT_EXPECTED_H