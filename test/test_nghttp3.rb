# frozen_string_literal: true

require "test_helper"

class TestNghttp3 < Minitest::Test
  def test_that_it_has_a_version_number
    refute_nil ::Nghttp3::VERSION
  end

  # Compile-time constant tests
  def test_nghttp3_version_constant
    assert_kind_of String, Nghttp3::NGHTTP3_VERSION
    assert_match(/\A\d+\.\d+\.\d+\z/, Nghttp3::NGHTTP3_VERSION)
  end

  def test_nghttp3_version_num_constant
    assert_kind_of Integer, Nghttp3::NGHTTP3_VERSION_NUM
    assert_operator Nghttp3::NGHTTP3_VERSION_NUM, :>, 0
  end

  def test_nghttp3_version_age_constant
    assert_kind_of Integer, Nghttp3::NGHTTP3_VERSION_AGE
    assert_equal 1, Nghttp3::NGHTTP3_VERSION_AGE
  end

  # Runtime version tests
  def test_library_version_returns_info_object
    info = Nghttp3.library_version
    assert_kind_of Nghttp3::Info, info
  end

  def test_library_version_info_attributes
    info = Nghttp3.library_version

    assert_respond_to info, :age
    assert_respond_to info, :version_num
    assert_respond_to info, :version_str
  end

  def test_library_version_info_age
    info = Nghttp3.library_version
    assert_kind_of Integer, info.age
    assert_equal 1, info.age
  end

  def test_library_version_info_version_num
    info = Nghttp3.library_version
    assert_kind_of Integer, info.version_num
    assert_equal Nghttp3::NGHTTP3_VERSION_NUM, info.version_num
  end

  def test_library_version_info_version_str
    info = Nghttp3.library_version
    assert_kind_of String, info.version_str
    assert_match(/\A\d+\.\d+\.\d+\z/, info.version_str)
    assert_equal Nghttp3::NGHTTP3_VERSION, info.version_str
  end

  def test_library_version_with_least_version_met
    info = Nghttp3.library_version(0)
    refute_nil info
    assert_kind_of Nghttp3::Info, info
  end

  def test_library_version_with_least_version_not_met
    info = Nghttp3.library_version(0xFFFFFF)
    assert_nil info
  end

  def test_info_cannot_be_instantiated_directly
    assert_raises(NoMethodError) do
      Nghttp3::Info.new
    end
  end

  def test_error_class_exists
    assert_kind_of Class, Nghttp3::Error
    assert_operator Nghttp3::Error, :<, StandardError
  end

  # Error code constant tests
  def test_error_code_constants_exist
    assert_equal(-101, Nghttp3::NGHTTP3_ERR_INVALID_ARGUMENT)
    assert_equal(-102, Nghttp3::NGHTTP3_ERR_INVALID_STATE)
    assert_equal(-103, Nghttp3::NGHTTP3_ERR_WOULDBLOCK)
    assert_equal(-104, Nghttp3::NGHTTP3_ERR_STREAM_IN_USE)
    assert_equal(-105, Nghttp3::NGHTTP3_ERR_MALFORMED_HTTP_HEADER)
    assert_equal(-107, Nghttp3::NGHTTP3_ERR_MALFORMED_HTTP_MESSAGING)
    assert_equal(-108, Nghttp3::NGHTTP3_ERR_QPACK_FATAL)
    assert_equal(-109, Nghttp3::NGHTTP3_ERR_QPACK_HEADER_TOO_LARGE)
    assert_equal(-110, Nghttp3::NGHTTP3_ERR_STREAM_NOT_FOUND)
    assert_equal(-111, Nghttp3::NGHTTP3_ERR_CONN_CLOSING)
    assert_equal(-112, Nghttp3::NGHTTP3_ERR_STREAM_DATA_OVERFLOW)
    assert_equal(-900, Nghttp3::NGHTTP3_ERR_FATAL)
    assert_equal(-901, Nghttp3::NGHTTP3_ERR_NOMEM)
    assert_equal(-902, Nghttp3::NGHTTP3_ERR_CALLBACK_FAILURE)
  end

  # err_is_fatal? tests
  def test_err_is_fatal_returns_true_for_fatal_errors
    # Fatal errors are those with error code < NGHTTP3_ERR_FATAL (-900)
    assert Nghttp3.err_is_fatal?(Nghttp3::NGHTTP3_ERR_NOMEM)          # -901
    assert Nghttp3.err_is_fatal?(Nghttp3::NGHTTP3_ERR_CALLBACK_FAILURE) # -902
  end

  def test_err_is_fatal_returns_false_for_non_fatal_errors
    # NGHTTP3_ERR_FATAL (-900) is the threshold, not a fatal error itself
    refute Nghttp3.err_is_fatal?(Nghttp3::NGHTTP3_ERR_FATAL)
    refute Nghttp3.err_is_fatal?(Nghttp3::NGHTTP3_ERR_INVALID_ARGUMENT)
    refute Nghttp3.err_is_fatal?(Nghttp3::NGHTTP3_ERR_WOULDBLOCK)
    refute Nghttp3.err_is_fatal?(Nghttp3::NGHTTP3_ERR_STREAM_NOT_FOUND)
  end

  # strerror tests
  def test_strerror_returns_string
    str = Nghttp3.strerror(Nghttp3::NGHTTP3_ERR_INVALID_ARGUMENT)
    assert_kind_of String, str
    refute_empty str
  end

  def test_strerror_returns_different_strings_for_different_errors
    str1 = Nghttp3.strerror(Nghttp3::NGHTTP3_ERR_INVALID_ARGUMENT)
    str2 = Nghttp3.strerror(Nghttp3::NGHTTP3_ERR_NOMEM)
    refute_equal str1, str2
  end

  # Error class hierarchy tests
  def test_specific_error_classes_exist
    error_classes = [
      Nghttp3::InvalidArgumentError,
      Nghttp3::InvalidStateError,
      Nghttp3::WouldBlockError,
      Nghttp3::StreamInUseError,
      Nghttp3::MalformedHTTPHeaderError,
      Nghttp3::MalformedHTTPMessagingError,
      Nghttp3::QPACKFatalError,
      Nghttp3::QPACKHeaderTooLargeError,
      Nghttp3::StreamNotFoundError,
      Nghttp3::ConnClosingError,
      Nghttp3::StreamDataOverflowError,
      Nghttp3::FatalError,
      Nghttp3::NoMemError,
      Nghttp3::CallbackFailureError
    ]

    error_classes.each do |klass|
      assert_kind_of Class, klass
      assert_operator klass, :<, Nghttp3::Error
    end
  end

  def test_error_classes_can_be_raised_and_caught
    assert_raises(Nghttp3::InvalidArgumentError) do
      raise Nghttp3::InvalidArgumentError, "test"
    end

    assert_raises(Nghttp3::Error) do
      raise Nghttp3::NoMemError, "test"
    end
  end

  # NV flag constant tests
  def test_nv_flag_constants_exist
    assert_equal 0x00, Nghttp3::NV_FLAG_NONE
    assert_equal 0x01, Nghttp3::NV_FLAG_NEVER_INDEX
    assert_equal 0x02, Nghttp3::NV_FLAG_NO_COPY_NAME
    assert_equal 0x04, Nghttp3::NV_FLAG_NO_COPY_VALUE
    assert_equal 0x08, Nghttp3::NV_FLAG_TRY_INDEX
  end

  # Settings class tests
  def test_settings_new_creates_empty_settings
    settings = Nghttp3::Settings.new
    assert_kind_of Nghttp3::Settings, settings
    assert_equal 0, settings.max_field_section_size
    assert_equal 0, settings.qpack_max_dtable_capacity
    assert_equal 0, settings.qpack_encoder_max_dtable_capacity
    assert_equal 0, settings.qpack_blocked_streams
    assert_equal false, settings.enable_connect_protocol
    assert_equal false, settings.h3_datagram
  end

  def test_settings_default_creates_settings_with_default_values
    settings = Nghttp3::Settings.default
    assert_kind_of Nghttp3::Settings, settings

    # Check default values from nghttp3_settings_default
    assert_operator settings.max_field_section_size, :>, 0
    assert_equal 0, settings.qpack_max_dtable_capacity
    assert_equal 4096, settings.qpack_encoder_max_dtable_capacity
    assert_equal 0, settings.qpack_blocked_streams
    assert_equal false, settings.enable_connect_protocol
  end

  def test_settings_max_field_section_size_getter_and_setter
    settings = Nghttp3::Settings.new
    settings.max_field_section_size = 16384
    assert_equal 16384, settings.max_field_section_size
  end

  def test_settings_qpack_max_dtable_capacity_getter_and_setter
    settings = Nghttp3::Settings.new
    settings.qpack_max_dtable_capacity = 4096
    assert_equal 4096, settings.qpack_max_dtable_capacity
  end

  def test_settings_qpack_encoder_max_dtable_capacity_getter_and_setter
    settings = Nghttp3::Settings.new
    settings.qpack_encoder_max_dtable_capacity = 8192
    assert_equal 8192, settings.qpack_encoder_max_dtable_capacity
  end

  def test_settings_qpack_blocked_streams_getter_and_setter
    settings = Nghttp3::Settings.new
    settings.qpack_blocked_streams = 100
    assert_equal 100, settings.qpack_blocked_streams
  end

  def test_settings_enable_connect_protocol_getter_and_setter
    settings = Nghttp3::Settings.new
    assert_equal false, settings.enable_connect_protocol

    settings.enable_connect_protocol = true
    assert_equal true, settings.enable_connect_protocol

    settings.enable_connect_protocol = false
    assert_equal false, settings.enable_connect_protocol
  end

  def test_settings_h3_datagram_getter_and_setter
    settings = Nghttp3::Settings.new
    assert_equal false, settings.h3_datagram

    settings.h3_datagram = true
    assert_equal true, settings.h3_datagram

    settings.h3_datagram = false
    assert_equal false, settings.h3_datagram
  end

  # NV class tests
  def test_nv_new_creates_nv_with_name_and_value
    nv = Nghttp3::NV.new(":method", "GET")
    assert_kind_of Nghttp3::NV, nv
    assert_equal ":method", nv.name
    assert_equal "GET", nv.value
    assert_equal 0, nv.flags
  end

  def test_nv_new_with_flags
    nv = Nghttp3::NV.new(":path", "/", flags: Nghttp3::NV_FLAG_NEVER_INDEX)
    assert_equal ":path", nv.name
    assert_equal "/", nv.value
    assert_equal Nghttp3::NV_FLAG_NEVER_INDEX, nv.flags
  end

  def test_nv_name_is_frozen
    nv = Nghttp3::NV.new("content-type", "text/html")
    assert nv.name.frozen?
    assert nv.value.frozen?
  end

  def test_nv_with_various_headers
    headers = [
      [":method", "POST"],
      [":path", "/api/v1/users"],
      [":scheme", "https"],
      [":authority", "example.com"],
      ["content-type", "application/json"],
      ["accept", "*/*"]
    ]

    headers.each do |name, value|
      nv = Nghttp3::NV.new(name, value)
      assert_equal name, nv.name
      assert_equal value, nv.value
    end
  end
end
