#pragma once

// Copyright 2020 DigitalBits Development Foundation and contributors. Licensed
// under the Apache License, Version 2.0. See the COPYING file at the root
// of this distribution or at http://www.apache.org/licenses/LICENSE-2.0

#include <exception>
#include <stdexcept>
#include <string>

namespace digitalbits
{
class CryptoError : public std::runtime_error
{
  public:
    CryptoError(std::string const& msg) : std::runtime_error(msg)
    {
    }
};
}
