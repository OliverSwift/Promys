#include <libcec/cecc.h>

#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "overscan.h"

int overscan;

static void keyPressHandler(void* clientData, const cec_keypress* key)
{
    (void)clientData;

    if (key->duration) {
	    switch (key->keycode) {
		    case 1: // Up
			    overscan_set(++overscan);
			    break;
		    case 2: // Down
			    if (overscan > 0)
				    overscan_set(--overscan);
			    break;
		    default:
			    break;
	    }
    }
}

static ICECCallbacks        g_callbacks = {
    .logMessage           = NULL,
    .keyPress             = keyPressHandler,
    .commandReceived      = NULL,
    .configurationChanged = NULL,
    .alert                = NULL,
    .menuStateChanged     = NULL,
    .sourceActivated      = NULL
};

static libcec_configuration g_config;
static libcec_connection_t g_iface;

int
cec_init() {
    overscan_init();

    overscan = overscan_get();
    
    libcec_clear_configuration(&g_config);
    g_config.clientVersion        = LIBCEC_VERSION_CURRENT;
    g_config.bActivateSource      = 1;
    g_config.callbacks            = &g_callbacks;
    snprintf(g_config.strDeviceName, sizeof(g_config.strDeviceName), "PROMYS");
    g_config.deviceTypes.types[0] = CEC_DEVICE_TYPE_PLAYBACK_DEVICE;
    
    // Initialization
    g_iface = libcec_initialise(&g_config);
    if (g_iface == NULL) {
        return -1;
    }
    
    libcec_init_video_standalone(g_iface);
    
    cec_adapter device;
    int8_t iDevicesFound;

    iDevicesFound = libcec_find_adapters(g_iface, &device, 1, NULL);
    if (iDevicesFound <= 0) {
        libcec_destroy(g_iface);
        return -2;
    }

    if (!libcec_open(g_iface, device.comm, 2000)) {
        libcec_destroy(g_iface);
        return 1;
    }

    //libcec_destroy(g_iface);

    return 0;
}
