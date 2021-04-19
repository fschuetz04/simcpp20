set(Targets
    all_of
    any_of
    bank
    carwash
    clocks
    ping_pong
    value_event
    value_process)

foreach(Target ${Targets})
    add_executable(${Target} ${Target}.cpp)
    target_link_libraries(${Target} PRIVATE simcpp20)
endforeach()
