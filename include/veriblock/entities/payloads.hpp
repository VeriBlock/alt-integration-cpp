// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_PAYLOADS_HPP
#define ALT_INTEGRATION_PAYLOADS_HPP

#include <vector>
#include <veriblock/entities/altblock.hpp>
#include <veriblock/entities/atv.hpp>
#include <veriblock/entities/vtb.hpp>
#include <veriblock/uint.hpp>

namespace altintegration {

struct AltPayloads {
  using id_t = uint256;

  AltBlock endorsed;
  AltBlock containingBlock;
  uint256 containingTx;
  bool hasAtv{false};
  ATV atv{};
  std::vector<VTB> vtbs{};

  /**
   * Read VBK data from the stream and convert it to Payloads
   * @param stream data stream to read from
   * @return Payloads
   */
  static AltPayloads fromVbkEncoding(ReadStream& stream);

  /**
   * Read VBK data from the string raw byte representation and convert it to
   * Payloads
   * @param string data bytes to read from
   * @return AltProof
   */
  static AltPayloads fromVbkEncoding(const std::string& bytes);

  /**
   * Convert Payloads to data stream using Vbk byte format
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;

  /**
   * Convert Payloads to raw bytes data using Vbk byte format
   * @return bytes data
   */
  std::vector<uint8_t> toVbkEncoding() const;

  /**
   * Calculate a Payloads id that is the sha256 hash of the payloads rawBytes
   * @return id sha256 hash
   */

  id_t getId() const;

  /**
   * Return a containing AltBlock
   * @return containing block from AltProof
   */
  AltBlock getContainingBlock() const;

  /**
   * Return a endorsed AltBlock from the AltProof
   * @return endorsed block
   */
  AltBlock getEndorsedBlock() const;

  /**
   * Return true if contains endorsement data
   * @return true if contains endorsement data
   */
  bool containsEndorsements() const;

  /**
   * Return VbkEndorsement which generates from AltPayloads
   * @return endorsement VbkEndorsement
   */
  VbkEndorsement getEndorsement() const;

  /**
   * Return VbkEndorsement id which generates from AltPayloads
   * @return id VbkEndorsement::id_t
   */
  typename VbkEndorsement::id_t getEndorsementId() const;

  friend bool operator==(const AltPayloads& a, const AltPayloads& b) {
    return a.getId() == b.getId();
  }
};

struct DummyPayloads {
  using id_t = bool;

  id_t getId() const { return true; }
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_PAYLOADS_HPP
