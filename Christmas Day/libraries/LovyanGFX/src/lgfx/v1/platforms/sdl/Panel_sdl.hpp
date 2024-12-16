/*----------------------------------------------------------------------------/
  Lovyan GFX - Graphics library for embedded devices.

Original Source:
 https://github.com/lovyan03/LovyanGFX/

Licence:
 [FreeBSD](https://github.com/lovyan03/LovyanGFX/blob/master/license.txt)

Author:
 [lovyan03](https://twitter.com/lovyan03)

Contributors:
 [ciniml](https://github.com/ciniml)
 [mongonta0716](https://github.com/mongonta0716)
 [tobozo](https://github.com/tobozo)

Porting for SDL:
 [imliubo](https://github.com/imliubo)
/----------------------------------------------------------------------------*/
#pragma once

#include "common.hpp"
#if defined (SDL_h_)
#include "../../panel/Panel_FrameBufferBase.hpp"
#include "../../misc/range.hpp"
#include "../../Touch.hpp"

namespace lgfx
{
 inline namespace v1
 {
  struct Panel_sdl;
  struct monitor_t
  {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* texture = nullptr;
    Panel_sdl* panel = nullptr;
    int scaling_x = 1;
    int scaling_y = 1;
    int touch_x, touch_y;
    bool touched = false;
    bool closing = false;
  };
//----------------------------------------------------------------------------

  struct Touch_sdl : public ITouch
  {
    bool init(void) override { return true; }
    void wakeup(void) override {}
    void sleep(void) override {}
    bool isEnable(void) override { return true; };
    uint_fast8_t getTouchRaw(touch_point_t* tp, uint_fast8_t count) override { return 0; }
  };

//----------------------------------------------------------------------------

  struct Panel_sdl : public Panel_FrameBufferBase
  {
  public:
    Panel_sdl(void);
    virtual ~Panel_sdl(void);

    bool init(bool use_reset) override;

    color_depth_t setColorDepth(color_depth_t depth) override;

    void display(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h) override;

    // void setInvert(bool invert) override {}
    void drawPixelPreclipped(uint_fast16_t x, uint_fast16_t y, uint32_t rawcolor) override;
    void writeFillRectPreclipped(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, uint32_t rawcolor) override;
    void writeBlock(uint32_t rawcolor, uint32_t length) override;
    void writeImage(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, pixelcopy_t* param, bool use_dma) override;
    void writeImageARGB(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, pixelcopy_t* param) override;
    void writePixels(pixelcopy_t* param, uint32_t len, bool use_dma) override;

    uint_fast8_t getTouchRaw(touch_point_t* tp, uint_fast8_t count) override;

    void setWindowTitle(const char* title);
    void setScaling(uint_fast8_t scaling_x, uint_fast8_t scaling_y);

    static int setup(void);
    static int loop(void);
    static int close(void);

    static int main(int(*fn)(bool*), uint32_t msec_step_exec = 512);

  protected:
    const char* _window_title = "LGFX Simulator";
    SDL_mutex *_sdl_mutex = nullptr;

    void sdl_create(monitor_t * m);
    void sdl_update(void);

    touch_point_t _touch_point;
    monitor_t monitor;

    rgb888_t* _texturebuf = nullptr;
    uint_fast16_t _modified_counter;
    uint_fast16_t _texupdate_counter;
    uint_fast16_t _display_counter;
    bool _invalidated;

    static void _event_proc(void);
    static void _update_proc(void);
    void sdl_invalidate(void) { _invalidated = true; }
    bool initFrameBuffer(size_t width, size_t height);
    void deinitFrameBuffer(void);

    struct lock_t {
      lock_t(Panel_sdl* parent);
      ~lock_t();
    protected:
      Panel_sdl* _parent;
    };
  };

//----------------------------------------------------------------------------
 }
}
#endif