set(html_files
    about.html
)

set(data_dirs
    mcu
)

install(DIRECTORY  ${data_dirs}  DESTINATION share/${CMAKE_PROJECT_NAME})
install(FILES      ${html_files} DESTINATION share/${CMAKE_PROJECT_NAME})
