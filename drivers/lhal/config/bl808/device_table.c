#include "bflb_core.h"
#include "bl808_memorymap.h"
#include "bl808_irq.h"

#define DMA_CHANNEL_OFFSET 0x100

#ifndef BFLB_BOOT2
struct bflb_device_s bl808_device_table[] = {
    { .name = BFLB_NAME_ADC0,
      .reg_base = AON_BASE,
#if defined(CPU_M0) || defined(CPU_LP)
      .irq_num = BL808_IRQ_GPADC_DMA,
#else
      .irq_num = 0xff,
#endif
      .idx = 0,
      .sub_idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_ADC,
      .user_data = NULL },
    { .name = BFLB_NAME_DAC0,
      .reg_base = GLB_BASE,
      .irq_num = 0xff,
      .idx = 0,
      .sub_idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_DAC,
      .user_data = NULL },
    { .name = BFLB_NAME_EF_CTRL,
      .reg_base = EF_CTRL_BASE,
      .irq_num = 0xff,
      .idx = 0,
      .sub_idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_EF_CTRL,
      .user_data = NULL },
    { .name = BFLB_NAME_GPIO,
      .reg_base = GLB_BASE,
#if defined(CPU_M0) || defined(CPU_LP)
      .irq_num = BL808_IRQ_GPIO_INT0,
#else
      .irq_num = 0xff,
#endif
      .idx = 0,
      .sub_idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_GPIO,
      .user_data = NULL },
    { .name = BFLB_NAME_UART0,
      .reg_base = UART0_BASE,
#if defined(CPU_M0) || defined(CPU_LP)
      .irq_num = BL808_IRQ_UART0,
#else
      .irq_num = 0xff,
#endif
      .idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_UART,
      .user_data = NULL },
    { .name = BFLB_NAME_UART1,
      .reg_base = UART1_BASE,
#if defined(CPU_M0) || defined(CPU_LP)
      .irq_num = BL808_IRQ_UART1,
#else
      .irq_num = 0xff,
#endif
      .idx = 1,
      .dev_type = BFLB_DEVICE_TYPE_UART,
      .user_data = NULL },
    { .name = BFLB_NAME_UART2,
      .reg_base = UART2_BASE,
#if defined(CPU_M0) || defined(CPU_LP)
      .irq_num = BL808_IRQ_UART2,
#else
      .irq_num = 0xff,
#endif
      .idx = 2,
      .dev_type = BFLB_DEVICE_TYPE_UART,
      .user_data = NULL },
    { .name = BFLB_NAME_UART3,
      .reg_base = UART3_BASE,
#if defined(CPU_M0) || defined(CPU_LP)
      .irq_num = 0xff,
#else
      .irq_num = BL808_IRQ_UART3,
#endif
      .idx = 3,
      .dev_type = BFLB_DEVICE_TYPE_UART,
      .user_data = NULL },
    { .name = BFLB_NAME_SPI0,
      .reg_base = SPI0_BASE,
#if defined(CPU_M0) || defined(CPU_LP)
      .irq_num = BL808_IRQ_SPI0,
#else
      .irq_num = 0xff,
#endif
      .idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_SPI,
      .user_data = NULL },
    { .name = BFLB_NAME_PWM_V2_PWM0,
      .reg_base = PWM_BASE,
#if defined(CPU_M0) || defined(CPU_LP)
      .irq_num = BL808_IRQ_PWM,
#else
      .irq_num = 0xff,
#endif
      .idx = 0,
      .sub_idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_PWM,
      .user_data = NULL },
    { .name = BFLB_NAME_DMA0_CH0,
      .reg_base = DMA0_BASE + 1 * DMA_CHANNEL_OFFSET,
#if defined(CPU_M0) || defined(CPU_LP)
      .irq_num = BL808_IRQ_DMA0_ALL,
#else
      .irq_num = 0xff,
#endif
      .idx = 0,
      .sub_idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_DMA,
      .user_data = NULL },
    { .name = BFLB_NAME_DMA0_CH1,
      .reg_base = DMA0_BASE + 2 * DMA_CHANNEL_OFFSET,
#if defined(CPU_M0) || defined(CPU_LP)
      .irq_num = BL808_IRQ_DMA0_ALL,
#else
      .irq_num = 0xff,
#endif
      .idx = 0,
      .sub_idx = 1,
      .dev_type = BFLB_DEVICE_TYPE_DMA,
      .user_data = NULL },
    { .name = BFLB_NAME_DMA0_CH2,
      .reg_base = DMA0_BASE + 3 * DMA_CHANNEL_OFFSET,
#if defined(CPU_M0) || defined(CPU_LP)
      .irq_num = BL808_IRQ_DMA0_ALL,
#else
      .irq_num = 0xff,
#endif
      .idx = 0,
      .sub_idx = 2,
      .dev_type = BFLB_DEVICE_TYPE_DMA,
      .user_data = NULL },
    { .name = BFLB_NAME_DMA0_CH3,
      .reg_base = DMA0_BASE + 4 * DMA_CHANNEL_OFFSET,
#if defined(CPU_M0) || defined(CPU_LP)
      .irq_num = BL808_IRQ_DMA0_ALL,
#else
      .irq_num = 0xff,
#endif
      .idx = 0,
      .sub_idx = 3,
      .dev_type = BFLB_DEVICE_TYPE_DMA,
      .user_data = NULL },
    { .name = BFLB_NAME_DMA0_CH4,
      .reg_base = DMA0_BASE + 5 * DMA_CHANNEL_OFFSET,
#if defined(CPU_M0) || defined(CPU_LP)
      .irq_num = BL808_IRQ_DMA0_ALL,
#else
      .irq_num = 0xff,
#endif
      .idx = 0,
      .sub_idx = 4,
      .dev_type = BFLB_DEVICE_TYPE_DMA,
      .user_data = NULL },
    { .name = BFLB_NAME_DMA0_CH5,
      .reg_base = DMA0_BASE + 6 * DMA_CHANNEL_OFFSET,
#if defined(CPU_M0) || defined(CPU_LP)
      .irq_num = BL808_IRQ_DMA0_ALL,
#else
      .irq_num = 0xff,
#endif
      .idx = 0,
      .sub_idx = 5,
      .dev_type = BFLB_DEVICE_TYPE_DMA,
      .user_data = NULL },
    { .name = BFLB_NAME_DMA0_CH6,
      .reg_base = DMA0_BASE + 7 * DMA_CHANNEL_OFFSET,
#if defined(CPU_M0) || defined(CPU_LP)
      .irq_num = BL808_IRQ_DMA0_ALL,
#else
      .irq_num = 0xff,
#endif
      .idx = 0,
      .sub_idx = 6,
      .dev_type = BFLB_DEVICE_TYPE_DMA,
      .user_data = NULL },
    { .name = BFLB_NAME_DMA0_CH7,
      .reg_base = DMA0_BASE + 8 * DMA_CHANNEL_OFFSET,
#if defined(CPU_M0) || defined(CPU_LP)
      .irq_num = BL808_IRQ_DMA0_ALL,
#else
      .irq_num = 0xff,
#endif
      .idx = 0,
      .sub_idx = 7,
      .dev_type = BFLB_DEVICE_TYPE_DMA,
      .user_data = NULL },
    { .name = BFLB_NAME_I2C0,
      .reg_base = I2C0_BASE,
#if defined(CPU_M0) || defined(CPU_LP)
      .irq_num = BL808_IRQ_I2C0,
#else
      .irq_num = 0xff,
#endif
      .idx = 0,
      .sub_idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_I2C,
      .user_data = NULL },
    { .name = BFLB_NAME_TIMER0,
      .reg_base = TIMER0_BASE,
#if defined(CPU_M0) || defined(CPU_LP)
      .irq_num = BL808_IRQ_TIMER0,
#else
      .irq_num = 0xff,
#endif
      .idx = 0,
      .sub_idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_TIMER,
      .user_data = NULL },
    { .name = BFLB_NAME_TIMER1,
      .reg_base = TIMER1_BASE,
#if defined(CPU_M0) || defined(CPU_LP)
      .irq_num = BL808_IRQ_TIMER1,
#else
      .irq_num = 0xff,
#endif
      .idx = 1,
      .sub_idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_TIMER,
      .user_data = NULL },
    { .name = BFLB_NAME_RTC,
      .reg_base = HBN_BASE,
#if defined(CPU_M0) || defined(CPU_LP)
      .irq_num = BL808_IRQ_HBN_OUT0,
#else
      .irq_num = 0xff,
#endif
      .idx = 0,
      .sub_idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_RTC,
      .user_data = NULL },
    { .name = BFLB_NAME_AES,
      .reg_base = SEC_ENG_BASE,
      .irq_num = 0xff,
      .idx = 0,
      .sub_idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_AES,
      .user_data = NULL },
    { .name = BFLB_NAME_SHA,
      .reg_base = SEC_ENG_BASE,
      .irq_num = 0xff,
      .idx = 0,
      .sub_idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_SHA,
      .user_data = NULL },
    { .name = BFLB_NAME_TRNG,
      .reg_base = SEC_ENG_BASE,
      .irq_num = 0xff,
      .idx = 0,
      .sub_idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_TRNG,
      .user_data = NULL },
    { .name = BFLB_NAME_PKA,
      .reg_base = SEC_ENG_BASE,
      .irq_num = 0xff,
      .idx = 0,
      .sub_idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_PKA,
      .user_data = NULL },
    { .name = BFLB_NAME_EMAC0,
      .reg_base = EMAC_BASE,
#if defined(CPU_M0) || defined(CPU_LP)
      .irq_num = BL808_IRQ_EMAC,
#else
      .irq_num = 0xff,
#endif
      .idx = 0,
      .sub_idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_ETH,
      .user_data = NULL },
    { .name = BFLB_NAME_IRTX,
      .reg_base = IR_BASE,
#if defined(CPU_M0) || defined(CPU_LP)
      .irq_num = BL808_IRQ_IRTX,
#else
      .irq_num = 0xff,
#endif
      .idx = 0,
      .sub_idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_IR,
      .user_data = NULL },
    { .name = BFLB_NAME_IRRX,
      .reg_base = IR_BASE,
#if defined(CPU_M0) || defined(CPU_LP)
      .irq_num = BL808_IRQ_IRRX,
#else
      .irq_num = 0xff,
#endif
      .idx = 0,
      .sub_idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_IR,
      .user_data = NULL },
    { .name = BFLB_NAME_CKS,
      .reg_base = CKS_BASE,
      .irq_num = 0,
      .idx = 0,
      .sub_idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_CKS,
      .user_data = NULL },
    { .name = BFLB_NAME_MJPEG,
      .reg_base = MJPEG_BASE,
#if defined(CPU_M0) || defined(CPU_LP)
      .irq_num = 0xff,
#else
      .irq_num = BL808_IRQ_MJPEG,
#endif
      .idx = 0,
      .sub_idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_MJPEG,
      .user_data = NULL },
    { .name = BFLB_NAME_WDT,
      .reg_base = TIMER0_BASE,
#if defined(CPU_M0) || defined(CPU_LP)
      .irq_num = BL808_IRQ_WDT,
#else
      .irq_num = 0xff,
#endif
      .idx = 0,
      .sub_idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_TIMER,
      .user_data = NULL },
    { .name = BFLB_NAME_CAM0,
      .reg_base = DVP0_BASE,
#if defined(CPU_D0)
      .irq_num = BL808_IRQ_DVP2BUS_INT0,
#else
      .irq_num = 0xff,
#endif
      .idx = 0,
      .sub_idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_CAMERA,
      .user_data = NULL },
    { .name = BFLB_NAME_CAM1,
      .reg_base = DVP1_BASE,
#if defined(CPU_D0)
      .irq_num = BL808_IRQ_DVP2BUS_INT1,
#else
      .irq_num = 0xff,
#endif
      .idx = 1,
      .sub_idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_CAMERA,
      .user_data = NULL },
    { .name = BFLB_NAME_CAM2,
      .reg_base = DVP2_BASE,
#if defined(CPU_D0)
      .irq_num = BL808_IRQ_DVP2BUS_INT2,
#else
      .irq_num = 0xff,
#endif
      .idx = 2,
      .sub_idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_CAMERA,
      .user_data = NULL },
    { .name = BFLB_NAME_CAM3,
      .reg_base = DVP3_BASE,
#if defined(CPU_D0)
      .irq_num = BL808_IRQ_DVP2BUS_INT3,
#else
      .irq_num = 0xff,
#endif
      .idx = 3,
      .sub_idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_CAMERA,
      .user_data = NULL },
    { .name = BFLB_NAME_CAM4,
      .reg_base = DVP4_BASE,
#if defined(CPU_D0)
      .irq_num = BL808_IRQ_DVP2BUS_INT4,
#else
      .irq_num = 0xff,
#endif
      .idx = 4,
      .sub_idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_CAMERA,
      .user_data = NULL },
    { .name = BFLB_NAME_CAM5,
      .reg_base = DVP5_BASE,
#if defined(CPU_D0)
      .irq_num = BL808_IRQ_DVP2BUS_INT5,
#else
      .irq_num = 0xff,
#endif
      .idx = 5,
      .sub_idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_CAMERA,
      .user_data = NULL },
    { .name = BFLB_NAME_CAM6,
      .reg_base = DVP6_BASE,
#if defined(CPU_D0)
      .irq_num = BL808_IRQ_DVP2BUS_INT6,
#else
      .irq_num = 0xff,
#endif
      .idx = 6,
      .sub_idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_CAMERA,
      .user_data = NULL },
    { .name = BFLB_NAME_CAM7,
      .reg_base = DVP7_BASE,
#if defined(CPU_D0)
      .irq_num = BL808_IRQ_DVP2BUS_INT7,
#else
      .irq_num = 0xff,
#endif
      .idx = 7,
      .sub_idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_CAMERA,
      .user_data = NULL },
    { .name = BFLB_NAME_CSI,
      .reg_base = CSI_BASE,
#if defined(CPU_D0)
      .irq_num = BL808_IRQ_MIPI_CSI,
#else
      .irq_num = 0xff,
#endif
      .idx = 0,
      .sub_idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_CSI,
      .user_data = NULL },
    { .name = BFLB_NAME_WO,
      .reg_base = GLB_BASE,
#if defined(CPU_M0) || defined(CPU_LP)
      .irq_num = BL808_IRQ_GPIO_DMA,
#else
      .irq_num = 0xff,
#endif
      .idx = 0,
      .sub_idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_WO,
      .user_data = NULL },
    { .name = BFLB_NAME_SDH,
      .reg_base = SDH_BASE,
#if defined(CPU_M0)
      .irq_num = BL808_IRQ_SDH,
#else
      .irq_num = 0xff,
#endif
      .idx = 0,
      .sub_idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_SDH,
      .user_data = NULL },
};
#else
struct bflb_device_s bl808_device_table[] = {
    { .name = BFLB_NAME_EF_CTRL,
      .reg_base = EF_CTRL_BASE,
      .irq_num = 0xff,
      .idx = 0,
      .sub_idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_EF_CTRL,
      .user_data = NULL },
    { .name = BFLB_NAME_GPIO,
      .reg_base = GLB_BASE,
      .irq_num = BL808_IRQ_GPIO_INT0,
      .idx = 0,
      .sub_idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_GPIO,
      .user_data = NULL },
    { .name = BFLB_NAME_UART0,
      .reg_base = UART0_BASE,
      .irq_num = BL808_IRQ_UART0,
      .idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_UART,
      .user_data = NULL },
    { .name = BFLB_NAME_UART1,
      .reg_base = UART1_BASE,
      .irq_num = BL808_IRQ_UART1,
      .idx = 1,
      .dev_type = BFLB_DEVICE_TYPE_UART,
      .user_data = NULL },
    { .name = BFLB_NAME_UART2,
      .reg_base = UART2_BASE,
      .irq_num = BL808_IRQ_UART2,
      .idx = 2,
      .dev_type = BFLB_DEVICE_TYPE_UART,
      .user_data = NULL },
      { .name = BFLB_NAME_TIMER0,
      .reg_base = TIMER0_BASE,
      .irq_num = BL808_IRQ_TIMER0,
      .idx = 0,
      .sub_idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_TIMER,
      .user_data = NULL },
    { .name = BFLB_NAME_TIMER1,
      .reg_base = TIMER1_BASE,
      .irq_num = BL808_IRQ_TIMER1,
      .idx = 1,
      .sub_idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_TIMER,
      .user_data = NULL },
    { .name = BFLB_NAME_AES,
      .reg_base = SEC_ENG_BASE,
      .irq_num = 0xff,
      .idx = 0,
      .sub_idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_AES,
      .user_data = NULL },
    { .name = BFLB_NAME_SHA,
      .reg_base = SEC_ENG_BASE,
      .irq_num = 0xff,
      .idx = 0,
      .sub_idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_SHA,
      .user_data = NULL },
    { .name = BFLB_NAME_TRNG,
      .reg_base = SEC_ENG_BASE,
      .irq_num = 0xff,
      .idx = 0,
      .sub_idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_TRNG,
      .user_data = NULL },
    { .name = BFLB_NAME_PKA,
      .reg_base = SEC_ENG_BASE,
      .irq_num = 0xff,
      .idx = 0,
      .sub_idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_PKA,
      .user_data = NULL },
    { .name = BFLB_NAME_WDT,
      .reg_base = TIMER0_BASE,
#if defined(CPU_M0) || defined(CPU_LP)
      .irq_num = BL808_IRQ_WDT,
#else
      .irq_num = 0xff,
#endif
      .idx = 0,
      .sub_idx = 0,
      .dev_type = BFLB_DEVICE_TYPE_TIMER,
      .user_data = NULL },
};
#endif

struct bflb_device_s *bflb_device_get_by_name(const char *name)
{
    for (uint8_t i = 0; i < sizeof(bl808_device_table) / sizeof(bl808_device_table[0]); i++) {
        if (strcmp(bl808_device_table[i].name, name) == 0) {
            return &bl808_device_table[i];
        }
    }
    LHAL_PARAM_ASSERT(0);
    return NULL;
}

void bflb_device_set_userdata(struct bflb_device_s *device, void *user_data)
{
    device->user_data = user_data;
}