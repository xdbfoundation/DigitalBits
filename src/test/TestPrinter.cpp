// Copyright 2017 DigitalBits Development Foundation and contributors. Licensed
// under the Apache License, Version 2.0. See the COPYING file at the root
// of this distribution or at http://www.apache.org/licenses/LICENSE-2.0

#include "test/TestPrinter.h"
#include "catchup/CatchupRange.h"
#include "test/TestMarket.h"
#include "util/XDRCereal.h"
#include <fmt/format.h>

namespace Catch
{
std::string
StringMaker<digitalbits::OfferState>::convert(digitalbits::OfferState const& os)
{
    return fmt::format(
        "selling: {}, buying: {}, price: {}, amount: {}, type: {}",
        xdr_to_string(os.selling), xdr_to_string(os.buying),
        xdr_to_string(os.price), os.amount,
        os.type == digitalbits::OfferType::PASSIVE ? "passive" : "active");
}

std::string
StringMaker<digitalbits::CatchupRange>::convert(digitalbits::CatchupRange const& cr)
{
    return fmt::format("[{},{}), applyBuckets: {}", cr.getReplayFirst(),
                       cr.getReplayLimit(),
                       cr.applyBuckets() ? cr.getBucketApplyLedger() : 0);
}

std::string
StringMaker<digitalbits::historytestutils::CatchupPerformedWork>::convert(
    digitalbits::historytestutils::CatchupPerformedWork const& cm)
{
    return fmt::format(
        "{}, {}, {}, {}, {}, {}, {}, {}", cm.mHistoryArchiveStatesDownloaded,
        cm.mCheckpointsDownloaded, cm.mLedgersVerified,
        cm.mLedgerChainsVerificationFailed, cm.mBucketsDownloaded,
        cm.mBucketsApplied, cm.mTxSetsDownloaded, cm.mTxSetsApplied);
}
}
