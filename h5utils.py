# h5utils.py

import h5py
import numpy as np

class HDF5Analyzer:
    def __init__(self, filepath):
        self.filepath = filepath
        self.h5struct = {}
    
    def list_all_nodes(self, name, obj):
        """
        This function is called by f.visititems() for every object in the file.
        'name' is the object's full path.
        'obj' is the h5py object (Group, Dataset, etc.).
        """
        indent = '  ' * name.count('/')
        
        # Check if the object is a Group (a container for other objects)
        if isinstance(obj, h5py.Group):
            print(f"{indent}GROUP: /{name}/")
        
        # Check if the object is a Dataset (contains data)
        elif isinstance(obj, h5py.Dataset):
            print(f"{indent}  DATASET: /{name} (Shape: {obj.shape}, Dtype: {obj.dtype})")
            # You can add code here to read the data if needed, e.g.,
            # data = obj[:]
            # print(f"{indent}    - First 5 values: {data.flatten()[:5]}")
        
        if obj.attrs:
            for attr_name, attr_value in obj.attrs.items():
                print(f"{indent}  - ATTRIBUTE: '{attr_name}' = '{attr_value}'")
    
    def list_h5_struct(self):
        try:
            with h5py.File(self.filepath, 'r') as f:
                # The `visititems` method will start at the root '/' and call our function
                # for every single item it finds, providing its full path and the object.
                print(f"--- Listing all groups and datasets in '{self.filepath}' by traversing the hierarchy ---")
                f.visititems(self.list_all_nodes)
                
        except FileNotFoundError:
            print(f"Error: The HDF5 file '{self.filepath}' was not found. Please ensure it exists.")
        except Exception as e:
            print(f"An error occurred while reading the HDF5 file: {e}")
    
    
    def build_h5_structure_dict(self, group, read_data=False):
        """
        Recursively builds a nested dictionary representing the HDF5 file's
        groups and datasets.
    
        Args:
            group (h5py.Group or h5py.File): The starting group/file to traverse.
            read_data (bool): If True, reads the dataset content into a NumPy array
                              and stores it in the dictionary. Be careful with large datasets.
    
        Returns:
            dict: A nested dictionary representing the structure.
        """
        structure = {}
        
        # Store attributes of the current group/file
        if group.attrs:
            structure['_attributes'] = {key: val for key, val in group.attrs.items()}
    
        # Iterate over the items in the current group
        for key, obj in group.items():
            if isinstance(obj, h5py.Group):
                # If the item is a group, recursively call the function
                structure[key] = self.build_h5_structure_dict(obj, read_data)
                
            elif isinstance(obj, h5py.Dataset):
                # If the item is a dataset, store its metadata
                dataset_info = {
                    'type': 'dataset',
                    'shape': obj.shape,
                    'dtype': str(obj.dtype),
                    'path': obj.name # Store the full path for easy access
                }
                
                if read_data:
                    # Read the entire dataset into a NumPy array
                    dataset_info['data'] = obj[:]
                    
                # Store attributes of the dataset
                if obj.attrs:
                     dataset_info['_attributes'] = {attr_key: attr_val for attr_key, attr_val in obj.attrs.items()}
    
                structure[key] = dataset_info
                
        return structure
    
    
    def build_struct_dict(self):
        
        try:
            print(f"--- Building a Python dictionary from HDF5 file '{self.filepath}' ---")
            
            with h5py.File(self.filepath, 'r') as f:
                # Start the recursion from the root group `f`
                # Pass `read_data=True` to store the actual NumPy arrays
                self.h5struct = self.build_h5_structure_dict(f, read_data=True)
            
            print("\nSuccessfully built the structured dictionary. File is now closed.")
    
            return h5_structure_dict
        except FileNotFoundError:
            print(f"Error: The file '{self.filepath}' was not found.")
        except Exception as e:
            print(f"An error occurred: {e}")

    def get_nearest_index(self, bias):

        h5_structure_dict = self.h5struct

        biasVector = h5_structure_dict['data']['bias']['data']
        
        index = np.argmin(np.abs(biasVector - bias))
        print(f"Bias sliced at index {index}, bias = {biasVector[index]}")
        return index
    
    def get_position_index(self, xIndex, yIndex):
        
        h5_structure_dict = self.h5struct
        pixels = h5_structure_dict['scanpixels']['values']['data']
        position_index = xIndex + yIndex * pixels[1]
        
        return position_index

    


