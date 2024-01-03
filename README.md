# MyContainers
My implementation of some STL containers such as std::vector, std::list, std::map, std::set, std::unordered_map, std::unordered_set

This is a short description of each file in this repository. All my container implementations are in the namespace My{}

# Vector.cpp
My implementation of std::vector. This file contains the implementation of My::Vector class, iterator inner class and function main(), which shows some of the capabilities of My::Vector

# List.cpp
My implementation of std::list. This file contains the implementation of My::List class, iterator inner class and function main(), which shows some of the capabilities of My::List

# Map.cpp 
My implementation of std:map. This file contains the implementation of My::Map class which is based on red-black tree, iterator inner class and function main(), which shows some of the capabilities of My::Map

# Set.cpp
My implementation of std::set. This file contains the implementation of My::Set class which is based on red-black tree, iterator inner class and function main(), which shows some of the capabilities of My::Set

# HashMap.cpp
My implementation of std::unordered_map. This file contains the implementation of My::HashMap class which is based on hash table with open addressing, iterator inner class which is based on My::Vector and function main(), which shows some of the capabilities of My::HashMap

# HashSet.cpp
My implementation of std::unordered_set. This file contains the implementation of My::HashSet class which is based on hash table with open addressing, iterator inner class which is based on My::Vector and function main(), which shows some of the capabilities of My::HashSet

# Additional files
Vector.hpp - I created this file as a header-only library to implement iterators for My::HashSet and My::HashMap using My::Vector

TestHashAndAllocator.hpp - This file contains the implementation of a custom Hasher and Allocator for tests
