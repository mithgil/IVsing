// Temporary storage to hold the converted C-style strings
        std::vector<std::vector<std::unique_ptr<char[]>>> temp_storage;

        for (const auto& row : data) {
            std::vector<std::unique_ptr<char[]>> row_cstr;
            for (const auto& cell : row) {
                // Allocate memory and copy string content
                auto temp = std::make_unique<char[]>(cell.size() + 1); // +1 for null terminator
                std::copy(cell.begin(), cell.end(), temp.get());
                temp[cell.size()] = '\0'; // Null-terminate the string
                row_cstr.push_back(std::move(temp));
            }
            
            // Store the pointers to the allocated memory in data_cstr
            std::vector<const char*> row_ptrs;
            for (auto& ptr : row_cstr) {
                row_ptrs.push_back(ptr.get());
            }
            
            // Move row_ptrs into data_cstr
            data_cstr.insert(data_cstr.end(), row_ptrs.begin(), row_ptrs.end());
            
            // Store row_cstr in temp_storage to keep memory valid
            temp_storage.push_back(std::move(row_cstr));
        }