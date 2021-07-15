// Copyright 2014 DigitalBits Development Foundation and contributors. Licensed
// under the Apache License, Version 2.0. See the COPYING file at the root
// of this distribution or at http://www.apache.org/licenses/LICENSE-2.0

#include "util/asio.h"
#include "crypto/Hex.h"
#include "crypto/SHA.h"
#include "herder/LedgerCloseData.h"
#include "ledger/LedgerManager.h"
#include "ledger/LedgerTxn.h"
#include "ledger/LedgerTxnHeader.h"
#include "ledger/LedgerHeaderUtils.h"
#include "lib/catch.hpp"
#include "main/Application.h"
#include "test/TestUtils.h"
#include "test/test.h"
#include "transactions/TransactionUtils.h"
#include "util/Logging.h"
#include "util/Timer.h"
#include "xdrpp/marshal.h"

#include "main/Config.h"

using namespace digitalbits;
using namespace std;

TEST_CASE("genesisledger", "[ledger]")
{
    VirtualClock clock{};
    auto cfg = getTestConfig(0);
    cfg.USE_CONFIG_FOR_GENESIS = false;
    auto app = Application::create<ApplicationImpl>(clock, cfg);
    app->start();

    auto const& lcl = app->getLedgerManager().getLastClosedLedgerHeader();
    // get the ledger closed right after genesis
    auto & db = app->getDatabase();
    auto const& header = *digitalbits::LedgerHeaderUtils::loadByHash(db,
        lcl.header.previousLedgerHash);;        

    REQUIRE(header.ledgerVersion == 0);
    REQUIRE(header.previousLedgerHash == Hash{});
    REQUIRE(header.scpValue.txSetHash == Hash{});
    REQUIRE(header.scpValue.closeTime == 0);
    REQUIRE(header.scpValue.upgrades.size() == 0);
    REQUIRE(header.txSetResultHash == Hash{});
    REQUIRE(binToHex(header.bucketListHash) ==
            "9e8b6238ebda174ff73d10c0075e5eace56c2e06d083ee44712f3c1393230b4f");
    REQUIRE(header.ledgerSeq == 1);
    REQUIRE(header.totalCoins == 200000000000000000);
    REQUIRE(header.feePool == 0);
    REQUIRE(header.inflationSeq == 0);
    REQUIRE(header.idPool == 0);
    REQUIRE(header.baseFee == 100);
    REQUIRE(header.baseReserve == 100000000);
    REQUIRE(header.maxTxSetSize == 100);
    REQUIRE(header.skipList.size() == 4);
    REQUIRE(header.skipList[0] == Hash{});
    REQUIRE(header.skipList[1] == Hash{});
    REQUIRE(header.skipList[2] == Hash{});
    REQUIRE(header.skipList[3] == Hash{});
    REQUIRE(binToHex(sha256(xdr::xdr_to_opaque(header))) ==
            "12896a70c1bb177cf57cabec71fd08768986cf891f4b82f5c43dce8265423ec8");
}

TEST_CASE("ledgerheader", "[ledger]")
{
    Config cfg(getTestConfig(0, Config::TESTDB_ON_DISK_SQLITE));

    Hash saved;
    {
        VirtualClock clock;
        Application::pointer app = Application::create(clock, cfg);
        app->start();

        auto const& lcl = app->getLedgerManager().getLastClosedLedgerHeader();
        auto const& lastHash = lcl.hash;
        TxSetFramePtr txSet = make_shared<TxSetFrame>(lastHash);

        // close this ledger
        DigitalBitsValue sv(txSet->getContentsHash(), 1, emptyUpgradeSteps,
                        DIGITALBITS_VALUE_BASIC);
        LedgerCloseData ledgerData(lcl.header.ledgerSeq + 1, txSet, sv);
        app->getLedgerManager().closeLedger(ledgerData);

        saved = app->getLedgerManager().getLastClosedLedgerHeader().hash;
    }

    SECTION("load existing ledger")
    {
        Config cfg2(cfg);
        cfg2.FORCE_SCP = false;
        VirtualClock clock2;
        Application::pointer app2 = Application::create(clock2, cfg2, false);
        app2->start();

        REQUIRE(saved ==
                app2->getLedgerManager().getLastClosedLedgerHeader().hash);
    }
}

TEST_CASE("base reserve", "[ledger]")
{
    Config const& cfg = getTestConfig();

    VirtualClock clock;
    auto app = createTestApplication(clock, cfg);

    app->start();

    auto const& lcl = app->getLedgerManager().getLastClosedLedgerHeader();
    REQUIRE(lcl.header.baseReserve == 100000000);
    const uint32 n = 20000;
    int64 expectedReserve = 2000200000000ll;

    for_versions_to(8, *app, [&]() {
        LedgerTxn ltx(app->getLedgerTxnRoot());
        REQUIRE(getMinBalance(ltx.loadHeader().current(), n, 0, 0) <
                expectedReserve);
    });
    for_versions_from(9, *app, [&]() {
        LedgerTxn ltx(app->getLedgerTxnRoot());
        REQUIRE(getMinBalance(ltx.loadHeader().current(), n, 0, 0) ==
                expectedReserve);
    });
}
