#include <H5Cpp.h>
#include <vector>
#include <string>
#include <iostream>

void write_data_with_headers(const std::string& filename, const std::string& dataset_name, const std::vector<std::vector<double>>& data, const std::vector<std::string>& headers) {
    // Open a new file using the default property lists.
    H5::H5File file(filename, H5F_ACC_TRUNC);

    // Create the data space for the dataset.
    hsize_t dims[2] = {static_cast<hsize_t>(data.size()), static_cast<hsize_t>(data[0].size())};
    H5::DataSpace dataspace(2, dims);

    // Flatten the 2D vector into a 1D array
    std::vector<double> flat_data;
    flat_data.reserve(data.size() * data[0].size());
    for (const auto& row : data) {
        flat_data.insert(flat_data.end(), row.begin(), row.end());
    }

    // Create the dataset.
    H5::DataSet dataset = file.createDataSet(dataset_name, H5::PredType::NATIVE_DOUBLE, dataspace);

    // Write the data to the dataset.
    dataset.write(flat_data.data(), H5::PredType::NATIVE_DOUBLE);

    // Create an attribute for column headers.
    hsize_t header_dims[1] = {static_cast<hsize_t>(headers.size())};
    H5::DataSpace header_space(1, header_dims);
    H5::StrType str_type(H5::PredType::C_S1, H5T_VARIABLE); // Variable length string type

    H5::Attribute header_attr = dataset.createAttribute("headers", str_type, header_space);
    std::vector<const char*> c_headers;
    for (const auto& header : headers) {
        c_headers.push_back(header.c_str());
    }

    header_attr.write(str_type, c_headers.data());

    std::cout << "Data written to " << filename << std::endl;

    // Close the HDF5 file
    file.close();
}

int main() {
    // Example data
    std::vector<std::vector<double>> iv_arr = {
        {0.1, 0.2, 0.3},
        {1.1, 1.2, 1.3},
        {2.1, 2.2, 2.3}
    };

    // Example headers
    std::vector<std::string> headers = {"Bias", "Current1", "Current2"};

    // Write data with headers to HDF5
    write_data_with_headers("test.h5", "2DArray", iv_arr, headers);

    return 0;
}
