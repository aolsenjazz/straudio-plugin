/**
 * Copyright (c) 2019 Paul-Louis Ageneau
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef RTC_C_API
#define RTC_C_API

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#define RTC_EXPORT __declspec(dllexport)
#else
#define RTC_EXPORT
#endif

#ifndef RTC_ENABLE_MEDIA
#define RTC_ENABLE_MEDIA 1
#endif

#ifndef RTC_ENABLE_WEBSOCKET
#define RTC_ENABLE_WEBSOCKET 1
#endif

#include <stdint.h>

// libdatachannel C API

typedef enum {
	RTC_NEW = 0,
	RTC_CONNECTING = 1,
	RTC_CONNECTED = 2,
	RTC_DISCONNECTED = 3,
	RTC_FAILED = 4,
	RTC_CLOSED = 5
} rtcState;

typedef enum {
	RTC_GATHERING_NEW = 0,
	RTC_GATHERING_INPROGRESS = 1,
	RTC_GATHERING_COMPLETE = 2
} rtcGatheringState;

typedef enum { // Don't change, it must match plog severity
	RTC_LOG_NONE = 0,
	RTC_LOG_FATAL = 1,
	RTC_LOG_ERROR = 2,
	RTC_LOG_WARNING = 3,
	RTC_LOG_INFO = 4,
	RTC_LOG_DEBUG = 5,
	RTC_LOG_VERBOSE = 6
} rtcLogLevel;

#define RTC_ERR_SUCCESS 0
#define RTC_ERR_INVALID -1 // invalid argument
#define RTC_ERR_FAILURE -2 // runtime error

typedef struct {
	const char **iceServers;
	int iceServersCount;
	uint16_t portRangeBegin;
	uint16_t portRangeEnd;
} rtcConfiguration;

typedef void (*rtcLogCallbackFunc)(rtcLogLevel level, const char *message);
typedef void (*rtcDataChannelCallbackFunc)(int dc, void *ptr);
typedef void (*rtcDescriptionCallbackFunc)(const char *sdp, const char *type, void *ptr);
typedef void (*rtcCandidateCallbackFunc)(const char *cand, const char *mid, void *ptr);
typedef void (*rtcStateChangeCallbackFunc)(rtcState state, void *ptr);
typedef void (*rtcGatheringStateCallbackFunc)(rtcGatheringState state, void *ptr);
typedef void (*rtcOpenCallbackFunc)(void *ptr);
typedef void (*rtcClosedCallbackFunc)(void *ptr);
typedef void (*rtcErrorCallbackFunc)(const char *error, void *ptr);
typedef void (*rtcMessageCallbackFunc)(const char *message, int size, void *ptr);
typedef void (*rtcBufferedAmountLowCallbackFunc)(void *ptr);
typedef void (*rtcAvailableCallbackFunc)(void *ptr);

// Log
RTC_EXPORT void rtcInitLogger(rtcLogLevel level, rtcLogCallbackFunc cb); // NULL cb to log to stdout

// User pointer
RTC_EXPORT void rtcSetUserPointer(int id, void *ptr);

// PeerConnection
RTC_EXPORT int rtcCreatePeerConnection(const rtcConfiguration *config); // returns pc id
RTC_EXPORT int rtcDeletePeerConnection(int pc);

RTC_EXPORT int rtcSetDataChannelCallback(int pc, rtcDataChannelCallbackFunc cb);
RTC_EXPORT int rtcSetLocalDescriptionCallback(int pc, rtcDescriptionCallbackFunc cb);
RTC_EXPORT int rtcSetLocalCandidateCallback(int pc, rtcCandidateCallbackFunc cb);
RTC_EXPORT int rtcSetStateChangeCallback(int pc, rtcStateChangeCallbackFunc cb);
RTC_EXPORT int rtcSetGatheringStateChangeCallback(int pc, rtcGatheringStateCallbackFunc cb);

RTC_EXPORT int rtcSetRemoteDescription(int pc, const char *sdp, const char *type);
RTC_EXPORT int rtcAddRemoteCandidate(int pc, const char *cand, const char *mid);

RTC_EXPORT int rtcGetLocalAddress(int pc, char *buffer, int size);
RTC_EXPORT int rtcGetRemoteAddress(int pc, char *buffer, int size);

// DataChannel
RTC_EXPORT int rtcCreateDataChannel(int pc, const char *label); // returns dc id
RTC_EXPORT int rtcDeleteDataChannel(int dc);

RTC_EXPORT int rtcGetDataChannelLabel(int dc, char *buffer, int size);

// WebSocket
#if RTC_ENABLE_WEBSOCKET
RTC_EXPORT int rtcCreateWebSocket(const char *url); // returns ws id
RTC_EXPORT int rtcDeleteWebsocket(int ws);
#endif

// DataChannel and WebSocket common API
RTC_EXPORT int rtcSetOpenCallback(int id, rtcOpenCallbackFunc cb);
RTC_EXPORT int rtcSetClosedCallback(int id, rtcClosedCallbackFunc cb);
RTC_EXPORT int rtcSetErrorCallback(int id, rtcErrorCallbackFunc cb);
RTC_EXPORT int rtcSetMessageCallback(int id, rtcMessageCallbackFunc cb);
RTC_EXPORT int rtcSendMessage(int id, const char *data, int size);

RTC_EXPORT int rtcGetBufferedAmount(int id); // total size buffered to send
RTC_EXPORT int rtcSetBufferedAmountLowThreshold(int id, int amount);
RTC_EXPORT int rtcSetBufferedAmountLowCallback(int id, rtcBufferedAmountLowCallbackFunc cb);

// DataChannel and WebSocket common extended API
RTC_EXPORT int rtcGetAvailableAmount(int id); // total size available to receive
RTC_EXPORT int rtcSetAvailableCallback(int id, rtcAvailableCallbackFunc cb);
RTC_EXPORT int rtcReceiveMessage(int id, char *buffer, int *size);

// Optional preload and cleanup
RTC_EXPORT void rtcPreload(void);
RTC_EXPORT void rtcCleanup(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
