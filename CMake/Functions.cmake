function(get_source_files subdir outputVariable)
    file(GLOB_RECURSE source_files CONFIGURE_DEPENDS
        "${subdir}/*.cpp"
        "${subdir}/*.h"
        "${subdir}/*.inl"
    )

    set(PATH_SEPARATOR "/")

    if(WIN32)
        list(FILTER source_files EXCLUDE REGEX "${subdir}(/.*)?/macos/.*")
        list(FILTER source_files EXCLUDE REGEX "${subdir}(/.*)?/linux/.*")
    elseif(LINUX)
        list(FILTER source_files EXCLUDE REGEX "${subdir}(/.*)?/macos/.*")
        list(FILTER source_files EXCLUDE REGEX "${subdir}(/.*)?/win/.*")
    elseif(APPLE)
        file(GLOB_RECURSE apple_source_files CONFIGURE_DEPENDS
                "${subdir}/*.mm"
                "${subdir}/*.swift"
        )

        list(APPEND source_files ${apple_source_files})
        list(FILTER source_files EXCLUDE REGEX "${subdir}(/.*)?/win/.*")
        list(FILTER source_files EXCLUDE REGEX "${subdir}(/.*)?/linux/.*")
    endif()

    source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}${PATH_SEPARATOR}${subdir}" FILES ${source_files})
    set(${outputVariable} ${source_files} PARENT_SCOPE)
endfunction()
