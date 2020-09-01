![CI](https://github.com/VeriBlock/alt-integration-cpp/workflows/CI/badge.svg)
![Coverage](https://codecov.io/gh/veriblock/alt-integration-cpp/branch/master/graphs/badge.svg?branch=master)

This repository contains a C++11 `veriblock-pop-cpp` library - an implementation of VeriBlock Proof-of-Proof ("POP") consensus protocol for prevention of 51% attacks.

POP overall consists of 2 protocols:
- POP Fork Resolution - given set of POP publications sent to Altchain from 2 chains, select best chain among these two, based on timeliness of publications in VBK tree, number of publications and other factors.
- POP Rewards - an economic incentive for POP miners to send endorsing VBK transactions in VBK blockchain, and sending back proofs to Altchain. POP Rewards are paid in Altchain coins.

## Getting Started 

1. [Build And Install](./docs/build.md)
2. [Data Structures](./docs/data_structures.md)
3. [pypopminer](./docs/pypopminer.md)
