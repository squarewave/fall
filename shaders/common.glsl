
#if VERTEX_SHADER
uniform mat3 view_transform;
in vec2 world_coords;
in vec2 uv_coords;
in vec4 color;

out vec2 frag_uv;
out vec4 frag_color;
#endif
#if FRAGMENT_SHADER
out vec4 out_color;
in vec2 frag_uv;
in vec4 frag_color;
#endif
