set(Targets
    bank
    carwash
    clocks)

foreach(Target ${Targets})
    add_executable(${Target} ${Target}.cpp)
    target_link_libraries(${Target} PRIVATE simcpp20)
endforeach()
