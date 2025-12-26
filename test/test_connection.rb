# frozen_string_literal: true

require "test_helper"

class TestNghttp3Connection < Minitest::Test
  def test_client_new_creates_connection
    conn = Nghttp3::Connection.client_new
    assert_kind_of Nghttp3::Connection, conn
    assert conn.client?
    refute conn.server?
    refute conn.closed?
  ensure
    conn&.close
  end

  def test_client_new_with_settings
    settings = Nghttp3::Settings.default
    conn = Nghttp3::Connection.client_new(settings)
    assert_kind_of Nghttp3::Connection, conn
  ensure
    conn&.close
  end

  def test_server_new_creates_connection
    conn = Nghttp3::Connection.server_new
    assert_kind_of Nghttp3::Connection, conn
    assert conn.server?
    refute conn.client?
    refute conn.closed?
  ensure
    conn&.close
  end

  def test_server_new_with_settings
    settings = Nghttp3::Settings.default
    conn = Nghttp3::Connection.server_new(settings)
    assert_kind_of Nghttp3::Connection, conn
  ensure
    conn&.close
  end

  def test_connection_cannot_be_instantiated_with_new
    # rb_undef_alloc_func causes TypeError with "allocator undefined" message
    assert_raises(TypeError) do
      Nghttp3::Connection.new
    end
  end

  def test_close_marks_connection_as_closed
    conn = Nghttp3::Connection.client_new
    refute conn.closed?
    conn.close
    assert conn.closed?
  end

  def test_close_can_be_called_multiple_times
    conn = Nghttp3::Connection.client_new
    conn.close
    conn.close # Should not raise
    assert conn.closed?
  end

  def test_bind_control_stream
    conn = Nghttp3::Connection.client_new
    # Stream ID 2 is typically used for control stream (client-initiated, unidirectional)
    result = conn.bind_control_stream(2)
    assert_same conn, result # Returns self for chaining
  ensure
    conn&.close
  end

  def test_bind_control_stream_raises_on_closed_connection
    conn = Nghttp3::Connection.client_new
    conn.close
    assert_raises(Nghttp3::InvalidStateError) do
      conn.bind_control_stream(2)
    end
  end

  def test_bind_qpack_streams
    conn = Nghttp3::Connection.client_new
    # Encoder stream ID 6, Decoder stream ID 10 (client-initiated, unidirectional)
    result = conn.bind_qpack_streams(6, 10)
    assert_same conn, result # Returns self for chaining
  ensure
    conn&.close
  end

  def test_bind_qpack_streams_raises_on_closed_connection
    conn = Nghttp3::Connection.client_new
    conn.close
    assert_raises(Nghttp3::InvalidStateError) do
      conn.bind_qpack_streams(6, 10)
    end
  end

  def test_method_chaining
    conn = Nghttp3::Connection.client_new
    result = conn
      .bind_control_stream(2)
      .bind_qpack_streams(6, 10)
    assert_same conn, result
  ensure
    conn&.close
  end
end
