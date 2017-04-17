
#ifndef PRIORITY_SCHEDULER_H__
#define PRIORITY_SCHEDULER_H__

#include <stdint.h>
#include "app_error.h"
#include "app_util.h"

#define PRIORITY_SCHED_EVENT_HEADER_SIZE 8       /**< Size of priority_scheduler.event_header_t (only for use inside PRIORITY_SCHED_BUF_SIZE()). */

/**@brief Compute number of bytes required to hold the scheduler buffer.
 *
 * @param[in] EVENT_SIZE   Maximum size of events to be passed through the scheduler.
 * @param[in] QUEUE_SIZE   Number of entries in scheduler queue (i.e. the maximum number of events
 *                         that can be scheduled for execution).
 *
 * @return    Required scheduler buffer size (in bytes).
 */
#define PRIORITY_SCHED_BUF_SIZE(EVENT_SIZE, QUEUE_SIZE)                                                 \
            (((EVENT_SIZE) + PRIORITY_SCHED_EVENT_HEADER_SIZE) * ((QUEUE_SIZE) + 1))
            
/**@brief Scheduler event handler type. */
typedef void (*priority_sched_event_handler_t)(void * p_event_data, uint16_t event_size);

/**@brief Macro for initializing the event scheduler.
 *
 * @details It will also handle dimensioning and allocation of the memory buffer required by the
 *          scheduler, making sure the buffer is correctly aligned.
 *
 * @param[in] EVENT_SIZE   Maximum size of events to be passed through the scheduler.
 * @param[in] QUEUE_SIZE   Number of entries in scheduler queue (i.e. the maximum number of events
 *                         that can be scheduled for execution).
 *
 * @note Since this macro allocates a buffer, it must only be called once (it is OK to call it
 *       several times as long as it is from the same location, e.g. to do a reinitialization).
 */
#define PRIORITY_SCHED_INIT(EVENT_SIZE, QUEUE_SIZE)                                                     \
    do                                                                                             \
    {                                                                                              \
        static uint32_t PRIORITY_SCHED_BUF[CEIL_DIV(PRIORITY_SCHED_BUF_SIZE((EVENT_SIZE), (QUEUE_SIZE)),     \
                                               sizeof(uint32_t))];                                 \
        uint32_t ERR_CODE = priority_sched_init((EVENT_SIZE), (QUEUE_SIZE), PRIORITY_SCHED_BUF);             \
        APP_ERROR_CHECK(ERR_CODE);                                                                 \
    } while (0)

/**@brief Function for initializing the Scheduler.
 *
 * @details It must be called before entering the main loop.
 *
 * @param[in]   max_event_size   Maximum size of events to be passed through the scheduler.
 * @param[in]   queue_size       Number of entries in scheduler queue (i.e. the maximum number of
 *                               events that can be scheduled for execution).
 * @param[in]   p_evt_buffer   Pointer to memory buffer for holding the scheduler queue. It must
 *                               be dimensioned using the PRIORITY_SCHED_BUFFER_SIZE() macro. The buffer
 *                               must be aligned to a 4 byte boundary.
 *
 * @note Normally initialization should be done using the PRIORITY_SCHED_INIT() macro, as that will both
 *       allocate the scheduler buffer, and also align the buffer correctly.
 *
 * @retval      NRF_SUCCESS               Successful initialization.
 * @retval      NRF_ERROR_INVALID_PARAM   Invalid parameter (buffer not aligned to a 4 byte
 *                                        boundary).
 */
uint32_t priority_sched_init(uint16_t max_event_size, uint16_t queue_size, void * p_evt_buffer);

/**@brief Function for executing all scheduled events.
 *
 * @details This function must be called from within the main loop. It will execute all events
 *          scheduled since the last time it was called.
 */
void priority_sched_execute(void);

/**@brief Function for scheduling an event.
 *
 * @details Puts an event into the event queue.
 *
 * @param[in]   p_event_data   Pointer to event data to be scheduled.
 * @param[in]   event_size   Size of event data to be scheduled.
 * @param[in]   handler        Event handler to receive the event.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t priority_sched_event_put(void *                         p_event_data,
                                  uint16_t                       event_size,
                                  priority_sched_event_handler_t handler);

#ifdef PRIORITY_SCHEDULER_WITH_PAUSE
/**@brief A function to pause the scheduler.
 *
 * @details When the scheduler is paused events are not pulled from the scheduler queue for
 *          processing. The function can be called multiple times. To unblock the scheduler the
 *          function @ref priority_sched_resume has to be called the same number of times.
 */
void priority_sched_pause(void);

/**@brief A function to resume a scheduler.
 *
 * @details To unblock the scheduler this function has to be called the same number of times as
 *          @ref priority_sched_pause function.
 */
void priority_sched_resume(void);
#endif
#endif // PRIORITY_SCHEDULER_H__

/** @} */
