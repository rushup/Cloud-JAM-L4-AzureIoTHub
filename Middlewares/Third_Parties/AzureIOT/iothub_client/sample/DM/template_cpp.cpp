<!DOCTYPE HTML>
<!DOCTYPE html PUBLIC "" ""><HTML><HEAD>
<META http-equiv="Content-Type" content="text/html; charset=utf-8"></HEAD>
<BODY>
<PRE>// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include &lt;stdio.h&gt;
#include &lt;string.h&gt;
#include &lt;stdlib.h&gt;

#include "iothub_client_sample_mqtt_dm.h"



void device_get_serial_number(char *&amp;out)
{
	out = _strdup("SERIAL 12345");
}

void device_get_firmware_version(char *&amp;out)
{
	out = _strdup("FWVERSION 1.2");
}

FIRMWARE_UPDATE_STATE device_get_firmware_update_state(void)
{
	FIRMWARE_UPDATE_STATE rv = FIRMWARE_UPDATE_STATE_NONE;

	return rv;
}

bool device_reboot(void)
{
    bool ret = true;
    LogInfo("\n\t REBOOT\n");

    return ret;
}

bool device_factoryreset(void)
{
    bool ret = true;
    LogInfo("\n\t FACTORY RESET\n");

    return ret;
}

bool device_download_firmware(const char *uri)
{
    bool ret = true;
    LogInfo("Downloading [%s]", uri);

    return ret;
}

bool device_update_firmware(void)
{
    bool ret = true;
    LogInfo("\n\t FIRMWARE UPDATE\n");

    return ret;
}

bool device_run_service(void)
{
    LogInfo("Running as service");

	return (true);
}
</PRE></BODY></HTML>
