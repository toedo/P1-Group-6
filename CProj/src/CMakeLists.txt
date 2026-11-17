# We create a library for the functions in the import script 
add_library(csv_importer csv_importer.c)
# Make header files in the current folder available for #include
target_include_directories(csv_importer PUBLIC .)

# We create an executable to run the program
add_executable(import_aalborg_data import_aalborg_data.c)
# We link our library to the executable, so the executable can access the functions in the library.
target_link_libraries(import_aalborg_data csv_importer)