cmake_minimum_required(VERSION 3.20)
project(GraphGenerator)

add_executable(${PROJECT_NAME} "Main.cpp")
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_sources(${PROJECT_NAME} PRIVATE "Octree.hpp" "Octree.ipp" "Vector3.hpp" "Matrix3.hpp" "PathGraph.hpp" "PathGraph.ipp" "DebugMemory.cpp" "Bitmap.hpp" "PathGraphInterface.hpp" "PathGraphInterface.cpp"  "AllocatorTraits.hpp" "Windows/ReservedVirtualMemory.hpp" "Windows/ReservedVirtualMemory.cpp" "Windows/MonotonicAllocator.hpp" "SimpleHashSet.hpp" "SimpleHashMap.hpp" "Unix/ReservedVirtualMemory.hpp" "Unix/ReservedVirtualMemory.cpp")

# find_package(absl CONFIG REQUIRED)
# target_link_libraries(${PROJECT_NAME} absl::any absl::base absl::bits absl::city)