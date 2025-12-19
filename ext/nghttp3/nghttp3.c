#include "nghttp3.h"

VALUE rb_mNghttp3;

RUBY_FUNC_EXPORTED void
Init_nghttp3(void)
{
  rb_mNghttp3 = rb_define_module("Nghttp3");
}
