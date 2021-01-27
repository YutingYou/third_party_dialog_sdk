/**
 * \addtogroup MID_SYS_ADAPTERS
 * \{
 *
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_xxx.c
 *
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

/*
 * This is a template source file for a controller adapter.
 *
 * */
#if dg_configXXX_ADAPTER
//#include <hw_xxx.h> //Replace with the header file of the respective Driver (which includes definitions of xxx_config, HW_XXX_ID)
#include <osal.h>
#include <hw_gpio.h>
#include <hw_sys.h>
#include <hw_dma.h>
#include <resmgmt.h>
#include <sys_bsr.h>
#include <ad_template.h>

/*
 * Definition of adapters data handle
 */
#define DECODE(h)               ((ad_xxx_data_t *)(handle))

/*
 * Data types definitions section
 */

/**
 *
 *
 */
typedef struct {
        const ad_xxx_controller_conf_t *conf;
        /**< Internal data */
        OS_TASK  owner; /**< The task which opened the controller */
        OS_EVENT event; /**< Semaphore for async calls  */
        OS_MUTEX busy;  /**< Semaphore for thread safety */
        /* More adapter-specific members can be added below */
} ad_xxx_data_t;

__RETAINED static ad_xxx_data_t xxx1_data;
__RETAINED static ad_xxx_data_t xxx2_data;

/*
 * Adapters mandatory function definition section
 *
 * This section includes the definitions of the functions
 * which are mandatory for all adapters
 */



void ad_xxx_init(void)
{
        /* Adapter internal initializations */
        OS_MUTEX_CREATE(xxx1_data.busy);
        OS_EVENT_CREATE(xxx1_data.event);

        OS_MUTEX_CREATE(xxx2_data.busy);
        OS_EVENT_CREATE(xxx2_data.event);
}

ad_xxx_handle_t ad_xxx_open(const ad_xxx_controller_conf_t *conf)
{
        if (conf->id == HW_XXX1) {
                RES_ID res_id = RES_ID_XXX1;
                int bsr_id = BSR_PERIPH_ID_XXX1;
        } else if (conf->id == HW_XXX2) {
                RES_ID res_id = RES_ID_XXX2;
                int bsr_id = BSR_PERIPH_ID_XXX2;
        } else {
                OS_ASSERT(0);
        }

        /* Check input validity*/

        OS_ASSERT(conf->drv);
        OS_ASSERT(conf->io);

        /* Acquire resources */

        sys_sw_bsr_acquire(SW_BSR_MASTER_SYSCPU, bsr_id);
        resource_acquire(RES_MASK(res_id), RES_WAIT_FOREVER);
        if (dma_channel >= 0) {
                resource_acquire(RES_MASK(RES_ID_DMA_CH0 + dma_channel) |
                        RES_MASK(RES_ID_DMA_CH0 + dma_channel + 1 ), RES_WAIT_FOREVER);
        }

        /* Update adapter data */

        /* Configure driver */
        }

        /* Release resources before returning NULL */
        sys_sw_bsr_release(SW_BSR_MASTER_SYSCPU, bsr_id);

        resource_release(RES_MASK(res_id));
        if (dma_channel >= 0) {
                resource_release(RES_MASK(RES_ID_DMA_CH0 + dma_channel) |
                        RES_MASK(RES_ID_DMA_CH0 + dma_channel + 1 ));
        }
        return NULL;
}

int ad_xxx_reconfig(ad_xxx_handle_t handle, const ad_xxx_driver_conf_t *conf)
{



        /* Configure driver using conf configuration*/


        return 0;
}

int ad_xxx_close(ad_xxx_handle_t handle)
{

        /* Abort any ongoing transactions */

        /* Release resources */
        sys_sw_bsr_release(SW_BSR_MASTER_SYSCPU,
                                   BSR_PERIPH_ID_XXX2);

        resource_release(RES_MASK(res_id));
        if (dma_channel >= 0) {
                resource_release(RES_MASK(RES_ID_DMA_CH0 + dma_channel) |
                        RES_MASK(RES_ID_DMA_CH0 + dma_channel + 1 ));
        }

        return 0;
}

int ad_xxx_io_config (HW_XXX_ID id, const ad_xxx_io_conf_t *io, AD_IO_CONF_STATE state)
{


        return 0;
}

/*
 * Adapter specific function prototypes section
 *
 * This section includes the definitions of the functions
 * which are specific to each adapter.
 * An example function implementation is shown below
 */

int ad_xxx_write(ad_xxx_handle_t handle, const uint8_t *wbuf, size_t wlen)
{

        /* Write call to driver. Block until write is completed */

        return 0;
}

int ad_xxx_write_async(ad_xxx_handle_t handle, const uint8_t *wbuf, size_t wlen, ad_xxx_user_cb cb,
        void *user_data)
{


        return 0;
}


int ad_xxx_read(ad_xxx_handle_t handle, uint8_t *rbuf, size_t rlen)
{


        return 0;
}


int ad_xxx_read_async(ad_xxx_handle_t handle, uint8_t *rbuf, size_t rlen, ad_xxx_user_cb cb,
        void *user_data)
{

        return 0;
}

#endif /* dg_configXXX_ADAPTER */

/**
 * \}
 * \}
 */
