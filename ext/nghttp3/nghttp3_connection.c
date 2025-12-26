#include "nghttp3.h"

VALUE rb_cNghttp3Connection;

typedef struct {
  nghttp3_conn *conn;
  VALUE settings;  /* Prevent Settings from being GC'd */
  VALUE callbacks; /* Prevent Callbacks from being GC'd */
  int is_closed;
  int is_server;
} ConnectionObj;

static void connection_mark(void *ptr) {
  ConnectionObj *obj = (ConnectionObj *)ptr;
  if (obj->settings != Qnil) {
    rb_gc_mark(obj->settings);
  }
  if (obj->callbacks != Qnil) {
    rb_gc_mark(obj->callbacks);
  }
}

static void connection_free(void *ptr) {
  ConnectionObj *obj = (ConnectionObj *)ptr;
  if (obj->conn != NULL && !obj->is_closed) {
    nghttp3_conn_del(obj->conn);
    obj->conn = NULL;
  }
  xfree(ptr);
}

static size_t connection_memsize(const void *ptr) {
  return sizeof(ConnectionObj);
}

static const rb_data_type_t connection_data_type = {
    .wrap_struct_name = "nghttp3_conn_rb",
    .function =
        {
            .dmark = connection_mark,
            .dfree = connection_free,
            .dsize = connection_memsize,
        },
    .flags = RUBY_TYPED_FREE_IMMEDIATELY,
};

static VALUE connection_alloc(VALUE klass) {
  ConnectionObj *obj;
  VALUE self =
      TypedData_Make_Struct(klass, ConnectionObj, &connection_data_type, obj);
  obj->conn = NULL;
  obj->settings = Qnil;
  obj->callbacks = Qnil;
  obj->is_closed = 0;
  obj->is_server = 0;
  return self;
}

/*
 * Returns a pointer to the underlying nghttp3_conn structure.
 * For internal use by other functions.
 */
nghttp3_conn *nghttp3_rb_get_conn(VALUE rb_conn) {
  ConnectionObj *obj;
  TypedData_Get_Struct(rb_conn, ConnectionObj, &connection_data_type, obj);

  if (obj->conn == NULL || obj->is_closed) {
    rb_raise(rb_eNghttp3InvalidStateError, "Connection is closed");
  }

  return obj->conn;
}

/*
 * Returns the callbacks object associated with the connection.
 * For internal use by callback wrapper functions.
 */
VALUE nghttp3_rb_get_callbacks(VALUE rb_conn) {
  ConnectionObj *obj;
  TypedData_Get_Struct(rb_conn, ConnectionObj, &connection_data_type, obj);
  return obj->callbacks;
}

/*
 * call-seq:
 *   Connection.client_new(settings = nil, callbacks = nil) -> Connection
 *
 * Creates a new client HTTP/3 connection.
 * If settings is nil, default settings are used.
 * If callbacks is provided, it will be used for HTTP/3 event notifications.
 */
static VALUE rb_nghttp3_connection_client_new(int argc, VALUE *argv,
                                              VALUE klass) {
  VALUE rb_settings, rb_callbacks;
  ConnectionObj *obj;
  nghttp3_settings settings;
  nghttp3_settings *settings_ptr;
  nghttp3_callbacks callbacks;
  int rv;
  VALUE self;

  rb_scan_args(argc, argv, "02", &rb_settings, &rb_callbacks);

  self = connection_alloc(klass);
  TypedData_Get_Struct(self, ConnectionObj, &connection_data_type, obj);

  if (NIL_P(rb_settings)) {
    nghttp3_settings_default(&settings);
    settings_ptr = &settings;
  } else {
    settings_ptr = nghttp3_rb_get_settings(rb_settings);
    obj->settings = rb_settings;
  }

  /* Initialize callbacks structure */
  memset(&callbacks, 0, sizeof(callbacks));

  if (!NIL_P(rb_callbacks)) {
    obj->callbacks = rb_callbacks;
    nghttp3_rb_setup_callbacks(&callbacks);
  }

  rv = nghttp3_conn_client_new(&obj->conn, &callbacks, settings_ptr, NULL,
                               (void *)self);

  if (rv != 0) {
    nghttp3_rb_raise(rv, "Failed to create client connection");
  }

  obj->is_server = 0;
  return self;
}

/*
 * call-seq:
 *   Connection.server_new(settings = nil, callbacks = nil) -> Connection
 *
 * Creates a new server HTTP/3 connection.
 * If settings is nil, default settings are used.
 * If callbacks is provided, it will be used for HTTP/3 event notifications.
 */
static VALUE rb_nghttp3_connection_server_new(int argc, VALUE *argv,
                                              VALUE klass) {
  VALUE rb_settings, rb_callbacks;
  ConnectionObj *obj;
  nghttp3_settings settings;
  nghttp3_settings *settings_ptr;
  nghttp3_callbacks callbacks;
  int rv;
  VALUE self;

  rb_scan_args(argc, argv, "02", &rb_settings, &rb_callbacks);

  self = connection_alloc(klass);
  TypedData_Get_Struct(self, ConnectionObj, &connection_data_type, obj);

  if (NIL_P(rb_settings)) {
    nghttp3_settings_default(&settings);
    settings_ptr = &settings;
  } else {
    settings_ptr = nghttp3_rb_get_settings(rb_settings);
    obj->settings = rb_settings;
  }

  /* Initialize callbacks structure */
  memset(&callbacks, 0, sizeof(callbacks));

  if (!NIL_P(rb_callbacks)) {
    obj->callbacks = rb_callbacks;
    nghttp3_rb_setup_callbacks(&callbacks);
  }

  rv = nghttp3_conn_server_new(&obj->conn, &callbacks, settings_ptr, NULL,
                               (void *)self);

  if (rv != 0) {
    nghttp3_rb_raise(rv, "Failed to create server connection");
  }

  obj->is_server = 1;
  return self;
}

/*
 * call-seq:
 *   connection.bind_control_stream(stream_id) -> self
 *
 * Binds the control stream to the given stream_id.
 * This must be called before sending any HTTP frames.
 */
static VALUE rb_nghttp3_connection_bind_control_stream(VALUE self,
                                                       VALUE rb_stream_id) {
  ConnectionObj *obj;
  int64_t stream_id;
  int rv;

  TypedData_Get_Struct(self, ConnectionObj, &connection_data_type, obj);

  if (obj->conn == NULL || obj->is_closed) {
    rb_raise(rb_eNghttp3InvalidStateError, "Connection is closed");
  }

  stream_id = NUM2LL(rb_stream_id);
  rv = nghttp3_conn_bind_control_stream(obj->conn, stream_id);

  if (rv != 0) {
    nghttp3_rb_raise(rv, "Failed to bind control stream");
  }

  return self;
}

/*
 * call-seq:
 *   connection.bind_qpack_streams(encoder_stream_id, decoder_stream_id) -> self
 *
 * Binds the QPACK encoder and decoder streams.
 * This must be called before sending any HTTP frames.
 */
static VALUE rb_nghttp3_connection_bind_qpack_streams(VALUE self,
                                                      VALUE rb_enc_stream_id,
                                                      VALUE rb_dec_stream_id) {
  ConnectionObj *obj;
  int64_t enc_stream_id, dec_stream_id;
  int rv;

  TypedData_Get_Struct(self, ConnectionObj, &connection_data_type, obj);

  if (obj->conn == NULL || obj->is_closed) {
    rb_raise(rb_eNghttp3InvalidStateError, "Connection is closed");
  }

  enc_stream_id = NUM2LL(rb_enc_stream_id);
  dec_stream_id = NUM2LL(rb_dec_stream_id);

  rv = nghttp3_conn_bind_qpack_streams(obj->conn, enc_stream_id, dec_stream_id);

  if (rv != 0) {
    nghttp3_rb_raise(rv, "Failed to bind QPACK streams");
  }

  return self;
}

/*
 * call-seq:
 *   connection.close -> nil
 *
 * Explicitly closes the connection and frees resources.
 * After calling this, the connection object cannot be used.
 */
static VALUE rb_nghttp3_connection_close(VALUE self) {
  ConnectionObj *obj;

  TypedData_Get_Struct(self, ConnectionObj, &connection_data_type, obj);

  if (obj->conn != NULL && !obj->is_closed) {
    nghttp3_conn_del(obj->conn);
    obj->conn = NULL;
    obj->is_closed = 1;
  }

  return Qnil;
}

/*
 * call-seq:
 *   connection.closed? -> true or false
 *
 * Returns true if the connection has been closed.
 */
static VALUE rb_nghttp3_connection_closed_p(VALUE self) {
  ConnectionObj *obj;
  TypedData_Get_Struct(self, ConnectionObj, &connection_data_type, obj);
  return obj->is_closed ? Qtrue : Qfalse;
}

/*
 * call-seq:
 *   connection.server? -> true or false
 *
 * Returns true if this is a server connection.
 */
static VALUE rb_nghttp3_connection_server_p(VALUE self) {
  ConnectionObj *obj;
  TypedData_Get_Struct(self, ConnectionObj, &connection_data_type, obj);
  return obj->is_server ? Qtrue : Qfalse;
}

/*
 * call-seq:
 *   connection.client? -> true or false
 *
 * Returns true if this is a client connection.
 */
static VALUE rb_nghttp3_connection_client_p(VALUE self) {
  ConnectionObj *obj;
  TypedData_Get_Struct(self, ConnectionObj, &connection_data_type, obj);
  return obj->is_server ? Qfalse : Qtrue;
}

void Init_nghttp3_connection(void) {
  rb_cNghttp3Connection =
      rb_define_class_under(rb_mNghttp3, "Connection", rb_cObject);

  /* Disable direct instantiation with new */
  rb_undef_alloc_func(rb_cNghttp3Connection);

  /* Class methods */
  rb_define_singleton_method(rb_cNghttp3Connection, "client_new",
                             rb_nghttp3_connection_client_new, -1);
  rb_define_singleton_method(rb_cNghttp3Connection, "server_new",
                             rb_nghttp3_connection_server_new, -1);

  /* Instance methods */
  rb_define_method(rb_cNghttp3Connection, "bind_control_stream",
                   rb_nghttp3_connection_bind_control_stream, 1);
  rb_define_method(rb_cNghttp3Connection, "bind_qpack_streams",
                   rb_nghttp3_connection_bind_qpack_streams, 2);
  rb_define_method(rb_cNghttp3Connection, "close", rb_nghttp3_connection_close,
                   0);
  rb_define_method(rb_cNghttp3Connection, "closed?",
                   rb_nghttp3_connection_closed_p, 0);
  rb_define_method(rb_cNghttp3Connection, "server?",
                   rb_nghttp3_connection_server_p, 0);
  rb_define_method(rb_cNghttp3Connection, "client?",
                   rb_nghttp3_connection_client_p, 0);
}
