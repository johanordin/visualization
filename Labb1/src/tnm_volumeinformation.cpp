#include "modules/tnm093/include/tnm_volumeinformation.h"
#include "voreen/core/datastructures/volume/volumeatomic.h"
#include <iostream>
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
                // use iX, iY, iZ, i, and the VolumeUInt16::voxel method to derive the measures here

                //
                // Intensity
                //
                //float intensity = -1.f;

                // Retrieve the intensity using the 'VolumeUInt16's voxel method
                float intensity = volume->voxel(i);

                _data->at(i).dataValues[0] = intensity;

                //
                // Average
                //
                float average = 0;
                int count =0;

                for(int x=int(iX-1); x<= int(iX+1); x++)
                {
                    if(x<0) {
                        x++;
                    }
                    if (x > int(dimensions.x)-1) {
                        break;
                    }
                    for(int y=int(iY-1); y <= int(iY+1); y++)
                    {
                        if(y<0) {
                            y++;
                        }
                        if (y > int(dimensions.y)-1) {
                            break;
                        }
                            for(int z=int(iZ-1); z<=int(iZ+1); z++)
                            {
                                if (z > int(dimensions.z)-1) {
                                    break;
                                }
                                if(z<0) {
                                    z++;
                                }
                                average += volume->voxel(x,y,z);
                                count++;
                            }
                    }
                }
                //average = average - (2 * intensity);
                average = average/count;

                //float average = -1.f;
                // Compute the average; the voxel method accepts both a single parameter
                // as well as three parameters

                _data->at(i).dataValues[1] = average;

                //
                // Standard deviation
                //
                //float stdDeviation = -1.f;
                float stdDeviation =0;
                int dcount = -1;

                for(int x=int(iX-1); x<=int(iX+1); x++)
                {
                    if(x<0)
                    {
                        x++;
                    }
                    if (x>int(dimensions.x)-1)
                    {
                        break;
                    }
                    for(int y=int(iY-1); y<=int(iY+1); y++)
                    {
                        if(y<0)
                        {
                            y++;
                        }
                        if (y>int(dimensions.y)-1)
                        {
                            break;
                        }
                        for(int z=int(iZ-1); z<=int(iZ+1); z++)
                        {
                            if (z>int(dimensions.z)-1)
                            {
                                break;
                            }
                            if(z<0)
                            {
                                z++;
                            }
                            stdDeviation += pow(volume->voxel(x,y,z) - average,2);
                            dcount++;
                        }
                    }
                }


                stdDeviation = sqrt(stdDeviation/dcount);

                //stdDeviation = -1.f;
                // Compute the standard deviation


                _data->at(i).dataValues[2] = stdDeviation;

                //
                // Gradient magnitude
                //
                float gradientMagnitude = -1.f;
                // Compute the gradient direction using either forward, central, or backward
                // calculation and then take the magnitude (=length) of the vector.
                // Hint:  tgt::vec3 is a class that can calculate the length for you

/*
                if (iX )
                float gx = voxel((iX+1,iY,iZ) - voxel(iX-1,iY,iZ))/2;
                float gy = voxel((iX,iY+1,iZ) - voxel(iX,iY-1,iZ))/2;
                float gz = voxel((iX,iY,iZ+1) - voxel(iX,iY,iZ-1))/2;
              */

                float gx1;
                float gx2;
                float gy1;
                float gy2;
                float gz1;
                float gz2;


                if(iX==int(dimensions.x-1))
                {
                     gx1=0;

                }
                else
                {
                    gx1 =volume->voxel(iX+1,iY,iZ);
                }

                if(iX==0)
                {
                     gx2=0;

                }
                else
                {
                    gx2 =volume->voxel(iX-1,iY,iZ);
                }
                //y

                if(iY==int(dimensions.y-1))
                {
                     gy1=0;

                }
                else
                {
                    gy1 =volume->voxel(iX,iY+1,iZ);
                }

                if(iY==0)
                {
                     gy2=0;

                }
                else
                {
                    gy2 =volume->voxel(iX,iY-1,iZ);
                }
                //z
                if(iZ==int(dimensions.z-1))
                {
                     gz1=0;

                }
                else
                {
                    gz1 =volume->voxel(iX,iY,iZ+1);
                }

                if(iZ==0)
                {
                     gz2=0;

                }
                else
                {
                    gz2 =volume->voxel(iX,iY,iZ-1);
                }


                float gx = (gx1 - gx2)/2;
                float gy = (gy1 - gy2)/2;
                float gz = (gz1 - gz2)/2;


                gradientMagnitude= sqrt(pow(gx,2) + pow(gy,2) + pow(gz,2) );

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
