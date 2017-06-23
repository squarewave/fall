#if VERTEX_SHADER

void main(void) {
  vec4 world_p = vec4(world_coords.xyz, 1.0);

  vec4 view_position = view_transform * world_p;

  gl_Position = vec4(view_position.xyz, 1.0);

  frag_color = color;
}

#endif

#if FRAGMENT_SHADER
void main(void) {

#if DEPTH_PEELING
  float depth = texelFetch(depth_sampler, ivec2(gl_FragCoord.xy), 0).r;
  float z = gl_FragCoord.z;
  if(z <= depth) {
    discard;
  }
#endif

  out_color = frag_color;

  if(out_color.a <= 0.01) {
    discard;
  }
}

#endif
