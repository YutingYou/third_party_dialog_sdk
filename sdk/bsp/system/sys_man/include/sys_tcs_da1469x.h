/**
 * \addtogroup MID_SYS_SERVICES
 * \{
 * \addtogroup SYS_TCS_HANDLER TCS Handler
 *
 * \brief TCS Handler
 *
 * \{
 */

/**
****************************************************************************************
*
* @file sys_tcs.h
*
* @brief TCS Handler header file.
*
* Copyright (C) 2015-2019 Dialog Semiconductor.
* This computer program includes Confidential, Proprietary Information
* of Dialog Semiconductor. All Rights Reserved.
*
****************************************************************************************
*/

#ifndef SYS_TCS_H_
#define SYS_TCS_H_

#include "sdk_defs.h"

#define CS_MAX_SIZE (256 * 4) //256 entries of 4 bytes

/**
 * \enum SYS_TCS_TYPE
 * \brief custom TCS value type.
 *
 */
typedef enum {
        SYS_TCS_TYPE_TRIM_VAL = 0,      /**< trimmed value */
        SYS_TCS_TYPE_REG_PAIR,          /**< register pair value */
        SYS_TCS_TYPE_RESERVED           /**< reserved */
} SYS_TCS_TYPE;

/**
 * \enum SYS_TCS_DOMAIN
 * \brief power domain of each TCS group.
 *
 */
typedef enum {
        SYS_TCS_DOMAIN_PD_SYS = 0,      /**< System power domain */
        SYS_TCS_DOMAIN_PD_COM,          /**< Communication power domain */
        SYS_TCS_DOMAIN_PD_MEM,          /**< Memory power domain */
        SYS_TCS_DOMAIN_PD_TMR,          /**< Timers power domain */
        SYS_TCS_DOMAIN_PD_PER,          /**< Peripherals power domain */
        SYS_TCS_DOMAIN_PD_RAD,          /**< Radio power domain */
        SYS_TCS_DOMAIN_PD_SYNTH,        /**< Synth power domain */
        SYS_TCS_DOMAIN_NA               /**< No power domain */
} SYS_TCS_DOMAIN;

/**
 * \enum SYS_TCS_GID
 * \brief the configured group ids.
 *
 */
typedef enum {
        SYS_TCS_GROUP_RESERVED = 0,             /**< reserved group id*/
        SYS_TCS_GROUP_PD_SYS,                   /**< PD_SYS group id = 1*/
        SYS_TCS_GROUP_PD_COMM,                  /**< PD_COMM group id = 2*/
        SYS_TCS_GROUP_PD_MEM,                   /**< PD_MEM group id = 3*/
        SYS_TCS_GROUP_PD_TMR,                   /**< PD_TMR group id = 4*/
        SYS_TCS_GROUP_PD_PER,                   /**< PD_PER group id = 5*/
        SYS_TCS_GROUP_PD_RAD,                   /**< PD_RAD group id = 6*/
        SYS_TCS_GROUP_PD_SYNTH,                 /**< PD_SYNTH group id = 7*/
        SYS_TCS_GROUP_PD_RAD_MODE1,             /**< PD_RAD_MODE1 group id = 8*/
        SYS_TCS_GROUP_PD_SYNTH_MODE1,           /**< PD_SYNTH_MODE1 group id = 9*/
        SYS_TCS_GROUP_PD_RAD_MODE2,             /**< PD_RAD_MODE2 group id = 10*/
        SYS_TCS_GROUP_PD_SYNTH_MODE2,           /**< PD_SYNTH_MODE2 group id = 11*/
        SYS_TCS_GROUP_PROD_INFO,                /**< PROD_INFO group id = 12*/
        SYS_TCS_GROUP_CHIP_ID,                  /**< CHIP_ID group id = 13*/
        SYS_TCS_GROUP_BD_ADDR,                  /**< BD_ADDR group id = 14*/
        SYS_TCS_GROUP_SD_ADC_SINGLE_MODE,       /**< SD_ADC_SINGLE_MODE group id = 15*/
        SYS_TCS_GROUP_SD_ADC_DIFF_MODE,         /**< SD_ADC_DIFF_MODE group id = 16*/
        SYS_TCS_GROUP_GP_ADC_SINGLE_MODE,       /**< GP_ADC_SINGLE_MODE group id = 17*/
        SYS_TCS_GROUP_GP_ADC_DIFF_MODE,         /**< GP_ADC_DIFF_MODE group id = 18*/
        SYS_TCS_GROUP_MAX                       /**< Not valid group id just marks the maximum number = 19*/
} SYS_TCS_GID;

/*
 * Reset values of trimmed registers
 */
#define DEFAULT_CHARGER_TEST_CTRL_REG   0x0F81
#define DEFAULT_PMU_TRIM_REG            0x7700

/**
 * \brief TCS custom trim values callback
 *
 * \param [in] values_group the TCS group id custom trim values belong to
 * \param [in] user_data user specific data
 * \param [in] values custom trim values
 * \param [in] size the number of the custom trim values
 *
 */
typedef void (*sys_tcs_custom_values_cb)( SYS_TCS_GID values_group , void *user_data, uint32_t *values, uint8_t size);

/**
 * \brief Get gain and offset correction values used by SDADC LLD
 *
 * \param[in]  mode indicates differential or single ended
 * \param[out] gain_corr pointer reference to gain correction value
 * \param[out] offs_corr pointer reference to offset correction value
 *
 */
void hw_sdadc_get_gain_offs_corr_cal_val (uint8_t mode, int16_t *gain_corr, int16_t *offs_corr);

/**
 * \struct sys_tcs_attr_t
 * \brief attributes per custom value group id
 *
 */
typedef struct {
        SYS_TCS_TYPE value_type;        /**< TCS entry type */
        SYS_TCS_DOMAIN power_domain;    /**< TCS entry power domain */
        uint8_t start;                  /**< TCS entry start position  */
        uint8_t size;                   /**< TCS entry type size in words */
} sys_tcs_attr_t;


/**
 * \brief retrieve the TCS values from CS located in OTP or flash and then store
 * TCS register pair address, value  and/or custom value custom_trim_value in the Global TCS array
 *
 */
void sys_tcs_get_trim_values_from_cs(void);

/**
 * \brief get the number of the register pair values or custom values per gid
 *
 * \param [in] gid the TCS group id
 *
 * \return the number of trim values per group id
 */
uint8_t sys_tcs_get_size(SYS_TCS_GID gid);

/**
 * \brief get value type, register pair values or custom values per gid
 *
 * \param [in] gid the TCS group id
 *
 * \return the type of trim value per group id
 */
SYS_TCS_TYPE sys_tcs_get_value_type(SYS_TCS_GID gid);

/**
 * \brief get the custom_trim_values per gid
 *
 * \param [in] gid the TCS group id of the requested custom trim values
 * \param [out] values the pointer to the start of the custom trim values
 * \param [out] size the number of the custom trim values
 *
 * \warning if size is zero then there are no custom trim values for this gid,
 *  values points to invalid data.
 *
 */
void sys_tcs_get_custom_values(SYS_TCS_GID gid, uint32_t **values, uint8_t *size);

/**
 * \brief handles the custom_trim_values per gid according to callback
 *
 * \param [in] gid the TCS group id of custom trim values to apply
 * \param [in] cb the callback that applies the custom trim values
 * \param [in] user_data the argument to callback function
 *
 * \warning callback is called only if custom trim values are configured
 */
void sys_tcs_apply_custom_values(SYS_TCS_GID gid, sys_tcs_custom_values_cb cb, void *user_data);

/**
 * \brief Get register value pairs contained in a group id of the TCS array
 * \param [in] gid the group id
 * \param [out] values the pointer to the start of the register pair values
 * \param [out] size the number of the register pair values
 *
 * \warning if size is zero then values is not a valid pointer.
 */
void sys_tcs_get_reg_pairs(SYS_TCS_GID gid, uint32_t **values, uint8_t *size);

/**
 * \brief Apply the register value pairs contained in a group id of the TCS array.
 * \param [in] gid the group id
 *
 */
__RETAINED_CODE void sys_tcs_apply_reg_pairs(SYS_TCS_GID gid);

/**
 * \brief Apply the register value pairs belonging to a power domain
 * Handle the custom trim values of many groups which belong to the same power domain.
 * \param [in] domain the power domain
 * \param [in] cb the callback that handles the custom trim values
 * \param [in] user_data the argument to callback function
 *
 * \warning callback will not be called if there are no custom trim values for this power domain
 *
 * \see sys_tcs_custom_values_cb
 */
void sys_tcs_apply_entries(SYS_TCS_DOMAIN domain, sys_tcs_custom_values_cb cb,  void *user_data);

/**
 * \brief Retrieve the configured group ids belong to a specific power domain
 *
 * \param [in] domain the power domain
 * \param [out] gids pointer to the array which will filled with the group ids.
 * \param [in] size the length of the gids
 * The last valid entry in the array is before SYS_TCS_GROUP_RESERVED which marks the end.
 *
 *
 */
void sys_tcs_get_gid_per_domain(SYS_TCS_DOMAIN domain, uint8_t *gids, uint8_t size);

/**
 * \brief Get register value pairs contained in a group id of the TCS array
 * \param [in] gid the group id
 *
 * \return the pointer to the array containing these values or this group id
 */
uint32_t *sys_tcs_snc_get_reg_pair(SYS_TCS_GID gid);

/**
 * \brief Get the number of register-value pairs for this specific gid
 * \param [in] gid the group id
 *
 * \return the number of register-value entries of a given gid
 */
uint32_t sys_tcs_snc_get_reg_pair_num_of_entries(SYS_TCS_GID gid);

/**
 * \brief handles the custom_trim_values per gid according to callback
 *
 * \param [in] gid the TCS group id of custom trim values to apply
 * \param [in] user_data the argument to callback function
 * \param [in] val pointer to the returned values
 * \param [in] len size of returned values (bytes)
 *
 * \warning callback is called only if custom trim values are configured
 */
void sys_tcs_custom_values_system_cb (SYS_TCS_GID gid, void *user_data, uint32_t *val, uint8_t len);

/**
 * \brief check if the register addresses included in reg_address are configured in CS
 * \param [in] reg_address pointer to array containing the register addresses
 * \param [in] num number of register addresses
 * \param [out] trimmed_reg pointer to array containing information whether the corresponding
 * register is included in CS or not
 *
 * \return true if all register addresses are included in CS
 *
 * \warning search register values in OTP only, OTP should be already enabled.
 */
bool sys_tcs_reg_pairs_in_cs(const uint32_t* reg_address, uint8_t num, bool *trimmed_reg);

#endif /* SYS_TCS_H_ */
/**
\}
\}
*/
