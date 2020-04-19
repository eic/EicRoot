MACRO (WRITE_CONFIG_FILE filename)

  SET(VMCWORKDIR ${CMAKE_SOURCE_DIR})
  #SET(VMCWORKDIR ${CMAKE_INSTALL_PREFIX}/share/fairbase) 
  
  # Nothing to add to PATH; just use a chance to remove duplicated;
  STRING(REGEX MATCHALL "[^:]+" PATH $ENV{PATH})
  UNIQUE(PATH "${PATH}")
  CONVERT_LIST_TO_STRING(${PATH})
  Set(MY_PATH ${output})

  #export LD_LIBRARY_PATH="@MY_LD_LIBRARY_PATH@:@EICSMEAR@/build:@OPENCASCADE@/lib"
  # Start with the existing LD_LIBRARY_PATH; add whatever needed; remove duplicates;
  SET( LD_LIBRARY_PATH ${CMAKE_BINARY_DIR}/lib:$ENV{LD_LIBRARY_PATH} )
  if (DEFINED GEANT3)
    SET( LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:${GEANT3}/lib64 )
  endif()
  if (DEFINED EICSMEAR)
    SET( LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:${EICSMEAR}/lib )
  endif()
  STRING(REGEX MATCHALL "[^:]+" LD_LIBRARY_PATH ${LD_LIBRARY_PATH})
  UNIQUE(LD_LIBRARY_PATH "${LD_LIBRARY_PATH}")
  CONVERT_LIST_TO_STRING(${LD_LIBRARY_PATH})
  Set(MY_LD_LIBRARY_PATH ${output})
  #message(${MY_LD_LIBRARY_PATH})	
  
  IF(${filename} MATCHES "[.]csh.*$")
    configure_file(${PROJECT_SOURCE_DIR}/cmake/scripts/config.csh.in
	           ${CMAKE_CURRENT_BINARY_DIR}/${filename}
                  )
  ELSE(${filename} MATCHES "[.]csh.*$")
    configure_file(${PROJECT_SOURCE_DIR}/cmake/scripts/config.sh.in
	           ${CMAKE_CURRENT_BINARY_DIR}/${filename}
                  )
  ENDIF(${filename} MATCHES "[.]csh.*$")

ENDMACRO (WRITE_CONFIG_FILE)
