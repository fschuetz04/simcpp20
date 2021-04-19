set(Targets
    bank
    carwash
    clocks
    ping-pong
    value-event
    value-process)

foreach(Target ${Targets})
    add_executable(${Target} ${Target}.cpp)
    target_link_libraries(${Target} PRIVATE simcpp20)
endforeach()
