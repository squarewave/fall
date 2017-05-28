#if VERTEX_SHADER

void main(void) {
  vec3 world_p = vec3(world_coords.xy, 1.0);

  vec3 view_position = view_transform * world_p;

  gl_Position = vec4(view_position.xy, 0, 1.0);

  frag_uv = uv_coords.xy;
  frag_color = color;
}

#endif

#if FRAGMENT_SHADER
uniform sampler2D texture_sampler;

void main(void) {
  vec4 texture_sample = texture(texture_sampler, frag_uv);

  out_color = frag_color * texture_sample;

  if(out_color.a <= 0) {
    discard;
  }
}

#endif
