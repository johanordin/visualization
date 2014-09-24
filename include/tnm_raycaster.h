#ifndef VRN_TNM_RAYCASTER_H
#define VRN_TNM_RAYCASTER_H

#include "voreen/core/processors/volumeraycaster.h"

#include "voreen/core/properties/transfuncproperty.h"
#include "voreen/core/properties/cameraproperty.h"
#include "voreen/core/properties/optionproperty.h"
#include "voreen/core/properties/floatproperty.h"

#include "voreen/core/ports/volumeport.h"

namespace voreen {

class TNMRaycaster : public VolumeRaycaster {
public:
    TNMRaycaster();
    Processor* create() const;

    std::string getClassName() const    { return "TNMRaycaster"; }
    std::string getCategory() const     { return "tnm093"; }
    CodeState getCodeState() const      { return CODE_STATE_STABLE; }

    bool isReady() const;

protected:
    void beforeProcess();
    void process();

    void initialize() throw (tgt::Exception);
    void deinitialize() throw (tgt::Exception);
    std::string generateHeader();
    void compile();

private:
    void adjustPropertyVisibilities();

    VolumePort volumeInport_;
    RenderPort entryPort_;
    RenderPort exitPort_;

    RenderPort outport_;

    tgt::Shader* raycastPrg_;         ///< The shader program used by this raycaster.

    TransFuncProperty transferFunc_;  ///< the property that controls the transfer-function
    CameraProperty camera_;           ///< the camera used for lighting calculations

    static const std::string loggerCat_; ///< category used in logging
};


} // namespace voreen

#endif // VRN_TNM_RAYCASTER_H
