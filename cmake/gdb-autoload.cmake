function(generate_gdb_autoload exe_name)
    configure_file("autoload-gdb.py" "${CMAKE_CURRENT_BINARY_DIR}/${exe_name}-gdb.py")
endfunction()