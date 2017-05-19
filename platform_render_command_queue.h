#ifndef PLATFORM_RENDER_COMMAND_QUEUE_H__
#define PLATFORM_RENDER_COMMAND_QUEUE_H__

struct PlatformTextureHandle {

};

enum PlatformRenderCommandType {
  CommandType_TEXTURED_QUAD,
};

struct PlatformRenderCommand {
  PlatformRenderCommandType type;
  union {
    struct {
      PlatformTextureHandle texture;
      f32* view_transform;
    } textured_quad;
  };
};

struct PlatformRenderCommandQueue {
  i32 viewport_width;
  i32 viewport_height;


};

#endif /* end of include guard: PLATFORM_RENDER_COMMAND_QUEUE_H__ */
