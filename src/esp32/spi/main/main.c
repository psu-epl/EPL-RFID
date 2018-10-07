#include "rc522.h"

void app_main(void)
{

    //ESP_LOGE(READER13, "ErrorReg %x ",read_reg(ErrorReg));

    uint8_t buffer[20];
    memset(&buffer, 0, 20);

    InitRc522();
    while (1)
    {
        if (PcdRequest(PICC_REQALL, &buffer) == TAG_OK)
        {
            ESP_LOGW(READER13, "buffer 0  [0]:%x [1]:%x [2]:%x [3]:%x [4]:%x", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]);
            ESP_LOGE(READER13, "ErrorReg %x ", PcdAnticoll(PICC_ANTICOLL1, &buffer));
            ESP_LOGI(READER13, "buffer 1  [0]:%x [1]:%x [2]:%x [3]:%x [4]:%x", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]);
        }
    }
}
