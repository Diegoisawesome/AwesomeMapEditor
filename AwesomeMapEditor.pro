#
# AwesomeMapEditor: A map editor for GBA Pokémon games.
# Copyright (C) 2016 Diegoisawesome, Pokedude
# License: General Public License 3.0
#


#
# QMake Settings, 1
#
QT         += core gui opengl widgets
TARGET      = AwesomeMapEditor
TEMPLATE    = app
CONFIG     += c++11
DEFINES    += AME_EDITOR


#
# QMake Settings, 2
#
VERSION = 1.0
win:QMAKE_TARGET_COMPANY     = DoMoreAwesome
win:QMAKE_TARGET_PRODUCT     = AwesomeMapEditor
win:QMAKE_TARGET_DESCRIPTION = A map editor for GBA Pokémon games.
win:QMAKE_TARGET_COPYRIGHT   = (C) 2016 Diegoisawesome, Pokedude


#
# QMake Settings, 3
#
INCLUDEPATH += include
INCLUDEPATH += include/AME/Forms
INCLUDEPATH += include/AME/Widgets
INCLUDEPATH += include/AME/Widgets/OpenGL
INCLUDEPATH += $$PWD/../QBoy/include
DEPENDPATH  += $$PWD/../QBoy/include
INCLUDEPATH += $$PWD/../yaml-cpp/include
DEPENDPATH += $$PWD/../yaml-cpp/include
QMAKE_LFLAGS += -static-libgcc -static-libstdc++
win32: LIBS += -lopengl32
unix:  LIBS += -lGL


#
# QMake Settings, 4
#
CONFIG(debug, debug|release) {
    unix|win32: LIBS += -L$$PWD/../QBoy/bin/debug/ -lQBoy
    unix|win32: LIBS += -L$$PWD/../yaml-cpp/bin/debug/ -lyaml-cpp
} else {
    unix|win32: LIBS += -L$$PWD/../QBoy/bin/release/ -lQBoy
    unix|win32: LIBS += -L$$PWD/../yaml-cpp/bin/release/ -lyaml-cpp
}


#
# Source Files
#
SOURCES += \
    src/main.cpp \
    src/Forms/MainWindow.cpp \
    src/System/WriteEntry.cpp \
    src/System/ErrorStack.cpp \
    src/Text/Tables.cpp \
    src/Text/String.cpp \
    src/Structures/WildPokemonTable.cpp \
    src/Structures/WildPokemonSubTable.cpp \
    src/Structures/WildPokemonArea.cpp \
    src/Entities/MapScriptTable.cpp \
    src/Entities/ConnectionTable.cpp \
    src/Entities/EventTable.cpp \
    src/Graphics/Tileset.cpp \
    src/Graphics/TilesetManager.cpp \
    src/Mapping/MapBorder.cpp \
    src/Mapping/MapHeader.cpp \
    src/Mapping/MapNameTable.cpp \
    src/Mapping/Map.cpp \
    src/Widgets/qhexedit.cpp \
    src/Widgets/chunks.cpp \
    src/Widgets/commands.cpp \
    src/Mapping/MapBank.cpp \
    src/Widgets/QHexLineEdit.cpp \
    src/Widgets/QHexSpinBox.cpp \
    src/Mapping/MapBankTable.cpp \
    src/System/Configuration.cpp \
    src/System/LoadedData.cpp \
    src/Widgets/Misc/Messages.cpp \
    src/Widgets/OpenGL/AMEMapView.cpp \
    src/Widgets/OpenGL/AMEBlockView.cpp \
    src/Widgets/AMEImageButton.cpp \
    src/Forms/ErrorWindow.cpp \
    src/Widgets/OpenGL/AMEBorderView.cpp \
    src/Structures/PokemonTable.cpp \
    src/Graphics/OverworldTable.cpp \
    src/Widgets/OpenGL/AMEEntityView.cpp \
    src/Graphics/PropertyTable.cpp \
    src/System/Settings.cpp \
    src/Entities/EntityRawData.cpp \
    src/Mapping/MapLayoutTable.cpp \
    src/Widgets/QFilterChildrenProxyModel.cpp \
    src/Structures/ItemTable.cpp \
    src/Forms/SettingsDialog.cpp \
    src/Forms/TilesetDialog.cpp \
    src/Algorithm/PaletteQuantizer.cpp


#
# Header Files
#
HEADERS  += \
    include/AME/Forms/MainWindow.h \
    include/AME/Structures/WildPokemonTable.hpp \
    include/AME/System/ErrorStack.hpp \
    include/AME/System/WriteEntry.hpp \
    include/AME/System/IUndoable.hpp \
    include/AME/Text/String.hpp \
    include/AME/Text/Tables.hpp \
    include/AME/Structures/WildPokemonSubTable.hpp \
    include/AME/Structures/StructureErrors.hpp \
    include/AME/Structures/WildPokemonArea.hpp \
    include/AME/Structures/WildPokemonEncounter.hpp \
    include/AME/Entities/Warp.hpp \
    include/AME/Entities/Sign.hpp \
    include/AME/Entities/EntityTypes.hpp \
    include/AME/Entities/Trigger.hpp \
    include/AME/Entities/Npc.hpp \
    include/AME/Entities/Connection.hpp \
    include/AME/Entities/MapScript.hpp \
    include/AME/Entities/AutoScript.hpp \
    include/AME/Entities/Tables/EntityErrors.hpp \
    include/AME/Entities/Tables/MapScriptTable.hpp \
    include/AME/Entities/Tables/ConnectionTable.hpp \
    include/AME/Entities/Tables/EventTable.hpp \
    include/AME/Graphics/Tileset.hpp \
    include/AME/Graphics/Tile.hpp \
    include/AME/Graphics/Block.hpp \
    include/AME/Graphics/GraphicsErrors.hpp \
    include/AME/Mapping/MapBlock.hpp \
    include/AME/Mapping/MapHeader.hpp \
    include/AME/Graphics/TilesetManager.hpp \
    include/AME/Mapping/MapBorder.hpp \
    include/AME/Mapping/MappingErrors.hpp \
    include/AME/Mapping/Map.hpp \
    include/AME/Mapping/MapBank.hpp \
    include/AME/Widgets/qhexedit.h \
    include/AME/Widgets/chunks.h \
    include/AME/Widgets/commands.h \
    include/AME/Widgets/QHexLineEdit.h \
    include/AME/Widgets/QHexSpinBox.h \
    include/AME/Mapping/MapBankTable.hpp \
    include/AME/Mapping/MapNameTable.hpp \
    include/AME/System/Configuration.hpp \
    include/AME/System/LoadedData.hpp \
    include/AME/Widgets/Misc/Messages.hpp \
    include/AME/Widgets/OpenGL/AMEMapView.h \
    include/AME/Widgets/OpenGL/AMEBlockView.h \
    include/AME/Widgets/AMEImageButton.h \
    include/AME/Forms/ErrorWindow.h \
    include/AME/Widgets/OpenGL/AMEBorderView.h \
    include/AME/Structures/PokemonTable.hpp \
    include/AME/Graphics/OverworldTable.hpp \
    include/AME/Widgets/OpenGL/AMEEntityView.h \
    include/AME/Graphics/PropertyTable.hpp \
    include/AME/Graphics/Property.hpp \
    include/AME/System/Settings.hpp \
    include/AME/Mapping/MapName.hpp \
    include/AME/Mapping/MapLayoutTable.hpp \
    include/AME/Widgets/QFilterChildrenProxyModel.h \
    include/AME/Structures/ItemTable.hpp \
    include/AME/Forms/SettingsDialog.h \
    include/AME/Forms/TilesetDialog.h \
    include/AME/Algorithm/PaletteQuantizer.hpp


#
# Forms
#
FORMS    += resources/UI/MainWindow.ui \
    resources/UI/ErrorWindow.ui \
    resources/UI/SettingsDialog.ui \
    resources/UI/TilesetDialog.ui

#
# Resources
#
RESOURCES += \
    resources/images.qrc \
    resources/shaders.qrc

DISTFILES += \
    resources/config/BPRE.yaml \
    resources/shaders/NormalFragmentShader \
    resources/shaders/PrimitiveVertexShader.glsl \
    resources/shaders/PrimitiveFragmentShader.glsl
