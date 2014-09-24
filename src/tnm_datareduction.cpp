#include "modules/tnm093/include/tnm_datareduction.h"

namespace voreen {

namespace {
	// We will sort the data using this function to ensure a strong ordering on the voxel indices
	bool sortByIndex(const VoxelDataItem& lhs, const VoxelDataItem& rhs) {
		return lhs.voxelIndex < rhs.voxelIndex;
	}

}

TNMDataReduction::TNMDataReduction()
    : _inport(Port::INPORT, "in.data")
    , _outport(Port::OUTPORT, "out.data")
    , _percentage("percentage", "Percentage of Dropped Data")
{
    addPort(_inport);
    addPort(_outport);
    addProperty(_percentage);
}

Processor* TNMDataReduction::create() const {
    return new TNMDataReduction;
}

void TNMDataReduction::process() {
    if (!_inport.hasData())
        return;

	// We have checked above that there is data, so the dereferencing is safe
    const Data& inportData = *(_inport.getData());
    const float percentage = _percentage.get();

	// Our new data
	Data* outportData = new Data;
    for (size_t i = 0; i < inportData.size(); ++i) {
        const VoxelDataItem& item = inportData[i];

        // Filter the 'inportData' based on the percentage and add it to 'outportData'

    }

	// sort the data by the voxel index for faster processing later
	std::sort(outportData->begin(), outportData->end(), sortByIndex);
	// Place the new data into the outport (and transferring ownership at the same time)
    _outport.setData(outportData);
}

} // namespace
