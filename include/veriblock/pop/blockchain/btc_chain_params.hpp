// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BTC_CHAIN_PARAMS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BTC_CHAIN_PARAMS_HPP_

#include <string>
#include <vector>
#include <veriblock/pop/entities/btcblock.hpp>
#include <veriblock/pop/serde.hpp>
#include <veriblock/pop/strutil.hpp>
#include <veriblock/pop/uint.hpp>
#include <veriblock/pop/validation_state.hpp>

namespace altintegration {

//! base class for BTC params
struct BtcChainParams {
  virtual ~BtcChainParams() = default;
  virtual uint256 getPowLimit() const = 0;
  virtual uint32_t getPowTargetTimespan() const noexcept = 0;
  virtual uint32_t getPowTargetSpacing() const noexcept = 0;
  virtual bool getAllowMinDifficultyBlocks() const noexcept = 0;
  virtual bool getPowNoRetargeting() const noexcept = 0;
  //  virtual BtcBlock getGenesisBlock() const noexcept = 0;
  virtual bool EnableTimeAdjustment() const noexcept = 0;
  uint32_t getDifficultyAdjustmentInterval() const noexcept {
    return getPowTargetTimespan() / getPowTargetSpacing();
  }
  //! minimum number of BTC blocks needed to bootstrap chain
  virtual uint32_t numBlocksForBootstrap() const noexcept = 0;
  virtual const char* networkName() const noexcept = 0;
  virtual uint32_t maxFutureBlockTime() const noexcept {
    return mMaxFutureBlockTime;
  }

  //! by default we store this many last BTC blocks in RAM
  int32_t getMaxReorgBlocks() const noexcept {
    VBK_ASSERT(static_cast<uint32_t>(mMaxReorgBlocks) >=
               getDifficultyAdjustmentInterval());
    return mMaxReorgBlocks;
  }

  virtual std::vector<std::string> getDnsSeeds() const noexcept = 0;
  virtual std::vector<std::string> getFixedSeeds() const noexcept = 0;
  virtual std::array<uint8_t, 4> getPchMessageStart() const noexcept = 0;
  virtual uint16_t getP2pPort() const noexcept = 0;

  //! when finalizeBlockImpl is called, this many blocks behind final block will
  //! be preserved in RAM. In BTC we can deallocate all blocks past final block.
  uint32_t preserveBlocksBehindFinal() const noexcept { return 0; }

  //! all blocks further than this number of blocks are considered "old"
  int32_t getOldBlocksWindow() const noexcept { return mOldBlocksWindow; }

  uint32_t mOldBlocksWindow = 1000;
  int32_t mMaxReorgBlocks = BTC_MAX_REORG_BLOCKS_MIN_VALUE;

 protected:
  uint32_t mMaxFutureBlockTime = 2 * 60 * 60;  // 2 hours
};

/**
 * @struct BtcChainParamsMain
 *
 * mainnet network params in Bitcoin chain.
 *
 */
struct BtcChainParamsMain : public BtcChainParams {
  ~BtcChainParamsMain() override = default;

  std::vector<std::string> getDnsSeeds() const noexcept override {
    std::vector<std::string> vSeeds;
    // clang-format off
    vSeeds.emplace_back("seed.bitcoin.sipa.be."); // Pieter Wuille, only supports x1, x5, x9, and xd
    vSeeds.emplace_back("dnsseed.bluematt.me."); // Matt Corallo, only supports x9
    vSeeds.emplace_back("dnsseed.bitcoin.dashjr.org."); // Luke Dashjr
    vSeeds.emplace_back("seed.bitcoinstats.com."); // Christian Decker, supports x1 - xf
    vSeeds.emplace_back("seed.bitcoin.jonasschnelli.ch."); // Jonas Schnelli, only supports x1, x5, x9, and xd
    vSeeds.emplace_back("seed.btc.petertodd.org."); // Peter Todd, only supports x1, x5, x9, and xd
    vSeeds.emplace_back("seed.bitcoin.sprovoost.nl."); // Sjors Provoost
    vSeeds.emplace_back("dnsseed.emzy.de."); // Stephan Oeste
    vSeeds.emplace_back("seed.bitcoin.wiz.biz."); // Jason Maurice
    // clang-format on
    return vSeeds;
  }

  std::vector<std::string> getFixedSeeds() const noexcept override {
    // https://github.com/bitcoin/bitcoin/blob/master/contrib/seeds/nodes_main.txt
    return std::vector<std::string>{
        "2.39.173.126:8333",     "3.14.168.201:48333",
        "4.36.112.44:8333",      "5.8.18.31:8333",
        "5.14.200.167:8333",     "5.56.20.2:8333",
        "5.102.146.99:8333",     "5.103.137.146:9333",
        "5.128.87.126:8333",     "5.133.65.82:8333",
        "5.187.55.242:8333",     "5.188.62.24:8333",
        "5.188.62.33:8333",      "5.199.133.193:8333",
        "8.38.89.152:8333",      "13.231.20.249:8333",
        "18.27.79.17:8333",      "20.184.15.116:8433",
        "23.28.205.97:8333",     "23.106.252.230:8333",
        "23.175.0.202:8333",     "23.175.0.212:8333",
        "23.241.250.252:8333",   "23.245.24.154:8333",
        "24.86.184.66:8333",     "24.116.246.9:8333",
        "24.141.34.166:8333",    "24.155.196.246:8333",
        "24.157.130.222:8333",   "24.188.176.255:8333",
        "24.237.70.53:8333",     "27.124.4.67:8333",
        "31.17.70.80:8333",      "31.21.8.32:8333",
        "31.45.118.10:8333",     "31.132.17.56:8333",
        "31.134.121.223:8333",   "32.214.183.114:8333",
        "35.137.236.32:8333",    "35.185.145.105:8333",
        "35.209.51.212:8333",    "35.245.175.76:8333",
        "37.116.95.41:8333",     "37.143.9.107:8333",
        "37.143.116.43:8333",    "37.191.244.149:8333",
        "37.211.78.253:8333",    "37.221.209.222:24333",
        "37.228.92.110:8333",    "43.225.62.107:8333",
        "43.225.157.152:8333",   "45.36.184.6:8333",
        "45.48.168.16:8333",     "45.85.85.8:8333",
        "45.85.85.9:8333",       "45.129.180.214:8333",
        "45.149.78.128:8333",    "45.151.125.218:8333",
        "45.154.255.46:8333",    "45.155.157.239:8333",
        "46.28.132.34:8333",     "46.28.204.21:8333",
        "46.32.50.98:8333",      "46.59.13.35:8333",
        "46.128.40.173:8333",    "46.128.140.193:8333",
        "46.146.248.89:8333",    "46.166.162.45:20001",
        "46.188.15.6:8333",      "46.229.165.142:8333",
        "46.229.238.187:8333",   "46.249.83.82:8333",
        "46.254.217.169:8333",   "47.74.191.34:8333",
        "47.115.53.163:8333",    "47.187.26.135:8333",
        "47.222.103.234:8333",   "47.253.5.99:8333",
        "49.232.82.76:8333",     "49.247.215.43:8333",
        "50.2.13.166:8333",      "50.34.39.72:8333",
        "50.45.232.189:8333",    "50.68.104.92:8333",
        "51.68.36.57:8333",      "51.154.60.34:8333",
        "52.169.238.66:8333",    "54.197.30.223:8333",
        "54.227.66.57:8333",     "58.158.0.86:8333",
        "58.171.135.242:8333",   "58.229.208.158:8333",
        "60.244.109.19:8333",    "62.38.75.208:8333",
        "62.74.143.11:8333",     "62.80.227.49:8333",
        "62.152.58.16:9421",     "62.210.167.199:8333",
        "62.234.188.160:8333",   "62.251.54.163:8333",
        "63.227.116.162:8333",   "65.19.155.82:8333",
        "65.95.49.102:8333",     "66.18.172.21:8333",
        "66.240.237.155:8333",   "67.210.228.203:8333",
        "69.30.215.42:8333",     "69.59.18.206:8333",
        "69.64.33.71:8333",      "69.119.193.9:8333",
        "69.209.23.72:8333",     "70.123.125.237:8333",
        "70.185.56.136:8333",    "71.38.90.235:8333",
        "72.12.73.70:8333",      "72.53.134.182:8333",
        "72.225.7.80:8333",      "72.234.182.39:8333",
        "72.250.184.57:8333",    "73.83.103.79:8333",
        "74.118.137.119:8333",   "74.133.100.74:8333",
        "74.215.219.214:8333",   "74.220.255.190:8333",
        "75.158.39.231:8333",    "77.53.53.196:8333",
        "77.70.16.245:8333",     "77.105.87.97:8333",
        "77.120.113.69:8433",    "77.120.122.22:8433",
        "77.166.83.167:8333",    "77.247.178.130:8333",
        "78.27.139.13:8333",     "78.63.28.146:8333",
        "78.83.103.4:8333",      "78.141.123.99:8333",
        "79.77.33.131:8333",     "79.77.133.30:8333",
        "79.101.1.25:8333",      "79.117.192.229:8333",
        "79.133.228.55:8333",    "79.146.21.163:8333",
        "80.89.203.172:8001",    "80.93.213.246:8333",
        "80.192.98.110:8334",    "80.229.28.60:8333",
        "80.232.247.210:8333",   "80.242.39.76:8333",
        "80.253.94.252:8333",    "81.0.198.25:8333",
        "81.7.13.84:8333",       "81.117.225.245:8333",
        "81.135.137.225:8333",   "81.171.22.143:8333",
        "81.191.233.134:8333",   "81.232.78.75:8333",
        "81.242.91.23:8333",     "82.29.58.109:8333",
        "82.136.99.22:8333",     "82.149.97.25:17567",
        "82.165.19.48:8333",     "82.194.153.233:8333",
        "82.197.215.125:8333",   "82.199.102.10:8333",
        "82.200.205.30:8333",    "82.202.68.231:8333",
        "82.221.128.31:8333",    "82.228.6.131:8333",
        "83.85.139.94:8333",     "83.99.245.20:8333",
        "83.137.41.10:8333",     "83.174.209.87:8333",
        "83.217.8.31:44420",     "84.38.3.249:8333",
        "84.38.185.122:8333",    "84.92.92.247:8333",
        "84.192.16.234:8333",    "84.194.158.124:8333",
        "84.212.145.24:8333",    "84.212.244.95:8333",
        "84.216.51.36:8333",     "84.255.249.163:8333",
        "85.25.255.147:8333",    "85.70.156.209:8333",
        "85.145.142.46:8333",    "85.170.233.95:8333",
        "85.184.138.108:8333",   "85.190.0.5:8333",
        "85.191.200.51:8333",    "85.192.191.6:18500",
        "85.194.238.131:8333",   "85.195.54.110:8333",
        "85.214.161.252:8333",   "85.214.185.51:8333",
        "85.241.106.203:8333",   "85.246.168.252:8333",
        "86.56.238.247:8333",    "87.61.90.230:8333",
        "87.79.68.86:8333",      "87.79.94.221:8333",
        "87.120.8.5:20008",      "87.246.46.132:8333",
        "87.247.111.222:8333",   "88.84.222.252:8333",
        "88.86.243.241:8333",    "88.87.93.52:1691",
        "88.119.197.200:8333",   "88.129.253.94:8333",
        "88.147.244.250:8333",   "88.208.3.195:8333",
        "88.212.44.33:8333",     "88.214.57.95:8333",
        "89.106.199.38:8333",    "89.108.126.228:8333",
        "89.115.120.43:8333",    "89.133.68.65:8333",
        "89.190.19.162:8333",    "89.248.172.10:8333",
        "90.146.153.21:8333",    "90.182.165.18:8333",
        "91.106.188.229:8333",   "91.193.237.116:8333",
        "91.204.99.178:8333",    "91.204.149.5:8333",
        "91.214.70.63:8333",     "91.228.152.236:8333",
        "92.12.154.115:8333",    "92.249.143.44:8333",
        "93.12.66.98:8333",      "93.46.54.4:8333",
        "93.115.20.130:8333",    "93.123.180.164:8333",
        "93.189.145.169:8333",   "93.241.228.102:8333",
        "94.19.7.55:8333",       "94.19.128.204:8333",
        "94.52.112.227:8333",    "94.154.96.130:8333",
        "94.156.174.201:8333",   "94.158.246.183:8333",
        "94.177.171.73:8333",    "94.199.178.233:8100",
        "94.237.125.30:8333",    "94.247.134.77:8333",
        "95.48.228.45:8333",     "95.69.249.63:8333",
        "95.82.146.70:8333",     "95.83.73.31:8333",
        "95.84.164.43:8333",     "95.87.226.56:8333",
        "95.110.234.93:8333",    "95.163.71.126:8333",
        "95.164.65.194:8333",    "95.174.66.211:8333",
        "95.211.174.137:8333",   "95.216.11.156:8433",
        "96.47.114.108:8333",    "97.84.232.105:8333",
        "97.99.205.241:8333",    "98.25.193.114:8333",
        "99.115.25.13:8333",     "101.32.19.184:8333",
        "101.100.174.240:8333",  "102.132.245.16:8333",
        "103.14.244.190:8333",   "103.76.48.5:8333",
        "103.84.84.250:8335",    "103.99.168.150:8333",
        "103.109.101.216:8333",  "103.122.247.102:8333",
        "103.129.13.45:8333",    "103.198.192.14:20008",
        "103.224.119.99:8333",   "103.231.191.7:8333",
        "103.235.230.196:8333",  "104.171.242.155:8333",
        "104.238.220.199:8333",  "106.163.158.127:8333",
        "107.150.41.179:8333",   "107.159.93.103:8333",
        "108.183.77.12:8333",    "109.9.175.65:8333",
        "109.99.63.159:8333",    "109.110.81.90:8333",
        "109.123.213.130:8333",  "109.134.232.81:8333",
        "109.169.20.168:8333",   "109.199.241.148:8333",
        "109.229.210.6:8333",    "109.236.105.40:8333",
        "109.248.206.13:8333",   "111.42.74.65:8333",
        "111.90.140.179:8333",   "112.215.205.236:8333",
        "113.52.135.125:8333",   "114.23.246.137:8333",
        "115.47.141.250:8885",   "115.70.110.4:8333",
        "116.34.189.55:8333",    "118.103.126.140:28333",
        "118.189.187.219:8333",  "119.3.208.236:8333",
        "119.8.47.225:8333",     "119.17.151.61:8333",
        "120.25.24.30:8333",     "120.241.34.10:8333",
        "121.98.205.100:8333",   "122.112.148.153:8339",
        "122.116.42.140:8333",   "124.217.235.180:8333",
        "125.236.215.133:8333",  "129.13.189.212:8333",
        "130.185.77.105:8333",   "131.188.40.191:8333",
        "131.193.220.15:8333",   "135.23.124.239:8333",
        "136.33.185.32:8333",    "136.56.170.96:8333",
        "137.226.34.46:8333",    "138.229.26.42:8333",
        "139.9.249.234:8333",    "141.101.8.36:8333",
        "143.176.224.104:8333",  "144.2.69.224:8333",
        "144.34.161.65:18333",   "144.91.116.44:8333",
        "144.137.29.181:8333",   "148.66.50.50:8335",
        "148.72.150.231:8333",   "148.170.212.44:8333",
        "149.167.99.190:8333",   "154.92.16.191:8333",
        "154.221.27.21:8333",    "156.19.19.90:8333",
        "156.241.5.190:8333",    "157.13.61.76:8333",
        "157.13.61.80:8333",     "157.230.166.98:14391",
        "158.75.203.2:8333",     "158.181.125.150:8333",
        "158.181.226.33:8333",   "159.100.242.254:8333",
        "159.100.248.234:8333",  "159.138.87.18:8333",
        "160.16.0.30:8333",      "162.0.227.54:8333",
        "162.0.227.56:8333",     "162.62.18.226:8333",
        "162.209.1.233:8333",    "162.243.175.86:8333",
        "162.244.80.208:8333",   "162.250.188.87:8333",
        "162.250.189.53:8333",   "163.158.202.112:8333",
        "163.158.243.230:8333",  "165.73.62.31:8333",
        "166.62.82.103:32771",   "166.70.94.106:8333",
        "167.86.90.239:8333",    "169.44.34.203:8333",
        "172.93.101.73:8333",    "172.105.7.47:8333",
        "173.23.103.30:8000",    "173.53.79.6:8333",
        "173.70.12.86:8333",     "173.89.28.137:8333",
        "173.176.184.54:8333",   "173.208.128.10:8333",
        "173.254.204.69:8333",   "173.255.204.124:8333",
        "174.94.155.224:8333",   "174.114.102.41:8333",
        "174.114.124.12:8333",   "176.10.227.59:8333",
        "176.31.224.214:8333",   "176.74.136.237:8333",
        "176.99.2.207:8333",     "176.106.191.2:8333",
        "176.160.228.9:8333",    "176.191.182.3:8333",
        "176.212.185.153:8333",  "176.241.137.183:8333",
        "177.38.215.73:8333",    "178.16.222.146:8333",
        "178.132.2.246:8333",    "178.143.191.171:8333",
        "178.148.172.209:8333",  "178.148.226.180:8333",
        "178.150.96.46:8333",    "178.182.227.50:8333",
        "178.236.137.63:8333",   "178.255.42.126:8333",
        "180.150.52.37:8333",    "181.39.32.99:8333",
        "181.48.77.26:8333",     "181.52.223.52:8333",
        "181.238.51.152:8333",   "183.88.223.208:8333",
        "183.110.220.210:30301", "184.95.58.166:8336",
        "184.164.147.82:41333",  "184.171.208.109:8333",
        "185.25.48.39:8333",     "185.25.48.184:8333",
        "185.64.116.15:8333",    "185.80.219.132:8333",
        "185.85.3.140:8333",     "185.95.219.53:8333",
        "185.108.244.41:8333",   "185.134.233.121:8333",
        "185.145.128.21:8333",   "185.148.3.227:8333",
        "185.153.196.240:8333",  "185.158.114.184:8333",
        "185.165.168.196:8333",  "185.181.230.74:8333",
        "185.185.26.141:8111",   "185.186.208.162:8333",
        "185.189.132.178:57780", "185.211.59.50:8333",
        "185.233.148.146:8333",  "185.238.129.113:8333",
        "185.249.199.106:8333",  "185.251.161.54:8333",
        "187.189.153.136:8333",  "188.37.24.190:8333",
        "188.42.40.234:18333",   "188.61.46.36:8333",
        "188.68.45.143:8333",    "188.127.229.105:8333",
        "188.134.6.84:8333",     "188.134.8.36:8333",
        "188.214.129.65:20012",  "188.230.168.114:8333",
        "189.34.14.93:8333",     "189.207.46.32:8333",
        "190.211.204.68:8333",   "191.209.21.188:8333",
        "192.3.11.20:8333",      "192.3.185.210:8333",
        "192.65.170.15:8333",    "192.65.170.50:8333",
        "192.146.137.18:8333",   "192.157.202.178:8333",
        "192.227.80.83:8333",    "193.10.203.23:8334",
        "193.25.6.206:8333",     "193.42.110.30:8333",
        "193.58.196.212:8333",   "193.106.28.8:8333",
        "193.189.190.123:8333",  "193.194.163.35:8333",
        "193.194.163.53:8333",   "194.14.246.205:8333",
        "194.36.91.253:8333",    "194.126.113.135:8333",
        "194.135.135.69:8333",   "195.56.63.4:8333",
        "195.56.63.5:8333",      "195.67.139.54:8333",
        "195.135.194.8:8333",    "195.202.169.149:8333",
        "195.206.105.42:8333",   "195.209.249.164:8333",
        "198.1.231.6:8333",      "198.200.43.215:8333",
        "199.182.184.204:8333",  "199.247.7.208:8333",
        "199.247.249.188:8333",  "200.7.252.118:8333",
        "200.20.186.254:8333",   "200.83.166.136:8333",
        "202.55.87.45:8333",     "202.79.167.65:8333",
        "202.108.211.135:8333",  "202.169.102.73:8333",
        "203.130.48.117:8885",   "203.132.95.10:8333",
        "203.151.166.123:8333",  "204.93.113.108:8333",
        "204.111.241.195:8333",  "206.124.149.66:8333",
        "207.115.102.98:8333",   "207.229.46.150:8333",
        "208.76.252.198:8333",   "208.100.13.56:8333",
        "208.100.178.175:8333",  "208.110.99.105:8333",
        "209.6.210.179:8333",    "209.133.220.74:8333",
        "209.141.57.57:8333",    "211.27.147.67:8333",
        "212.34.225.118:8333",   "212.89.173.216:8333",
        "212.99.226.36:9020",    "212.237.96.98:8333",
        "213.89.131.53:8333",    "216.38.129.164:8333",
        "216.134.165.55:8333",   "216.146.251.8:8333",
        "216.189.190.95:8333",   "216.226.128.189:8333",
        "216.236.164.82:8333",   "217.19.216.210:8333",
        "217.26.32.10:8333",     "217.64.47.138:8333",
        "217.64.133.220:8333",   "217.92.55.246:8333",
        "218.31.113.245:8333",   "218.255.242.114:8333",
        "220.133.39.61:8333",    "223.16.30.175:8333"};
  }

  std::array<uint8_t, 4> getPchMessageStart() const noexcept override {
    return {0xf9, 0xbe, 0xb4, 0xd9};
  }

  uint16_t getP2pPort() const noexcept override { return 8333; }

  bool EnableTimeAdjustment() const noexcept override { return true; }

  const char* networkName() const noexcept override { return "main"; }

  uint32_t numBlocksForBootstrap() const noexcept override {
    return getDifficultyAdjustmentInterval();
  };

  uint256 getPowLimit() const override {
    return uint256::fromHex(
        "ffffffffffffffffffffffffffffffffffffffffffffffffffffffff00000000");
  }

  uint32_t getPowTargetTimespan() const noexcept override {
    return 14 * 24 * 60 * 60;  // two weeks
  }
  uint32_t getPowTargetSpacing() const noexcept override { return 10 * 60; }
  bool getAllowMinDifficultyBlocks() const noexcept override { return false; }
  bool getPowNoRetargeting() const noexcept override { return false; }
  //  BtcBlock getGenesisBlock() const noexcept override {
  //    BtcBlock block;
  //    block.version = 1;
  //    block.timestamp = 1231006505;
  //    block.nonce = 2083236893;
  //    block.bits = 0x1d00ffff;
  //    block.merkleRoot = uint256::fromHex(
  //        "4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b");
  //
  //    VBK_ASSERT(
  //        block.getHash() ==
  //        uint256::fromHex("000000000019d6689c085ae165831e934ff763ae46a2a6c172"
  //                         "b3f1b60a8ce26f"));
  //
  //    return block;
  //  }
};

/**
 * @struct BtcChainParamsTest
 *
 * testnet3 network params in Bitcoin chain.
 *
 */
struct BtcChainParamsTest : public BtcChainParams {
  ~BtcChainParamsTest() override = default;

  std::vector<std::string> getDnsSeeds() const noexcept override {
    std::vector<std::string> vSeeds;
    // clang-format off
    vSeeds.emplace_back("testnet-seed.bitcoin.jonasschnelli.ch.");
    vSeeds.emplace_back("seed.tbtc.petertodd.org.");
    vSeeds.emplace_back("seed.testnet.bitcoin.sprovoost.nl.");
    vSeeds.emplace_back("testnet-seed.bluematt.me."); // Just a static list of stable node(s), only supports x9
    // clang-format on
    return vSeeds;
  }

  std::vector<std::string> getFixedSeeds() const noexcept override {
    return {};
  }

  std::array<uint8_t, 4> getPchMessageStart() const noexcept override {
    return {0x0b, 0x11, 0x09, 0x07};
  }

  uint16_t getP2pPort() const noexcept override { return 18333; }

  bool EnableTimeAdjustment() const noexcept override { return true; }

  const char* networkName() const noexcept override { return "test"; }

  uint32_t numBlocksForBootstrap() const noexcept override {
    return getDifficultyAdjustmentInterval();
  };

  uint256 getPowLimit() const override {
    return uint256::fromHex(
        "ffffffffffffffffffffffffffffffffffffffffffffffffffffffff00000000");
  }

  uint32_t getPowTargetTimespan() const noexcept override {
    return 14 * 24 * 60 * 60;
  }
  uint32_t getPowTargetSpacing() const noexcept override { return 10 * 60; }
  bool getAllowMinDifficultyBlocks() const noexcept override { return true; }
  bool getPowNoRetargeting() const noexcept override { return false; }
  //  BtcBlock getGenesisBlock() const noexcept override {
  //    BtcBlock block;
  //    block.version = 1;
  //    block.timestamp = 1296688602;
  //    block.nonce = 414098458;
  //    block.bits = 0x1d00ffff;
  //    block.merkleRoot = uint256::fromHex(
  //        "4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b");
  //
  //    VBK_ASSERT(
  //        block.getHash() ==
  //        uint256::fromHex("000000000933ea01ad0ee984209779baaec3ced90fa3f40871"
  //                         "9526f8d77f4943"));
  //
  //    return block;
  //  }
};

/**
 * @struct BtcChainParamsRegTest
 *
 * regtest network params in Bitcoin chain.
 *
 */
struct BtcChainParamsRegTest : public BtcChainParams {
  ~BtcChainParamsRegTest() override = default;

  std::vector<std::string> getDnsSeeds() const noexcept override { return {}; }

  std::vector<std::string> getFixedSeeds() const noexcept override {
    return {};
  }

  uint16_t getP2pPort() const noexcept override { return 18444; }

  std::array<uint8_t, 4> getPchMessageStart() const noexcept override {
    return {0xfa, 0xbf, 0xb5, 0xda};
  }

  //! time adjustment is disabled in regtest mode
  bool EnableTimeAdjustment() const noexcept override { return false; }

  const char* networkName() const noexcept override { return "regtest"; }

  uint32_t numBlocksForBootstrap() const noexcept override { return 1; };

  uint256 getPowLimit() const override {
    return uint256::fromHex(
        "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff7f");
  }

  uint32_t getPowTargetTimespan() const noexcept override {
    return 14 * 24 * 60 * 60;
  }
  uint32_t getPowTargetSpacing() const noexcept override { return 10 * 60; }
  bool getAllowMinDifficultyBlocks() const noexcept override { return true; }
  bool getPowNoRetargeting() const noexcept override { return true; }
  //  BtcBlock getGenesisBlock() const noexcept override {
  //    BtcBlock block;
  //    block.version = 1;
  //    block.timestamp = 1296688602;
  //    block.nonce = 2;
  //    block.bits = 0x207fffff;
  //    block.merkleRoot = uint256::fromHex(
  //        "4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b");
  //
  //    VBK_ASSERT(
  //        block.getHash() ==
  //        uint256::fromHex("0f9188f13cb7b2c71f2a335e3a4fc328bf5beb436012afca59"
  //                         "0b1a11466e2206"));
  //
  //    return block;
  //  }
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BTC_CHAIN_PARAMS_HPP_
