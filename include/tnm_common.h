#ifndef VRN_TNM_COMMON_H
#define VRN_TNM_COMMON_H

#include "voreen/core/ports/genericport.h"

#include <vector>

namespace voreen {

// The total number of data values that will be extracted
// Intensity + Average + Standard Deviation + Gradient Magnitude
const int NUM_DATA_VALUES = 4;


struct VoxelDataItem { // There is one VoxelDataItem struct for each voxel in the dataset
    unsigned int voxelIndex; // This is the index of the voxel from which the data was retrieved
    float dataValues[NUM_DATA_VALUES]; // The list of data values for this specific voxel
};

// Just a typedef to make handling the data values easier
typedef std::vector<VoxelDataItem> Data;
// This port will be added to processors in order to exchange Data objects
typedef GenericPort<Data> DataPort;

} // namespace

#endif // VRN_TNM_COMMON_H