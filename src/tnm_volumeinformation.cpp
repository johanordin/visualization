#include "modules/tnm093/include/tnm_volumeinformation.h"
#include "voreen/core/datastructures/volume/volumeatomic.h"

namespace voreen {

	const std::string loggerCat_ = "TNMVolumeInformation";

namespace {
	// This ordering function allows us to sort the Data vector by the voxelIndex
	// The extraction *should* produce a sorted list, but you never know
	bool sortByIndex(const VoxelDataItem& lhs, const VoxelDataItem& rhs) {
		return lhs.voxelIndex < rhs.voxelIndex;
	}

}

TNMVolumeInformation::TNMVolumeInformation()
    : Processor()
    , _inport(Port::INPORT, "in.volume")
    , _outport(Port::OUTPORT, "out.data")
    , _data(0)
{
    addPort(_inport);
    addPort(_outport);
}

TNMVolumeInformation::~TNMVolumeInformation() {
    delete _data;
}

void TNMVolumeInformation::process() {
    const VolumeHandleBase* volumeHandle = _inport.getData();
    const Volume* baseVolume = volumeHandle->getRepresentation<Volume>();
    const VolumeUInt16* volume = dynamic_cast<const VolumeUInt16*>(baseVolume);
    if (volume == 0)
        return;
	// If we get this far, there actually is a volume to work with

	// If this is the first call, we will create the Data object
	if (_data == 0)
        _data = new Data;

	// Retrieve the size of the three dimensions of the volume
    const tgt::svec3 dimensions = volume->getDimensions();
	// Create as many data entries as there are voxels in the volume
    _data->resize(dimensions.x * dimensions.y * dimensions.z);

	// iX is the index running over the 'x' dimension
	// iY is the index running over the 'y' dimension
	// iZ is the index running over the 'z' dimension
    for (size_t iX = 0; iX < dimensions.x; ++iX) {
        for (size_t iY = 0; iY < dimensions.y; ++iY) {
            for (size_t iZ = 0; iZ < dimensions.z; ++iZ) {
				// i is a unique identifier for the voxel calculated by the following
				// (probably one of the most important) formulas:
				// iZ*dimensions.x*dimensions.y + iY*dimensions.x + iX;
				const size_t i = VolumeUInt16::calcPos(volume->getDimensions(), tgt::svec3(iX, iY, iZ));

				// Setting the unique identifier as the voxelIndex
				_data->at(i).voxelIndex = i;
				//
				// use iX, iY, iZ, i, and the VolumeUInt16::voxel method to derive the measures here
				//
				//volume->voxel(ix,iy,iz)
				//
				// Intensity
				//
				//float intensity = -1.f;
				// Retrieve the intensity using the 'VolumeUInt16's voxel method
				float intensity = volume->voxel(iX,iY,iZ);

				_data->at(i).dataValues[0] = intensity;

				//
				// Average
				//
				float average = 0.f;
				int counter = 0;

// 				for(int z = -1; z < 2; z++)
// 				{
// 				    for(int y = -1; y < 2; y++)
// 				    {
// 					for(int x = -1; x < 2; x++)
// 					{
// 					    if( y > 0 && y < dimensions.y || x > 0 && x < dimensions.x || z > 0 && z < dimensions.z ){
// 					    
// 					    average += volume->voxel(iX,iY,iZ);
// 					    counter++;
// 					    }
// 					}
// 				    }
// 				}

				// Compute the average; the voxel method accepts both a single parameter
				// as well as three parameters

				_data->at(i).dataValues[1] = average/counter;

				//
				// Standard deviation
				//
				float stdDeviation = -1.f;
				// Compute the standard deviation


				_data->at(i).dataValues[2] = stdDeviation;

				//
				// Gradient magnitude
				//
				float gradientMagnitude = -1.f;
				// Compute the gradient direction using either forward, central, or backward
				// calculation and then take the magnitude (=length) of the vector.
				// Hint:  tgt::vec3 is a class that can calculate the length for you


				_data->at(i).dataValues[3] = gradientMagnitude;
            }
        }
    }

	// sort the data by the voxel index for faster processing later
	std::sort(_data->begin(), _data->end(), sortByIndex);

	// And provide access to the data using the outport
    _outport.setData(_data, false);
}

} // namespace
