// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <benchmark/benchmark.h>

#include <veriblock/pop/literals.hpp>
#include <veriblock/pop/stateless_validation.hpp>

using namespace altintegration;

static auto defaultPrivateKeyVbk =
    "303e020100301006072a8648ce3d020106052b8104000a0427302502010104203abf83fa470423d4788a760ef6b7aae1dacf98784b0646057a0adca24e522acb"_unhex;

static auto defaultPublicKeyVbk =
    "3056301006072a8648ce3d020106052b8104000a034200042fca63a20cb5208c2a55ff5099ca1966b7f52e687600784d1de062c1dd9c8a5fe55b2ba5d906c703d37cbd02ecd9c97a806110fa05d9014a102a0513dd354ec5"_unhex;

static auto defaultMsg = "Hello world"_v;

static auto defaultSignatureVbk =
    "3044022008d07afee77324d0bced6f3bce19892d0413981e83e68401cd83d1e1ed3bc37c022005273429062dcf623ccd04c8d9c9e601e7fc45b5db32900c9b0ffda2dbc8f452"_unhex;

static void VerifySecp256k1Sig(benchmark::State& state) {
  auto privateKey =
      altintegration::secp256k1::privateKeyFromVbk(defaultPrivateKeyVbk);
  auto publicKey = altintegration::secp256k1::derivePublicKey(privateKey);

  for (auto _ : state) {
    altintegration::secp256k1::verify(
        defaultMsg, defaultSignatureVbk, publicKey);
  }
}
// Register the function as a benchmark
BENCHMARK(VerifySecp256k1Sig);
// Run the benchmark
BENCHMARK_MAIN();
