#
# Accumulate flags
#
# https://github.com/simdjson/simdjson/blob/master/cmake/simdjson-props.cmake

set(props_script "${PROJECT_BINARY_DIR}/props.cmake")
set(props_content "")
set(props_flushed NO)

function(add_props command)
  set(args "")
  math(EXPR limit "${ARGC} - 1")
  foreach(i RANGE 1 "${limit}")
    set(value "${ARGV${i}}")
    if(value MATCHES "^(PRIVATE|PUBLIC)$")
      string(TOLOWER "${value}" value)
      set(value "\${${value}}")
    else()
      set(value "[==[${value}]==]")
    endif()
    string(APPEND args " ${value}")
  endforeach()

  set(props_flushed NO PARENT_SCOPE)
  set(
      props_content
      "${props_content}${command}(\"\${target}\"${args})\n"
      PARENT_SCOPE
  )
endfunction()

macro(flush_props)
  if(NOT props_flushed)
    set(props_flushed YES PARENT_SCOPE)
    file(WRITE "${props_script}" "${props_content}")
  endif()
endmacro()

function(apply_props target)
  set(private PRIVATE)
  set(public PUBLIC)
  get_target_property(TYPE "${target}" TYPE)
  if(TYPE STREQUAL "INTERFACE_LIBRARY")
    set(private INTERFACE)
    set(public INTERFACE)
  endif()

  flush_props()
  include("${props_script}")
endfunction()