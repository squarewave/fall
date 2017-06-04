#ifndef ASSET_DIRECTION
#define ASSET_DIRECTION(id)
#endif

#ifndef ASSET_MOVE_STATE
#define ASSET_MOVE_STATE(id)
#endif

#ifndef ASSET_CLASS
#define ASSET_CLASS(id)
#endif

#ifndef ASSET_COLOR
#define ASSET_COLOR(id)
#endif

#ifndef ASSET_TYPE
#define ASSET_TYPE(id)
#endif

#ifndef ENTITY_TYPE
#define ENTITY_TYPE(id)
#endif

ASSET_DIRECTION(unspecified)
ASSET_DIRECTION(left)
ASSET_DIRECTION(right)
ASSET_DIRECTION(forward)
ASSET_DIRECTION(backward)
ASSET_DIRECTION(horizontal)
ASSET_DIRECTION(vertical)
ASSET_DIRECTION(count)

ASSET_MOVE_STATE(unspecified)
ASSET_MOVE_STATE(standing)
ASSET_MOVE_STATE(running)
ASSET_MOVE_STATE(walking)
ASSET_MOVE_STATE(jumping)
ASSET_MOVE_STATE(falling)
ASSET_MOVE_STATE(count)

ASSET_CLASS(unspecified)
ASSET_CLASS(science)
ASSET_CLASS(count)

ASSET_COLOR(unspecified)
ASSET_COLOR(dark)
ASSET_COLOR(light)
ASSET_COLOR(caramel)
ASSET_COLOR(blue)
ASSET_COLOR(green)
ASSET_COLOR(red)
ASSET_COLOR(count)

ASSET_TYPE(unspecified)
ASSET_TYPE(crew)
ASSET_TYPE(selection_line)
ASSET_TYPE(selection_circle)
ASSET_TYPE(count)

ENTITY_TYPE(crew)

#undef ASSET_DIRECTION
#undef ASSET_MOVE_STATE
#undef ASSET_CLASS
#undef ASSET_COLOR
#undef ASSET_TYPE
#undef ENTITY_TYPE
