# liballocator Modernization — Task Breakdown

## Context

`liballocator` is a C++ embedded memory allocator library located at `/home/kuba/projects/kubasejdak/libs/liballocator`.
`osal` is the reference repository at `/home/kuba/projects/kubasejdak/libs/osal` — it represents the target structure and
conventions that all libs in this workspace should follow.

Every phase below is a **self-contained PR-sized unit of work**. Each can be executed independently by an AI agent
that reads this document, without needing the broader conversation context.

---

## Phase 1 — devcontainers

**Goal:** Align all devcontainer configurations with osal (the reference).

**Context:**
Both repos have 4 devcontainer configurations (one per toolchain):

- `.devcontainer/gcc-13/devcontainer.json`
- `.devcontainer/clang-18/devcontainer.json`
- `.devcontainer/aarch64-none-linux-gnu-gcc-13/devcontainer.json`
- `.devcontainer/arm-none-eabi-gcc-13/devcontainer.json`

**osal devcontainer (reference)** — gcc-13 example:

```json
{
  "name": "kubasejdak gcc:13-24.04",
  "image": "kubasejdak/gcc:13-24.04",
  "mounts": [
    "source=${localEnv:HOME}/.gitconfig,target=/home/ubuntu/.gitconfig,type=bind,consistency=cached",
    "source=${localEnv:HOME}/.gitconfig-private,target=/home/ubuntu/.gitconfig-private,type=bind,consistency=cached",
    "source=${localEnv:HOME}/.ssh,target=/home/ubuntu/.ssh,type=bind,consistency=cached"
  ],
  "customizations": {
    "vscode": {
      "extensions": [
        "bierner.markdown-mermaid",
        "davidanson.vscode-markdownlint",
        "esbenp.prettier-vscode",
        "hediet.vscode-drawio",
        "mads-hartmann.bash-ide-vscode",
        "matepek.vscode-catch2-test-adapter",
        "ms-python.python",
        "ms-vscode.cpptools-extension-pack",
        "ms-vsliveshare.vsliveshare",
        "waderyan.gitblame",
        "xaver.clang-format",
        "yzhang.markdown-all-in-one"
      ]
    }
  },
  "runArgs": ["--network=host"]
}
```

**liballocator devcontainer (current)** — gcc-13 example:

```json
{
  "name": "kubasejdak gcc:13-24.04",
  "image": "kubasejdak/gcc:13-24.04",
  "mounts": [
    "source=${localEnv:HOME}/.gitconfig,target=/home/ubuntu/.gitconfig,type=bind,consistency=cached",
    "source=${localEnv:HOME}/.gitconfig-private,target=/home/ubuntu/.gitconfig-private,type=bind,consistency=cached",
    "source=${localEnv:HOME}/.ssh,target=/home/ubuntu/.ssh,type=bind,consistency=cached"
  ],
  "customizations": {
    "vscode": {
      "extensions": [
        "github.copilot",
        "github.vscode-github-actions",
        "mads-hartmann.bash-ide-vscode",
        "matepek.vscode-catch2-test-adapter",
        "ms-vscode.cpptools-extension-pack",
        "ms-vsliveshare.vsliveshare",
        "waderyan.gitblame",
        "xaver.clang-format",
        "yzhang.markdown-all-in-one"
      ]
    }
  },
  "runArgs": ["--network=host"]
}
```

**Changes required (apply to all 4 devcontainer files):**

1. Fix JSON indentation from 4-space to 2-space (match osal)
2. Remove extensions: `github.copilot`, `github.vscode-github-actions`
3. Add extensions (in alphabetical order, matching osal):
   - `bierner.markdown-mermaid`
   - `davidanson.vscode-markdownlint`
   - `esbenp.prettier-vscode`
   - `hediet.vscode-drawio`
   - `ms-python.python`
4. The `name` and `image` fields are already correct per-toolchain — do not change them
5. `runArgs` format: keep as `["--network=host"]` (inline array, matching osal)

**Verification:** Each of the 4 files should be byte-for-byte identical in structure to the corresponding osal file,
with only `name` and `image` fields differing (correct toolchain-specific values).

---

## Phase 2 — clang-format config

**Goal:** Bring `.clang-format` up to date with osal's version, which uses a newer clang-format feature set.

**Context:**
The osal `.clang-format` is the reference. It lives at `/home/kuba/projects/kubasejdak/libs/osal/.clang-format`.
The liballocator `.clang-format` is outdated (written for an older clang-format version).

**Changes required to `.clang-format`:**

Remove deprecated/renamed options:

- `AlwaysBreakAfterDefinitionReturnType` — remove entirely
- `CommentPragmas` — remove entirely
- `ForEachMacros` — remove entirely
- `IndentRequires: false` — replace with `IndentRequiresClause: false`
- `SpaceBeforeParens: ControlStatements` — replace with the full `SpaceBeforeParensOptions` block from osal
- `SpaceInEmptyParentheses` — remove (deprecated)
- `SpacesInAngles: false` — change to `SpacesInAngles: Never`
- `SpacesInCStyleCastParentheses` — remove (deprecated)
- `SpacesInConditionalStatement` — remove (deprecated)
- `SpacesInParentheses` — replace with `SpacesInParens: Never`
- `UseCRLF: false` — replace with `LineEnding: LF`
- `Standard: Latest` — change to `Standard: c++20`

Add missing options (add them at the correct alphabetical positions as in osal):

- `AlignConsecutiveShortCaseStatements:` block (with `Enabled`, `AcrossEmptyLines`, `AcrossComments`, `AlignCaseColons`)
- `AllowBreakBeforeNoexceptSpecifier: OnlyWithParen`
- `AllowShortCompoundRequirementOnASingleLine: true`
- `BracedInitializerIndentWidth: 4`
- `BreakAdjacentStringLiterals: true`
- `BreakAfterAttributes: Never`
- `BreakArrays: false`
- `BreakBeforeInlineASMColon: OnlyMultiline`
- `InsertBraces: false`
- `InsertNewlineAtEOF: true`
- `IntegerLiteralSeparator:` block (Binary: 4, Decimal: -1, Hex: -1)
- `KeepEmptyLinesAtEOF: false`
- `RemoveBracesLLVM: true`
- `RemoveParentheses: ReturnStatement`
- `RemoveSemicolon: true`
- `RequiresClausePosition: OwnLine`
- `RequiresExpressionIndentation: OuterScope`
- `SkipMacroDefinitionBody: false`

Update `IncludeCategories` — the external libraries regex in Priority 3 should match osal:

```yaml
- Regex: '<(boost|catch2|cxxopts|fakeit|fmt|glaze|magic_enum|range|spdlog|toml\+\+)([A-Za-z0-9.\/_-])+>'
  Priority: 3
```

(liballocator currently has a different, older list)

**The simplest approach:** copy `.clang-format` directly from osal, then verify the file is identical to
`/home/kuba/projects/kubasejdak/libs/osal/.clang-format`.

**Also create `.clang-format-ignore`** (new file, copy from osal):

```
./cmake-build-*
./out
./build
./lib/main/freertos-arm/freertos-*
```

Adjust if needed — in liballocator there is no `lib/main/freertos-arm/` directory, but the pattern is harmless.

**Verification:** `.clang-format` should be structurally identical to osal's version.

---

## Phase 3 — clang-tidy config

**Goal:** Modernize `.clang-tidy` to match osal's format, option set, and naming conventions.

**Context:**
The osal `.clang-tidy` is at `/home/kuba/projects/kubasejdak/libs/osal/.clang-tidy`.
The liballocator `.clang-tidy` uses the old YAML list format for `CheckOptions` and has different disabled checks.

**Changes to `.clang-tidy`:**

Replace the entire `CheckOptions` section with the inline `Key: 'Value'` format matching osal.

Update the `Checks` disabled list. The final list should match osal exactly:

```yaml
Checks: '
  *,
  -altera-*,
  -bugprone-easily-swappable-parameters,
  -cppcoreguidelines-avoid-do-while,
  -cppcoreguidelines-pro-bounds-constant-array-index,
  -cppcoreguidelines-pro-bounds-pointer-arithmetic,
  -fuchsia-*,
  -google-default-arguments,
  -llvm-header-guard,
  -llvm-include-order,
  -llvmlibc-*,
  -misc-const-correctness,
  -modernize-use-trailing-return-type,
  -performance-enum-size,
  -readability-identifier-length,
  -readability-redundant-access-specifiers
'
```

Remove these top-level fields that exist in liballocator but not in osal:

- `HeaderFilterRegex`
- `AnalyzeTemporaryDtors`
- `User`

The `CheckOptions` section should match osal exactly:

```yaml
CheckOptions:
  abseil-string-find-str-contains.StringLikeClasses: "::absl::string_view"
  bugprone-dangling-handle.HandleClasses: "std::basic_string_view;std::experimental::basic_string_view"
  cppcoreguidelines-special-member-functions.AllowSoleDefaultDtor: "true"
  google-readability-braces-around-statements.ShortStatementLines: "2"
  hicpp-braces-around-statements.ShortStatementLines: "2"
  hicpp-special-member-functions.AllowSoleDefaultDtor: "true"
  misc-include-cleaner.IgnoreHeaders: "/usr/include/.*;/opt/toolchains/.*;VerboseReporter.hpp"
  misc-non-private-member-variables-in-classes.IgnoreClassesWithAllMemberVariablesBeingPublic: "true"
  readability-braces-around-statements.ShortStatementLines: "2"
  readability-function-cognitive-complexity.IgnoreMacros: "true"
  readability-identifier-naming.AbstractClassCase: "CamelCase"
  readability-identifier-naming.AbstractClassPrefix: "I"
  readability-identifier-naming.ClassCase: "CamelCase"
  readability-identifier-naming.EnumCase: "CamelCase"
  readability-identifier-naming.EnumConstantCase: "CamelCase"
  readability-identifier-naming.FunctionCase: "camelBack"
  readability-identifier-naming.MacroDefinitionCase: "UPPER_CASE"
  readability-identifier-naming.NamespaceCase: "lower_case"
  readability-identifier-naming.ParameterCase: "camelBack"
  readability-identifier-naming.PrivateMemberCase: "camelBack"
  readability-identifier-naming.PrivateMemberPrefix: "m_"
  readability-identifier-naming.ProtectedMemberCase: "camelBack"
  readability-identifier-naming.ProtectedMemberPrefix: "m_"
  readability-identifier-naming.PublicMemberCase: "camelBack"
  readability-identifier-naming.TemplateParameterCase: "CamelCase"
  readability-identifier-naming.TypeAliasCase: "CamelCase"
  readability-identifier-naming.TypedefCase: "CamelCase"
  readability-identifier-naming.UnionCase: "CamelCase"
  readability-identifier-naming.ValueTemplateParameterCase: "camelBack"
  readability-identifier-naming.VariableCase: "camelBack"
  readability-inconsistent-declaration-parameter-name.Strict: "true"
```

**Also create `tests/.clang-tidy`** (new file — matches osal's `tests/.clang-tidy`):

```yaml
---
InheritParentConfig: true
Checks: '
  -clang-analyzer-optin.core.EnumCastOutOfRange,
  -*-magic-numbers
'
...
```

**The simplest approach:** copy `.clang-tidy` directly from osal (it can be used unchanged since the naming conventions
and check set should be identical), then create `tests/.clang-tidy` from the osal version.

**Verification:** `.clang-tidy` should be structurally identical to osal's. `tests/.clang-tidy` should exist and match osal's `tests/.clang-tidy`.

---

## Phase 4 — Conan migration (1.x → 2.x)

**Goal:** Migrate from Conan 1.x to Conan 2.x dependency management.

**Context:**
liballocator currently uses Conan 1.x via `cmake/conan.cmake` (fetches cmake-conan 0.18.1). osal uses Conan 2.x via
`cmake/conan_provider.cmake` (the standard Conan 2.x CMake dependency provider). The provider file is large (~600 lines)
and should be copied from osal.

**Changes:**

1. **Update `conanfile.txt`** — replace content entirely:

   ```
   [test_requires]
   catch2/3.13.0

   [generators]
   CMakeDeps

   [options]
   catch2/*:default_reporter=verbose
   catch2/*:no_posix_signals=True
   ```

   Explanation of changes:
   - `[requires]` → `[test_requires]` (Conan 2.x: Catch2 is a test-only dependency)
   - `catch2/3.3.0` → `catch2/3.13.0` (version bump)
   - `fmt/9.1.0` removed (fmt is not a library dependency; evaluate test files to confirm it's not needed — if it IS used in test code, move it to `[test_requires]` as well)
   - `[generators] cmake` → `[generators] CMakeDeps` (Conan 2.x generator)
   - Added `catch2` options (verbose reporter, no posix signals)

2. **Copy `cmake/conan_provider.cmake` from osal** (`/home/kuba/projects/kubasejdak/libs/osal/cmake/conan_provider.cmake`)
   — copy the file verbatim.

3. **Delete `cmake/conan.cmake`** — this file contains the old Conan 1.x cmake-conan wrapper fetching logic. It is no
   longer needed with the Conan 2.x provider approach.

**Note:** `cmake/coverage.cmake`, `cmake/sanitizers.cmake`, `cmake/platform.cmake` will be deleted in Phase 7.
The old Conan invocations in `CMakeLists.txt` will be cleaned up in Phase 7.

**Verification:** `conanfile.txt` matches the format above. `cmake/conan_provider.cmake` exists and is identical to
osal's copy. `cmake/conan.cmake` is deleted.

---

## Phase 5 — CMakePresets restructure

**Goal:** Modernize `CMakePresets.json` to match osal's modular preset structure.

**Context:**
osal uses CMake presets version 8 with modular JSON files included by the root `CMakePresets.json`. liballocator uses
version 3 with a single monolithic file. The osal preset files live in `cmake/presets/`.

**Reference files** (read from osal at `/home/kuba/projects/kubasejdak/libs/osal/`):

- `CMakePresets.json` — root orchestrator
- `cmake/presets/linux.json` — linux platform + toolchain presets
- `cmake/presets/type.json` — debug/release presets
- `cmake/presets/app.json` — sanitizer presets (asan, lsan, tsan, ubsan)
- `cmake/presets/baremetal.json` — FreeRTOS/ARM preset
- `cmake/presets/dependencies.json` — conan provider preset

**Changes:**

1. **Create `cmake/presets/linux.json`** — copy from osal verbatim. This defines:
   - `linux` (hidden, PLATFORM=linux)
   - `linux-native-gcc`, `linux-native-clang` (hidden, toolchain)
   - `linux-arm64`, `linux-arm64-gcc`, `linux-arm64-clang` (hidden, cross-compile)
   - `yocto-sdk-gcc`, `yocto-sdk-clang` (hidden)

2. **Create `cmake/presets/type.json`** — copy from osal verbatim. Defines `debug` and `release` presets.

3. **Create `cmake/presets/app.json`** — copy from osal verbatim. Defines `asan`, `lsan`, `tsan`, `ubsan` presets.

4. **Create `cmake/presets/baremetal.json`** — copy from osal verbatim. Defines `freertos-armv7-m4` preset.

5. **Create `cmake/presets/dependencies.json`** — copy from osal verbatim. Defines `conan` preset with
   `CMAKE_PROJECT_TOP_LEVEL_INCLUDES = ${sourceDir}/cmake/conan_provider.cmake`.

6. **Replace `CMakePresets.json`** with a new file:
   ```json
   {
     "version": 8,
     "cmakeMinimumRequired": {
       "major": 3,
       "minor": 28,
       "patch": 0
     },
     "include": [
       "cmake/presets/app.json",
       "cmake/presets/baremetal.json",
       "cmake/presets/dependencies.json",
       "cmake/presets/linux.json",
       "cmake/presets/type.json"
     ],
     "configurePresets": [
       {
         "name": "linux-native-gcc-debug",
         "inherits": ["linux-native-gcc", "debug"]
       },
       {
         "name": "linux-native-gcc-release",
         "inherits": ["linux-native-gcc", "release"]
       },
       {
         "name": "linux-native-clang-debug",
         "inherits": ["linux-native-clang", "debug"]
       },
       {
         "name": "linux-native-clang-release",
         "inherits": ["linux-native-clang", "release"]
       },
       {
         "name": "linux-native-gcc-debug-asan",
         "inherits": ["linux-native-gcc-debug", "asan"]
       },
       {
         "name": "linux-native-gcc-debug-lsan",
         "inherits": ["linux-native-gcc-debug", "lsan"]
       },
       {
         "name": "linux-native-gcc-debug-tsan",
         "inherits": ["linux-native-gcc-debug", "tsan"]
       },
       {
         "name": "linux-native-gcc-debug-ubsan",
         "inherits": ["linux-native-gcc-debug", "ubsan"]
       },
       {
         "name": "linux-native-conan-gcc-debug",
         "inherits": ["linux-native-gcc-debug", "conan"]
       },
       {
         "name": "linux-native-conan-gcc-release",
         "inherits": ["linux-native-gcc-release", "conan"]
       },
       {
         "name": "linux-native-conan-clang-debug",
         "inherits": ["linux-native-clang-debug", "conan"]
       },
       {
         "name": "linux-native-conan-clang-release",
         "inherits": ["linux-native-clang-release", "conan"]
       },
       {
         "name": "linux-native-conan-gcc-debug-asan",
         "inherits": ["linux-native-conan-gcc-debug", "asan"]
       },
       {
         "name": "linux-native-conan-gcc-debug-lsan",
         "inherits": ["linux-native-conan-gcc-debug", "lsan"]
       },
       {
         "name": "linux-native-conan-gcc-debug-tsan",
         "inherits": ["linux-native-conan-gcc-debug", "tsan"]
       },
       {
         "name": "linux-native-conan-gcc-debug-ubsan",
         "inherits": ["linux-native-conan-gcc-debug", "ubsan"]
       },
       {
         "name": "linux-arm64-conan-gcc-debug",
         "inherits": ["linux-arm64-gcc", "debug", "conan"]
       },
       {
         "name": "linux-arm64-conan-gcc-release",
         "inherits": ["linux-arm64-gcc", "release", "conan"]
       },
       {
         "name": "linux-arm64-conan-clang-debug",
         "inherits": ["linux-arm64-clang", "debug", "conan"]
       },
       {
         "name": "linux-arm64-conan-clang-release",
         "inherits": ["linux-arm64-clang", "release", "conan"]
       },
       {
         "name": "yocto-sdk-gcc-debug",
         "inherits": ["yocto-sdk-gcc", "debug"]
       },
       {
         "name": "yocto-sdk-gcc-release",
         "inherits": ["yocto-sdk-gcc", "release"]
       },
       {
         "name": "yocto-sdk-clang-debug",
         "inherits": ["yocto-sdk-clang", "debug"]
       },
       {
         "name": "yocto-sdk-clang-release",
         "inherits": ["yocto-sdk-clang", "release"]
       },
       {
         "name": "freertos-armv7-m4-conan-gcc-debug",
         "inherits": ["freertos-armv7-m4", "debug", "conan"]
       },
       {
         "name": "freertos-armv7-m4-conan-gcc-release",
         "inherits": ["freertos-armv7-m4", "release", "conan"]
       }
     ]
   }
   ```

**Verification:** `cmake preset list` should list all expected presets (linux-native-_, linux-arm64-_, yocto-sdk-_, freertos-_, sanitizer variants). The old presets (`tests-linux-gcc-debug`, `demo-baremetal-arm-debug`, etc.) should be gone.

---

## Phase 6 — CMake structure cleanup

**Goal:** Clean up CMakeLists.txt files and replace the old cmake helper files with osal-style structure.

**Context:**
osal's root `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.28)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

find_package(platform COMPONENTS toolchain)

project(osal ASM C CXX)

include(cmake/compilation-flags.cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_subdirectory(tests)
```

osal's `cmake/compilation-flags.cmake`:

```cmake
add_compile_options(-Wall -Wextra -Wpedantic -Werror $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 23)
```

osal's `cmake/components.cmake`:

```cmake
if (NOT osal_FIND_COMPONENTS)
    file(GLOB osal_FIND_COMPONENTS LIST_DIRECTORIES true RELATIVE ${osal_SOURCE_DIR}/lib ${osal_SOURCE_DIR}/lib/*)
endif ()

include(FetchContent)
foreach (component IN LISTS osal_FIND_COMPONENTS)
    FetchContent_Declare(osal-${component}
        SOURCE_DIR      ${osal_SOURCE_DIR}
        SOURCE_SUBDIR   lib/${component}
        SYSTEM
    )

    FetchContent_MakeAvailable(osal-${component})
endforeach ()
```

osal's `cmake/modules/Findosal.cmake`:

```cmake
set(osal_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../..)
include(${CMAKE_CURRENT_LIST_DIR}/../components.cmake)
```

**Changes:**

1. **Replace root `CMakeLists.txt`**:

   ```cmake
   cmake_minimum_required(VERSION 3.28)

   list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

   find_package(platform COMPONENTS toolchain)

   project(liballocator ASM C CXX)

   include(cmake/compilation-flags.cmake)

   set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
   add_subdirectory(lib)
   add_subdirectory(tests)
   ```

   Note: `add_subdirectory(tests)` replaces old `add_subdirectory(test)` — the directory rename happens in Phase 10.
   For now if the `tests/` dir doesn't exist yet, this is fine — the rename will be coordinated.
   Actually: to avoid breakage, keep `add_subdirectory(test)` in this phase and change to `add_subdirectory(tests)`
   as part of Phase 10 (tests rename).

2. **Create `cmake/compilation-flags.cmake`**:

   ```cmake
   add_compile_options(-Wall -Wextra -Wpedantic -Werror $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>)
   set(CMAKE_C_STANDARD 17)
   set(CMAKE_CXX_STANDARD 23)
   ```

3. **Create `cmake/components.cmake`** (same as osal but for liballocator):

   ```cmake
   if (NOT liballocator_FIND_COMPONENTS)
       file(GLOB liballocator_FIND_COMPONENTS LIST_DIRECTORIES true RELATIVE ${liballocator_SOURCE_DIR}/lib ${liballocator_SOURCE_DIR}/lib/*)
   endif ()

   include(FetchContent)
   foreach (component IN LISTS liballocator_FIND_COMPONENTS)
       FetchContent_Declare(liballocator-${component}
           SOURCE_DIR      ${liballocator_SOURCE_DIR}
           SOURCE_SUBDIR   lib/${component}
           SYSTEM
       )

       FetchContent_MakeAvailable(liballocator-${component})
   endforeach ()
   ```

4. **Create `cmake/modules/Findliballocator.cmake`**:

   ```cmake
   set(liballocator_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../..)
   include(${CMAKE_CURRENT_LIST_DIR}/../components.cmake)
   ```

5. **Update `lib/CMakeLists.txt`**:
   - Bump `cmake_minimum_required(VERSION 3.28)`
   - Remove `configure_file(version.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/version.hpp)` line (and the `message(STATUS ...)` above it)
   - Remove the duplicate `add_compile_options(...)` line (flags are now in `cmake/compilation-flags.cmake`)
   - Add namespace alias after the `add_library(liballocator ...)` block:
     ```cmake
     add_library(liballocator::liballocator ALIAS liballocator)
     ```

6. **Delete the following files**:
   - `cmake/conan.cmake`
   - `cmake/coverage.cmake`
   - `cmake/platform.cmake`
   - `cmake/sanitizers.cmake`
   - `lib/version.hpp.in`
   - `lib/version.hpp` (if it exists as a generated file)

**Verification:** The root `CMakeLists.txt` has no references to APP, conan 1.x, coverage, sanitizers, or platform.cmake. `lib/CMakeLists.txt` has the namespace alias. All deleted files are gone. The `cmake/` directory now contains: `conan_provider.cmake` (from Phase 4), `compilation-flags.cmake`, `components.cmake`, `modules/Findliballocator.cmake`.

---

## Phase 7 — Remove external/, add Findstm32f4xx.cmake

**Goal:** Remove the vendored STM32F4 HAL from the repo and replace with a FetchContent-based Find module.

**Context:**
liballocator vendors the entire STM32F4 HAL in `external/stm32f4xx/`. osal does not vendor anything — it has
`cmake/modules/Findstm32f4xx.cmake` which uses FetchContent to download the HAL on demand.

osal's `cmake/modules/Findstm32f4xx.cmake`:

```cmake
include(FetchContent)

FetchContent_Declare(CMSIS_5
    GIT_REPOSITORY      https://github.com/ARM-software/CMSIS_5.git
    GIT_TAG             5.9.0
    SYSTEM
)

FetchContent_Declare(cmsis_device_f4
    GIT_REPOSITORY      https://github.com/STMicroelectronics/cmsis_device_f4.git
    GIT_TAG             v2.6.10
    SYSTEM
)

FetchContent_Declare(stm32f4xx
    GIT_REPOSITORY      https://github.com/STMicroelectronics/stm32f4xx_hal_driver.git
    GIT_TAG             v1.8.3
    SYSTEM
)

FetchContent_MakeAvailable(CMSIS_5)
FetchContent_MakeAvailable(cmsis_device_f4)
FetchContent_MakeAvailable(stm32f4xx)

add_library(stm32f4xx EXCLUDE_FROM_ALL
    ${cmsis_device_f4_SOURCE_DIR}/Source/Templates/gcc/startup_stm32f407xx.s
    ${cmsis_device_f4_SOURCE_DIR}/Source/Templates/system_stm32f4xx.c
    ${stm32f4xx_SOURCE_DIR}/Src/stm32f4xx_hal.c
    ${stm32f4xx_SOURCE_DIR}/Src/stm32f4xx_hal_cortex.c
    ${stm32f4xx_SOURCE_DIR}/Src/stm32f4xx_hal_gpio.c
    ${stm32f4xx_SOURCE_DIR}/Src/stm32f4xx_hal_rcc.c
    ${stm32f4xx_SOURCE_DIR}/Src/stm32f4xx_hal_uart.c
)

target_compile_definitions(stm32f4xx
    PUBLIC
        STM32F407xx
        USE_HAL_DRIVER
)

target_include_directories(stm32f4xx
    PUBLIC
        ${cmsis_5_SOURCE_DIR}/CMSIS/Core/Include
        ${stm32f4xx_SOURCE_DIR}/Inc
        ${stm32f4xx_SOURCE_DIR}/Inc/Legacy
        ${cmsis_device_f4_SOURCE_DIR}/Include
)

target_compile_options(stm32f4xx
    PRIVATE
        -w
)

set(stm32f4xx_FOUND TRUE)
```

**Changes:**

1. **Delete `external/` directory** entirely (contains `external/stm32f4xx/` with vendored CMSIS, drivers, startup files, HAL config)

2. **Create `cmake/modules/Findstm32f4xx.cmake`** — copy verbatim from osal (content shown above)

3. **Check `test/init/baremetal-arm/` CMakeLists.txt** — if it references `external/stm32f4xx` directly (e.g., `add_subdirectory(${CMAKE_SOURCE_DIR}/external/stm32f4xx ...)`), replace with `find_package(stm32f4xx)`.

4. **Check `test/liballocator-demo/CMakeLists.txt`** — same as above: replace any direct `external/` reference with `find_package(stm32f4xx)`.

**Verification:** `external/` directory is gone. `cmake/modules/Findstm32f4xx.cmake` exists and matches osal's version. Any CMakeLists.txt that referenced `external/stm32f4xx` now uses `find_package(stm32f4xx)`.

---

## Phase 8 — Replace GitLab CI with GitHub Actions

**Goal:** Remove all GitLab CI config and replace with GitHub Actions workflows matching osal's pattern.

**Context:**
liballocator has:

- `.gitlab-ci.yml` (root)
- `.gitlab/ci/*.yml` (8 files: build-baremetal, build-linux, coverage-linux, demo-baremetal, deploy, quality, test-linux, valgrind-linux)
- `.gitlab/issue_templates/*.md` (Bug.md, Feature.md, Generic.md)

osal has 4 GitHub Actions workflows:

- `.github/workflows/build-test-linux.yml`
- `.github/workflows/build-test-baremetal.yml`
- `.github/workflows/code-coverage.yml`
- `.github/workflows/static-analysis.yml`

**Changes:**

1. **Delete all GitLab files:**
   - `.gitlab-ci.yml`
   - `.gitlab/` directory (entire tree)

2. **Create `.github/workflows/build-test-linux.yml`** — copy from osal, but replace:
   - All occurrences of `osal-tests` → `liballocator-tests` (the test binary name)
   - Keep all preset names as-is (they are now aligned from Phase 5)

3. **Create `.github/workflows/build-test-baremetal.yml`** — copy from osal verbatim (preset names match from Phase 5)

4. **Create `.github/workflows/code-coverage.yml`** — copy from osal, replacing `osal-tests` → `liballocator-tests`

5. **Create `.github/workflows/static-analysis.yml`** — copy from osal verbatim

Reference osal workflow files at `/home/kuba/projects/kubasejdak/libs/osal/.github/workflows/`.

**Verification:** `.gitlab*` files are gone. `.github/workflows/` contains exactly 4 files matching osal's structure with correct binary name substitution.

---

## Phase 9 — License (root file + file headers)

**Goal:** Change license from BSD 2-Clause to MIT and update copyright year to single creation year.

**Context:**

- osal uses MIT License with single creation year (e.g., `Copyright (c) 2019 Kuba Sejdak (kuba.sejdak@gmail.com)`)
- liballocator uses BSD 2-Clause with year range (`Copyright (c) 2017-2023, Kuba Sejdak <kuba.sejdak@gmail.com>`)
- liballocator was created in 2017 — that is the single year to use

The osal file header format (MIT):

```cpp
/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright MIT License
///
/// Copyright (c) 2017 Kuba Sejdak (kuba.sejdak@gmail.com)
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
///
/////////////////////////////////////////////////////////////////////////////////////
```

**Changes:**

1. **Replace `LICENSE`** with MIT License text, copyright `2017`:

   ```
   MIT License

   Copyright (c) 2017 Kuba Sejdak (kuba.sejdak@gmail.com)

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
   ```

2. **Update file headers in every `.cpp`, `.hpp`, `.h` file** under `lib/` and `test/`:
   - Replace the BSD 2-Clause block (lines 1–31 of each file) with the MIT header shown above
   - The `/// Copyright (c) 2017` line should use exactly `2017` (not a range)
   - Note: the `@copyright` line should read `@copyright MIT License`

   Files to update (all source files in lib/ and test/):
   - `lib/allocator.cpp`
   - `lib/group.cpp`
   - `lib/group.hpp`
   - `lib/include/allocator/allocator.hpp`
   - `lib/include/allocator/Region.hpp`
   - `lib/ListNode.hpp`
   - `lib/PageAllocator.cpp`
   - `lib/PageAllocator.hpp`
   - `lib/Page.cpp`
   - `lib/Page.hpp`
   - `lib/RegionInfo.cpp`
   - `lib/RegionInfo.hpp`
   - `lib/utils.hpp`
   - `lib/ZoneAllocator.cpp`
   - `lib/ZoneAllocator.hpp`
   - `lib/Zone.cpp`
   - `lib/Zone.hpp`
   - `test/liballocator-tests/unit/*.cpp` (all files)
   - `test/liballocator-tests/integration/*.cpp` (all files)
   - `test/liballocator-tests/perf/*.cpp` (all files)
   - `test/liballocator-tests/appMain.cpp`
   - `test/liballocator-demo/appMain.cpp`
   - `test/init/baremetal-arm/init.cpp`
   - `test/init/freertos-arm/init.cpp`
   - `test/init/linux/init.cpp`

**Verification:** `LICENSE` is MIT. All source files have the MIT header block with `2017`. No file has `BSD` or `2017-2023` in its header.

---

## Phase 10 — Tests directory rename + assertion style

**Goal:** Rename `test/` to `tests/` and fix Catch2 assertion style.

**Context:**
osal uses a `tests/` directory (plural). liballocator uses `test/` (singular).

Catch2 assertion conventions (both repos):

- `REQUIRE` — use for guards: a failure would cause a crash or undefined behavior if execution continued
- `CHECK` — use for verifiable assertions where continued execution is safe
- Never use negation inside assertions. Use `REQUIRE_FALSE(...)` / `CHECK_FALSE(...)` instead of `REQUIRE(!...)` / `CHECK(!...)`

Current negation violations in liballocator tests (all use `REQUIRE(!...)`):

- `test/liballocator-tests/unit/Page.cpp:58`: `REQUIRE(!page->isUsed())`
- `test/liballocator-tests/unit/allocator.cpp:132`: `REQUIRE(!allocator::init(regions.data(), cPageSize))`
- `test/liballocator-tests/unit/RegionInfo.cpp:98,137,154,206,260,314,326`
- `test/liballocator-tests/unit/PageAllocator.cpp:77,91,215,250`
- `test/liballocator-tests/unit/Zone.cpp:196,202,208,214,219,224`
- `test/liballocator-tests/unit/ZoneAllocator.cpp:98`
  (Find all occurrences with: `grep -rn "REQUIRE(!" test/ && grep -rn "CHECK(!" test/`)

**Changes:**

1. **Rename directory `test/` → `tests/`** (git mv to preserve history):

   ```bash
   git mv test tests
   ```

2. **Update `CMakeLists.txt` (root)**: change `add_subdirectory(test)` → `add_subdirectory(tests)`

3. **Update `tests/CMakeLists.txt`**: any internal `add_subdirectory` paths that were relative to `test/` should still work since they're relative — verify they do.

4. **Fix all negation assertions**: replace every `REQUIRE(!expr)` with `REQUIRE_FALSE(expr)` and every `CHECK(!expr)` with `CHECK_FALSE(expr)`. Search with grep and fix systematically.

5. **Review REQUIRE vs CHECK semantics** across all test files:
   - Assertions that initialize or set up state (e.g., `REQUIRE(allocator::init(...))`) should remain `REQUIRE` — if init fails, further assertions are meaningless or dangerous
   - Assertions on observable values (e.g., checking stats after an operation) can often be `CHECK` to allow all failures to be reported in one run
   - This is a judgment call — be conservative; only change `REQUIRE` → `CHECK` when you are confident continued execution cannot crash

**Verification:** `test/` directory is gone. `tests/` directory exists with same content. All `REQUIRE(!...)` and `CHECK(!...)` patterns are gone. Build system references are updated.

---

## Phase 11 — CONTRIBUTING.md

**Goal:** Replace the empty CONTRIBUTING.md with full content matching osal's conventions.

**Context:**
`CONTRIBUTING.md` in liballocator is currently empty (0 bytes). osal's CONTRIBUTING.md at
`/home/kuba/projects/kubasejdak/libs/osal/CONTRIBUTING.md` is the reference.

**Changes:**
Copy osal's `CONTRIBUTING.md` to liballocator, replacing the empty file. No content changes needed — the document
is written generically for any kubasejdak-org library.

**Verification:** `CONTRIBUTING.md` is non-empty and matches osal's content.

---

## Phase 12 — README.md rewrite

**Goal:** Rewrite the README to match osal's modern format.

**Context:**
The current liballocator README is outdated — it references GitLab, uses `add_subdirectory` integration, has
old performance tables, and lacks proper documentation structure. osal's README is the format reference.

**Target README structure:**

```markdown
# liballocator

<one-paragraph description of what liballocator is and does>

Main features:

- **page allocator**: ...
- **zone allocator**: ...

## Supported Platforms

| Variable    | Backend details |
| ----------- | --------------- |
| `linux`     | Linux           |
| `freertos`  | FreeRTOS        |
| `baremetal` | Bare metal      |

## Architecture

### Components

- **`liballocator`** — ...

### Technologies

- **Language**: C++23, C17
- **Build System**: CMake (minimum version 3.28)
- **Package Manager**: Conan (test dependencies only)
- **Static Analysis**: clang-format, clang-tidy
- **CI/CD**: GitHub Actions

### Repository Structure

<bash code block showing tree>

## Usage

### CMake Integration

<FetchContent example using Findliballocator.cmake pattern, not add_subdirectory>

### Configuration

<table of CMake variables>

### Linking

<target_link_libraries example>

## Development

> [!NOTE]
> This section is relevant when working on `liballocator` itself in standalone mode.

### Commands

- **Configure**: ...
- **Build**: ...
- **Run tests**: ...
- **Reformat code**: ...
- **Run linter**: ...

### Available CMake Presets

<list matching the presets from Phase 5>

### Code Quality

- **Zero Warning Policy**: ...
- **No Exceptions**: ...
- **Code Formatting**: ...
- **Static Analysis**: ...
- **Sanitizers**: ...
```

**Key points:**

- Remove all GitLab URLs; use `https://github.com/kubasejdak-org/liballocator.git`
- Replace `add_subdirectory` integration with FetchContent + `find_package(liballocator)` example
- Use `>[!NOTE]` and `>[!IMPORTANT]` GitHub callout blocks (matching osal style)
- Include a Mermaid architecture diagram showing PageAllocator → ZoneAllocator relationship
- Remove the outdated performance tables
- The preset list should match exactly what Phase 5 defines

---

## Phase 13 — Miscellaneous cleanup

**Goal:** Clean up remaining small inconsistencies and add missing tooling files.

**Changes:**

1. **Update `.gitignore`**:
   - Remove `.vscode/` entry (not in osal's gitignore; VS Code settings that should be committed should be, those that shouldn't are already in `.devcontainer/`)
   - Remove `version.hpp` entry (the file is being removed in Phase 6, so this entry is now orphaned)
   - Final `.gitignore` content should match osal's exactly:

     ```
     # Object and executable files
     cmake-build-*/
     out/
     build/

     # Build systems
     CMakeLists.txt.user
     CMakeUserPresets.json

     # IDE
     .idea/
     ```

2. **Add `.prettierrc`** — copy from osal verbatim:

   ```json
   {
     "printWidth": 120,
     "proseWrap": "always",
     "overrides": [
       {
         "files": "*.md",
         "options": {
           "tabWidth": 4
         }
       }
     ]
   }
   ```

3. **Add `tools/adjust-compilation-db.py`** — copy from osal (`/home/kuba/projects/kubasejdak/libs/osal/tools/adjust-compilation-db.py`)

4. **Add `tools/run-clang-format.py`** — copy from osal (`/home/kuba/projects/kubasejdak/libs/osal/tools/run-clang-format.py`)

5. **Remove obsolete files**:
   - `tools/ci/` directory (with `logs-reader.py`, `program-openocd.py`) — these were GitLab CI helpers
   - `tools/profile.in` — Conan 1.x profile template, replaced by Conan 2.x in Phase 4

**Verification:** `.gitignore` matches osal's. `.prettierrc` exists and matches osal's. `tools/adjust-compilation-db.py` and `tools/run-clang-format.py` exist. `tools/ci/` and `tools/profile.in` are gone.

---

## Execution Order

Phases can be executed in this order (each is independent but later phases build on earlier ones):

1. Phase 1 — devcontainers
2. Phase 2 — clang-format
3. Phase 3 — clang-tidy
4. Phase 4 — Conan migration
5. Phase 5 — CMakePresets restructure
6. Phase 6 — CMake structure cleanup _(depends on Phase 4 for conan_provider.cmake, Phase 5 for preset names)_
7. Phase 7 — Remove external/
8. Phase 8 — GitHub Actions _(depends on Phase 5 for preset names)_
9. Phase 9 — License
10. Phase 10 — Tests rename + assertions
11. Phase 11 — CONTRIBUTING.md
12. Phase 12 — README.md
13. Phase 13 — Miscellaneous cleanup
