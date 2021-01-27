/**
****************************************************************************************
*
* @file rwip.h
*
* @brief RW IP SW main module
*
* Copyright (C) RivieraWaves 2009-2014
 *
 * Copyright (c) 2015-2018 Modified by Dialog Semiconductor
*
*
****************************************************************************************
*/


#ifndef _RWIP_H_
#define _RWIP_H_



/**
 ****************************************************************************************
 * @brief Function called when packet transmission/reception is finished.
 *
 * @param[in]  status ok if action correctly performed, else reason status code.
 *****************************************************************************************
 */
typedef void (*rwip_eif_callback) (uint8_t);

/**
 * Transport layer communication interface.
 */
struct rwip_eif_api
{
    /**
     *************************************************************************************
     * @brief Starts a data reception.
     *
     * @param[out] bufptr      Pointer to the RX buffer
     * @param[in]  size        Size of the expected reception
     * @param[in]  callback    Pointer to the function called back when transfer finished
     *************************************************************************************
     */
    void (*read) (uint8_t *bufptr, uint32_t size, rwip_eif_callback callback);

    /**
     *************************************************************************************
     * @brief Starts a data transmission.
     *
     * @param[in]  bufptr      Pointer to the TX buffer
     * @param[in]  size        Size of the transmission
     * @param[in]  callback    Pointer to the function called back when transfer finished
     *************************************************************************************
     */
    void (*write)(uint8_t *bufptr, uint32_t size, rwip_eif_callback callback);

    /**
     *************************************************************************************
     * @brief Enable Interface flow.
     *************************************************************************************
     */
    void (*flow_on)(void);

    /**
     *************************************************************************************
     * @brief Disable Interface flow.
     *
     * @return True if flow has been disabled, False else.
     *************************************************************************************
     */
    bool (*flow_off)(void);
};


#endif // _RWIP_H_
