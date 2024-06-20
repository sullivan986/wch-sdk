#ifndef LV_PORT_WCH_HPP
#define LV_PORT_WCH_HPP

#include "FreeRTOS.h"
#include "debug.h"
#include "lvgl.h"
#include "src/hal/lv_hal_disp.h"
#include "task.h"
#include "utensil_rtos.hpp"
#include <vector>

typedef void (*this_port_flush_cb)(struct _lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);
typedef void (*this_port_indev_cb)(struct _lv_indev_drv_t *indev_drv, lv_indev_data_t *data);

template <int32_t hor_res, int32_t ver_res> class lvgl_controller
{
  public:
    lvgl_controller(this_port_flush_cb flush_cb, this_port_indev_cb indev_cb)
    {
        buf_1.resize(hor_res * 20);
        buf_2.resize(hor_res * 20);

        lv_init();
        lv_disp_draw_buf_init(&disp_buf, buf_1.data(), buf_2.data(), buf_1.size());

        lv_disp_drv_init(&disp_drv);
        disp_drv.hor_res = hor_res;
        disp_drv.ver_res = ver_res;
        disp_drv.flush_cb = flush_cb;
        disp_drv.draw_buf = &disp_buf;
        lv_disp_drv_register(&disp_drv);

        lv_indev_drv_init(&indev_drv);
        indev_drv.type = LV_INDEV_TYPE_POINTER;
        indev_drv.read_cb = indev_cb;
        my_indev = lv_indev_drv_register(&indev_drv);
    }

    void start()
    {
        xTaskCreate(lvgl_tick_task, "lvgl_task", 256, nullptr, 1, &lvgl_tick_handler);

        xTaskCreate(lvgl_task, "lvgl_task", 2048, nullptr, 1, &lvgl_task_handler);
    }

  private:
    TaskHandle_t lvgl_task_handler;
    TaskHandle_t lvgl_tick_handler;

    lv_disp_drv_t disp_drv;
    lv_disp_draw_buf_t disp_buf;
    std::vector<lv_color_t> buf_1;
    std::vector<lv_color_t> buf_2;

    lv_indev_drv_t indev_drv;
    lv_indev_t *my_indev;

    static void lvgl_tick_task(void *pvParameters)
    {
        while (true)
        {
            lv_tick_inc(10);
            task_sleep_ms(10);
        }
    }

    static void lvgl_task(void *pvParameters)
    {
        while (true)
        {
            lv_timer_handler();
            task_sleep_ms(1);
        }
    }
};

#endif