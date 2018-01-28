/*
Copyright (c) 2012, Broadcom Europe Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// Video deocode demo using OpenMAX IL though the ilcient helper library

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bcm_host.h"
#include "ilclient.h"

static COMPONENT_T *video_render = NULL;

// ######### OVERSCAN PART ###########
static int current_overscan = 0;

static void set_overscan(int overscan) {
	OMX_CONFIG_DISPLAYREGIONTYPE disp_teg_type;
	int ret;

	memset(&disp_teg_type, 0, sizeof(OMX_CONFIG_DISPLAYREGIONTYPE));
	disp_teg_type.nSize = sizeof(OMX_CONFIG_DISPLAYREGIONTYPE);
	disp_teg_type.nVersion.nVersion = OMX_VERSION;
	disp_teg_type.nPortIndex = 90;
	disp_teg_type.fullscreen = 0;
	disp_teg_type.set = OMX_DISPLAY_SET_FULLSCREEN | OMX_DISPLAY_SET_DEST_RECT;
	disp_teg_type.dest_rect.x_offset = overscan;
	disp_teg_type.dest_rect.y_offset = overscan;
	disp_teg_type.dest_rect.width = 1920 - overscan*2;
	disp_teg_type.dest_rect.height = 1080 - overscan*2;

	ret = OMX_SetParameter(ILC_GET_HANDLE(video_render), OMX_IndexConfigDisplayRegion, &disp_teg_type);
	if (ret != OMX_ErrorNone) {
	        fprintf(stderr, "Set Ret: %x\n", ret);
	}
}

static void change_overscan(int dir) {
	current_overscan += dir;
	set_overscan(current_overscan);
}

// ######### CEC PART ###########
static unsigned char image_view_on[] = { 0x04 };
static unsigned char active_source[] = { 0x82, 0xff, 0xff }; // Last two must be updated with physical address
static unsigned char allow_source[]  = { 0x8e, 0x00 };

struct CecMessage {
	unsigned length;
	unsigned char dest;
	const unsigned char *buffer;
} cec_commands[] = {
	{ sizeof(image_view_on), 0x0, image_view_on },
	{ sizeof(active_source), 0xf, active_source },
	{ sizeof(allow_source), 0x0, allow_source },
	{ 0, 0, NULL }

};

int next_cmd = 0;

static void send_next_cmd() {
	if (cec_commands[next_cmd].length == 0) return;

	vc_cec_send_message(cec_commands[next_cmd].dest, cec_commands[next_cmd].buffer, cec_commands[next_cmd].length, VCOS_FALSE);
	next_cmd++;
}

static void rpi_cec_callback(void *callback_data, uint32_t p0, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4)
{
	VC_CEC_NOTIFY_T reason = (VC_CEC_NOTIFY_T)CEC_CB_REASON(p0);
	unsigned button;

	switch (reason) {
		case VC_CEC_BUTTON_PRESSED:
		case VC_CEC_REMOTE_PRESSED:
		  button = (p1>>16)&0xff;
		  switch (button) {
			  case 1: // UP
				  change_overscan(-1);
				  break;
			  case 2: // DOWN
				  change_overscan(1);
				  break;
			  case 3: // LEFT
				  change_overscan(4);
				  break;
			  case 4: // RIGHT
				  change_overscan(-4);
				  break;
			  case 44: // EXIT
				  exit(2);
				  break;
			  default:
				  fprintf(stderr, "CEC: button %d\n", button);
				  break;
		  }
		  break;
		case VC_CEC_LOGICAL_ADDR:
		  send_next_cmd();
		  break;
		case VC_CEC_TX:
		  send_next_cmd();
		  break;
		default:
		  break;
	}
}

static void cec_init() {
	unsigned short physical_address;

	bcm_host_init();

	vc_cec_set_passive(VCOS_TRUE);

	vc_cec_register_callback(rpi_cec_callback, NULL);

	vc_cec_get_physical_address(&physical_address);

	// Update physical address in active source cec command buffer
	active_source[1] = physical_address >> 8;
	active_source[2] = physical_address & 0xff;

	vc_cec_set_logical_address(8, CEC_DeviceType_Playback, CEC_VENDOR_ID_BROADCOM);
}

// ############ VIDEO DECODING PART ############

int
video_decode(int stream, int overscan)
{
   OMX_VIDEO_PARAM_PORTFORMATTYPE format;
   OMX_TIME_CONFIG_CLOCKSTATETYPE cstate;
   COMPONENT_T *video_decode = NULL, *video_scheduler = NULL, *clock = NULL;
   COMPONENT_T *list[5];
   TUNNEL_T tunnel[4];
   ILCLIENT_T *client;
   int status = 0;
   unsigned int data_len = 0;

   bcm_host_init();

   cec_init();

   memset(list, 0, sizeof(list));
   memset(tunnel, 0, sizeof(tunnel));

   if((client = ilclient_init()) == NULL)
   {
      return -3;
   }

   if(OMX_Init() != OMX_ErrorNone)
   {
      ilclient_destroy(client);
      return -4;
   }

   // create video_decode
   if(ilclient_create_component(client, &video_decode, "video_decode", ILCLIENT_DISABLE_ALL_PORTS | ILCLIENT_ENABLE_INPUT_BUFFERS) != 0)
      status = -14;
   list[0] = video_decode;

   // create video_render
   if(status == 0 && ilclient_create_component(client, &video_render, "video_render", ILCLIENT_DISABLE_ALL_PORTS) != 0)
      status = -14;
   list[1] = video_render;

   // create clock
   if(status == 0 && ilclient_create_component(client, &clock, "clock", ILCLIENT_DISABLE_ALL_PORTS) != 0)
      status = -14;
   list[2] = clock;

   memset(&cstate, 0, sizeof(cstate));
   cstate.nSize = sizeof(cstate);
   cstate.nVersion.nVersion = OMX_VERSION;
   cstate.eState = OMX_TIME_ClockStateWaitingForStartTime;
   cstate.nWaitMask = 1;
   if(clock != NULL && OMX_SetParameter(ILC_GET_HANDLE(clock), OMX_IndexConfigTimeClockState, &cstate) != OMX_ErrorNone)
      status = -13;

   // create video_scheduler
   if(status == 0 && ilclient_create_component(client, &video_scheduler, "video_scheduler", ILCLIENT_DISABLE_ALL_PORTS) != 0)
      status = -14;
   list[3] = video_scheduler;

   set_tunnel(tunnel, video_decode, 131, video_scheduler, 10);
   set_tunnel(tunnel+1, video_scheduler, 11, video_render, 90);
   set_tunnel(tunnel+2, clock, 80, video_scheduler, 12);

   // setup clock tunnel first
   if(status == 0 && ilclient_setup_tunnel(tunnel+2, 0, 0) != 0)
      status = -15;
   else
      ilclient_change_component_state(clock, OMX_StateExecuting);

   if(status == 0)
      ilclient_change_component_state(video_decode, OMX_StateIdle);

   memset(&format, 0, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
   format.nSize = sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE);
   format.nVersion.nVersion = OMX_VERSION;
   format.nPortIndex = 130;
   format.eCompressionFormat = OMX_VIDEO_CodingAVC;

   if(status == 0 &&
      OMX_SetParameter(ILC_GET_HANDLE(video_decode), OMX_IndexParamVideoPortFormat, &format) == OMX_ErrorNone &&
      ilclient_enable_port_buffers(video_decode, 130, NULL, NULL, NULL) == 0)
   {
      OMX_BUFFERHEADERTYPE *buf;
      int port_settings_changed = 0;
      int first_packet = 1;
      struct timeval to;
      fd_set fdset;

      ilclient_change_component_state(video_decode, OMX_StateExecuting);

      while((buf = ilclient_get_input_buffer(video_decode, 130, 1)) != NULL)
      {
         // feed data and wait until we get port settings changed
         unsigned char *dest = buf->pBuffer;
	 int ret;

         FD_ZERO(&fdset);
         FD_SET(stream, &fdset);
         to.tv_sec = 10;
         to.tv_usec = 0;

	 ret = select(stream+1, &fdset, NULL, NULL, &to);
	 if (ret == 0) {
		 status = -8;
		 break; // Timeout
	 }

         data_len = read(stream, dest, buf->nAllocLen-data_len);
	 if (data_len <= 0) {
	     status = -9;
             break;
	 }

         if(port_settings_changed == 0 &&
            ((data_len > 0 && ilclient_remove_event(video_decode, OMX_EventPortSettingsChanged, 131, 0, 0, 1) == 0) ||
             (data_len == 0 && ilclient_wait_for_event(video_decode, OMX_EventPortSettingsChanged, 131, 0, 0, 1,
                                                       ILCLIENT_EVENT_ERROR | ILCLIENT_PARAMETER_CHANGED, 10000) == 0)))
         {
            port_settings_changed = 1;

            if(ilclient_setup_tunnel(tunnel, 0, 0) != 0) {
               status = -7;
               break;
            }

            ilclient_change_component_state(video_scheduler, OMX_StateExecuting);

            // now setup tunnel to video_render
            if(ilclient_setup_tunnel(tunnel+1, 0, 1000) != 0)
            {
               status = -12;
               break;
            }

            ilclient_change_component_state(video_render, OMX_StateExecuting);

	    if (overscan) {
		   set_overscan(overscan);
	    }
         }
         if(!data_len) {
	    status = -10;
            break;
	 }

         buf->nFilledLen = data_len;
         data_len = 0;

         buf->nOffset = 0;
         if(first_packet)
         {
            buf->nFlags = OMX_BUFFERFLAG_STARTTIME;
            first_packet = 0;
         }
         else
            buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN;

         if(OMX_EmptyThisBuffer(ILC_GET_HANDLE(video_decode), buf) != OMX_ErrorNone)
         {
            status = -6;
            break;
         }
      }

      buf->nFilledLen = 0;
      buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN | OMX_BUFFERFLAG_EOS;

      if(OMX_EmptyThisBuffer(ILC_GET_HANDLE(video_decode), buf) != OMX_ErrorNone)
         status = -20;

      // wait for EOS from render
      ilclient_wait_for_event(video_render, OMX_EventBufferFlag, 90, 0, OMX_BUFFERFLAG_EOS, 0,
                              ILCLIENT_BUFFER_FLAG_EOS, -1);

      // need to flush the renderer to allow video_decode to disable its input port
      ilclient_flush_tunnels(tunnel, 0);

   }

   ilclient_disable_tunnel(tunnel);
   ilclient_disable_tunnel(tunnel+1);
   ilclient_disable_tunnel(tunnel+2);
   ilclient_disable_port_buffers(video_decode, 130, NULL, NULL, NULL);
   ilclient_teardown_tunnels(tunnel);

   ilclient_state_transition(list, OMX_StateIdle);
   ilclient_state_transition(list, OMX_StateLoaded);

   ilclient_cleanup_components(list);

   OMX_Deinit();

   ilclient_destroy(client);
   return status;
}
