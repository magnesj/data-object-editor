# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

The Data Object Editor is a Qt6-based application for viewing and editing Eclipse DATA files. It uses the ResInsight Application Framework (AppFwk) and opm-common libraries for data management and UI components.

## Build System

The project uses CMake with vcpkg for dependency management. Key requirements:
- CMake 3.26+
- C++23 standard
- Qt 6.4+
- Visual Studio 2022 17.8+ (Windows) / GCC 13+ or Clang 19+ (Linux)

### Build Configuration

```bash
# Windows (from repository root)
cmake --preset x64-relwithdebinfo
cmake --build --preset x64-relwithdebinfo

# Or using existing build directory
cd build
cmake --build . --target DataObjectEditorApp
```

Key CMake targets:
- `DataObjectEditorApp`: Main application executable
- `custom-opm-common`: OPM common library for Eclipse data parsing
- AppFwk libraries: `cafUserInterface`, `cafCommand`, `cafPdmCore`, etc.

## Architecture

### Core Components

1. **MainWindow** (`src/MainWindow.cpp/.h`)
   - Main application window with Qt-based UI
   - Manages project document and UI views
   - Handles file operations and recent files

2. **ProjectDocument** (`src/MainWindow.cpp`)
   - Root PDM document containing DataDecks
   - Manages collection of imported DATA files

3. **DataDeck Framework** (`src/DataDeck/`)
   - `RimDataDeck`: Main data structure for Eclipse DATA files
   - `RimDataSection`: Represents sections like RUNSPEC, GRID, etc.
   - `RimDataKeyword`: Individual keywords within sections
   - `RimDataItem`: Data items within keywords
   - `RimDataDeckTextEditor`: Text editor with syntax highlighting
   - `DataFileSyntaxHighlighter`: Eclipse DATA file syntax highlighting

4. **Command Framework**
   - `RicImportDataDeckFeature`: Handles DATA file import functionality
   - Uses AppFwk command pattern for operations

### Key Dependencies

- **Qt6**: Core UI framework (Core, Gui, Widgets, Svg)
- **AppFwk**: ResInsight application framework for PDM and UI
  - `cafPdmCore`: Project Data Model core system
  - `cafUserInterface`: UI components (tree view, property view)
  - `cafCommand`: Command pattern implementation
- **opm-common**: Eclipse data file parsing (custom build)

### PDM (Project Data Model) System

The application uses ResInsight's PDM system:
- Objects inherit from `caf::PdmObject`
- Fields use `caf::PdmField<T>` or `caf::PdmChildArrayField<T>`
- Automatic UI generation from field definitions
- Serialization support

Example PDM class structure:
```cpp
class RimDataItem : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    caf::PdmField<QString> m_name;
    caf::PdmField<QString> m_value;
};
```

## File Organization

```
src/
├── main.cpp                 # Application entry point
├── MainWindow.cpp/.h        # Main window implementation
└── DataDeck/
    ├── RimDataDeck.cpp/.h           # Main DATA file container
    ├── RimDataSection.cpp/.h        # DATA file sections
    ├── RimDataKeyword.cpp/.h        # Individual keywords
    ├── RimDataItem.cpp/.h           # Data items
    ├── RimDataDeckTextEditor.cpp/.h # Text editor with syntax highlighting
    ├── DataFileSyntaxHighlighter.cpp/.h # Syntax highlighting
    └── RicImportDataDeckFeature.cpp/.h  # Import functionality
```

## Development Guidelines

### Code Style
- Follow existing ResInsight conventions
- Use CAF_PDM macros for PDM objects
- Prefer Qt containers and types
- Use meaningful variable and function names

### Adding New Features
1. Create PDM objects for data structures
2. Use AppFwk command pattern for operations
3. Implement UI using cafUserInterface components
4. Add syntax highlighting rules if needed

### Text Editor Integration
The application includes a synchronized text editor:
- `RimDataDeckTextEditor`: Custom QPlainTextEdit with line numbers
- `DataFileSyntaxHighlighter`: Handles Eclipse DATA syntax
- Bidirectional sync between tree view and text editor
- Font: Cascadia Mono (Visual Studio 2022 style)

### Testing
- Build the application after changes: `cmake --build . --target DataObjectEditorApp`
- Test DATA file import functionality
- Verify syntax highlighting works correctly
- Check UI responsiveness and layout

## Common Tasks

### Adding New DATA File Keywords
1. Extend `RimDataKeyword` if needed
2. Update `DataFileSyntaxHighlighter` rules
3. Test with sample DATA files

### Modifying UI Layout
1. Use Qt Designer patterns or code-based layout
2. Leverage AppFwk's automatic UI generation from PDM fields
3. Update dock panels in `MainWindow::createDockPanels()`

### Adding File Format Support
1. Extend `RicImportDataDeckFeature` parsing logic
2. Add new file type handling in `MainWindow::importDataFile()`
3. Update recent files management if needed

## Dependencies Management

### vcpkg Integration
- Dependencies managed through `vcpkg.json`
- Automatic dependency resolution via CMake toolchain
- Bootstrap vcpkg: `external/vcpkg/bootstrap-vcpkg.bat` (Windows)

### AppFwk Components
Sourced from ResInsight submodule:
- Core PDM system
- UI components (tree view, property view)
- Command framework
- Selection management

### Custom opm-common
- Modified OPM library for Eclipse data parsing
- Located in `external/ResInsight/ThirdParty/custom-opm-common/`
- Handles ECLIPSE binary and text file formats

## Recent Changes

### Boost Spirit Removal
- Replaced Boost Spirit parsing with standard C++ in StarToken.cpp
- Uses `std::from_chars()` and `std::strtod()` for performance
- Maintains Fortran 'D' exponent notation support

### Demo Objects Cleanup
- Removed demo/example objects (`DemoDocument`, `DemoObject`)
- Application now starts with clean empty project
- Focus on actual DATA file editing functionality

### Syntax Highlighting
- Enhanced Eclipse DATA file syntax highlighting
- Cascadia Mono font for modern development experience
- Color scheme matching Visual Studio 2022 style

## Building and Testing

```bash
# Full clean build
cd build
cmake --build . --clean-first --target DataObjectEditorApp

# Build specific components
cmake --build . --target custom-opm-common
cmake --build . --target cafUserInterface

# Run the application
./DataObjectEditorApp.exe  # Windows
./DataObjectEditorApp      # Linux
```

## Git and Version Control

### Commit Guidelines
- Use descriptive commit messages following conventional commit format
- Do not add Claude as co-author when creating commits
- Focus commit messages on the actual changes and their purpose
- Example: "Remove demo objects and simplify project initialization"

### Code Attribution
- Attribute work to the human developer making the changes
- Claude's assistance is acknowledged but not formally credited in git history
- Maintain clean git history focused on actual development contributions

## Troubleshooting

### Build Issues
- Ensure Qt6 is properly installed and CMAKE_PREFIX_PATH is set
- Verify vcpkg toolchain is configured
- Check C++23 compiler support

### Runtime Issues
- Verify all Qt DLLs are available (handled by qt_generate_deploy_app_script)
- Check PDM object initialization with CAF_PDM_SOURCE_INIT
- Ensure DATA file paths are accessible

### UI Issues
- Verify PDM field definitions match UI expectations
- Check dock panel creation and layout
- Test with different window sizes and DPI settings