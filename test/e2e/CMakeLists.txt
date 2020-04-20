add_subdirectory(scenario_0)

addtest(e2e_test scenario_1.cpp
                 scenario_2.cpp
                 scenario_3.cpp
                 scenario_4.cpp
                 scenario_5.cpp
)
target_link_libraries(e2e_test ${LIB_NAME})
