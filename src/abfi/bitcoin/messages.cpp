#include <cstdint>
#include <string>
#include <vector>
#include <veriblock/pop/abfi/bitcoin/messages.hpp>
namespace altintegration {
namespace abfi {
namespace btc {

const char *VERSION = "version";
const char *VERACK = "verack";
const char *ADDR = "addr";
const char *ADDRV2 = "addrv2";
const char *SENDADDRV2 = "sendaddrv2";
const char *INV = "inv";
const char *GETDATA = "getdata";
const char *MERKLEBLOCK = "merkleblock";
const char *GETBLOCKS = "getblocks";
const char *GETHEADERS = "getheaders";
const char *TX = "tx";
const char *HEADERS = "headers";
const char *BLOCK = "block";
const char *GETADDR = "getaddr";
const char *MEMPOOL = "mempool";
const char *PING = "ping";
const char *PONG = "pong";
const char *NOTFOUND = "notfound";
const char *FILTERLOAD = "filterload";
const char *FILTERADD = "filteradd";
const char *FILTERCLEAR = "filterclear";
const char *SENDHEADERS = "sendheaders";
const char *FEEFILTER = "feefilter";
const char *SENDCMPCT = "sendcmpct";
const char *CMPCTBLOCK = "cmpctblock";
const char *GETBLOCKTXN = "getblocktxn";
const char *BLOCKTXN = "blocktxn";
const char *GETCFILTERS = "getcfilters";
const char *CFILTER = "cfilter";
const char *GETCFHEADERS = "getcfheaders";
const char *CFHEADERS = "cfheaders";
const char *GETCFCHECKPT = "getcfcheckpt";
const char *CFCHECKPT = "cfcheckpt";
const char *WTXIDRELAY = "wtxidrelay";

const static std::string allNetMessageTypes[] = {
    VERSION,     VERACK,       ADDR,        ADDRV2,     SENDADDRV2,
    INV,         GETDATA,      MERKLEBLOCK, GETBLOCKS,  GETHEADERS,
    TX,          HEADERS,      BLOCK,       GETADDR,    MEMPOOL,
    PING,        PONG,         NOTFOUND,    FILTERLOAD, FILTERADD,
    FILTERCLEAR, SENDHEADERS,  FEEFILTER,   SENDCMPCT,  CMPCTBLOCK,
    GETBLOCKTXN, BLOCKTXN,     GETCFILTERS, CFILTER,    GETCFHEADERS,
    CFHEADERS,   GETCFCHECKPT, CFCHECKPT,   WTXIDRELAY,
};

const static std::vector<std::string> allNetMessageTypesVec(
    std::begin(allNetMessageTypes), std::end(allNetMessageTypes));

void Serialize(WriteStream &stream, const MessageHeader &header) {
  stream.write(header.magic);
  stream.write(header.command);
  stream.writeLE<uint32_t>(header.payloadSize);
  stream.write(header.checksum);
}

}  // namespace btc
}  // namespace abfi
}  // namespace altintegration