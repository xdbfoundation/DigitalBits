// Copyright 2020 DigitalBits Development Foundation and contributors. Licensed
// under the Apache License, Version 2.0. See the COPYING file at the root
// of this distribution or at http://www.apache.org/licenses/LICENSE-2.0

#include "HashOfHash.h"
#include "crypto/ShortHash.h"

namespace std
{

size_t
hash<digitalbits::uint256>::operator()(digitalbits::uint256 const& x) const noexcept
{
    size_t res =
        digitalbits::shortHash::computeHash(digitalbits::ByteSlice(x.data(), 8));

    return res;
}
}
