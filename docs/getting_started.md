@mainpage Getting Started

This repository contains a C++11 `veriblock-pop-cpp` library - an implementation of VeriBlock Proof-of-Proof ("POP") consensus protocol for prevention of 51% attacks.

POP overall consists of 2 protocols:
- POP Fork Resolution - given set of POP publications sent to Altchain from 2 chains, select best chain among these two, based on timeliness of publications in VBK tree, number of publications and other factors.
- POP Rewards - an economic incentive for POP miners to send endorsing VBK transactions in VBK blockchain, and sending back proofs to Altchain. POP Rewards are paid in Altchain coins.


#### Guides:

1. @subpage build
2. @subpage description
3. @subpage integration
4. @subpage rebootstrap
