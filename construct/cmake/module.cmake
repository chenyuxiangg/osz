#[[
define_module - 定义OSZ项目的CMake模块

这个宏用于定义OSZ项目中的各种模块（库、可执行文件、接口库）。它提供了统一的配置接口，
支持反汇编生成、QEMU模拟运行、GDB调试等嵌入式开发特性。

参数分类：
----------

### 选项参数 (options)
- 无

### 单值参数 (oneValueArgs)
- NAME (必需): 模块名称
- TYPE (必需): 模块类型，必须是以下之一：
  * STATIC: 静态库
  * SHARED: 共享库  
  * INTERFACE: 接口库
  * EXECUTABLE: 可执行文件
- LINKER_SCRIPT: 链接器脚本文件路径
- DISASSEMBLE: 是否生成反汇编文件 (ON/OFF)，默认可执行文件为ON，其他为OFF
- DISASSEMBLE_TOOL: 反汇编工具路径（如objdump），自动从编译器推导
- DISASSEMBLE_FORMAT: 反汇编文件格式，默认"dis"
- QEMU_EXECUTABLE: QEMU模拟器路径
- QEMU_MACHINE: QEMU机器类型，默认"virt"（针对RISC-V）
- QEMU_CPU: QEMU CPU类型，默认"rv32"（针对RISC-V）
- QEMU_SERIAL: 串口设置
- QEMU_MONITOR: 监控器设置
- GDB_EXECUTABLE: GDB调试器路径
- GDB_PORT: GDB调试端口，默认1234
- ENABLE_QEMU_RUN: 是否启用QEMU运行目标 (ON/OFF)，默认OFF
- ENABLE_GDB_DEBUG: 是否启用GDB调试目标 (ON/OFF)，默认OFF

### 多值参数 (multiValueArgs)
- SOURCES: C/C++源文件列表
- ASM_SOURCES: 汇编源文件列表
- PUBLIC_INCLUDES: 公共头文件目录（传递给依赖目标）
- PRIVATE_INCLUDES: 私有头文件目录（仅本模块使用）
- PUBLIC_DEFINITIONS: 公共宏定义
- PRIVATE_DEFINITIONS: 私有宏定义
- PUBLIC_COMPILE_OPTIONS: 公共编译选项
- PRIVATE_COMPILE_OPTIONS: 私有编译选项
- PUBLIC_LINK_OPTIONS: 公共链接选项
- PRIVATE_LINK_OPTIONS: 私有链接选项
- PUBLIC_DEPENDENCIES: 公共依赖库
- PRIVATE_DEPENDENCIES: 私有依赖库
- INTERFACE_DEPENDENCIES: 接口依赖库
- DISASSEMBLE_FLAGS: 反汇编工具标志，默认"-d -S"
- QEMU_OPTIONS: QEMU额外选项
- GDB_OPTIONS: GDB额外选项
- GDB_SCRIPT: GDB脚本文件

功能特性：
---------

1. 自动目标创建：根据TYPE创建相应的CMake目标（add_library或add_executable）
2. 反汇编生成：对于可执行文件，可自动生成反汇编文件（.dis）和内存映射文件（.map）
3. QEMU集成：支持在QEMU中运行和调试嵌入式程序
4. GDB调试：支持通过GDB连接QEMU进行调试
5. 链接器脚本：支持指定自定义链接器脚本
6. 标准配置：自动设置C++11标准、版本号等
7. 安装规则：自动生成安装规则（库和可执行文件）

示例用法：
----------

```cmake
# 定义静态库
define_module(
    NAME my_lib
    TYPE STATIC
    SOURCES src1.c src2.c
    PUBLIC_INCLUDES include
    PUBLIC_DEFINITIONS MYLIB_EXPORT
)

# 定义可执行文件（带反汇编和QEMU支持）
define_module(
    NAME my_app
    TYPE EXECUTABLE
    SOURCES main.c
    PUBLIC_DEPENDENCIES my_lib
    DISASSEMBLE ON
    ENABLE_QEMU_RUN ON
    QEMU_MACHINE virt
    QEMU_CPU rv32
    LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/linker.ld
)
```

注意事项：
---------

1. NAME和TYPE是必需参数
2. 非接口模块必须指定SOURCES或ASM_SOURCES
3. 相对路径会自动转换为基于当前源目录的绝对路径
4. 反汇编默认只对可执行文件启用
5. QEMU和GDB配置需要相应工具已安装

创建的目标：
-----------

- ${MODULE_NAME}: 主构建目标
- ${MODULE_NAME}_disasm: 生成反汇编文件（如果DISASSEMBLE=ON）
- ${MODULE_NAME}_all: 构建并生成反汇编文件
- ${MODULE_NAME}_run: 在QEMU中运行（如果ENABLE_QEMU_RUN=ON）
- ${MODULE_NAME}_debug: 启动QEMU并连接GDB调试（如果ENABLE_GDB_DEBUG=ON）
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
        QEMU_EXECUTABLE        # QEMU模拟器路径
        QEMU_MACHINE           # QEMU机器类型
        QEMU_CPU               # QEMU CPU类型
        QEMU_SERIAL            # 串口设置
        QEMU_MONITOR           # 监控器设置
        GDB_EXECUTABLE         # GDB调试器路径
        GDB_PORT               # GDB调试端口
        ENABLE_QEMU_RUN        # 是否启用QEMU运行目标
        ENABLE_GDB_DEBUG       # 是否启用GDB调试目标
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
        QEMU_OPTIONS           # QEMU额外选项
        GDB_OPTIONS            # GDB额外选项
        GDB_SCRIPT             # GDB脚本文件
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
        
        # 设置默认的QEMU和GDB配置
        if(NOT DEFINED MODULE_ENABLE_QEMU_RUN)
            set(MODULE_ENABLE_QEMU_RUN OFF)
        endif()
        
        if(NOT DEFINED MODULE_ENABLE_GDB_DEBUG)
            set(MODULE_ENABLE_GDB_DEBUG OFF)
        endif()
        
        # 设置默认的QEMU可执行文件
        if(MODULE_ENABLE_QEMU_RUN AND NOT MODULE_QEMU_EXECUTABLE)
            # 尝试查找QEMU
            find_program(QEMU_SYSTEM qemu-system-riscv32)
            if(QEMU_SYSTEM)
                set(MODULE_QEMU_EXECUTABLE ${QEMU_SYSTEM})
            else()
                # 尝试其他可能的QEMU名称
                find_program(QEMU_SYSTEM_X86 qemu-system-x86_64)
                if(QEMU_SYSTEM_X86)
                    set(MODULE_QEMU_EXECUTABLE ${QEMU_SYSTEM_X86})
                else()
                    set(MODULE_QEMU_EXECUTABLE "qemu-system-riscv32")
                endif()
            endif()
        endif()
        
        # 设置默认的QEMU机器类型（针对RISC-V）
        if(MODULE_ENABLE_QEMU_RUN AND NOT MODULE_QEMU_MACHINE)
            set(MODULE_QEMU_MACHINE "virt")
        endif()
        
        # 设置默认的QEMU CPU类型（针对RISC-V）
        if(MODULE_ENABLE_QEMU_RUN AND NOT MODULE_QEMU_CPU)
            set(MODULE_QEMU_CPU "rv32")
        endif()
        
        # 设置默认的GDB可执行文件
        if(MODULE_ENABLE_GDB_DEBUG AND NOT MODULE_GDB_EXECUTABLE)
            # 尝试从编译器推导GDB工具
            if(CMAKE_C_COMPILER)
                get_filename_component(COMPILER_PATH ${CMAKE_C_COMPILER} DIRECTORY)
                get_filename_component(COMPILER_NAME ${CMAKE_C_COMPILER} NAME_WE)
                
                # 尝试几种常见的GDB工具命名规则
                if(EXISTS "${COMPILER_PATH}/${COMPILER_NAME}-gdb")
                    set(MODULE_GDB_EXECUTABLE "${COMPILER_PATH}/${COMPILER_NAME}-gdb")
                elseif(EXISTS "${COMPILER_PATH}/gdb")
                    set(MODULE_GDB_EXECUTABLE "${COMPILER_PATH}/gdb")
                else()
                    find_program(GDB_TOOL gdb)
                    if(GDB_TOOL)
                        set(MODULE_GDB_EXECUTABLE ${GDB_TOOL})
                    else()
                        set(MODULE_GDB_EXECUTABLE "gdb")
                    endif()
                endif()
            else()
                find_program(GDB_TOOL gdb)
                if(GDB_TOOL)
                    set(MODULE_GDB_EXECUTABLE ${GDB_TOOL})
                else()
                    set(MODULE_GDB_EXECUTABLE "gdb")
                endif()
            endif()
        endif()
        
        # 设置默认的GDB端口
        if(MODULE_ENABLE_GDB_DEBUG AND NOT MODULE_GDB_PORT)
            set(MODULE_GDB_PORT 1234)
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
            
            # 创建反汇编目标 - 使用列表形式传递参数
            add_custom_target(${DISASM_TARGET}
                COMMAND ${CMAKE_COMMAND} -E echo "生成 ${MODULE_NAME} 的反汇编文件..."
                COMMAND ${MODULE_DISASSEMBLE_TOOL} ${MODULE_DISASSEMBLE_FLAGS} $<TARGET_FILE:${MODULE_NAME}> > ${DISASM_FILE}
                COMMAND ${CMAKE_COMMAND} -E echo "反汇编文件已生成: ${DISASM_FILE}"
                DEPENDS ${MODULE_NAME}
                COMMENT "为 ${MODULE_NAME} 生成反汇编文件"
                VERBATIM
            )
            
            # 可选：生成内存映射文件（对于嵌入式开发很有用）
            add_custom_command(TARGET ${DISASM_TARGET} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E echo "生成内存映射文件..."
                COMMAND ${MODULE_DISASSEMBLE_TOOL} -x $<TARGET_FILE:${MODULE_NAME}> > ${MAP_FILE}
                COMMAND ${CMAKE_COMMAND} -E echo "内存映射文件已生成: ${MAP_FILE}"
                COMMENT "生成 ${MODULE_NAME} 的内存映射文件"
                VERBATIM
            )
            
            # 创建合并目标：构建并生成反汇编
            add_custom_target(${MODULE_NAME}_all
                DEPENDS ${MODULE_NAME} ${DISASM_TARGET}
                COMMENT "构建 ${MODULE_NAME} 并生成反汇编文件"
            )
        endif()
        
        # 如果启用了QEMU运行，添加运行目标
        if(MODULE_ENABLE_QEMU_RUN)
            message(STATUS "  [${MODULE_NAME}] QEMU配置: 启用")
            message(STATUS "  [${MODULE_NAME}] QEMU可执行文件: ${MODULE_QEMU_EXECUTABLE}")
            message(STATUS "  [${MODULE_NAME}] QEMU机器类型: ${MODULE_QEMU_MACHINE}")
            message(STATUS "  [${MODULE_NAME}] QEMU CPU类型: ${MODULE_QEMU_CPU}")
            
            # 定义QEMU运行目标
            set(QEMU_TARGET "${MODULE_NAME}_run")
            
            # 构建QEMU命令列表
            set(QEMU_CMD ${MODULE_QEMU_EXECUTABLE})
            
            # 添加基本参数
            list(APPEND QEMU_CMD -machine ${MODULE_QEMU_MACHINE})
            list(APPEND QEMU_CMD -cpu ${MODULE_QEMU_CPU})
            
            # 添加串口设置
            if(MODULE_QEMU_SERIAL)
                list(APPEND QEMU_CMD -serial ${MODULE_QEMU_SERIAL})
            endif()
            
            # 添加监控器设置
            if(MODULE_QEMU_MONITOR)
                list(APPEND QEMU_CMD -monitor ${MODULE_QEMU_MONITOR})
            endif()
            
            # 添加额外选项
            foreach(option ${MODULE_QEMU_OPTIONS})
                list(APPEND QEMU_CMD ${option})
            endforeach()
            
            # 添加内核文件
            list(APPEND QEMU_CMD -kernel $<TARGET_FILE:${MODULE_NAME}>)
            
            # 创建QEMU运行目标 - 正确传递列表参数
            add_custom_target(${QEMU_TARGET}
                COMMAND ${CMAKE_COMMAND} -E echo "在QEMU中运行 ${MODULE_NAME}..."
                COMMAND ${CMAKE_COMMAND} -E echo "命令: $<JOIN:${QEMU_CMD}, >"
                COMMAND ${QEMU_CMD}
                DEPENDS ${MODULE_NAME}
                COMMENT "在QEMU中运行 ${MODULE_NAME}"
                VERBATIM
            )
        endif()

        # 如果需要调试支持，添加-s、-S和-gdb选项，并后台执行
        if(MODULE_ENABLE_GDB_DEBUG)
            message(STATUS "  [${MODULE_NAME}] GDB配置: 启用")
            message(STATUS "  [${MODULE_NAME}] GDB可执行文件: ${MODULE_GDB_EXECUTABLE}")
            message(STATUS "  [${MODULE_NAME}] GDB端口: ${MODULE_GDB_PORT}")

            # 设置调试目标名
            set(DEBUG_TARGET "${MODULE_NAME}_debug")

            # 设置调试目标参数
            # =================================
            # 构建QEMU命令列表
            set(DEBUG_QEMU_CMD ${MODULE_QEMU_EXECUTABLE})
            
            # 添加基本参数
            list(APPEND DEBUG_QEMU_CMD -machine ${MODULE_QEMU_MACHINE})
            list(APPEND DEBUG_QEMU_CMD -cpu ${MODULE_QEMU_CPU})
            
            # 添加串口设置
            if(MODULE_QEMU_SERIAL)
                list(APPEND DEBUG_QEMU_CMD -serial ${MODULE_QEMU_SERIAL})
            endif()
            
            # 添加监控器设置
            if(MODULE_QEMU_MONITOR)
                list(APPEND DEBUG_QEMU_CMD -monitor ${MODULE_QEMU_MONITOR})
            endif()
            
            # 添加额外选项
            foreach(option ${MODULE_QEMU_OPTIONS})
                list(APPEND DEBUG_QEMU_CMD ${option})
            endforeach()
            
            # 添加内核文件
            list(APPEND DEBUG_QEMU_CMD -kernel $<TARGET_FILE:${MODULE_NAME}>)

            list(APPEND DEBUG_QEMU_CMD -s -S)

            # 构建GDB命令列表
            set(GDB_CMD ${MODULE_GDB_EXECUTABLE})

            # 添加GDB脚本
            if(MODULE_GDB_SCRIPT)
                if(NOT IS_ABSOLUTE ${MODULE_GDB_SCRIPT})
                    set(MODULE_GDB_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/${MODULE_GDB_SCRIPT})
                endif()
                list(APPEND GDB_CMD -x ${MODULE_GDB_SCRIPT})
            endif()
            
            # 添加GDB启动命令
            list(APPEND GDB_CMD -q)
            
            # 添加额外选项（使用列表形式）
            foreach(option ${MODULE_GDB_OPTIONS})
                list(APPEND GDB_CMD ${option})
            endforeach()
            
            # 添加要调试的可执行文件
            list(APPEND GDB_CMD $<TARGET_FILE:${MODULE_NAME}>)
            
            # 创建QEMU运行目标 - 使用shell执行以支持后台操作
            add_custom_target(${DEBUG_TARGET}
                COMMAND ${CMAKE_COMMAND} -E echo "在QEMU中运行 ${MODULE_NAME}..."
                COMMAND ${CMAKE_COMMAND} -E echo "QEMU命令: $<JOIN:${DEBUG_QEMU_CMD}, >"
                COMMAND ${CMAKE_COMMAND} -E echo "GDB命令: $<JOIN:${GDB_CMD}, >"
                COMMAND bash -c "$<JOIN:${DEBUG_QEMU_CMD}, > & $<JOIN:${GDB_CMD}, >"
                DEPENDS ${MODULE_NAME}
                COMMENT "在QEMU中运行 ${MODULE_NAME} 并启动GDB调试"
                VERBATIM
            )
            # =================================

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
