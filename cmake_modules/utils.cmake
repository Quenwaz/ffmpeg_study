
function(GetCurrentFolderName FolderName)
    # remove last end of "/"
    string(REGEX REPLACE "/$" "" CURRENT_FOLDER_ABSOLUTE ${CMAKE_CURRENT_SOURCE_DIR})
    # get current relative dir name and set target name
    string(REGEX REPLACE ".*/(.*)" "\\1" FolderName ${CURRENT_FOLDER_ABSOLUTE})
    set(FolderName ${FolderName} PARENT_SCOPE)
    unset(CURRENT_FOLDER_ABSOLUTE)
endfunction(GetCurrentFolderName)
