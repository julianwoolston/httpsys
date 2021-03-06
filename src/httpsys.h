#pragma once

#ifndef __HTTPSYS_H
#define __HTTPSYS_H

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

// TODO: implement httpsys_resume
#include <nan.h>
#include <SDKDDKVer.h>
#include <node.h>
#include <node_buffer.h>
#include <v8.h>
#include <uv.h>
#include <http.h>

using namespace v8;

#pragma comment(lib, "httpapi.lib")
#pragma comment(lib, "crypt32.lib")

#define Log(format, ...)															\
	memset(logBuf, 0, 1023);														\
	_snprintf(logBuf, 1023, format, __VA_ARGS__);									\
	for (int i = 0; i < 1024; i++) { if (logBuf[i] == '%') { logBuf[i] = ' '; } }	\
	if (debugOut) { fprintf(stderr, logBuf); }

#define ErrorIf(expr, hresult)    \
    if (expr)                     \
    {                             \
        hr = hresult;             \
        goto Error;               \
    }

#define CheckError(hresult)       \
    {                             \
        HRESULT tmp_hr = hresult; \
        if (S_OK != tmp_hr)       \
        {                         \
            hr = tmp_hr;          \
            goto Error;           \
        }                         \
    }

// RtlTimeToSecondsSince1970 declaration

typedef BOOLEAN(WINAPI *RtlTimeToSecondsSince1970Func)(PLARGE_INTEGER, PULONG);

// Wrapper of the uv_prepare_t associated with an active server

struct uv_httpsys_s;

typedef struct uv_httpsys_server_s {
	uv_prepare_t uv_prepare;
	HTTP_SERVER_SESSION_ID sessionId;
	HTTP_URL_GROUP_ID groupId;
	HANDLE requestQueue;
	unsigned int readsToInitialize;
	int refCount;
	BOOL closing;
	Nan::Persistent<v8::Object> event;
} uv_httpsys_server_t;

// Wrapper of the uv_async_t with HTTP.SYS specific data

typedef struct uv_httpsys_s {
	uv_async_t* uv_async;
	HTTP_REQUEST_ID requestId;
	HTTP_RESPONSE response;
	void* buffer;
	unsigned int bufferSize;
	HTTP_DATA_CHUNK chunk;
	int lastChunkSent;
	BOOL responseStarted;
	BOOL disconnect;
	BOOL disconnectProcessed;
	BOOL closed;
    BOOL synchronous;
	BOOL synchronousWrite;
	int refCount;
	uv_httpsys_server_t* uv_httpsys_server;
	struct uv_httpsys_s* uv_httpsys_peer;
	Nan::Persistent<v8::Object> event;
} uv_httpsys_t;

// Types of events passed to the JavaScript callback from native

typedef enum {
	HTTPSYS_ERROR_INITIALIZING_REQUEST = 1,
	HTTPSYS_ERROR_NEW_REQUEST,
	HTTPSYS_NEW_REQUEST,
	HTTPSYS_ERROR_INITIALIZING_READ_REQUEST_BODY,
	HTTPSYS_END_REQUEST,
	HTTPSYS_ERROR_READ_REQUEST_BODY,
	HTTPSYS_REQUEST_BODY,
	HTTPSYS_WRITTEN,
	HTTPSYS_ERROR_WRITING,
	HTTPSYS_SERVER_CLOSED
} uv_httpsys_event_type;

#define HTTPSYS_HTTP_TRACE 99

// Utility functions

Handle<Object> httpsys_create_event(uv_httpsys_t* uv_httpsys, int eventType);
Handle<Object> httpsys_create_event(uv_httpsys_server_t* uv_httpsys_server, int eventType);
Handle<Value> httpsys_notify_error(uv_httpsys_t* uv_httpsys, uv_httpsys_event_type errorType, unsigned int code);
Handle<Value> httpsys_notify_error(uv_httpsys_server_t* uv_httpsys_server, uv_httpsys_event_type errorType, unsigned int code);
void httpsys_free_chunks(uv_httpsys_t* uv_httpsys);
void httpsys_free(uv_httpsys_t* uv_httpsys, BOOL error);
Handle<Value> httpsys_make_callback(Handle<Value> options);
HRESULT httpsys_initialize_body_chunks(Handle<Object> options, uv_httpsys_t* uv_httpsys, ULONG* flags);
HRESULT httpsys_uv_httpsys_init(uv_httpsys_t* uv_httpsys, uv_async_cb callback);
HRESULT httpsys_uv_httpsys_close(uv_httpsys_t* uv_httpsys);
void httpsys_close_uv_async_cb(uv_handle_t* uv_handle);
Handle<Object> httpsys_create_client_cert_info(PHTTP_SSL_CLIENT_CERT_INFO info);

// HTTP processing state machine actions and events

void httpsys_new_request_callback(uv_async_t* handle);
void httpsys_prepare_new_requests(uv_prepare_t* handle);
void httpsys_read_request_body_callback(uv_async_t* handle);
void httpsys_write_callback(uv_async_t* handle);

HRESULT httpsys_initiate_new_request(uv_httpsys_t* uv_httpsys);
HRESULT httpsys_read_request_body_loop(uv_httpsys_t* uv_httpsys);
HRESULT httpsys_initiate_read_request_body(uv_httpsys_t* uv_httpsys);

// Exports

void httpsys_init(const Nan::FunctionCallbackInfo<v8::Value>& info);
void httpsys_listen(const Nan::FunctionCallbackInfo<v8::Value>& info);
void httpsys_stop_listen(const Nan::FunctionCallbackInfo<v8::Value>& info);
void httpsys_resume(const Nan::FunctionCallbackInfo<v8::Value>& info);
void httpsys_write_headers(const Nan::FunctionCallbackInfo<v8::Value>& info);
void httpsys_write_body(const Nan::FunctionCallbackInfo<v8::Value>& info);

void init(v8::Local<v8::Object> target);

#endif
