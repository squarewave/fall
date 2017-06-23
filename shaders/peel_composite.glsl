#if VERTEX_SHADER

void main(void) {
  gl_Position = vec4(world_coords.xyz, 1);
  frag_uv = uv_coords.xy;
  frag_color = color;
}

#endif

#if FRAGMENT_SHADER
uniform sampler2D peel_sampler0;
uniform sampler2D peel_sampler1;
uniform sampler2D peel_sampler2;
uniform sampler2D peel_sampler3;

void main(void) {
  vec4 p0 = texture(peel_sampler0, frag_uv);
  vec4 p1 = texture(peel_sampler1, frag_uv);
  vec4 p2 = texture(peel_sampler2, frag_uv);
  vec4 p3 = texture(peel_sampler3, frag_uv);

  out_color.rgb = p3.rgb * p3.a;
  out_color.rgb = p2.rgb * p2.a + (1 - p2.a)*out_color.rgb;
  out_color.rgb = p1.rgb * p1.a + (1 - p1.a)*out_color.rgb;
  out_color.rgb = p0.rgb * p0.a + (1 - p0.a)*out_color.rgb;
}

#endif
