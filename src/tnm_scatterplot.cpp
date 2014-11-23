#include "modules/tnm093/include/tnm_scatterplot.h"

#include <algorithm>
#include <limits>

namespace voreen {

TNMScatterPlot::TNMScatterPlot()
    : RenderProcessor()
    , _inport(Port::INPORT, "in.data")
    , _outport(Port::OUTPORT, "out.image")
	, _shader(0)
    , _firstAxis("firstAxis", "First Axis")
    , _secondAxis("secondAxis", "Second Axis")
	, _brushingIndices("brushingIndices", "Brushing Indices")
	, _linkingIndices("linkingIndices", "Linking Indices")
{
    addPort(_inport);
    addPort(_outport);

    addProperty(_firstAxis);
    addProperty(_secondAxis);
	addProperty(_brushingIndices);
	addProperty(_linkingIndices);

	// Assign the option value "Intensity" to the value 0 etc
    _firstAxis.addOption("0", "Intensity", 0);
    _firstAxis.addOption("1", "Average", 1);
    _firstAxis.addOption("2", "Standard Deviation", 2);
    _firstAxis.addOption("3", "Gradient Magnitude", 3);

    _secondAxis.addOption("0", "Intensity", 0);
    _secondAxis.addOption("1", "Average", 1);
    _secondAxis.addOption("2", "Standard Deviation", 2);
    _secondAxis.addOption("3", "Gradient Magnitude", 3);
}

void TNMScatterPlot::initialize() throw (tgt::Exception) {
	// Load the shaders and return the pointer to the shader program
	_shader = ShdrMgr.loadSeparate("scatterplot.vert", "scatterplot.frag");
}

void TNMScatterPlot::deinitialize() throw (tgt::Exception) {
	ShdrMgr.dispose(_shader);
}

void TNMScatterPlot::process() {
    if (!_inport.hasData())
        return;

	// Activate the outport as the rendering target
    _outport.activateTarget();
	// Clear the buffer
    _outport.clearTarget();

	// Access the provided data. We have already checked before that it exists, so dereferencing it here is safe
    const Data& data = *(_inport.getData());

	// The set contains all indices of voxels that should be ignored
	const std::set<unsigned int>& brushingIndices = _brushingIndices.get();
	// The set contains all indices of voxels that should be visually selected
	const std::set<unsigned int>& selectionIndices = _linkingIndices.get();

	// The number of points is equal to the number in the original dataset minus the number we are ignoring
	const size_t dataSize = data.size() - brushingIndices.size();
	// There are 2 coordinate components for each point
	const size_t nCoordinateComponents = dataSize * 2;

	// The vector containing the position data
	std::vector<float> positionData;
	// Make room in the vector
	positionData.resize(nCoordinateComponents); 

	// The vector containing boolean flags saying for each position if it is selected
	// OpenGL doesn't support boolean values for the vertex buffer, so we take the next best thing instead
	std::vector<unsigned char> selectionData(dataSize, 0);
	
	// In order to map the value ranges to [-1,1] we need to find the mininum and maximum values
	float minimumFirstCoordinate = std::numeric_limits<float>::max();
	float maximumFirstCoordinate = -std::numeric_limits<float>::max();
	float minimumSecondCoordinate = std::numeric_limits<float>::max();
	float maximumSecondCoordinate = -std::numeric_limits<float>::max();
	// i: index into the data
	// j: index into the coordinates
	for (size_t i = 0, j = 0; i < data.size(); ++i) {
		//// See if the index i is in the vector for brushing
		if (brushingIndices.find(data[i].voxelIndex) != brushingIndices.end())
			// If it is, we ignore it
			continue;
		else {
			// otherwise add it to the position data
			// _firstAxis.getValue() and _secondAxis.getValue() returns the integer value specified above
			// to determine which selection was chosen in the GUI
			const float firstCoordinate = data[i].dataValues[_firstAxis.getValue()];
			const float secondCoordinate = data[i].dataValues[_secondAxis.getValue()];
			positionData[j] = firstCoordinate;
			positionData[j+1] = secondCoordinate;
			j += 2;

			minimumFirstCoordinate = std::min(minimumFirstCoordinate, firstCoordinate);
			maximumFirstCoordinate = std::max(maximumFirstCoordinate, firstCoordinate);
			minimumSecondCoordinate = std::min(minimumSecondCoordinate, secondCoordinate);
			maximumSecondCoordinate = std::max(maximumSecondCoordinate, secondCoordinate);
		}
	}

	// In a second step, we need to normalize the found data. By now we have looked at each value and found the
	// min/max values
	// Normalizing the data values to the range [-1,1]
	for (size_t i = 0; i < nCoordinateComponents; i+=2) {
		// First normalize to [0,1]
		positionData[i] = (positionData[i] - minimumFirstCoordinate) / (maximumFirstCoordinate - minimumFirstCoordinate);
		positionData[i+1] = (positionData[i+1] - minimumSecondCoordinate) / (maximumSecondCoordinate - minimumSecondCoordinate);
		
		// Then shift the normalized values to [-1,1]
		positionData[i] = (positionData[i] - 0.5f) * 2.f;
		positionData[i+1] = (positionData[i+1] - 0.5f) * 2.f;
	}

	// Set the selection array for all selected indices
	for (std::set<unsigned int>::const_iterator i = selectionIndices.begin();
		i != selectionIndices.end();
		++i) 
	{
		const unsigned int thisIndex = *i;
		selectionData[thisIndex] = 1;
	}

	// We want to be able to set the point size from the vertex shader
	glEnable(GL_PROGRAM_POINT_SIZE);

	// Activate, create, and fill the vbo containing the position data
	GLuint vbo;
	glEnableVertexAttribArray(0);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, positionData.size() * sizeof(float), &(positionData[0]), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

	// Activate, create, and fill the vbo containing the selection data
	GLuint selectionVbo;
	glEnableVertexAttribArray(1);
	glGenBuffers(1, &selectionVbo);
	glBindBuffer(GL_ARRAY_BUFFER, selectionVbo);
	glBufferData(GL_ARRAY_BUFFER, selectionData.size() * sizeof(unsigned char), &(selectionData[0]), GL_STATIC_DRAW);
	glVertexAttribIPointer(1, 1, GL_UNSIGNED_BYTE, 0, 0);

	// Activate the shader required for rendering
	_shader->activate();

	// Draw the points
	glDrawArrays(GL_POINTS, 0, dataSize);

	// And be a good citizen and clean up
	_shader->deactivate();
	glDeleteBuffers(1, &vbo);
	glDisable(GL_PROGRAM_POINT_SIZE);
    _outport.deactivateTarget();
}


} // namespace
