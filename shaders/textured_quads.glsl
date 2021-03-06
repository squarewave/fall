#if VERTEX_SHADER

void main(void) {
  vec4 world_p = vec4(world_coords.xyz, 1.0);

  vec4 view_position = view_transform * world_p;

  gl_Position = vec4(view_position.xyz, 1.0);

  frag_uv = uv_coords.xy;
  frag_color = color;
}

#endif

#if FRAGMENT_SHADER
uniform sampler2D texture_sampler;

void main(void) {
#if DEPTH_PEELING
  float depth = texelFetch(depth_sampler, ivec2(gl_FragCoord.xy), 0).r;
  float z = gl_FragCoord.z;
  if(z <= depth) {
    discard;
  }
#endif

  vec4 texture_sample = texture(texture_sampler, frag_uv);

  out_color = frag_color * texture_sample;

  if(out_color.a <= 0.01) {
    discard;
  }
}

#endif
