project (stats)
INCLUDE(GNUInstallDirs)

set(SOURCES
        main.cpp
        traversal.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME cryfs-stats)
target_link_libraries(${PROJECT_NAME} PUBLIC cryfs cpp-utils gitversion)
target_enable_style_warnings(${PROJECT_NAME})
target_activate_cpp14(${PROJECT_NAME})

#add_executable(${PROJECT_NAME}_bin main.cpp)
#set_target_properties(${PROJECT_NAME}_bin PROPERTIES OUTPUT_NAME cryfs-stats)
#target_enable_style_warnings(${PROJECT_NAME}_bin)
#target_activate_cpp14(${PROJECT_NAME}_bin)
#
#install(TARGETS ${PROJECT_NAME}_bin
#        CONFIGURATIONS Debug Release RelWithDebInfo
#        DESTINATION ${CMAKE_INSTALL_BINDIR}
#)
