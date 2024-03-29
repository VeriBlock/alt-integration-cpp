[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=alt-integration-cpp&metric=alert_status)](https://sonarcloud.io/summary/new_code?id=alt-integration-cpp)
![CI](https://github.com/VeriBlock/alt-integration-cpp/workflows/CI/badge.svg)
[![codecov](https://codecov.io/gh/VeriBlock/alt-integration-cpp/branch/master/graph/badge.svg?token=jMNSLO1LCr)](https://codecov.io/gh/VeriBlock/alt-integration-cpp)
![loc](https://tokei.rs/b1/github/VeriBlock/alt-integration-cpp?category=code)

This repository contains a C++11 `veriblock-pop-cpp` library - an implementation of VeriBlock Proof-of-Proof ("POP") consensus protocol for prevention of 51% attacks.

POP overall consists of 2 protocols:
- POP Fork Resolution - given set of POP publications sent to Altchain from 2 chains, select best chain among these two, based on timeliness of publications in VBK tree, number of publications and other factors.
- POP Rewards - an economic incentive for POP miners to send endorsing VBK transactions in VBK blockchain, and sending back proofs to Altchain. POP Rewards are paid in Altchain coins.

See https://veriblock.github.io/alt-integration-cpp/ for details.
