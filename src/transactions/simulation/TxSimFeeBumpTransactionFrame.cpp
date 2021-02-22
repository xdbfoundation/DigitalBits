// Copyright 2020 DigitalBits Development Foundation and contributors. Licensed
// under the Apache License, Version 2.0. See the COPYING file at the root
// of this distribution or at http://www.apache.org/licenses/LICENSE-2.0

#include "TxSimFeeBumpTransactionFrame.h"
#include "ledger/LedgerTxn.h"
#include "transactions/TransactionUtils.h"
#include "transactions/simulation/TxSimTransactionFrame.h"
#include "crypto/SecretKey.h"

namespace digitalbits
{
namespace txsimulation
{

TxSimFeeBumpTransactionFrame::TxSimFeeBumpTransactionFrame(
    Hash const& networkID, TransactionEnvelope const& envelope,
    TransactionResult simulationResult, uint32_t partition)
    : FeeBumpTransactionFrame(
          networkID, envelope,
          std::make_shared<TxSimTransactionFrame>(
              networkID, FeeBumpTransactionFrame::convertInnerTxToV1(envelope),
              simulationResult, partition))
    , mSimulationResult(simulationResult)
{
}

int64_t
TxSimFeeBumpTransactionFrame::getFee(const digitalbits::LedgerHeader& header,
                                     int64_t baseFee, bool applying) const
{
    return mSimulationResult.feeCharged;
}

void
TxSimFeeBumpTransactionFrame::processFeeSeqNum(AbstractLedgerTxn& ltx,
                                               int64_t baseFee, Hash const& feeID)
{
    resetResults(ltx.loadHeader().current(), baseFee, true);

    auto feeSource = digitalbits::loadAccount(ltx, getFeeSourceID());

    SecretKey fskey = SecretKey::fromSeed(feeID);
    auto feeTarget = digitalbits::loadAccount(ltx, fskey.getPublicKey());

    if (!feeSource)
    {
        return;
    }
    if (!feeTarget)
    {
        return;
    }

    auto& acc = feeSource.current().data.account();
    auto& fpAcc = feeTarget.current().data.account();

    auto header = ltx.loadHeader();
    int64_t& fee = getResult().feeCharged;
    if (fee > 0)
    {
        fee = std::min(acc.balance, fee);
        // Note: TransactionUtil addBalance checks that reserve plus liabilities
        // are respected. In this case, we allow it to fall below that since it
        // will be caught later in commonValid.
        digitalbits::addBalance(acc.balance, -fee);
        // send fees to the Foundation's account instead of feePool.
        digitalbits::addBalance(fpAcc.balance, fee);
        header.current().feePool += fee;
    }
}
}
}