// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef BFI_BITCOIN_TRANSACTION_HPP
#define BFI_BITCOIN_TRANSACTION_HPP

#include <veriblock/bfi/bitcoin/serialize.hpp>
#include <veriblock/pop/uint.hpp>

namespace altintegration {

namespace btc {

static const int SERIALIZE_TRANSACTION_NO_WITNESS = 0x40000000;

using Script = std::vector<uint8_t>;
using ScriptWitness = std::vector<std::vector<uint8_t>>;
/** Amount in satoshis (Can be negative) */
using Amount = int64_t;

/** An outpoint - a combination of a transaction hash and an index n into its
 * vout */
struct OutPoint {
  uint256 hash;
  uint32_t n;

  ADD_SERIALIZE_METHODS;

  template <typename Stream, typename Operation>
  inline void SerializationOp(Stream& s, Operation ser_action) {
    READWRITE(hash);
    READWRITE(n);
  }

  friend bool operator==(const OutPoint& a, const OutPoint& b) {
    return a.hash == b.hash && a.n == b.n;
  }

  friend bool operator!=(const OutPoint& a, const OutPoint& b) {
    return !(a == b);
  }
};

/** An input of a transaction.  It contains the location of the previous
 * transaction's output that it claims and a signature that matches the
 * output's public key.
 */
struct TxIn {
  OutPoint prevout;
  Script scriptSig;
  uint32_t nSequence;
  ScriptWitness scriptWitness;  //!< Only serialized through CTransaction

  ADD_SERIALIZE_METHODS;

  template <typename Stream, typename Operation>
  inline void SerializationOp(Stream& s, Operation ser_action) {
    READWRITE(prevout);
    READWRITE(scriptSig);
    READWRITE(nSequence);
  }

  friend bool operator==(const TxIn& a, const TxIn& b) {
    return a.prevout == b.prevout && a.scriptSig == b.scriptSig &&
           a.nSequence == b.nSequence && a.scriptWitness == b.scriptWitness;
  }

  friend bool operator!=(const TxIn& a, const TxIn& b) { return !(a == b); }
};

/** An output of a transaction.  It contains the public key that the next input
 * must be able to sign with to claim it.
 */
struct TxOut {
  Amount nValue;
  Script scriptPubKey;

  ADD_SERIALIZE_METHODS;

  template <typename Stream, typename Operation>
  inline void SerializationOp(Stream& s, Operation ser_action) {
    READWRITE(nValue);
    READWRITE(scriptPubKey);
  }

  friend bool operator==(const TxOut& a, const TxOut& b) {
    return a.nValue == b.nValue && a.scriptPubKey == b.scriptPubKey;
  }

  friend bool operator!=(const TxOut& a, const TxOut& b) { return !(a == b); }
};

/** The basic transaction that is broadcasted on the network and contained in
 * blocks.  A transaction can contain multiple inputs and outputs.
 */
struct Transaction {
  // The local variables are made const to prevent unintended modification
  // without updating the cached hash value. However, CTransaction is not
  // actually immutable; deserialization and assignment are implemented,
  // and bypass the constness. This is safe, as they update the entire
  // structure, including the hash.
  std::vector<TxIn> vin;
  std::vector<TxOut> vout;
  int32_t nVersion;
  uint32_t nLockTime;

  template <typename Stream>
  inline void Serialize(Stream& s) const {
    SerializeTransaction(*this, s);
  }

  template <typename Stream>
  inline void Unserialize(Stream& s) {
    UnserializeTransaction(*this, s);
  }

  friend bool operator==(const Transaction& a, const Transaction& b) {
    return a.vin == b.vin && a.vout == b.vout && a.nVersion == b.nVersion &&
           a.nLockTime == b.nLockTime;
  }

  friend bool operator!=(const Transaction& a, const Transaction& b) {
    return !(a == b);
  }

  bool HasWitness() const {
    for (size_t i = 0; i < vin.size(); i++) {
      if (!vin[i].scriptWitness.empty()) {
        return true;
      }
    }
    return false;
  }
};

template <typename Stream>
inline void UnserializeTransaction(Transaction& tx, Stream& s) {
  const bool fAllowWitness =
      !(s.getVersion() & SERIALIZE_TRANSACTION_NO_WITNESS);

  Unserialize(s, tx.nVersion);
  unsigned char flags = 0;
  tx.vin.clear();
  tx.vout.clear();
  /* Try to read the vin. In case the dummy is there, this will be read as an
   * empty vector. */
  Unserialize(s, tx.vin);
  if (tx.vin.size() == 0 && fAllowWitness) {
    /* We read a dummy or an empty vin. */
    Unserialize(s, flags);
    if (flags != 0) {
      Unserialize(s, tx.vin);
      Unserialize(s, tx.vout);
    }
  } else {
    /* We read a non-empty vin. Assume a normal vout follows. */
    Unserialize(s, tx.vout);
  }
  if ((flags & 1) && fAllowWitness) {
    /* The witness flag is present, and we support witnesses. */
    flags ^= 1;
    for (size_t i = 0; i < tx.vin.size(); i++) {
      Unserialize(s, tx.vin[i].scriptWitness);
    }
    if (!tx.HasWitness()) {
      /* It's illegal to encode witnesses when all witness stacks are empty. */
      throw std::ios_base::failure("Superfluous witness record");
    }
  }
  if (flags) {
    /* Unknown flag in the serialization */
    throw std::ios_base::failure("Unknown transaction optional data");
  }
  Unserialize(s, tx.nLockTime);
}

template <typename Stream>
inline void SerializeTransaction(const Transaction& tx, Stream& s) {
  const bool fAllowWitness =
      !(s.getVersion() & SERIALIZE_TRANSACTION_NO_WITNESS);

  Serialize(s, tx.nVersion);
  unsigned char flags = 0;
  // Consistency check
  if (fAllowWitness) {
    /* Check whether witnesses need to be serialized. */
    if (tx.HasWitness()) {
      flags |= 1;
    }
  }
  if (flags) {
    /* Use extended format in case witnesses are to be serialized. */
    std::vector<TxIn> vinDummy;
    Serialize(s, vinDummy);
    Serialize(s, flags);
  }
  Serialize(s, tx.vin);
  Serialize(s, tx.vout);
  if (flags & 1) {
    for (size_t i = 0; i < tx.vin.size(); i++) {
      Serialize(s, tx.vin[i].scriptWitness);
    }
  }
  Serialize(s, tx.nLockTime);
}

}  // namespace btc

}  // namespace altintegration

#endif