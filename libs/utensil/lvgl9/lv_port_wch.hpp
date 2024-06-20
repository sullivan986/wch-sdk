#ifndef LV_PORT_WCH_HPP
#define LV_PORT_WCH_HPP

#include "FreeRTOS.h"
#include "debug.h"
#include "lvgl.h"
#include "src/lv_api_map_v8.h"
#include "task.h"
#include "utensil_rtos.hpp"
#include <vector>

template <int32_t hor_res, int32_t ver_res> class lvgl_controller
{
  public:
    lvgl_controller(lv_display_flush_cb_t flush_cb)
    {
        buf_1.resize(hor_res * 10);
        buf_2.resize(hor_res * 10);

        lv_init();
        display = lv_display_create(hor_res, ver_res);
        lv_display_set_flush_cb(display, flush_cb);
        lv_display_set_buffers(display, buf_1.data(), buf_2.data(), buf_1.size(), LV_DISPLAY_RENDER_MODE_PARTIAL);
        lv_tick_set_cb((lv_tick_get_cb_t)xTaskGetTickCount);
    }

    void start()
    {
        xTaskCreate(lvgl_task, "lvgl_task", 4096, nullptr, 1, &lvgl_task_handler);
    }

  private:
    lv_display_t *display;
    TaskHandle_t lvgl_task_handler;
    TaskHandle_t lvgl_tick_handler;

    std::vector<lv_color_t> buf_1;
    std::vector<lv_color_t> buf_2;

    static void lvgl_task(void *pvParameters)
    {
        while (true)
        {
            uint32_t time_till_next = lv_task_handler();
            task_sleep_ms(time_till_next);
        }
    }
};

#endif