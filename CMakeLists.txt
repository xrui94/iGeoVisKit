cmake_minimum_required(VERSION 3.20)

# 项目定义
project(iGeoVisKit VERSION 1.0.0 LANGUAGES CXX C)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 Qt6 组件
# 查找 Qt6 的 Core、Widgets、Gui、OpenGLWidgets 模块
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui OpenGLWidgets)
find_package(OpenGL REQUIRED)
find_package(GDAL REQUIRED)
find_package(Stb REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)

# 启用 Qt 的自动 MOC、RCC、UIC 处理
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Windows 平台特定设置
if(WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    # 为 Windows GUI 应用设置链接器子系统（已注释，默认使用控制台）
    # set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
    set(WIN_LIBS Advapi32)
endif()

# 添加 iGeoVisKit 头文件目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# 定义预处理器宏（用于兼容性和跨平台）
add_definitions(
    -DNOMINMAX
    -DWINE_NOWINSOCK
    -DNOCRYPT
    -DWINE_UNICODE_NATIVE
    -D_REENTRANT
    # -D__WINE__
    -DWIN32
    -D_WIN32
    -D__WIN32__
    # -DWINVER=0x0500
    # -D_WIN32_IE=0x600
    # -D_WIN32_WINNT=0x501
)

# 设置源代码文件和头文件、以及资源文件
set(SRC_FILES "")

# 源文件、资源文件
file(GLOB_RECURSE SOURCE_CODE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
list(APPEND SRC_FILES
    ${SOURCE_CODE_FILES}
    ${CMAKE_SOURCE_DIR}/resources/app.rc
    ${CMAKE_SOURCE_DIR}/resources/app.qrc
)

# 头文件
file(GLOB_RECURSE HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp")

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SRC_FILES} ${HEADER_FILES})

# MSVC 编译器特定设置
if(MSVC)
    # 设置 Visual Studio 启动项目
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT iGeoVisKit)

    # 设置链接器子系统为 WINDOWS（隐藏控制台窗口）
    set_target_properties(iGeoVisKit PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")

    # Debug 模式下设置 GDAL 环境变量（如 proj.db 路径）
    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        # 设置 GDAL 所需的 PROJ 数据库路径
        set(GDALEnv
            PROJ_LIB=${VCPKG_ROOT_DIR}/installed/x64-windows/share/proj
        )

        # 配置 Visual Studio 调试器环境变量
        set_target_properties(
            iGeoVisKit PROPERTIES 
            VS_DEBUGGER_ENVIRONMENT "${GDALEnv}\n"
        )
    endif()
endif()

# stb
target_include_directories(iGeoVisKit PRIVATE ${Stb_INCLUDE_DIR})

# glm
target_link_libraries(iGeoVisKit PRIVATE glm::glm)

# 链接 Qt6 库
target_link_libraries(iGeoVisKit 
    PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::OpenGLWidgets
)

# gdal
target_link_libraries(iGeoVisKit PRIVATE GDAL::GDAL)

# glad
target_link_libraries(iGeoVisKit PRIVATE glad::glad)

# 链接 OpenGL 相关库
target_link_libraries(iGeoVisKit 
    PRIVATE
    OpenGL::GL
    # OpenGL::GLU
    # ${GDAL_LIBRARY}
    # comctl32
    # opengl32
    # glu32
    # odbc32
    # ole32
    # oleaut32
    # winspool
    # odbccp32
    # uuid
    # ${WIN_LIBS}
)

# 在 Debug 配置下启用控制台支持（使 Console::open 生效并显示 printf 输出）
target_compile_definitions(iGeoVisKit PRIVATE $<$<CONFIG:Debug>:USE_CONSOLE=1>)

# 编译选项
if(MSVC)
    target_compile_options(iGeoVisKit PRIVATE
        /std:c++17
        /permissive-
        /Zc:__cplusplus
        /utf-8
    )
else()
    target_compile_options(iGeoVisKit PRIVATE
        -fshort-wchar
        -ggdb
        -Wno-deprecated
        -fPIC
    )
endif()

# 设置 Windows 可执行属性
set_target_properties(iGeoVisKit PROPERTIES
    WIN32_EXECUTABLE TRUE
    LINK_FLAGS "/MANIFEST:NO"
)

# Windows: 部署资源文件和 Qt DLL
# -----------------------------
if(WIN32 AND MSVC)
    if(NOT DEFINED Qt6_DIR)
        message(FATAL_ERROR "Qt6_DIR is not set. Please set CMAKE_PREFIX_PATH or Qt6_DIR")
    endif()

    # 查找 windeployqt 工具
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt)

    # 使用 windeployqt 自动复制 Qt 运行时依赖
    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET iGeoVisKit POST_BUILD
            COMMAND ${WINDEPLOYQT_EXECUTABLE}
                $<TARGET_FILE:iGeoVisKit>       # 目标可执行文件路径
                --no-compiler-runtime               # 不复制 VC 运行时
                --no-system-d3d-compiler            # 不复制 D3D 编译器
                --no-translations                   # 不复制翻译文件
            COMMENT "Running windeployqt to copy Qt libraries..."
        )
    endif()
endif()


# # 安装规则
# install(TARGETS iGeoVisKit
#     RUNTIME DESTINATION bin
#     LIBRARY DESTINATION lib
#     ARCHIVE DESTINATION lib
# )

# # 安装额外文件
# install(FILES iGeoVisKit/iGeoVisKit.ico DESTINATION bin)
# install(DIRECTORY doc/ DESTINATION doc)
# install(DIRECTORY web/ DESTINATION web)