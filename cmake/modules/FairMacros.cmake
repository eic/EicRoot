  
  ###############################################################
  #
  # Exchange file extention of LIST from
  # FILE_EXT1 to FILE_EXT2 and assign the
  # newly created list to OUTVAR. The input
  # list LIST is not changed at all
  # Ex: CHANGE_FILE_EXTENSION(*.cxx *.h TRD_HEADERS "${TRD_SRCS}")
  #
  ################################################################

MACRO (CHANGE_FILE_EXTENSION FILE_EXT1 FILE_EXT2 OUTVAR LIST)

   SET(BLA)

   IF (${FILE_EXT1} MATCHES "^[*][.]+.*$")
     STRING(REGEX REPLACE "^[*]+([.].*)$" "\\1" FILE_EXT1_NEW ${FILE_EXT1}) 
   ENDIF  (${FILE_EXT1} MATCHES "^[*][.]+.*$")

   IF (${FILE_EXT2} MATCHES "^[*][.]+.*$")
     STRING(REGEX REPLACE "^[*]+([.].*)" "\\1" FILE_EXT2_NEW ${FILE_EXT2}) 
   ENDIF  (${FILE_EXT2} MATCHES "^[*][.]+.*$")

   foreach (_current_FILE ${LIST})

     STRING(REGEX REPLACE "^(.*)${FILE_EXT1_NEW}$" "\\1${FILE_EXT2_NEW}" test ${_current_FILE})
     SET (BLA ${BLA} ${test})

   endforeach (_current_FILE ${ARGN})
   
   SET (${OUTVAR} ${BLA})

ENDMACRO (CHANGE_FILE_EXTENSION)

# -------------------------------------------------------------------------------------------------

MACRO(UNIQUE var_name list)

  #######################################################################
  # Make the given list have only one instance of each unique element and
  # store it in var_name.
  #######################################################################

  SET(unique_tmp "")
  FOREACH(l ${list})
    STRING(REGEX REPLACE "[+]" "\\\\+" l1 ${l})
    IF(NOT "${unique_tmp}" MATCHES "(^|;)${l1}(;|$)")
      SET(unique_tmp ${unique_tmp} ${l})
    ENDIF(NOT "${unique_tmp}" MATCHES "(^|;)${l1}(;|$)")
  ENDFOREACH(l)
  SET(${var_name} ${unique_tmp})
ENDMACRO(UNIQUE)

# -------------------------------------------------------------------------------------------------

Macro (SetBasicVariables)
  Set(BASE_INCLUDE_DIRECTORIES
      ${ROOT_INCLUDE_DIR}
      ${CMAKE_SOURCE_DIR}/fairtools
      ${CMAKE_SOURCE_DIR}/geobase
      ${CMAKE_SOURCE_DIR}/parbase
      ${CMAKE_SOURCE_DIR}/base
      ${CMAKE_SOURCE_DIR}/dbase
      ${CMAKE_SOURCE_DIR}/dbase/dbInterface
      ${CMAKE_SOURCE_DIR}/dbase/dbValidation
      ${CMAKE_SOURCE_DIR}/dbase/dbUtils 
  )  

EndMacro (SetBasicVariables)

# -------------------------------------------------------------------------------------------------

MACRO (CONVERT_LIST_TO_STRING)

  set (tmp "")
  foreach (_current ${ARGN})

    set(tmp1 ${tmp})
    set(tmp "")
    set(tmp ${tmp1}:${_current})

  endforeach (_current ${ARGN})
  If(tmp)
    STRING(REGEX REPLACE "^:(.*)" "\\1" output ${tmp}) 
  Else(tmp)
    Set(output "")
  EndIf(tmp)

ENDMACRO (CONVERT_LIST_TO_STRING LIST)

# -------------------------------------------------------------------------------------------------