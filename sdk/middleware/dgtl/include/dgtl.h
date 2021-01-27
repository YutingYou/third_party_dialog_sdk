/**
 * \addtogroup MID_INT_DGTL
 * \{
 * \addtogroup DGTL_DEFINES Definitions
 *
 * \brief DGTL Declarations
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file dgtl.h
 *
 * @brief Declarations for DGTL
 *
 * Copyright (C) 2016-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef DGTL_H_
#define DGTL_H_

#include <stdbool.h>
#include <stdint.h>
#include "dgtl_msg.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief DGTL queue type
 */
typedef enum {
        DGTL_QUEUE_HCI,
        DGTL_QUEUE_APP,
        DGTL_QUEUE_LOG,
} dgtl_queue_t;

/**
 * \brief Callback called once tx is completed
 *
 * This callback is called when DGTL message was sent using dgtl_send_ex() function.
 *
 * \param [in] user_data  data passed by the user
 *
 */
typedef void (*dgtl_sent_cb_t)(void *user_data);

/**
 * Callback for application specific HCI commands
 *
 * This function is called by DGTL when HCI command from application specific opcode range is
 * received. It is called when \p DGTL_APP_SPECIFIC_HCI_ENABLE option is non-zero. Application shall
 * define this function in order to override weak reference defined by DGTL code internally.
 *
 * \warning
 * Application is responsible for freeing \p msg when no longer needed.
 *
 * \param [in] msg  DGTL message
 *
 */
void dgtl_app_specific_hci_cb(const dgtl_msg_t *msg);

/**
 * Initialize DGTL
 *
 * This function initializes internal DGTL structures and thus shall be called by the application
 * before using any other DGTL API.
 *
 */
void dgtl_init(void);

/**
 * \brief Register current task for given queue
 *
 * This function allows the calling task to register itself as a client to receive messages from
 * a given queue. The DGTL interface will notify application task using \p notif bitmask whenever
 * a new message is available in the queue, which can be received using dgtl_receive().
 *
 * \note
 * Only one task can be registered to get notifications for a queue.
 *
 * \param [in] queue  queue type
 * \param [in] notif  notification bitmask to be used
 *
 */
void dgtl_register(dgtl_queue_t queue, uint32_t notif);

/**
 * \brief Send message to the DGTL interface
 *
 * This function sends a message to the DGTL interface. The target queue is automatically selected
 * based on the packet type indicator present in the message. If callback is set, it will be called
 * when tx is completed.
 *
 * \note
 * After calling this function, the sender is no longer the owner of the message and should not use
 * it anymore.
 *
 * \param [in] msg        DGTL message
 * \param [in] cb         callback called once tx is completed
 * \param [in] user_data  user data passed to callback
 *
 * \return true if message has been put to TX queue, otherwise false.
 *
 */
bool dgtl_send_ex(dgtl_msg_t *msg, dgtl_sent_cb_t cb, void *user_data);

/**
 * \brief Send message to the DGTL interface
 *
 * This function sends a message to the DGTL interface. The target queue is automatically selected
 * based on the packet type indicator present in the message.
 *
 * After calling this function, the sender is no longer the owner of the message and should not use
 * it anymore.
 *
 * \param [in] msg  DGTL message
 *
 */
void dgtl_send(dgtl_msg_t *msg);

/**
 * \brief Receive message from the DGTL interface
 *
 * This function receives a message from a specified queue from the DGTL interface. The application
 * can only receive messages from the queue it previously registered to using dgtl_register().
 *
 * The receiver becomes owner of the message and shall free it when it is no longer in use.
 *
 * \param [in] queue  queue to receive message from
 *
 * \return received message or NULL if either no message is present in queue or the application is
 *         not allowed to receive message from queue
 *
 */
dgtl_msg_t *dgtl_receive(dgtl_queue_t queue);


/**
 * \brief Wake up handler for dgtl task
 *
 * Start data receiving in DGTL task. It should be called after closing the DGTL with dgtl_close().
 *
 * \note It may be called from both ISR or non ISR context
 *
 */
void dgtl_wkup_handler(void);

/**
 * \brief Send signal to the DGTL task for closing transport
 *
 * It will inform the DGTL task to stop exchanging data. In order to reopen transport,
 * dgtl_wkup_handler() function should be called. This function is blocking and waits until
 * transport has been closed.
 *
 */
void dgtl_close(void);

#ifdef __cplusplus
}
#endif

#endif /* DGTL_H_ */

/**
 * \}
 * \}
 */
