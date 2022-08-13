# Coding style

## Use these tools to find and fix issues.

- Use `clang-format` to format the code.
- Use `clang-tidy` to check the code for other potential issues.

## Follow these guidelines while writing code.

- **Indentation** : 2 spaces, no tabulation
- **Opening brace** at the end of the line
- **Naming** : Choose self-describing variable name
  - **class** : PascalCase
  - **namespace** : PascalCase
  - **variable** : camelCase, **no** prefix/suffix (`_`, `m_`,...) for class members
- **Include guard** : `#pragma once` (no `#ifdef __MODULE__ / #define __MODULE__ / #endif`)
- **Includes** :
  - files from the project : `#include "relative/path/to/the/file.h"`
  - external files and std : `#include <file.h>`
  - use includes relative to included directories like `src`, not relative to the current file. Don't do: `#include "../file.h"`
- Only use [primary spellings for operators and tokens](https://en.cppreference.com/w/cpp/language/operator_alternative)
- Use `auto` sparingly. Don't use `auto` for [fundamental/built-in types](https://en.cppreference.com/w/cpp/language/types) and [fixed width integer types](https://en.cppreference.com/w/cpp/types/integer), except when initializing with a cast to avoid duplicating the type name.
  ```c++
  // Examples:
  auto* app = static_cast<DisplayApp*>(instance);
  auto number = static_cast<uint8_t>(variable);
  uint8_t returnValue = MyFunction();
  ```
- Use `nullptr` instead of `NULL`
