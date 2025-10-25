# Data Object Editor

A demonstration application using AppFwk (Application Framework) and opm-common from ResInsight.

## Features

- **AppFwk PDM System**: Project Data Model with automatic UI generation
- **Property Editors**: Automatic UI generation from PDM fields
- **Tree View**: Hierarchical object navigation
- **XML Serialization**: Load/save projects
- **Command Framework**: Undo/redo support
- **opm-common**: Eclipse deck parsing capabilities

## Prerequisites

### Windows
- Visual Studio 2022 (17.8+) with C++ workload
- CMake 3.26+
- Qt 6.4+
- Git for Windows
- (Optional) Ninja build system for faster builds

### Linux (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install build-essential cmake git
sudo apt install qt6-base-dev qt6-base-private-dev libqt6svg6-dev
```

## Building

### 1. Bootstrap vcpkg

First, bootstrap vcpkg from the ResInsight submodule:

**Windows:**
```powershell
external\ResInsight\ThirdParty\vcpkg\bootstrap-vcpkg.bat
```

**Linux:**
```bash
external/ResInsight/ThirdParty/vcpkg/bootstrap-vcpkg.sh
```

### 2. Configure Qt Path

Copy the appropriate CMakeUserPresets example:

**Windows:**
```powershell
copy CMakeUserPresets-example-windows.json CMakeUserPresets.json
```

**Linux:**
```bash
cp CMakeUserPresets-example-linux.json CMakeUserPresets.json
```

Edit `CMakeUserPresets.json` and update the Qt installation path.

### 3. Build the Project

**Windows (Visual Studio 2022):**
```powershell
cmake --preset windows-release
cmake --build build --config Release
```

**Windows (Ninja - faster builds):**
```powershell
# Run from Visual Studio Developer Command Prompt or Developer PowerShell
cmake --preset windows-ninja-release
cmake --build build --config Release
```

**Linux:**
```bash
cmake --preset linux-release
cmake --build build
```

### 4. Run the Application

**Windows:**
```powershell
build\src\Release\DataObjectEditorApp.exe
```

**Linux:**
```bash
./build/src/DataObjectEditorApp
```

## Project Structure

```
data-object-editor/
├── CMakeLists.txt              # Root CMake configuration
├── vcpkg.json                  # Dependency management
├── CMakeUserPresets.json       # User-specific CMake settings (not in git)
├── external/
│   └── ResInsight/             # Git submodule
│       ├── Fwk/AppFwk/        # Application Framework libraries
│       └── ThirdParty/
│           ├── vcpkg/         # Package manager
│           └── custom-opm-common/  # Eclipse deck parsing
└── src/
    ├── CMakeLists.txt         # Application CMake
    ├── main.cpp               # Entry point
    ├── MainWindow.h/cpp       # Main application window
    └── ...
```

## AppFwk Libraries Included

- **cafPdmCore** - PDM object and field types
- **cafPdmUiCore** - UI capabilities for PDM
- **cafPdmXml** - XML serialization
- **cafProjectDataModel** - Document and object management
- **cafUserInterface** - Property editors and views
- **cafCommand** - Undo/redo system
- **cafCommandFeatures** - Standard commands
- **cafPdmCvf** - CVF type support (vectors, matrices, colors)
- **cafTensor** - Tensor data structures
- **LibCore** - Core utilities (minimal VizFwk)

## Usage Example

The application demonstrates basic PDM functionality:

1. **View Objects**: The tree view shows the object hierarchy
2. **Edit Properties**: Select an object to edit its properties in the property view
3. **Save/Load**: Projects can be saved/loaded as XML files
4. **Extend**: Add your own PDM objects by inheriting from `caf::PdmObject`

### Creating Custom PDM Objects

```cpp
class MyCustomObject : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    MyCustomObject()
    {
        CAF_PDM_InitObject("My Object", "", "", "");
        CAF_PDM_InitField(&m_myField, "MyField", 0.0, "My Field", "", "", "");
    }

    caf::PdmField<double> m_myField;
};

CAF_PDM_SOURCE_INIT(MyCustomObject, "MyCustomObject");
```

## Using opm-common

The `custom-opm-common` library is included for parsing Eclipse deck files:

```cpp
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>

// Parse an Eclipse deck file
Opm::Parser parser;
auto deck = parser.parseFile("path/to/DECK.DATA");
```

## Configuration Options

Key CMake options (set in root CMakeLists.txt):

- `CEE_USE_QT6` - Use Qt6 (ON by default)
- `CAF_EXCLUDE_CVF` - Exclude VizFwk dependencies (OFF - includes LibCore)
- `BUILD_SHARED_LIBS` - Build as shared libraries (OFF - static linking)
- `ENABLE_ECL_INPUT/OUTPUT` - Enable Eclipse file support (ON)

## License

This is a demonstration project. Check the ResInsight repository for license information on the AppFwk and opm-common components.

## Resources

- [ResInsight GitHub](https://github.com/OPM/ResInsight)
- [ResInsight Documentation](https://resinsight.org/)
- [opm-common](https://github.com/OPM/opm-common)
