
# Example executable
set(TriCalc_sources
        examples.cpp
        )

file(GLOB_RECURSE objc_headers *.h)
file(GLOB_RECURSE objc_sources *.mm)
set(objc_files ${objc_headers} ${objc_sources})

add_executable(TriCalc ${objc_files})
target_include_directories(TriCalc PRIVATE ${CMAKE_SOURCE_DIR}/headers)
target_link_libraries(TriCalc PUBLIC ${PROJECT_NAME} "-framework AppKit" "-framework CoreGraphics")

set_property(TARGET TriCalc PROPERTY OBJC_STANDARD 11)

# Code sign to avoid macOS permission dialogs
if(APPLE)
    set_target_properties(TriCalc PROPERTIES
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "-"
        XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO"
        XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO"
    )
endif()
