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
    for (int iX = 0; iX < dimensions.x; ++iX) {
        for (int iY = 0; iY < dimensions.y; ++iY) {
            for (int iZ = 0; iZ < dimensions.z; ++iZ) {
				// i is a unique identifier for the voxel calculated by the following
				// (probably one of the most important) formulas:
				// iZ*dimensions.x*dimensions.y + iY*dimensions.x + iX;
                const size_t i = VolumeUInt16::calcPos(volume->getDimensions(), tgt::svec3(iX, iY, iZ));

				// Setting the unique identifier as the voxelIndex
				_data->at(i).voxelIndex = i;
                //
				// use iX, iY, iZ, i, and the VolumeUInt16::voxel method to derive the measures here
                //

				//
				// Intensity
				//
				
				// Retrieve the intensity using the 'VolumeUInt16's voxel method
				float intensity = volume->voxel(i);

				_data->at(i).dataValues[0] = intensity;

				//
				// Average
				//
				float average = -1.f;
				// Compute the average; the voxel method accepts both a single parameter
				// as well as three parameters
				float averageCounter = 0.0f;
				int divideCounter = 0;
				for(int jX = 0; jX < 3; jX++){
				  for(int jY = 0; jY < 3; jY++){
				    for(int jZ = 0; jZ < 3; jZ++){
				      
				      int tempX = iX-1+jX;
				      int tempY = iY-1+jY;
				      int tempZ = iZ-1+jZ;
				      
				      if(tempX < dimensions.x && tempX >= 0 &&
					 tempY < dimensions.y && tempY >= 0 &&
					 tempZ < dimensions.z && tempZ >= 0 ){
					  averageCounter += volume->voxel(tempX, tempY, tempZ);
					  divideCounter++;
				      }
				    }
				  }
				}
				
				
				
				average = averageCounter/divideCounter;
				
				_data->at(i).dataValues[1] = average;

				//
				// Standard deviation
				//
				float stdDeviation = -1.f;
				// Compute the standard deviation
				
				float deviationCounter = 0.0f;
				
				for(int jX = 0; jX < 3; jX++){
				  for(int jY = 0; jY < 3; jY++){
				    for(int jZ = 0; jZ < 3; jZ++){
				      int tempX = iX-1+jX;
				      int tempY = iY-1+jY;
				      int tempZ = iZ-1+jZ;
				      
				      if(tempX < dimensions.x && tempX >= 0 &&
					 tempY < dimensions.y && tempY >= 0 &&
					 tempZ < dimensions.z && tempZ >= 0 ){
					  deviationCounter += pow(volume->voxel(tempX, tempY, tempZ) - average, 2);
				      }
				    }
				  }
				}
				
				stdDeviation = sqrt(deviationCounter/divideCounter);
				_data->at(i).dataValues[2] = stdDeviation;

				//
				// Gradient magnitude
				//
				// Compute the gradient direction using either forward, central, or backward
				// calculation and then take the magnitude (=length) of the vector.
				// Hint:  tgt::vec3 is a class that can calculate the length for you
				
				//Forward difference?
				float gradX, gradY, gradZ;
				if (iX < dimensions.x-1)
				  gradX = volume->voxel(iX, iY, iZ) - volume->voxel(iX+1, iY, iZ);
				else
				  gradX = 0;
				if (iY < dimensions.y-1)
				  gradY = volume->voxel(iX, iY, iZ) - volume->voxel(iX, iY+1, iZ);
				else
				  gradY = 0;
				if (iZ < dimensions.z-1)
				  gradZ = volume->voxel(iX, iY, iZ) - volume->voxel(iX, iY, iZ+1);
				else
				  gradZ = 0;
				
				//tgt::vec3* grad = new tgt::vec3(gradX, gradY, gradZ);
				tgt::vec3 grad(gradX, gradY, gradZ);
				float gradientMagnitude = tgt::length(grad);
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
