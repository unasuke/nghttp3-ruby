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
end
