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
end
