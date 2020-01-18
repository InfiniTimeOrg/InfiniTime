#pragma once

@VERSION_EDIT_WARNING@

namespace Pinetime {
  class Version {
    public:
      static uint32_t Major() {return major;}
      static uint32_t Minor() {return minor;}
      static uint32_t Patch() {return patch;}
    private:
      static constexpr uint32_t major = @PROJECT_VERSION_MAJOR@;
      static constexpr uint32_t minor = @PROJECT_VERSION_MINOR@;
      static constexpr uint32_t patch = @PROJECT_VERSION_PATCH@;
  };
}