vcpkg_from_git(
        OUT_SOURCE_PATH SOURCE_PATH
        URL "https://git.code.sf.net/p/qwt/git"
        REF "907846e0e981b216349156ee83b13208faae2934"
        FETCH_REF qwt-6.2
)

file(COPY_FILE ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt ${SOURCE_PATH}/CMakeLists.txt)
#file(COPY_FILE ${CMAKE_CURRENT_LIST_DIR}/qwt-config.cmake ${SOURCE_PATH}/qwt-config.cmake)
file(COPY_FILE ${CMAKE_CURRENT_LIST_DIR}/QwtConfig.cmake.in ${SOURCE_PATH}/QwtConfig.cmake.in)

vcpkg_configure_cmake(
        SOURCE_PATH ${SOURCE_PATH}
        PREFER_NINJA
)

vcpkg_install_cmake()

vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/Qwt)

set(Qwt_DIR ${CURRENT_PACKAGES_DIR}/debug/share/qwt)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

