#pragma once

// Copyright 2018 DigitalBits Development Foundation and contributors. Licensed
// under the Apache License, Version 2.0. See the COPYING file at the root
// of this distribution or at http://www.apache.org/licenses/LICENSE-2.0

#include "crypto/ShortHash.h"
#include "ledger/InternalLedgerEntry.h"
#include "xdr/DigitalBits-ledger.h"
#include <functional>

// implements a default hasher for "LedgerKey"
namespace std
{
template <> class hash<digitalbits::Asset>
{
  public:
    size_t
    operator()(digitalbits::Asset const& asset) const
    {
        size_t res = asset.type();
        switch (asset.type())
        {
        case digitalbits::ASSET_TYPE_NATIVE:
            break;
        case digitalbits::ASSET_TYPE_CREDIT_ALPHANUM4:
        {
            auto& a4 = asset.alphaNum4();
            res ^= digitalbits::shortHash::computeHash(
                digitalbits::ByteSlice(a4.issuer.ed25519().data(), 8));
            res ^= a4.assetCode[0];
            break;
        }
        case digitalbits::ASSET_TYPE_CREDIT_ALPHANUM12:
        {
            auto& a12 = asset.alphaNum12();
            res ^= digitalbits::shortHash::computeHash(
                digitalbits::ByteSlice(a12.issuer.ed25519().data(), 8));
            res ^= a12.assetCode[0];
            break;
        }
        }
        return res;
    }
};

template <> class hash<digitalbits::LedgerKey>
{
  public:
    size_t
    operator()(digitalbits::LedgerKey const& lk) const
    {
        size_t res;
        switch (lk.type())
        {
        case digitalbits::ACCOUNT:
            res = digitalbits::shortHash::computeHash(
                digitalbits::ByteSlice(lk.account().accountID.ed25519().data(), 8));
            break;
        case digitalbits::TRUSTLINE:
        {
            auto& tl = lk.trustLine();
            res = digitalbits::shortHash::computeHash(
                digitalbits::ByteSlice(tl.accountID.ed25519().data(), 8));
            res ^= hash<digitalbits::Asset>()(tl.asset);
            break;
        }
        case digitalbits::DATA:
            res = digitalbits::shortHash::computeHash(
                digitalbits::ByteSlice(lk.data().accountID.ed25519().data(), 8));
            res ^= digitalbits::shortHash::computeHash(digitalbits::ByteSlice(
                lk.data().dataName.data(), lk.data().dataName.size()));
            break;
        case digitalbits::OFFER:
            res = digitalbits::shortHash::computeHash(digitalbits::ByteSlice(
                &lk.offer().offerID, sizeof(lk.offer().offerID)));
            break;
        case digitalbits::CLAIMABLE_BALANCE:
            res = digitalbits::shortHash::computeHash(digitalbits::ByteSlice(
                lk.claimableBalance().balanceID.v0().data(), 8));
            break;
        default:
            abort();
        }
        return res;
    }
};

template <> class hash<digitalbits::InternalLedgerKey>
{
  public:
    size_t
    operator()(digitalbits::InternalLedgerKey const& glk) const
    {
        switch (glk.type())
        {
        case digitalbits::InternalLedgerEntryType::LEDGER_ENTRY:
            return hash<digitalbits::LedgerKey>()(glk.ledgerKey());
        case digitalbits::InternalLedgerEntryType::SPONSORSHIP:
            return digitalbits::shortHash::computeHash(digitalbits::ByteSlice(
                glk.sponsorshipKey().sponsoredID.ed25519().data(), 8));
        case digitalbits::InternalLedgerEntryType::SPONSORSHIP_COUNTER:
            return digitalbits::shortHash::computeHash(digitalbits::ByteSlice(
                glk.sponsorshipCounterKey().sponsoringID.ed25519().data(), 8));
        default:
            abort();
        }
    }
};
}
