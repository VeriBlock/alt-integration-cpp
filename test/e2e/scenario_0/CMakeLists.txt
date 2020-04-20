gencpp(atv atv.txt atv)
gencpp(vtbs vtbs.txt vtbs)
gencpp(btcbootstraps btcbootstraps_start1717746_testnet.txt btcblocks)
gencpp(vbkbootstraps vbkbootstraps_start438198_testnet.txt vbkblocks)

addtest(scenario_0
        scenario_0.cpp
        ${atv}
        ${vtbs}
        ${btcblocks}
        ${vbkblocks}
        )
target_link_libraries(scenario_0 ${LIB_NAME})
