include(CheckCompilerFlag)
include(CMakePushCheckState)

# Choose the compiler driver option for LLVM_USE_LINKER. Linker flavors use
# -fuse-ld=, while paths use --ld-path= when supported.
function(llvm_get_linker_flag out_var)
  set(linker_flag "-fuse-ld=${LLVM_USE_LINKER}")
  if(IS_ABSOLUTE "${LLVM_USE_LINKER}" OR
     LLVM_USE_LINKER MATCHES "[/\\\\]")
    # try_compile runs outside the build directory, so anchor relative paths to
    # it. Preserve path components: collapsing '..' changes their meaning when
    # the preceding directory is a symlink.
    set(linker_path "${LLVM_USE_LINKER}")
    if(NOT IS_ABSOLUTE "${linker_path}")
      set(linker_path "${CMAKE_BINARY_DIR}/${linker_path}")
    endif()
    set(linker_flag "-fuse-ld=${linker_path}")

    # Check driver option support without invoking a linker. Passing the option
    # only when compiling also avoids sending it to the static library archiver.
    # Its unused-argument warning is expected here; an unknown option still fails.
    # The placeholder avoids quoting paths and need not name an executable in
    # this compile-only check. HandleLLVMOptions validates the selected linker.
    cmake_push_check_state(RESET)
    set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
    set(CMAKE_REQUIRED_FLAGS "-Wno-unused-command-line-argument")
    set(use_ld_path TRUE)
    foreach(lang C CXX)
      if(CMAKE_${lang}_COMPILER_LOADED)
        # Clang's MSVC target uses -fuse-ld= for linker selection even though
        # --ld-path= is accepted by the compile-only check below.
        if(CMAKE_${lang}_SIMULATE_ID STREQUAL "MSVC")
          set(use_ld_path FALSE)
          break()
        endif()
        check_compiler_flag(${lang} "--ld-path=ld"
          ${lang}_COMPILER_SUPPORTS_LD_PATH)
        if(NOT ${lang}_COMPILER_SUPPORTS_LD_PATH)
          set(use_ld_path FALSE)
          break()
        endif()
      endif()
    endforeach()
    cmake_pop_check_state()
    if(use_ld_path)
      set(linker_flag "--ld-path=${linker_path}")
    endif()
  endif()
  set(${out_var} "${linker_flag}" PARENT_SCOPE)
endfunction()
