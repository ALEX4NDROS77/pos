# Embeds a file as a C++ byte array header.
# Usage: cmake -DINPUT=<path> -DOUTPUT=<path> -DVARNAME=<name> -P EmbedResource.cmake

if(NOT DEFINED INPUT OR NOT DEFINED OUTPUT OR NOT DEFINED VARNAME)
	message(FATAL_ERROR "Usage: cmake -DINPUT=<path> -DOUTPUT=<path> -DVARNAME=<name> -P EmbedResource.cmake")
endif()

file(READ "${INPUT}" hex_content HEX)
string(LENGTH "${hex_content}" hex_length)
math(EXPR byte_count "${hex_length} / 2")

string(REGEX REPLACE "([0-9a-fA-F][0-9a-fA-F])" "0x\\1," array_body "${hex_content}")

set(header_content "// Auto-generated from ${INPUT} by EmbedResource.cmake. Do not edit.\n#pragma once\n\nstatic const unsigned char ${VARNAME}[] = { ${array_body} };\nstatic const unsigned int ${VARNAME}_len = ${byte_count};\n")

file(WRITE "${OUTPUT}" "${header_content}")
