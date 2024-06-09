find_package(PkgConfig QUIET)
pkg_check_modules(LibNL libnl-3.0 libnl-genl-3.0 libnl-route-3.0)

find_path(LibNL_INCLUDE_DIR
    PATH_SUFFIXES
        include/libnl3
        include
    NAMES
        netlink/netlink.h
    PATHS
        /usr
        /usr/local
        /opt
    HINTS
        "${LibNL_libnl-3.0_INCLUDEDIR}"
)

find_library(LibNL_LIBRARY NAMES nl nl-3 HINTS "${LibNL_libnl-3.0_LIBDIR}")
find_library(LibNL_ROUTE_LIBRARY NAMES nl-route nl-route-3 HINTS "${LibNL_libnl-3.0_LIBDIR}")
find_library(LibNL_NETFILTER_LIBRARY NAMES nl-nf nl-nf-3 HINTS "${LibNL_libnl-3.0_LIBDIR}")
find_library(LibNL_GENL_LIBRARY NAMES nl-genl nl-genl-3 HINTS "${LibNL_libnl-3.0_LIBDIR}")

if (LibNL_INCLUDE_DIR AND LibNL_LIBRARY)
    set(LibNL_FOUND TRUE)
endif()

if (LibNL_FOUND)
    set(LibNL_LIBRARIES ${LibNL_LIBRARY} ${LibNL_ROUTE_LIBRARY} ${LibNL_NETFILTER_LIBRARY} ${LibNL_GENL_LIBRARY})
    message("Found netlink libraries:  ${LibNL_LIBRARIES}")
    message("Found netlink include directories: ${LibNL_INCLUDE_DIR}")
else()
    if (LibNL_FIND_REQUIRED)
        message("Netlink version 3 development packages cannot be found.")
        message("In Debian-based, they may be called:")
        message("libnl-3-dev libnl-genl-3dev libnl-nf-3-dev libnl-route-3-dev")
        message(FATAL_ERROR "Could not find netlink library.")
    endif()
endif()
