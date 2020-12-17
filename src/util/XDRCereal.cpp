// Copyright 2020 DigitalBits Development Foundation and contributors. Licensed
// under the Apache License, Version 2.0. See the COPYING file at the root
// of this distribution or at http://www.apache.org/licenses/LICENSE-2.0

#include "util/XDRCereal.h"

void
cereal_override(cereal::JSONOutputArchive& ar, const digitalbits::PublicKey& s,
                const char* field)
{
    xdr::archive(ar, digitalbits::KeyUtils::toStrKey<digitalbits::PublicKey>(s), field);
}

void
cereal_override(cereal::JSONOutputArchive& ar,
                const digitalbits::MuxedAccount& muxedAccount, const char* field)
{
    switch (muxedAccount.type())
    {
    case digitalbits::KEY_TYPE_ED25519:
        xdr::archive(ar, digitalbits::KeyUtils::toStrKey(toAccountID(muxedAccount)),
                     field);
        return;
    case digitalbits::KEY_TYPE_MUXED_ED25519:
        xdr::archive(
            ar,
            std::make_tuple(
                cereal::make_nvp("id", muxedAccount.med25519().id),
                cereal::make_nvp("accountID", digitalbits::KeyUtils::toStrKey(
                                                  toAccountID(muxedAccount)))),
            field);
        return;
    default:
        // this would be a bug
        abort();
    }
}

void
cereal_override(cereal::JSONOutputArchive& ar, const digitalbits::Asset& s,
                const char* field)
{
    xdr::archive(ar, digitalbits::assetToString(s), field);
}
