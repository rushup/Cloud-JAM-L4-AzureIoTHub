// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef TLSIO_STM32Cube_H
#define TLSIO_STM32Cube_H

#ifdef __cplusplus
extern "C" {
#include <cstddef>
#else
#include <stddef.h>
#endif /* __cplusplus */

#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/umock_c_prod.h"

/** @brief	Return the tlsio table of functions.
*
* @param	void.
*
* @return	The tlsio interface (IO_INTERFACE_DESCRIPTION).
*/
MOCKABLE_FUNCTION(, const IO_INTERFACE_DESCRIPTION*, tlsio_STM32Cube_get_interface_description);


/** Expose tlsio state for test proposes.
*/
#define TLSIO_STM32Cube_STATE_VALUES  \
    TLSIO_STM32Cube_STATE_CLOSED,     \
    TLSIO_STM32Cube_STATE_OPENING,    \
    TLSIO_STM32Cube_STATE_OPEN,       \
    TLSIO_STM32Cube_STATE_CLOSING,    \
    TLSIO_STM32Cube_STATE_ERROR,      \
    TLSIO_STM32Cube_STATE_NULL
DEFINE_ENUM(TLSIO_STM32Cube_STATE, TLSIO_STM32Cube_STATE_VALUES);


/** @brief	Return the tlsio state for test proposes.
*
* @param	Unique handle that identifies the tlsio instance.
*
* @return	The tlsio state (TLSIO_STM32Cube_STATE).
*/
TLSIO_STM32Cube_STATE tlsio_STM32Cube_get_state(CONCRETE_IO_HANDLE tlsio_handle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TLSIO_STM32Cube_H */

