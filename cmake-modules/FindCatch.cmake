# provides:
#
# CATCH_FOUND
# CATCH_INCLUDE_DIR

find_path(CATCH_INCLUDE_DIR NAMES catch.hpp
    HINTS
    "${CATCH_ROOT}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Catch DEFAULT_MSG
                                  CATCH_INCLUDE_DIR)
