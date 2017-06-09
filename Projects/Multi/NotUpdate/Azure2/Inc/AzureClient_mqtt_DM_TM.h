// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _AZURE_CLIENT_MQTT_H
#define _AZURE_CLIENT_MQTT_H

#ifdef __cplusplus
extern "C" {
#endif
#include "azure_c_shared_utility/crt_abstractions.h"

/* enum values are in lower case per design */
#define FIRMWARE_UPDATE_STATUS_VALUES \
        waiting, \
        downloading, \
        downloadFailed, \
        downloadComplete, \
        applying, \
        applyFailed, \
        applyComplete
DEFINE_ENUM(FIRMWARE_UPDATE_STATUS, FIRMWARE_UPDATE_STATUS_VALUES)

extern FIRMWARE_UPDATE_STATUS UpdateStatus;
extern void AzureClient_mqtt_DM_TM(void);
extern void ReportState(FIRMWARE_UPDATE_STATUS status);

#ifdef __cplusplus
}
#endif

#endif /* _AZURE_CLIENT_MQTT_H */
