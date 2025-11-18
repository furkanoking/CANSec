cmake_minimum_required(VERSION 3.25)
project(CANSecKod)


cmake_minimum_required(VERSION 3.25)
project(CANSecKod)

set(CMAKE_CXX_STANDARD 23)

add_library(CANSecKod STATIC CANSec.cpp
        CANFDStructure/CANFD.cpp)

target_include_directories(CANSecKod PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/CANFDStructure)

# Link OpenSSL (required for encryption/decryption)
find_package(OpenSSL REQUIRED)
target_link_libraries(CANSecKod PRIVATE OpenSSL::SSL OpenSSL::Crypto)

enable_testing()
find_package(Threads REQUIRED) # GTest'in buna da ihtiyacı var

# 1. Gerekli tüm yolları en başta tanımlayın
set(GTEST_INCLUDE_DIR "/usr/include")
set(GTEST_LIBRARY "/usr/lib/libgtest.a")
set(GTEST_MAIN_LIBRARY "/usr/lib/libgtest_main.a")

# 2. GTest::gtest hedefini ve özelliklerini tanımlayın (HATANIN ÇÖZÜMÜ)
add_library(GTest::gtest STATIC IMPORTED)
set_target_properties(GTest::gtest PROPERTIES
        IMPORTED_LOCATION ${GTEST_LIBRARY}
        INTERFACE_INCLUDE_DIRECTORIES ${GTEST_INCLUDE_DIR}
)

# 3. GTest::gtest_main hedefini ve özelliklerini tanımlayın (Bu sizde vardı)
add_library(GTest::gtest_main STATIC IMPORTED)
set_target_properties(GTest::gtest_main PROPERTIES
        IMPORTED_LOCATION ${GTEST_MAIN_LIBRARY}
        INTERFACE_LINK_LIBRARIES GTest::gtest  # gtest_main'i gtest'e bağla
)

# 4. Executable'ı oluşturun
add_executable(test_main tests/test.cpp)

# 5. Linklemeyi yapın (tek komut yeterli)
target_link_libraries(test_main PRIVATE
        CANSecKod
        GTest::gtest_main # gtest_main zaten gtest'i ve Threads'i getirecek
        Threads::Threads
)