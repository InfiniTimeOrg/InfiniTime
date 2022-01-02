# Coding convention

## Language

The language of this project is **C++**, and all new code must be written in C++. (Modern) C++ provides a lot of useful tools and functionalities that are beneficial for embedded software development like `constexpr`, `template` and anything that provides zero-cost abstraction.

C code is accepted if it comes from another library like FreeRTOS, NimBLE, LVGL or the NRF-SDK.

## Coding style

The most important rule to follow is to try to keep the code as easy to read and maintain as possible.

Using an autoformatter is highly recommended, but make sure it's configured properly.

There are preconfigured autoformatter rules for:

  * CLion (IntelliJ) in [.idea/codeStyles/Project.xml](/.idea/codeStyles/Project.xml)
  * `clang-format`

Also use `clang-tidy` to check the code for other issues.

If there are no preconfigured rules for your IDE, you can use one of the existing ones to configure your IDE.

 - **Indentation** : 2 spaces, no tabulation
 - **Opening brace** at the end of the line
 - **Naming** : Choose self-describing variable name
    - **class** : PascalCase
    - **namespace** : PascalCase
    - **variable** : camelCase, **no** prefix/suffix ('_', 'm_',...) for class members
 - **Include guard** : `#pragma once` (no `#ifdef __MODULE__ / #define __MODULE__ / #endif`)
 - **Includes** :
    - files from the project : `#include "relative/path/to/the/file.h"`
    - external files and std : `#include <file.h>`
    - use includes relative to included directories like `src`, not relative to the current file. Don't do: `#include "../file.h"`
 - Only use [primary spellings for operators and tokens](https://en.cppreference.com/w/cpp/language/operator_alternative)
 - Use auto sparingly. Don't use auto for [fundamental/built-in types](https://en.cppreference.com/w/cpp/language/types) and [fixed width integer types](https://en.cppreference.com/w/cpp/types/integer), except when initializing with a cast to avoid duplicating the type name.
 - Examples:
   - `auto* app = static_cast<DisplayApp*>(instance);`
   - `auto number = static_cast<uint8_t>(variable);`
   - `uint8_t returnValue = MyFunction();`
 - Use nullptr instead of NULL
