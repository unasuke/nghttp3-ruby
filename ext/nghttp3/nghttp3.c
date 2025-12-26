#include "nghttp3.h"
#include <stdarg.h>

VALUE rb_mNghttp3;
VALUE rb_cNghttp3Info;
VALUE rb_cNghttp3Settings;
VALUE rb_cNghttp3NV;
VALUE rb_eNghttp3Error;

/* Error classes */
VALUE rb_eNghttp3InvalidArgumentError;
VALUE rb_eNghttp3InvalidStateError;
VALUE rb_eNghttp3WouldBlockError;
VALUE rb_eNghttp3StreamInUseError;
VALUE rb_eNghttp3MalformedHTTPHeaderError;
VALUE rb_eNghttp3MalformedHTTPMessagingError;
VALUE rb_eNghttp3QPACKFatalError;
VALUE rb_eNghttp3QPACKHeaderTooLargeError;
VALUE rb_eNghttp3StreamNotFoundError;
VALUE rb_eNghttp3ConnClosingError;
VALUE rb_eNghttp3StreamDataOverflowError;
VALUE rb_eNghttp3FatalError;
VALUE rb_eNghttp3NoMemError;
VALUE rb_eNghttp3CallbackFailureError;

/*
 * call-seq:
 *   Nghttp3.library_version -> Nghttp3::Info
 *   Nghttp3.library_version(least_version) -> Nghttp3::Info or nil
 *
 * Returns version information about the nghttp3 library.
 * If least_version is specified and the library version is less than
 * that value, returns nil.
 */
static VALUE rb_nghttp3_library_version(int argc, VALUE *argv, VALUE self) {
  VALUE rb_least_version;
  int least_version = 0;
  const nghttp3_info *info;
  VALUE info_obj;

  rb_scan_args(argc, argv, "01", &rb_least_version);

  if (!NIL_P(rb_least_version)) {
    least_version = NUM2INT(rb_least_version);
  }

  info = nghttp3_version(least_version);

  if (info == NULL) {
    return Qnil;
  }

  info_obj = rb_obj_alloc(rb_cNghttp3Info);
  rb_iv_set(info_obj, "@age", INT2NUM(info->age));
  rb_iv_set(info_obj, "@version_num", INT2NUM(info->version_num));
  rb_iv_set(info_obj, "@version_str", rb_str_new_cstr(info->version_str));

  return info_obj;
}

/*
 * Returns the age of the nghttp3 library.
 */
static VALUE rb_nghttp3_info_age(VALUE self) { return rb_iv_get(self, "@age"); }

/*
 * Returns the numerical version of the nghttp3 library.
 */
static VALUE rb_nghttp3_info_version_num(VALUE self) {
  return rb_iv_get(self, "@version_num");
}

/*
 * Returns the version string of the nghttp3 library.
 */
static VALUE rb_nghttp3_info_version_str(VALUE self) {
  return rb_iv_get(self, "@version_str");
}

/*
 * Returns the appropriate Ruby error class for the given nghttp3 error code.
 */
VALUE nghttp3_rb_error_class_for_code(int error_code) {
  switch (error_code) {
  case NGHTTP3_ERR_INVALID_ARGUMENT:
    return rb_eNghttp3InvalidArgumentError;
  case NGHTTP3_ERR_INVALID_STATE:
    return rb_eNghttp3InvalidStateError;
  case NGHTTP3_ERR_WOULDBLOCK:
    return rb_eNghttp3WouldBlockError;
  case NGHTTP3_ERR_STREAM_IN_USE:
    return rb_eNghttp3StreamInUseError;
  case NGHTTP3_ERR_MALFORMED_HTTP_HEADER:
    return rb_eNghttp3MalformedHTTPHeaderError;
  case NGHTTP3_ERR_MALFORMED_HTTP_MESSAGING:
    return rb_eNghttp3MalformedHTTPMessagingError;
  case NGHTTP3_ERR_QPACK_FATAL:
    return rb_eNghttp3QPACKFatalError;
  case NGHTTP3_ERR_QPACK_HEADER_TOO_LARGE:
    return rb_eNghttp3QPACKHeaderTooLargeError;
  case NGHTTP3_ERR_STREAM_NOT_FOUND:
    return rb_eNghttp3StreamNotFoundError;
  case NGHTTP3_ERR_CONN_CLOSING:
    return rb_eNghttp3ConnClosingError;
  case NGHTTP3_ERR_STREAM_DATA_OVERFLOW:
    return rb_eNghttp3StreamDataOverflowError;
  case NGHTTP3_ERR_NOMEM:
    return rb_eNghttp3NoMemError;
  case NGHTTP3_ERR_CALLBACK_FAILURE:
    return rb_eNghttp3CallbackFailureError;
  default:
    if (nghttp3_err_is_fatal(error_code)) {
      return rb_eNghttp3FatalError;
    }
    return rb_eNghttp3Error;
  }
}

/*
 * Raises an appropriate Ruby exception for the given nghttp3 error code.
 */
void nghttp3_rb_raise(int error_code, const char *fmt, ...) {
  VALUE error_class;
  char buf[256];
  va_list args;

  error_class = nghttp3_rb_error_class_for_code(error_code);

  if (fmt != NULL) {
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    rb_raise(error_class, "%s: %s", buf, nghttp3_strerror(error_code));
  } else {
    rb_raise(error_class, "%s", nghttp3_strerror(error_code));
  }
}

/*
 * call-seq:
 *   Nghttp3.err_is_fatal?(error_code) -> true or false
 *
 * Returns true if the given error code represents a fatal error.
 */
static VALUE rb_nghttp3_err_is_fatal(VALUE self, VALUE rb_error_code) {
  int error_code = NUM2INT(rb_error_code);
  return nghttp3_err_is_fatal(error_code) ? Qtrue : Qfalse;
}

/*
 * call-seq:
 *   Nghttp3.strerror(error_code) -> String
 *
 * Returns a textual representation of the given error code.
 */
static VALUE rb_nghttp3_strerror(VALUE self, VALUE rb_error_code) {
  int error_code = NUM2INT(rb_error_code);
  const char *str = nghttp3_strerror(error_code);
  return rb_str_new_cstr(str);
}

/* ============================================================
 * Settings class
 * ============================================================ */

typedef struct {
  nghttp3_settings settings;
} SettingsObj;

static void settings_free(void *ptr) { xfree(ptr); }

static size_t settings_memsize(const void *ptr) { return sizeof(SettingsObj); }

static const rb_data_type_t settings_data_type = {
    .wrap_struct_name = "nghttp3_settings_rb",
    .function =
        {
            .dmark = NULL,
            .dfree = settings_free,
            .dsize = settings_memsize,
        },
    .flags = RUBY_TYPED_FREE_IMMEDIATELY,
};

static VALUE settings_alloc(VALUE klass) {
  SettingsObj *obj;
  VALUE self =
      TypedData_Make_Struct(klass, SettingsObj, &settings_data_type, obj);
  memset(&obj->settings, 0, sizeof(nghttp3_settings));
  return self;
}

/*
 * Returns a pointer to the underlying nghttp3_settings structure.
 */
nghttp3_settings *nghttp3_rb_get_settings(VALUE rb_settings) {
  SettingsObj *obj;
  TypedData_Get_Struct(rb_settings, SettingsObj, &settings_data_type, obj);
  return &obj->settings;
}

/*
 * call-seq:
 *   Settings.new -> Settings
 *
 * Creates a new Settings object with all values set to zero.
 */
static VALUE rb_nghttp3_settings_initialize(VALUE self) { return self; }

/*
 * call-seq:
 *   Settings.default -> Settings
 *
 * Creates a new Settings object with default values from nghttp3.
 */
static VALUE rb_nghttp3_settings_default(VALUE klass) {
  VALUE self = settings_alloc(klass);
  SettingsObj *obj;
  TypedData_Get_Struct(self, SettingsObj, &settings_data_type, obj);
  nghttp3_settings_default(&obj->settings);
  return self;
}

/* Settings getters */
static VALUE rb_nghttp3_settings_get_max_field_section_size(VALUE self) {
  SettingsObj *obj;
  TypedData_Get_Struct(self, SettingsObj, &settings_data_type, obj);
  return ULL2NUM(obj->settings.max_field_section_size);
}

static VALUE rb_nghttp3_settings_get_qpack_max_dtable_capacity(VALUE self) {
  SettingsObj *obj;
  TypedData_Get_Struct(self, SettingsObj, &settings_data_type, obj);
  return SIZET2NUM(obj->settings.qpack_max_dtable_capacity);
}

static VALUE
rb_nghttp3_settings_get_qpack_encoder_max_dtable_capacity(VALUE self) {
  SettingsObj *obj;
  TypedData_Get_Struct(self, SettingsObj, &settings_data_type, obj);
  return SIZET2NUM(obj->settings.qpack_encoder_max_dtable_capacity);
}

static VALUE rb_nghttp3_settings_get_qpack_blocked_streams(VALUE self) {
  SettingsObj *obj;
  TypedData_Get_Struct(self, SettingsObj, &settings_data_type, obj);
  return SIZET2NUM(obj->settings.qpack_blocked_streams);
}

static VALUE rb_nghttp3_settings_get_enable_connect_protocol(VALUE self) {
  SettingsObj *obj;
  TypedData_Get_Struct(self, SettingsObj, &settings_data_type, obj);
  return obj->settings.enable_connect_protocol ? Qtrue : Qfalse;
}

static VALUE rb_nghttp3_settings_get_h3_datagram(VALUE self) {
  SettingsObj *obj;
  TypedData_Get_Struct(self, SettingsObj, &settings_data_type, obj);
  return obj->settings.h3_datagram ? Qtrue : Qfalse;
}

/* Settings setters */
static VALUE rb_nghttp3_settings_set_max_field_section_size(VALUE self,
                                                            VALUE val) {
  SettingsObj *obj;
  TypedData_Get_Struct(self, SettingsObj, &settings_data_type, obj);
  obj->settings.max_field_section_size = NUM2ULL(val);
  return val;
}

static VALUE rb_nghttp3_settings_set_qpack_max_dtable_capacity(VALUE self,
                                                               VALUE val) {
  SettingsObj *obj;
  TypedData_Get_Struct(self, SettingsObj, &settings_data_type, obj);
  obj->settings.qpack_max_dtable_capacity = NUM2SIZET(val);
  return val;
}

static VALUE
rb_nghttp3_settings_set_qpack_encoder_max_dtable_capacity(VALUE self,
                                                          VALUE val) {
  SettingsObj *obj;
  TypedData_Get_Struct(self, SettingsObj, &settings_data_type, obj);
  obj->settings.qpack_encoder_max_dtable_capacity = NUM2SIZET(val);
  return val;
}

static VALUE rb_nghttp3_settings_set_qpack_blocked_streams(VALUE self,
                                                           VALUE val) {
  SettingsObj *obj;
  TypedData_Get_Struct(self, SettingsObj, &settings_data_type, obj);
  obj->settings.qpack_blocked_streams = NUM2SIZET(val);
  return val;
}

static VALUE rb_nghttp3_settings_set_enable_connect_protocol(VALUE self,
                                                             VALUE val) {
  SettingsObj *obj;
  TypedData_Get_Struct(self, SettingsObj, &settings_data_type, obj);
  obj->settings.enable_connect_protocol = RTEST(val) ? 1 : 0;
  return val;
}

static VALUE rb_nghttp3_settings_set_h3_datagram(VALUE self, VALUE val) {
  SettingsObj *obj;
  TypedData_Get_Struct(self, SettingsObj, &settings_data_type, obj);
  obj->settings.h3_datagram = RTEST(val) ? 1 : 0;
  return val;
}

/* ============================================================
 * NV class
 * ============================================================ */

/*
 * Converts a Ruby NV object to a C nghttp3_nv structure.
 * Note: The returned structure contains pointers to Ruby string data,
 * so the Ruby strings must remain valid while the structure is in use.
 */
nghttp3_nv nghttp3_rb_nv_to_c(VALUE rb_nv) {
  nghttp3_nv nv;
  VALUE name = rb_iv_get(rb_nv, "@name");
  VALUE value = rb_iv_get(rb_nv, "@value");
  VALUE flags = rb_iv_get(rb_nv, "@flags");

  StringValue(name);
  StringValue(value);

  nv.name = (uint8_t *)RSTRING_PTR(name);
  nv.namelen = RSTRING_LEN(name);
  nv.value = (uint8_t *)RSTRING_PTR(value);
  nv.valuelen = RSTRING_LEN(value);
  nv.flags = NIL_P(flags) ? 0 : NUM2UINT(flags);

  return nv;
}

/*
 * call-seq:
 *   NV.new(name, value, flags: 0) -> NV
 *
 * Creates a new NV (name-value pair) object representing an HTTP header.
 */
static VALUE rb_nghttp3_nv_initialize(int argc, VALUE *argv, VALUE self) {
  VALUE name, value, opts;
  VALUE flags = INT2FIX(0);

  rb_scan_args(argc, argv, "2:", &name, &value, &opts);

  StringValue(name);
  StringValue(value);

  if (!NIL_P(opts)) {
    VALUE rb_flags = rb_hash_aref(opts, ID2SYM(rb_intern("flags")));
    if (!NIL_P(rb_flags)) {
      flags = rb_flags;
    }
  }

  rb_iv_set(self, "@name", rb_str_freeze(rb_str_dup(name)));
  rb_iv_set(self, "@value", rb_str_freeze(rb_str_dup(value)));
  rb_iv_set(self, "@flags", flags);

  return self;
}

static VALUE rb_nghttp3_nv_get_name(VALUE self) {
  return rb_iv_get(self, "@name");
}

static VALUE rb_nghttp3_nv_get_value(VALUE self) {
  return rb_iv_get(self, "@value");
}

static VALUE rb_nghttp3_nv_get_flags(VALUE self) {
  return rb_iv_get(self, "@flags");
}

RUBY_FUNC_EXPORTED void Init_nghttp3(void) {
  /* Define main module */
  rb_mNghttp3 = rb_define_module("Nghttp3");

  /* Define compile-time version constants */
  rb_define_const(rb_mNghttp3, "NGHTTP3_VERSION",
                  rb_str_new_cstr(NGHTTP3_VERSION));
  rb_define_const(rb_mNghttp3, "NGHTTP3_VERSION_NUM",
                  INT2NUM(NGHTTP3_VERSION_NUM));
  rb_define_const(rb_mNghttp3, "NGHTTP3_VERSION_AGE",
                  INT2NUM(NGHTTP3_VERSION_AGE));

  /* Define module method for runtime version */
  rb_define_singleton_method(rb_mNghttp3, "library_version",
                             rb_nghttp3_library_version, -1);

  /* Define Info class */
  rb_cNghttp3Info = rb_define_class_under(rb_mNghttp3, "Info", rb_cObject);
  rb_undef_method(rb_singleton_class(rb_cNghttp3Info), "new");
  rb_define_method(rb_cNghttp3Info, "age", rb_nghttp3_info_age, 0);
  rb_define_method(rb_cNghttp3Info, "version_num", rb_nghttp3_info_version_num,
                   0);
  rb_define_method(rb_cNghttp3Info, "version_str", rb_nghttp3_info_version_str,
                   0);

  /* Define base error class */
  rb_eNghttp3Error =
      rb_define_class_under(rb_mNghttp3, "Error", rb_eStandardError);

  /* Define error code constants */
  rb_define_const(rb_mNghttp3, "NGHTTP3_ERR_INVALID_ARGUMENT",
                  INT2NUM(NGHTTP3_ERR_INVALID_ARGUMENT));
  rb_define_const(rb_mNghttp3, "NGHTTP3_ERR_INVALID_STATE",
                  INT2NUM(NGHTTP3_ERR_INVALID_STATE));
  rb_define_const(rb_mNghttp3, "NGHTTP3_ERR_WOULDBLOCK",
                  INT2NUM(NGHTTP3_ERR_WOULDBLOCK));
  rb_define_const(rb_mNghttp3, "NGHTTP3_ERR_STREAM_IN_USE",
                  INT2NUM(NGHTTP3_ERR_STREAM_IN_USE));
  rb_define_const(rb_mNghttp3, "NGHTTP3_ERR_MALFORMED_HTTP_HEADER",
                  INT2NUM(NGHTTP3_ERR_MALFORMED_HTTP_HEADER));
  rb_define_const(rb_mNghttp3, "NGHTTP3_ERR_MALFORMED_HTTP_MESSAGING",
                  INT2NUM(NGHTTP3_ERR_MALFORMED_HTTP_MESSAGING));
  rb_define_const(rb_mNghttp3, "NGHTTP3_ERR_QPACK_FATAL",
                  INT2NUM(NGHTTP3_ERR_QPACK_FATAL));
  rb_define_const(rb_mNghttp3, "NGHTTP3_ERR_QPACK_HEADER_TOO_LARGE",
                  INT2NUM(NGHTTP3_ERR_QPACK_HEADER_TOO_LARGE));
  rb_define_const(rb_mNghttp3, "NGHTTP3_ERR_STREAM_NOT_FOUND",
                  INT2NUM(NGHTTP3_ERR_STREAM_NOT_FOUND));
  rb_define_const(rb_mNghttp3, "NGHTTP3_ERR_CONN_CLOSING",
                  INT2NUM(NGHTTP3_ERR_CONN_CLOSING));
  rb_define_const(rb_mNghttp3, "NGHTTP3_ERR_STREAM_DATA_OVERFLOW",
                  INT2NUM(NGHTTP3_ERR_STREAM_DATA_OVERFLOW));
  rb_define_const(rb_mNghttp3, "NGHTTP3_ERR_FATAL", INT2NUM(NGHTTP3_ERR_FATAL));
  rb_define_const(rb_mNghttp3, "NGHTTP3_ERR_NOMEM", INT2NUM(NGHTTP3_ERR_NOMEM));
  rb_define_const(rb_mNghttp3, "NGHTTP3_ERR_CALLBACK_FAILURE",
                  INT2NUM(NGHTTP3_ERR_CALLBACK_FAILURE));

  /* Define specific error classes */
  rb_eNghttp3InvalidArgumentError = rb_define_class_under(
      rb_mNghttp3, "InvalidArgumentError", rb_eNghttp3Error);
  rb_eNghttp3InvalidStateError =
      rb_define_class_under(rb_mNghttp3, "InvalidStateError", rb_eNghttp3Error);
  rb_eNghttp3WouldBlockError =
      rb_define_class_under(rb_mNghttp3, "WouldBlockError", rb_eNghttp3Error);
  rb_eNghttp3StreamInUseError =
      rb_define_class_under(rb_mNghttp3, "StreamInUseError", rb_eNghttp3Error);
  rb_eNghttp3MalformedHTTPHeaderError = rb_define_class_under(
      rb_mNghttp3, "MalformedHTTPHeaderError", rb_eNghttp3Error);
  rb_eNghttp3MalformedHTTPMessagingError = rb_define_class_under(
      rb_mNghttp3, "MalformedHTTPMessagingError", rb_eNghttp3Error);
  rb_eNghttp3QPACKFatalError =
      rb_define_class_under(rb_mNghttp3, "QPACKFatalError", rb_eNghttp3Error);
  rb_eNghttp3QPACKHeaderTooLargeError = rb_define_class_under(
      rb_mNghttp3, "QPACKHeaderTooLargeError", rb_eNghttp3Error);
  rb_eNghttp3StreamNotFoundError = rb_define_class_under(
      rb_mNghttp3, "StreamNotFoundError", rb_eNghttp3Error);
  rb_eNghttp3ConnClosingError =
      rb_define_class_under(rb_mNghttp3, "ConnClosingError", rb_eNghttp3Error);
  rb_eNghttp3StreamDataOverflowError = rb_define_class_under(
      rb_mNghttp3, "StreamDataOverflowError", rb_eNghttp3Error);
  rb_eNghttp3FatalError =
      rb_define_class_under(rb_mNghttp3, "FatalError", rb_eNghttp3Error);
  rb_eNghttp3NoMemError =
      rb_define_class_under(rb_mNghttp3, "NoMemError", rb_eNghttp3Error);
  rb_eNghttp3CallbackFailureError = rb_define_class_under(
      rb_mNghttp3, "CallbackFailureError", rb_eNghttp3Error);

  /* Define error utility methods */
  rb_define_singleton_method(rb_mNghttp3, "err_is_fatal?",
                             rb_nghttp3_err_is_fatal, 1);
  rb_define_singleton_method(rb_mNghttp3, "strerror", rb_nghttp3_strerror, 1);

  /* Define NV flag constants */
  rb_define_const(rb_mNghttp3, "NV_FLAG_NONE", UINT2NUM(NGHTTP3_NV_FLAG_NONE));
  rb_define_const(rb_mNghttp3, "NV_FLAG_NEVER_INDEX",
                  UINT2NUM(NGHTTP3_NV_FLAG_NEVER_INDEX));
  rb_define_const(rb_mNghttp3, "NV_FLAG_NO_COPY_NAME",
                  UINT2NUM(NGHTTP3_NV_FLAG_NO_COPY_NAME));
  rb_define_const(rb_mNghttp3, "NV_FLAG_NO_COPY_VALUE",
                  UINT2NUM(NGHTTP3_NV_FLAG_NO_COPY_VALUE));
  rb_define_const(rb_mNghttp3, "NV_FLAG_TRY_INDEX",
                  UINT2NUM(NGHTTP3_NV_FLAG_TRY_INDEX));

  /* Define Settings class */
  rb_cNghttp3Settings =
      rb_define_class_under(rb_mNghttp3, "Settings", rb_cObject);
  rb_define_alloc_func(rb_cNghttp3Settings, settings_alloc);
  rb_define_method(rb_cNghttp3Settings, "initialize",
                   rb_nghttp3_settings_initialize, 0);
  rb_define_singleton_method(rb_cNghttp3Settings, "default",
                             rb_nghttp3_settings_default, 0);

  /* Settings getters */
  rb_define_method(rb_cNghttp3Settings, "max_field_section_size",
                   rb_nghttp3_settings_get_max_field_section_size, 0);
  rb_define_method(rb_cNghttp3Settings, "qpack_max_dtable_capacity",
                   rb_nghttp3_settings_get_qpack_max_dtable_capacity, 0);
  rb_define_method(rb_cNghttp3Settings, "qpack_encoder_max_dtable_capacity",
                   rb_nghttp3_settings_get_qpack_encoder_max_dtable_capacity,
                   0);
  rb_define_method(rb_cNghttp3Settings, "qpack_blocked_streams",
                   rb_nghttp3_settings_get_qpack_blocked_streams, 0);
  rb_define_method(rb_cNghttp3Settings, "enable_connect_protocol",
                   rb_nghttp3_settings_get_enable_connect_protocol, 0);
  rb_define_method(rb_cNghttp3Settings, "h3_datagram",
                   rb_nghttp3_settings_get_h3_datagram, 0);

  /* Settings setters */
  rb_define_method(rb_cNghttp3Settings, "max_field_section_size=",
                   rb_nghttp3_settings_set_max_field_section_size, 1);
  rb_define_method(rb_cNghttp3Settings, "qpack_max_dtable_capacity=",
                   rb_nghttp3_settings_set_qpack_max_dtable_capacity, 1);
  rb_define_method(rb_cNghttp3Settings, "qpack_encoder_max_dtable_capacity=",
                   rb_nghttp3_settings_set_qpack_encoder_max_dtable_capacity,
                   1);
  rb_define_method(rb_cNghttp3Settings, "qpack_blocked_streams=",
                   rb_nghttp3_settings_set_qpack_blocked_streams, 1);
  rb_define_method(rb_cNghttp3Settings, "enable_connect_protocol=",
                   rb_nghttp3_settings_set_enable_connect_protocol, 1);
  rb_define_method(rb_cNghttp3Settings,
                   "h3_datagram=", rb_nghttp3_settings_set_h3_datagram, 1);

  /* Define NV class */
  rb_cNghttp3NV = rb_define_class_under(rb_mNghttp3, "NV", rb_cObject);
  rb_define_method(rb_cNghttp3NV, "initialize", rb_nghttp3_nv_initialize, -1);
  rb_define_method(rb_cNghttp3NV, "name", rb_nghttp3_nv_get_name, 0);
  rb_define_method(rb_cNghttp3NV, "value", rb_nghttp3_nv_get_value, 0);
  rb_define_method(rb_cNghttp3NV, "flags", rb_nghttp3_nv_get_flags, 0);
}
