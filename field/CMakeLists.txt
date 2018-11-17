
set(INCLUDE_DIRECTORIES
${ROOT_INCLUDE_DIR} 
${BASE_INCLUDE_DIRECTORIES} 
${CMAKE_SOURCE_DIR}/field 

${CMAKE_SOURCE_DIR}/pnddata
${CMAKE_SOURCE_DIR}/eic/base
${CMAKE_SOURCE_DIR}/eic/field
)

#include_directories( ${INCLUDE_DIRECTORIES} )

# List of source files;
set( CXXSOURCES
PndConstField.cxx       
PndFieldContFact.cxx 
PndFieldMap.cxx      
PndFieldMapData.cxx  
PndSolenoidMap.cxx   
PndFieldPar.cxx      
PndFieldCreator.cxx  
PndDipoleMap.cxx     
PndTransMap.cxx      
PndRegion.cxx        
PndDipolePar.cxx     
PndDipole1Par.cxx     
PndDipole2Par.cxx     
PndDipole3Par.cxx     
PndTransPar.cxx
PndSolenoidPar.cxx
PndSolenoid1Par.cxx
PndSolenoid2Par.cxx
PndSolenoid3Par.cxx   
PndSolenoid4Par.cxx
PndMapPar.cxx        
PndMultiFieldPar.cxx 
PndConstPar.cxx    
PndMultiField.cxx 
)

SET(DEPENDENCIES Base ParBase)
GENERATE_EICROOT_LIBRARY(Field FieldLinkDef.h)
	 
