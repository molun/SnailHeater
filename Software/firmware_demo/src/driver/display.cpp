#include "display.h"
#include "network.h"
#include "lvgl.h"
#include "lv_port_indev.h"
// #include "lv_demo_encoder.h"
#include "common.h"

#define CONFIG_SCREEN_HOR_RES 280
#define CONFIG_SCREEN_VER_RES 240
#define CONFIG_SCREEN_BUFFER_SIZE (CONFIG_SCREEN_HOR_RES * CONFIG_SCREEN_VER_RES / 12)

#define DISP_HOR_RES CONFIG_SCREEN_HOR_RES
#define DISP_VER_RES CONFIG_SCREEN_VER_RES
#define DISP_BUF_SIZE CONFIG_SCREEN_BUFFER_SIZE

// static lv_color_t lv_full_disp_buf[DISP_BUF_SIZE];
// lv_color_t *lv_disp_buf_p;
static lv_color_t lv_disp_buf_p[DISP_BUF_SIZE];

static lv_disp_draw_buf_t disp_buf;
static lv_disp_drv_t disp_drv;

// void my_print(lv_log_level_t level, const char *file, uint32_t line, const char *fun, const char *dsc)
// {
//     Serial.printf("%s(%s)@%d->%s\r\n", file, fun, line, dsc);
//     Serial.flush();
// }

void my_print(const char *buf)
{
    Serial0.printf(buf);
    Serial0.flush();
}

void disp_flush_cb(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft->startWrite();
    tft->setAddrWindow(area->x1, area->y1, w, h);
    // tft->writePixels(&color_p->full, w * h);
    tft->pushColors((uint16_t *)(&color_p->full), w * h, true); //
    tft->endWrite();
    // Initiate DMA - blocking only if last DMA is not complete
    // tft->pushImageDMA(area->x1, area->y1, w, h, bitmap, &color_p->full);

    lv_disp_flush_ready(disp);
}

static void disp_wait_cb(lv_disp_drv_t *disp_drv)
{
    //    __wfi();
}

void Display::init()
{
#ifdef USER_PWM_CTRL_SRCEEN
    ledcSetup(LCD_BL_PWM_CHANNEL, PWM_GROUP_3_FREQ, PWM_GROUP_3_BIT_WIDTH);
    ledcAttachPin(BL_PWM_PIN, LCD_BL_PWM_CHANNEL);
#else
    pinMode(BL_PWM_PIN, OUTPUT);
#endif

    lv_init();

    setBackLight(0); // 设置亮度 为了先不显示初始化时的"花屏"

    tft->begin(); /* TFT init */
    tft->fillScreen(TFT_BLACK);
    tft->writecommand(ST7789_DISPON); // Display on
    // tft->fillScreen(BLACK);

    // 以下setRotation函数是经过更改的第4位兼容原版 高四位设置镜像
    // 正常方向需要设置为0 如果加上分光棱镜需要镜像改为4 如果是侧显示的需要设置为5
    // tft->setRotation(g_cfg.extern_info.rotation); /* mirror 修改反转，如果加上分光棱镜需要改为4镜像*/
    tft->setRotation(1);

    // setBackLight(g_cfg.extern_info.backLight); // 设置亮度
    setBackLight(100); // 设置亮度

    // lv_log_register_print_cb(
    //     reinterpret_cast<lv_log_print_g_cb_t>(my_print)); /* register print function for debugging */

    // lv_log_register_print_cb(my_print); /* register print function for debugging */

    /* Move the malloc process to Init() to make sure that the largest heap can be used for this buffer.
     *
    lv_disp_buf_p = static_cast<lv_color_t*>(malloc(DISP_BUF_SIZE * sizeof(lv_color_t)));
    if (lv_disp_buf_p == nullptr)
        LV_LOG_WARN("lv_port_disp_init malloc failed!\n");
    */

    lv_disp_draw_buf_init(&disp_buf, lv_disp_buf_p, nullptr, DISP_BUF_SIZE);

    /*Initialize the display*/
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = DISP_HOR_RES;
    disp_drv.ver_res = DISP_VER_RES;
    disp_drv.flush_cb = disp_flush_cb;
    disp_drv.wait_cb = disp_wait_cb;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.user_data = tft;
    lv_disp_drv_register(&disp_drv);
}

void Display::routine()
{
    lv_task_handler();
    delay(5);
}

void Display::setBackLight(int duty)
{
#ifdef USER_PWM_CTRL_SRCEEN
    duty = constrain(duty, 0, 100);
    ledcWrite(LCD_BL_PWM_CHANNEL, (int)(duty * PWM_GROUP_3_DUTY));
#else
    if (0 == duty)
    {
        digitalWrite(BL_PWM_PIN, LOW);
    }
    else
    {
        digitalWrite(BL_PWM_PIN, HIGH);
    }
#endif
}
