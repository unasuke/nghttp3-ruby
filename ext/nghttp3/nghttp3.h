#ifndef NGHTTP3_RUBY_H
#define NGHTTP3_RUBY_H 1

#include "ruby.h"
#include <nghttp3/nghttp3.h>

extern VALUE rb_mNghttp3;
extern VALUE rb_cNghttp3Info;
extern VALUE rb_eNghttp3Error;

/* Error classes */
extern VALUE rb_eNghttp3InvalidArgumentError;
extern VALUE rb_eNghttp3InvalidStateError;
extern VALUE rb_eNghttp3WouldBlockError;
extern VALUE rb_eNghttp3StreamInUseError;
extern VALUE rb_eNghttp3MalformedHTTPHeaderError;
extern VALUE rb_eNghttp3MalformedHTTPMessagingError;
extern VALUE rb_eNghttp3QPACKFatalError;
extern VALUE rb_eNghttp3QPACKHeaderTooLargeError;
extern VALUE rb_eNghttp3StreamNotFoundError;
extern VALUE rb_eNghttp3ConnClosingError;
extern VALUE rb_eNghttp3StreamDataOverflowError;
extern VALUE rb_eNghttp3FatalError;
extern VALUE rb_eNghttp3NoMemError;
extern VALUE rb_eNghttp3CallbackFailureError;

/* Data structure classes */
extern VALUE rb_cNghttp3Settings;
extern VALUE rb_cNghttp3NV;
extern VALUE rb_cNghttp3Connection;
extern VALUE rb_cNghttp3Callbacks;

/* QPACK module and classes */
extern VALUE rb_mNghttp3QPACK;
extern VALUE rb_cNghttp3QPACKEncoder;
extern VALUE rb_cNghttp3QPACKDecoder;

/* TypedData type for Settings (used by Connection) */
extern const rb_data_type_t settings_data_type;

/* TypedData type for Callbacks */
extern const rb_data_type_t callbacks_data_type;

/* Helper functions */
VALUE nghttp3_rb_error_class_for_code(int error_code);
void nghttp3_rb_raise(int error_code, const char *fmt, ...);

/* Settings helper */
nghttp3_settings *nghttp3_rb_get_settings(VALUE rb_settings);

/* NV helper */
nghttp3_nv nghttp3_rb_nv_to_c(VALUE rb_nv);

/* Connection helper */
nghttp3_conn *nghttp3_rb_get_conn(VALUE rb_conn);

/* Callbacks helper */
VALUE nghttp3_rb_get_callbacks(VALUE rb_conn);
void nghttp3_rb_setup_callbacks(nghttp3_callbacks *callbacks);

/* Init functions */
void Init_nghttp3_settings(void);
void Init_nghttp3_nv(void);
void Init_nghttp3_connection(void);
void Init_nghttp3_callbacks(void);
void Init_nghttp3_qpack(void);

#endif /* NGHTTP3_RUBY_H */
