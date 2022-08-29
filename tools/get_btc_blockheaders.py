#!/usr/bin/env python3
# Copyright (c) 2019-2022 Xenios SEZC
# https://www.veriblock.org
# Distributed under the MIT software license, see the accompanying
# file LICENSE or http://www.opensource.org/licenses/mit-license.php.

import requests
import time

filename = 'btc_blockheaders'
file = open(filename, 'w')

nBlocks = 10000

start_block = 30000
file.write(str(start_block) + '\n')

session = requests.Session()
tic = time.perf_counter()
for i in range(nBlocks):
    response = session.get('https://blockchain.info/block/%d?format=hex' % (i + start_block))
    if response.ok:
        file.write(response.text[:160] + '\n')

toc = time.perf_counter()
print(f"time: {toc - tic:0.4f} seconds")

session.close()
file.close()
