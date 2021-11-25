#include <cstring>
#include <veriblock/abfi/bitcoin/messages.hpp>
#include <veriblock/abfi/bitcoin/util.hpp>
#include <veriblock/pop/assert.hpp>
#include <veriblock/pop/blockchain/btc_chain_params.hpp>
#include <veriblock/pop/hashutil.hpp>
#include <veriblock/pop/write_stream.hpp>

namespace altintegration {
namespace abfi {
namespace btc {

CSipHasher GetDeterministicRandomizer(uint64_t seed0,
                                      uint64_t seed1,
                                      uint64_t id) {
  return CSipHasher(seed0, seed1).Write(id);
}

std::vector<uint8_t> Serialize(const BtcChainParams& params,
                               const char* command,
                               const std::vector<uint8_t>& body) {
  MessageHeader header;
  header.payloadSize = body.size();
  // we never send more bytes than max allowed by a protocol
  VBK_ASSERT(header.payloadSize <= MessageHeader::MAX_SIZE);
  header.magic = params.getPchMessageStart();

  // fill command
  int len = strnlen(command, MessageHeader::MAX_COMMAND_SIZE);
  std::copy(command, command + len, header.command);

  // calculate checksum
  // auto checksum = sha256twice({body.data(), body.size()}).template trim<4>();
  // header.checksum;

  WriteStream stream(MessageHeader::HEADER_SIZE + body.size());
  Serialize(stream, header);
  stream.write(body);
  return stream.data();
}

}  // namespace btc
}  // namespace abfi
}  // namespace altintegration
