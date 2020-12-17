#pragma once

// Copyright 2017 DigitalBits Development Foundation and contributors. Licensed
// under the Apache License, Version 2.0. See the COPYING file at the root
// of this distribution or at http://www.apache.org/licenses/LICENSE-2.0

#include "xdr/DigitalBits-types.h"
#include <vector>

namespace digitalbits
{

struct SCPEnvelope;
struct SCPStatement;
struct DigitalBitsValue;

std::vector<Hash> getTxSetHashes(SCPEnvelope const& envelope);
std::vector<DigitalBitsValue> getDigitalBitsValues(SCPStatement const& envelope);
}
