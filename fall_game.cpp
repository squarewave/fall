// TODO:

#define TYPEINFO_FILE "generated_typeinfo.h"

#include "xxhash.c"

#include "game.cpp"
#include "render_commands.cpp"
#include "asset_manager.cpp"
#include "meat_space.cpp"
#include "serialize.cpp"
#include "tools.cpp"

#ifdef FALL_INTERNAL
#include "debug.cpp"
#include "asset_editor.cpp"
#include "entity_editor.cpp"
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#define RND_IMPLEMENTATION
#include "rnd.h"

#include "imgui_extensions.cpp"
#include "imgui/imgui.cpp"
#include "imgui/imgui_draw.cpp"
