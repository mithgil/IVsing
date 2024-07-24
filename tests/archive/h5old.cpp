// Create HDF5 file
    // File name
    const std::string FILE_NAME(output_file);

    // Dataset name
    const std::string DATASET_NAME("2DArray");
    
    H5::H5File file(FILE_NAME, H5F_ACC_TRUNC);

    // Create the data space for the dataset.
    hsize_t dims[2] = {static_cast<hsize_t>(pixels), static_cast<hsize_t>(file_count + 1)};
    H5::DataSpace dataspace(2, dims);

    // Flatten the 2D vector into a 1D array: since data writing expect the data to be continuous from a pointer but it does not. 
    std::vector<double> flat_data;
    flat_data.reserve(pixels * (file_count + 1));
    for (const auto& row : iv_arr) {
        flat_data.insert(flat_data.end(), row.begin(), row.end());
    }

    // Create the dataset.
    H5::DataSet dataset = file.createDataSet(DATASET_NAME, H5::PredType::NATIVE_DOUBLE, dataspace);

    // Write the data to the dataset.
    dataset.write(flat_data.data(), H5::PredType::NATIVE_DOUBLE);

    std::cout << "Data written to " << FILE_NAME << std::endl;

    // Close the HDF5 file
    file.close();