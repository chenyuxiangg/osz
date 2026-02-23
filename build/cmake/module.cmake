# ====================================================
# 模块定义宏
# ====================================================
#[[
define_module: 统一模块定义宏

这个宏提供了一个统一的接口来定义各种类型的CMake模块，包括静态库、共享库、接口库和可执行文件。
它封装了目标的创建、属性设置、依赖管理和安装规则，简化了CMakeLists.txt的编写。

语法：
    define_module(
        NAME <target_name>                    # 必需：目标名称
        TYPE <type>                           # 必需：目标类型，可选值：STATIC, SHARED, INTERFACE, EXECUTABLE
        
        # 源文件相关
        SOURCES <source_file>...              # 可选：C/C++源文件列表
        ASM_SOURCES <asm_file>...             # 可选：汇编源文件列表
        
        # 包含目录
        PUBLIC_INCLUDES <include_dir>...      # 可选：公共包含目录，会传递给依赖此目标的目标
        PRIVATE_INCLUDES <include_dir>...     # 可选：私有包含目录，仅本目标使用
        
        # 编译定义
        PUBLIC_DEFINITIONS <definition>...    # 可选：公共编译定义
        PRIVATE_DEFINITIONS <definition>...   # 可选：私有编译定义
        
        # 编译选项
        PUBLIC_COMPILE_OPTIONS <option>...    # 可选：公共编译选项
        PRIVATE_COMPILE_OPTIONS <option>...   # 可选：私有编译选项
        
        # 链接选项
        PUBLIC_LINK_OPTIONS <option>...       # 可选：公共链接选项
        PRIVATE_LINK_OPTIONS <option>...      # 可选：私有链接选项
        
        # 依赖关系
        PUBLIC_DEPENDENCIES <target>...       # 可选：公共依赖，会传递给依赖此目标的目标
        PRIVATE_DEPENDENCIES <target>...      # 可选：私有依赖，仅本目标使用
        INTERFACE_DEPENDENCIES <target>...    # 可选：接口依赖（主要用于接口库）
    )

参数说明：
    NAME: 目标的名称，必需参数。
    
    TYPE: 目标类型，必需参数。支持以下值：
        - STATIC: 创建静态库
        - SHARED: 创建共享库
        - INTERFACE: 创建接口库（无源文件）
        - EXECUTABLE: 创建可执行文件
    
    SOURCES: C/C++源文件列表。对于非INTERFACE类型的目标是必需的。
            支持相对路径和绝对路径，相对路径相对于当前CMakeLists.txt所在目录。
    
    ASM_SOURCES: 汇编源文件列表。如果需要汇编支持，可以在这里指定。
                注意：汇编文件会自动设置LANGUAGE属性为ASM。
    
    PUBLIC_INCLUDES: 公共包含目录列表。这些目录会被添加到目标的包含路径中，
                    并且会传递给任何链接此目标的其他目标。
    
    PRIVATE_INCLUDES: 私有包含目录列表。这些目录仅被添加到本目标的包含路径中，
                      不会传递给其他目标。
    
    PUBLIC_DEFINITIONS: 公共编译定义（宏定义）。这些定义会被添加到目标的编译选项中，
                        并且会传递给任何链接此目标的其他目标。
    
    PRIVATE_DEFINITIONS: 私有编译定义（宏定义）。这些定义仅被添加到本目标的编译选项中，
                         不会传递给其他目标。
    
    PUBLIC_COMPILE_OPTIONS: 公共编译选项。这些选项会被添加到目标的编译命令中，
                            并且会传递给任何链接此目标的其他目标。
    
    PRIVATE_COMPILE_OPTIONS: 私有编译选项。这些选项仅被添加到本目标的编译命令中，
                             不会传递给其他目标。
    
    PUBLIC_LINK_OPTIONS: 公共链接选项。这些选项会被添加到目标的链接命令中，
                         并且会传递给任何链接此目标的其他目标。
    
    PRIVATE_LINK_OPTIONS: 私有链接选项。这些选项仅被添加到本目标的链接命令中，
                          不会传递给其他目标。
    
    PUBLIC_DEPENDENCIES: 公共依赖目标列表。这些目标会被链接到本目标，
                         并且它们的公共属性会传递给任何链接此目标的其他目标。
    
    PRIVATE_DEPENDENCIES: 私有依赖目标列表。这些目标会被链接到本目标，
                          但它们的属性不会传递给其他目标。
    
    INTERFACE_DEPENDENCIES: 接口依赖目标列表。主要用于接口库，
                            表示这些目标的接口属性应该被继承。

特性：
    1. 自动处理相对路径：所有包含目录的相对路径会自动转换为绝对路径。
    2. 汇编文件支持：ASM_SOURCES中的文件会自动设置为汇编语言。
    3. 统一的标准设置：所有目标都使用C++11标准，且标准要求严格。
    4. 安装规则：库目标会自动添加到安装规则中，可执行文件也会被安装。
    5. 目标导出：库目标会被添加到导出集中，方便其他项目使用。
    6. 详细日志：配置过程中会输出详细的日志信息，便于调试。

示例：
    # 创建静态库
    define_module(
        NAME my_static_lib
        TYPE STATIC
        SOURCES src/lib1.cpp src/lib2.cpp
        PUBLIC_INCLUDES include
        PRIVATE_INCLUDES src/internal
        PUBLIC_DEFINITIONS MYLIB_API=EXPORT
        PRIVATE_DEPENDENCIES some_utility
    )
    
    # 创建共享库
    define_module(
        NAME my_shared_lib
        TYPE SHARED
        SOURCES src/shared.cpp
        PUBLIC_INCLUDES include
        PUBLIC_DEFINITIONS MYSHAREDLIB_EXPORTS
        PUBLIC_DEPENDENCIES my_static_lib
    )
    
    # 创建接口库
    define_module(
        NAME my_interface
        TYPE INTERFACE
        PUBLIC_INCLUDES interface
        PUBLIC_COMPILE_OPTIONS -Wall -Wextra
        INTERFACE_DEPENDENCIES some_interface_lib
    )
    
    # 创建可执行文件
    define_module(
        NAME my_app
        TYPE EXECUTABLE
        SOURCES src/main.cpp src/app.cpp
        PRIVATE_INCLUDES src
        PRIVATE_DEPENDENCIES my_static_lib my_shared_lib
        PRIVATE_COMPILE_OPTIONS -O2
    )
    
    # 创建带汇编文件的目标
    define_module(
        NAME asm_library
        TYPE STATIC
        SOURCES src/code.cpp
        ASM_SOURCES src/asm/vector_ops.S src/asm/matrix_mult.S
        PRIVATE_INCLUDES src/asm
    )

注意事项：
    1. INTERFACE类型的目标不需要指定SOURCES或ASM_SOURCES。
    2. 对于EXECUTABLE类型，某些属性（如VERSION）不会设置。
    3. 使用相对路径时，宏会自动将其转换为绝对路径。
    4. 汇编文件需要正确设置文件扩展名（.S, .s, .asm等），以便CMake识别。
]]
# ====================================================

#[[
define_module: 统一模块定义宏（增强版）
功能：
1. 支持创建静态库、共享库、接口库和可执行文件
2. 支持链接器脚本配置
3. 支持为可执行文件自动生成反汇编文件
4. 支持自定义汇编文件处理
]]
macro(define_module)
    # 解析参数 - 添加反汇编相关参数
    set(options "")
    set(oneValueArgs 
        NAME 
        TYPE 
        LINKER_SCRIPT 
        DISASSEMBLE            # 是否生成反汇编 (ON/OFF)
        DISASSEMBLE_TOOL       # 反汇编工具路径 (如objdump)
        DISASSEMBLE_FORMAT     # 反汇编文件格式 (默认为dis)
    )
    set(multiValueArgs
        SOURCES
        ASM_SOURCES
        PUBLIC_INCLUDES
        PRIVATE_INCLUDES
        PUBLIC_DEFINITIONS
        PRIVATE_DEFINITIONS
        PUBLIC_COMPILE_OPTIONS
        PRIVATE_COMPILE_OPTIONS
        PUBLIC_LINK_OPTIONS
        PRIVATE_LINK_OPTIONS
        PUBLIC_DEPENDENCIES
        PRIVATE_DEPENDENCIES
        INTERFACE_DEPENDENCIES
        DISASSEMBLE_FLAGS      # 反汇编工具标志
    )
    
    cmake_parse_arguments(MODULE
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN}
    )
    
    # 检查必需参数
    if(NOT MODULE_NAME)
        message(FATAL_ERROR "模块必须指定 NAME 参数")
    endif()
    
    if(NOT MODULE_TYPE)
        message(FATAL_ERROR "模块必须指定 TYPE 参数 (STATIC, SHARED, INTERFACE, or EXECUTABLE)")
    endif()

    # 检查类型是否有效
    if(NOT MODULE_TYPE MATCHES "^(STATIC|SHARED|INTERFACE|EXECUTABLE)$")
        message(FATAL_ERROR "无效的模块类型: ${MODULE_TYPE}。必须是 STATIC, SHARED, INTERFACE 或 EXECUTABLE")
    endif()

    # 检查必需参数
    if(NOT MODULE_SOURCES AND NOT MODULE_ASM_SOURCES AND NOT MODULE_TYPE STREQUAL "INTERFACE")
        message(FATAL_ERROR "非接口模块必须指定 SOURCES 或 ASM_SOURCES 参数")
    endif()
    
    # 设置默认的反汇编配置
    if(MODULE_TYPE STREQUAL "EXECUTABLE")
        # 对于可执行文件，默认启用反汇编生成
        if(NOT DEFINED MODULE_DISASSEMBLE)
            set(MODULE_DISASSEMBLE ON)
        endif()
        
        # 设置默认的反汇编工具
        if(NOT MODULE_DISASSEMBLE_TOOL)
            # 尝试从编译器推导反汇编工具
            if(CMAKE_C_COMPILER)
                get_filename_component(COMPILER_PATH ${CMAKE_C_COMPILER} DIRECTORY)
                get_filename_component(COMPILER_NAME ${CMAKE_C_COMPILER} NAME_WE)
                
                # 尝试几种常见的反汇编工具命名规则
                if(EXISTS "${COMPILER_PATH}/${COMPILER_NAME}-objdump")
                    set(MODULE_DISASSEMBLE_TOOL "${COMPILER_PATH}/${COMPILER_NAME}-objdump")
                elseif(EXISTS "${COMPILER_PATH}/llvm-objdump")
                    set(MODULE_DISASSEMBLE_TOOL "${COMPILER_PATH}/llvm-objdump")
                elseif(EXISTS "${COMPILER_PATH}/objdump")
                    set(MODULE_DISASSEMBLE_TOOL "${COMPILER_PATH}/objdump")
                else()
                    # 回退到系统路径查找
                    find_program(OBJDUMP_TOOL objdump)
                    if(OBJDUMP_TOOL)
                        set(MODULE_DISASSEMBLE_TOOL ${OBJDUMP_TOOL})
                    else()
                        set(MODULE_DISASSEMBLE_TOOL "objdump")
                    endif()
                endif()
            else()
                set(MODULE_DISASSEMBLE_TOOL "objdump")
            endif()
        endif()
        
        # 设置默认的反汇编文件格式
        if(NOT MODULE_DISASSEMBLE_FORMAT)
            set(MODULE_DISASSEMBLE_FORMAT "dis")
        endif()
        
        # 设置默认的反汇编标志
        if(NOT MODULE_DISASSEMBLE_FLAGS)
            set(MODULE_DISASSEMBLE_FLAGS -d -S)  # 反汇编代码段，并与源码混合显示
        endif()
    else()
        # 对于非可执行文件，默认不生成反汇编
        if(NOT DEFINED MODULE_DISASSEMBLE)
            set(MODULE_DISASSEMBLE OFF)
        endif()
    endif()
    
    # 根据类型创建目标
    if(MODULE_TYPE STREQUAL "INTERFACE")
        message(STATUS "创建接口库: ${MODULE_NAME}")
        add_library(${MODULE_NAME} INTERFACE)
    elseif(MODULE_TYPE STREQUAL "EXECUTABLE")
        message(STATUS "创建可执行文件: ${MODULE_NAME}")
        add_executable(${MODULE_NAME}
            ${MODULE_SOURCES}
            ${MODULE_ASM_SOURCES}
        )
        
        # 如果启用了反汇编，添加自定义目标
        if(MODULE_DISASSEMBLE)
            message(STATUS "  [${MODULE_NAME}] 反汇编配置: 启用")
            message(STATUS "  [${MODULE_NAME}] 反汇编工具: ${MODULE_DISASSEMBLE_TOOL}")
            message(STATUS "  [${MODULE_NAME}] 反汇编标志: ${MODULE_DISASSEMBLE_FLAGS}")
            
            # 定义反汇编目标名称
            set(DISASM_TARGET "${MODULE_NAME}_disasm")
            set(DISASM_FILE "${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}.${MODULE_DISASSEMBLE_FORMAT}")
            set(MAP_FILE "${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}.map")
            
            # 创建反汇编目标
            add_custom_target(${DISASM_TARGET}
                COMMAND ${CMAKE_COMMAND} -E echo "生成 ${MODULE_NAME} 的反汇编文件..."
                COMMAND ${MODULE_DISASSEMBLE_TOOL} ${MODULE_DISASSEMBLE_FLAGS} 
                    $<TARGET_FILE:${MODULE_NAME}> > ${DISASM_FILE} 2>&1 || true
                COMMAND ${CMAKE_COMMAND} -E echo "反汇编文件已生成: ${DISASM_FILE}"
                DEPENDS ${MODULE_NAME}
                COMMENT "为 ${MODULE_NAME} 生成反汇编文件"
            )
            
            # 可选：生成内存映射文件（对于嵌入式开发很有用）
            add_custom_command(TARGET ${DISASM_TARGET} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E echo "生成内存映射文件..."
                COMMAND ${MODULE_DISASSEMBLE_TOOL} -x $<TARGET_FILE:${MODULE_NAME}> > ${MAP_FILE} 2>&1 || true
                COMMAND ${CMAKE_COMMAND} -E echo "内存映射文件已生成: ${MAP_FILE}"
                COMMENT "生成 ${MODULE_NAME} 的内存映射文件"
            )
            
            # 创建合并目标：构建并生成反汇编
            add_custom_target(${MODULE_NAME}_all
                DEPENDS ${MODULE_NAME} ${DISASM_TARGET}
                COMMENT "构建 ${MODULE_NAME} 并生成反汇编文件"
            )
        endif()
    else()
        message(STATUS "创建库: ${MODULE_NAME} (${MODULE_TYPE})")
        add_library(${MODULE_NAME} ${MODULE_TYPE}
            ${MODULE_SOURCES}
            ${MODULE_ASM_SOURCES}
        )
    endif()
    
    # 为汇编文件设置特定属性
    if(MODULE_ASM_SOURCES)
        # 设置汇编文件的编译选项
        set_source_files_properties(${MODULE_ASM_SOURCES}
            PROPERTIES
                LANGUAGE ASM  # 明确指定为汇编语言
        )
        message(STATUS "  [${MODULE_NAME}] 汇编源文件: ${MODULE_ASM_SOURCES}")
    endif()
    
    message(STATUS "  [${MODULE_NAME}] 源文件: ${MODULE_SOURCES}")
    
    # 设置目标属性（仅对库目标）
    if(NOT MODULE_TYPE STREQUAL "EXECUTABLE")
        set_target_properties(${MODULE_NAME} PROPERTIES
            CXX_STANDARD 11
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF
            VERSION ${PROJECT_VERSION}
            SOVERSION ${PROJECT_VERSION_MAJOR}
        )
    else()
        # 对可执行文件，只设置编译标准
        set_target_properties(${MODULE_NAME} PROPERTIES
            CXX_STANDARD 11
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF
        )
    endif()
    
    # 公共包含目录（传递给依赖此模块的目标）
    if(MODULE_PUBLIC_INCLUDES)
        foreach(include_dir ${MODULE_PUBLIC_INCLUDES})
            # 处理相对路径
            if(NOT IS_ABSOLUTE ${include_dir})
                set(include_dir ${CMAKE_CURRENT_SOURCE_DIR}/${include_dir})
            endif()
            
            if(MODULE_TYPE STREQUAL "INTERFACE")
                target_include_directories(${MODULE_NAME}
                    INTERFACE $<BUILD_INTERFACE:${include_dir}>
                )
            else()
                target_include_directories(${MODULE_NAME}
                    PUBLIC $<BUILD_INTERFACE:${include_dir}>
                )
            endif()
            message(STATUS "  [${MODULE_NAME}] 公共包含目录: ${include_dir}")
        endforeach()
    endif()
    
    # 私有包含目录（仅本模块使用）
    if(MODULE_PRIVATE_INCLUDES)
        foreach(include_dir ${MODULE_PRIVATE_INCLUDES})
            if(NOT IS_ABSOLUTE ${include_dir})
                set(include_dir ${CMAKE_CURRENT_SOURCE_DIR}/${include_dir})
            endif()
            target_include_directories(${MODULE_NAME}
                PRIVATE ${include_dir}
            )
            message(STATUS "  [${MODULE_NAME}] 私有包含目录: ${include_dir}")
        endforeach()
    endif()
    
    # 公共宏定义
    if(MODULE_PUBLIC_DEFINITIONS)
        if(MODULE_TYPE STREQUAL "INTERFACE")
            target_compile_definitions(${MODULE_NAME}
                INTERFACE ${MODULE_PUBLIC_DEFINITIONS}
            )
        else()
            target_compile_definitions(${MODULE_NAME}
                PUBLIC ${MODULE_PUBLIC_DEFINITIONS}
            )
        endif()
        message(STATUS "  [${MODULE_NAME}] 公共宏定义: ${MODULE_PUBLIC_DEFINITIONS}")
    endif()
    
    # 私有宏定义
    if(MODULE_PRIVATE_DEFINITIONS)
        target_compile_definitions(${MODULE_NAME}
            PRIVATE ${MODULE_PRIVATE_DEFINITIONS}
        )
        message(STATUS "  [${MODULE_NAME}] 私有宏定义: ${MODULE_PRIVATE_DEFINITIONS}")
    endif()
    
    # 公共编译选项
    if(MODULE_PUBLIC_COMPILE_OPTIONS)
        if(MODULE_TYPE STREQUAL "INTERFACE")
            target_compile_options(${MODULE_NAME}
                INTERFACE ${MODULE_PUBLIC_COMPILE_OPTIONS}
            )
        else()
            target_compile_options(${MODULE_NAME}
                PUBLIC ${MODULE_PUBLIC_COMPILE_OPTIONS}
            )
        endif()
        message(STATUS "  [${MODULE_NAME}] 公共编译选项: ${MODULE_PUBLIC_COMPILE_OPTIONS}")
    endif()
    
    # 私有编译选项
    if(MODULE_PRIVATE_COMPILE_OPTIONS)
        target_compile_options(${MODULE_NAME}
            PRIVATE ${MODULE_PRIVATE_COMPILE_OPTIONS}
        )
        message(STATUS "  [${MODULE_NAME}] 私有编译选项: ${MODULE_PRIVATE_COMPILE_OPTIONS}")
    endif()
    
    # 链接选项
    if(MODULE_PUBLIC_LINK_OPTIONS)
        if(MODULE_TYPE STREQUAL "INTERFACE")
            target_link_options(${MODULE_NAME}
                INTERFACE ${MODULE_PUBLIC_LINK_OPTIONS}
            )
        else()
            target_link_options(${MODULE_NAME}
                PUBLIC ${MODULE_PUBLIC_LINK_OPTIONS}
            )
        endif()
    endif()
    
    if(MODULE_PRIVATE_LINK_OPTIONS)
        target_link_options(${MODULE_NAME}
            PRIVATE ${MODULE_PRIVATE_LINK_OPTIONS}
        )
    endif()
    
    # 依赖关系
    if(MODULE_PUBLIC_DEPENDENCIES)
        target_link_libraries(${MODULE_NAME}
            PUBLIC ${MODULE_PUBLIC_DEPENDENCIES}
        )
        message(STATUS "  [${MODULE_NAME}] 公共依赖: ${MODULE_PUBLIC_DEPENDENCIES}")
    endif()
    
    if(MODULE_PRIVATE_DEPENDENCIES)
        target_link_libraries(${MODULE_NAME}
            PRIVATE ${MODULE_PRIVATE_DEPENDENCIES}
        )
        message(STATUS "  [${MODULE_NAME}] 私有依赖: ${MODULE_PRIVATE_DEPENDENCIES}")
    endif()
    
    if(MODULE_INTERFACE_DEPENDENCIES)
        if(MODULE_TYPE STREQUAL "INTERFACE")
            target_link_libraries(${MODULE_NAME}
                INTERFACE ${MODULE_INTERFACE_DEPENDENCIES}
            )
        else()
            # 对于非接口目标，接口依赖可能不适用，我们可以将其视为公共依赖
            target_link_libraries(${MODULE_NAME}
                PUBLIC ${MODULE_INTERFACE_DEPENDENCIES}
            )
        endif()
    endif()

    # 添加链接器脚本支持
    if(MODULE_LINKER_SCRIPT)
        # 确保路径存在
        if(NOT IS_ABSOLUTE ${MODULE_LINKER_SCRIPT})
            set(MODULE_LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/${MODULE_LINKER_SCRIPT})
        endif()
        
        # 检查文件是否存在
        if(NOT EXISTS ${MODULE_LINKER_SCRIPT})
            message(WARNING "链接器脚本文件不存在: ${MODULE_LINKER_SCRIPT}")
        else()
            # 设置链接器选项
            if(MODULE_TYPE STREQUAL "EXECUTABLE")
                target_link_options(${MODULE_NAME}
                    PRIVATE "-T${MODULE_LINKER_SCRIPT}"
                )
            else()
                target_link_options(${MODULE_NAME}
                    PRIVATE "-T${MODULE_LINKER_SCRIPT}"
                )
            endif()
            message(STATUS "  [${MODULE_NAME}] 链接器脚本: ${MODULE_LINKER_SCRIPT}")
        endif()
    endif()
    
    # 安装规则（仅对库目标）
    if(NOT MODULE_TYPE STREQUAL "EXECUTABLE")
        install(TARGETS ${MODULE_NAME}
            EXPORT ${PROJECT_NAME}Targets
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
    else()
        # 可执行文件的安装规则
        install(TARGETS ${MODULE_NAME}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            BUNDLE DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
    endif()
    
    # 将模块添加到全局列表
    set(ALL_MODULES ${ALL_MODULES} ${MODULE_NAME} PARENT_SCOPE)
    
    message(STATUS "模块 ${MODULE_NAME} (${MODULE_TYPE}) 配置完成")
endmacro()

#[[
辅助函数：为目标生成多种格式的反汇编文件
可用于更复杂的反汇编需求
]]
function(generate_extended_disassembly target_name)
    # 解析参数
    set(options "")
    set(oneValueArgs 
        TOOL
        OUTPUT_DIR
    )
    set(multiValueArgs "")
    
    cmake_parse_arguments(ARG
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN}
    )
    
    if(NOT ARG_TOOL)
        # 尝试查找objdump工具
        find_program(OBJDUMP_TOOL objdump)
        if(NOT OBJDUMP_TOOL)
            message(WARNING "未找到objdump工具，无法生成扩展反汇编")
            return()
        endif()
        set(ARG_TOOL ${OBJDUMP_TOOL})
    endif()
    
    if(NOT ARG_OUTPUT_DIR)
        set(ARG_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
    endif()
    
    # 生成基本反汇编
    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "生成基本反汇编..."
        COMMAND ${ARG_TOOL} -d $<TARGET_FILE:${target_name}> > ${ARG_OUTPUT_DIR}/${target_name}_basic.dis
        COMMENT "生成 ${target_name} 的基本反汇编"
    )
    
    # 生成带源码的反汇编
    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "生成带源码的反汇编..."
        COMMAND ${ARG_TOOL} -d -S $<TARGET_FILE:${target_name}> > ${ARG_OUTPUT_DIR}/${target_name}_source.dis
        COMMENT "生成 ${target_name} 的带源码反汇编"
    )
    
    # 生成节头信息
    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "生成节头信息..."
        COMMAND ${ARG_TOOL} -h $<TARGET_FILE:${target_name}> > ${ARG_OUTPUT_DIR}/${target_name}_sections.txt
        COMMENT "生成 ${target_name} 的节头信息"
    )
    
    # 生成符号表
    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "生成符号表..."
        COMMAND ${ARG_TOOL} -t $<TARGET_FILE:${target_name}> > ${ARG_OUTPUT_DIR}/${target_name}_symbols.txt
        COMMENT "生成 ${target_name} 的符号表"
    )
    
    message(STATUS "已为 ${target_name} 配置扩展反汇编生成")
endfunction()
