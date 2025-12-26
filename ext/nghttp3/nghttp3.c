#include "nghttp3.h"

VALUE rb_mNghttp3;
VALUE rb_cNghttp3Info;
VALUE rb_eNghttp3Error;

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
}
