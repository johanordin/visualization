
#include "modules/tnm093/include/tnm_parallelcoordinates.h"

namespace voreen {
    
TNMParallelCoordinates::AxisHandle::AxisHandle(AxisHandlePosition location, int index, const tgt::vec2& position)
    : _location(location)
    , _index(index)
    , _position(position)
{}

void TNMParallelCoordinates::AxisHandle::setPosition(const tgt::vec2& position) {
    _position = position;
}

int TNMParallelCoordinates::AxisHandle::index() const {
    return _index;
}

void TNMParallelCoordinates::AxisHandle::render() const {
    glColor3f(0.8f, 0.8f, 0.8f);
    renderInternal();
}

void TNMParallelCoordinates::AxisHandle::renderPicking() const {
	// Mapping the integer index to a float value between 1/255 and 1
    const float color = (_index + 1) / 255.f;
	// The picking information is rendered only in the red channel
    glColor3f(color, 0.f, 0.f);
    renderInternal();
}

void TNMParallelCoordinates::AxisHandle::renderInternal() const {
    const float xDiff = 0.05f;
    float yDiff;
    if (_location == AxisHandlePositionTop)
        yDiff = 0.05f;
    else if (_location == AxisHandlePositionBottom)
        yDiff = -0.05;
    glBegin(GL_TRIANGLES);
    glVertex2f(_position.x, _position.y - yDiff / 2.f);
    glVertex2f(_position.x - xDiff, _position.y + yDiff / 2.f);
    glVertex2f(_position.x + xDiff, _position.y + yDiff / 2.f);
    glEnd();
}


TNMParallelCoordinates::TNMParallelCoordinates()
    : RenderProcessor()
    , _inport(Port::INPORT, "in.data")
    , _outport(Port::OUTPORT, "out.image")
    , _privatePort(Port::OUTPORT, "private.image", false, Processor::INVALID_RESULT, GL_RGBA32F)
    , _pickedHandle(-1)
	, _brushingIndices("brushingIndices", "Brushing Indices")
	, _linkingIndices("linkingIndices", "Linking Indices")
{
    addPort(_inport);
    addPort(_outport);
    addPrivateRenderPort(_privatePort);

	addProperty(_brushingIndices);
	addProperty(_linkingIndices);

    _mouseClickEvent = new EventProperty<TNMParallelCoordinates>(
        "mouse.click", "Mouse Click",
        this, &TNMParallelCoordinates::handleMouseClick,
        tgt::MouseEvent::MOUSE_BUTTON_LEFT, tgt::MouseEvent::CLICK, tgt::Event::MODIFIER_NONE);
    addEventProperty(_mouseClickEvent);

    _mouseMoveEvent = new EventProperty<TNMParallelCoordinates>(
        "mouse.move", "Mouse Move",
        this, &TNMParallelCoordinates::handleMouseMove,
        tgt::MouseEvent::MOUSE_BUTTON_LEFT, tgt::MouseEvent::MOTION, tgt::Event::MODIFIER_NONE);
    addEventProperty(_mouseMoveEvent);

    _mouseReleaseEvent = new EventProperty<TNMParallelCoordinates>(
        "mouse.release", "Mouse Release",
        this, &TNMParallelCoordinates::handleMouseRelease,
        tgt::MouseEvent::MOUSE_BUTTON_LEFT, tgt::MouseEvent::RELEASED, tgt::Event::MODIFIER_NONE);
    addEventProperty(_mouseReleaseEvent);

	//
    // Create AxisHandles here with a unique id
    // _handles.push_back(AxisHandle(AxisHandle::AxisHandlePositionTop, 0, /* fix startposition */));
    // ...
	//
}

TNMParallelCoordinates::~TNMParallelCoordinates() {
    delete _mouseClickEvent;
    delete _mouseMoveEvent;
}

void TNMParallelCoordinates::process() {
	// Activate the user-outport as the rendering target
    _outport.activateTarget();
	// Clear the buffer
    _outport.clearTarget();

	// Render the handles
    renderHandles();
	// Render the parallel coordinates lines
    renderLines();

	// We are done with the visual part
    _outport.deactivateTarget();

	// Activate the internal port used for picking
    _privatePort.activateTarget();
	// Clear that buffer as well
    _privatePort.clearTarget();
	// Render the handles with the picking information encoded in the red channel
    renderHandlesPicking();
	// Render the lines with the picking information encoded in the green/blue/alpha channel
	renderLinesPicking();
	// We are done with the private render target
    _privatePort.deactivateTarget();
}

void TNMParallelCoordinates::handleMouseClick(tgt::MouseEvent* e) {
	// The picking texture is the result of the previous rendering in the private render port
    tgt::Texture* pickingTexture = _privatePort.getColorTexture();
	// Retrieve the texture from the graphics memory and get it to the RAM
	pickingTexture->downloadTexture();

	// The texture coordinates are flipped in the y direction, so we take care of that here
    const tgt::ivec2 screenCoords = tgt::ivec2(e->coord().x, pickingTexture->getDimensions().y - e->coord().y);
	// And then go from integer pixel coordinates to [-1,1] coordinates
    const tgt::vec2& normalizedDeviceCoordinates = (tgt::vec2(screenCoords) / tgt::vec2(_privatePort.getSize()) - 0.5f) * 2.f;

	// The picking information for the handles is stored in the red color channel
    int handleId = static_cast<int>(pickingTexture->texelAsFloat(screenCoords).r * 255 - 1);

    LINFOC("Picking", "Picked handle index: " << handleId);
    // Use the 'id' and the 'normalizedDeviceCoordinates' to move the correct handle
    // The id is the id of the AxisHandle that has been clicked (the same id you assigned in the constructor)
    // id == -1 if no handle was clicked
    // Use the '_pickedIndex' member variable to store the picked index



	int lineId = -1;
	// Derive the id of the line that was clicked based on the color scheme that you devised in the
	// renderLinesPicking method



	LINFOC("Picking", "Picked line index: " << lineId);
	if (lineId != -1)
		// We want to add it only if a line was clicked
		_linkingList.insert(lineId);

	// if the right mouse button is pressed and no line is clicked, clear the list:
	if ((e->button() == tgt::MouseEvent::MOUSE_BUTTON_RIGHT) && (lineId == -1))
		_linkingList.clear();

	// Make the list of selected indices available to the Scatterplot
	_linkingIndices.set(_linkingList);
}

void TNMParallelCoordinates::handleMouseMove(tgt::MouseEvent* e) {
    tgt::Texture* pickingTexture = _privatePort.getColorTexture();
    const tgt::ivec2 screenCoords = tgt::ivec2(e->coord().x, pickingTexture->getDimensions().y - e->coord().y);
    const tgt::vec2& normalizedDeviceCoordinates = (tgt::vec2(screenCoords) / tgt::vec2(_privatePort.getSize()) - 0.5f) * 2.f;

    // Move the stored index along its axis (if it is a valid picking point)


	// update the _brushingList with the indices of the lines that are not rendered anymore


	_brushingIndices.set(_brushingList);

    // This re-renders the scene (which will call process in turn)
    invalidate();
}

void TNMParallelCoordinates::handleMouseRelease(tgt::MouseEvent* e) {

}

void TNMParallelCoordinates::renderLines() {
	//
    // Implement your line drawing
	//

}

void TNMParallelCoordinates::renderLinesPicking() {
	// Use the same code to render lines (without duplicating it), but think of a way to encode the
	// voxel identifier into the color. The red color channel is already occupied, so you have 3
	// channels with 32-bit each at your disposal (green, blue, alpha)

}

void TNMParallelCoordinates::renderHandles() {
    for (size_t i = 0; i < _handles.size(); ++i) {
        const AxisHandle& handle = _handles[i];
        handle.render();
    }
}

void TNMParallelCoordinates::renderHandlesPicking() {
    for (size_t i = 0; i < _handles.size(); ++i) {
        const AxisHandle& handle = _handles[i];
        handle.renderPicking();
    }
}


} // namespace
