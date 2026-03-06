
# Enable target-specific compiler launcher behavior
cmake_policy(SET CMP0141 NEW)

# === Build Caching Integration ===
# This section checks for and integrates with build caching tools.
# ccache is the classic choice, while sccache is more modern and supports
# remote/cloud-based caching.

if(ENABLE_BUILD_CACHE)
    # Check for sccache (preferable for remote caching)
    find_program(SCCACHE_EXECUTABLE sccache)
    if(SCCACHE_EXECUTABLE)
        message(STATUS "Found sccache: ${SCCACHE_EXECUTABLE}. Enabling build caching.")
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${SCCACHE_EXECUTABLE}")
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${SCCACHE_EXECUTABLE}")
    else()
        # Fallback to ccache if sccache is not found
        find_program(CCACHE_EXECUTABLE ccache)
        if(CCACHE_EXECUTABLE)
            message(STATUS "Found ccache: ${CCACHE_EXECUTABLE}. Enabling build caching.")
            set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_EXECUTABLE}")
        else()
            message(WARNING "Neither sccache nor ccache found. Build caching will not be used.")
        endif()
    endif()
endif()
