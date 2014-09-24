#ifndef VRN_TNM_SCATTERPLOT_H
#define VRN_TNM_SCATTERPLOT_H

#include "voreen/core/processors/renderprocessor.h"
#include "modules/tnm093/include/tnm_common.h"
#include "modules/tnm093/include/indexproperty.h"


namespace voreen {

class TNMScatterPlot : public RenderProcessor {
public:
    TNMScatterPlot();
    std::string getClassName() const   { return "TNMScatterPlot";           }
    std::string getCategory() const    { return "tnm093"               ; }
    CodeState getCodeState() const     { return CODE_STATE_EXPERIMENTAL; }

    Processor* create() const          { return new TNMScatterPlot;         }

	void initialize() throw (tgt::Exception);
	void deinitialize() throw (tgt::Exception);


	bool isReady() const { return true; }

protected:
    void process();

private:
    DataPort _inport; // The data that is to be rendered
    RenderPort _outport; // A wrapping class for multiple framebufferobjects that can be rendered to

	tgt::Shader* _shader; // The shader object that will do the rendering for us

	// A wrapper for an integer member variable that can be set using the GUI
    IntOptionProperty _firstAxis; 
    IntOptionProperty _secondAxis;

	IndexProperty _brushingIndices; // A list of voxel indices that should be ignored in the rendering
	IndexProperty _linkingIndices; // A list of voxel indices that should be enhanced during rendering
};

} // namespace

#endif // VRN_TNM_SCATTERPLOT_H
