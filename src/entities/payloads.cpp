// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/entities/payloads.hpp"
#include "veriblock/hashutil.hpp"

namespace altintegration {

AltPayloads AltPayloads::fromVbkEncoding(ReadStream& stream) {
  AltPayloads p;
  p.endorsed = AltBlock::fromVbkEncoding(stream);
  p.containingBlock = AltBlock::fromVbkEncoding(stream);
  p.containingTx = stream.readSlice(uint256::size());
  p.hasAtv = stream.readBE<uint8_t>();
  if (p.hasAtv) {
    p.atv = ATV::fromVbkEncoding(stream);
  }

  p.vtbs =
      readArrayOf<VTB>(stream, 0, MAX_CONTEXT_COUNT, [](ReadStream& stream) {
        return VTB::fromVbkEncoding(stream);
      });

  return p;
}

AltPayloads AltPayloads::fromVbkEncoding(const std::string& bytes) {
  ReadStream stream(bytes);
  return fromVbkEncoding(stream);
}

void AltPayloads::toVbkEncoding(WriteStream& stream) const {
  endorsed.toVbkEncoding(stream);
  containingBlock.toVbkEncoding(stream);
  stream.write(containingTx);
  stream.writeBE<uint8_t>(hasAtv);
  if (hasAtv) {
    atv.toVbkEncoding(stream);
  }

  writeSingleBEValue(stream, vtbs.size());
  for (const auto& el : vtbs) {
    el.toVbkEncoding(stream);
  }
}

std::vector<uint8_t> AltPayloads::toVbkEncoding() const {
  WriteStream stream;
  toVbkEncoding(stream);
  return stream.data();
}

AltPayloads::id_t AltPayloads::getId() const {
  return sha256(containingTx, containingBlock.hash);
}

AltBlock AltPayloads::getContainingBlock() const { return containingBlock; }

AltBlock AltPayloads::getEndorsedBlock() const { return endorsed; }

bool AltPayloads::containsEndorsements() const { return hasAtv; }

VbkEndorsement AltPayloads::getEndorsement() const {
  return VbkEndorsement::fromContainer(*this);
}

typename VbkEndorsement::id_t AltPayloads::getEndorsementId() const {
  return VbkEndorsement::getId(*this);
}

}  // namespace altintegration
