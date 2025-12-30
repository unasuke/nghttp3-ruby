# frozen_string_literal: true

require_relative "nghttp3/version"
require_relative "nghttp3/nghttp3"

# High-level API
require_relative "nghttp3/headers"
require_relative "nghttp3/request"
require_relative "nghttp3/response"
require_relative "nghttp3/stream_manager"
require_relative "nghttp3/client"
require_relative "nghttp3/server"

module Nghttp3
  class Error < StandardError; end

  # Raised when an operation is attempted in an invalid state
  class InvalidStateError < Error; end
end
