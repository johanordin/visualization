#include "modules/tnm093/include/tnm_raycaster.h"

#include "tgt/textureunit.h"
#include "voreen/core/ports/conditions/portconditionvolumetype.h"

#include <sstream>

using tgt::vec3;
using tgt::TextureUnit;

namespace voreen {

const std::string TNMRaycaster::loggerCat_("voreen.TNMRaycaster");

TNMRaycaster::TNMRaycaster()
    : VolumeRaycaster()
    , volumeInport_(Port::INPORT, "volumehandle.volumehandle", false, Processor::INVALID_PROGRAM)
    , entryPort_(Port::INPORT, "image.entrypoints")
    , exitPort_(Port::INPORT, "image.exitpoints")
    , outport_(Port::OUTPORT, "image.output", true, Processor::INVALID_PROGRAM)
    , raycastPrg_(0)
    , transferFunc_("transferFunction", "Transfer Function")
    , camera_("camera", "Camera", tgt::Camera(vec3(0.f, 0.f, 3.5f), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f)))
{
    // ports
    volumeInport_.addCondition(new PortConditionVolumeTypeGL());
    addPort(volumeInport_);
    addPort(entryPort_);
    addPort(exitPort_);
    addPort(outport_);

    // shading / classification props
    addProperty(transferFunc_);
    addProperty(camera_);
    
    // lighting
    addProperty(lightPosition_);
    addProperty(lightAmbient_);
    addProperty(lightDiffuse_);
    addProperty(lightSpecular_);
    addProperty(materialShininess_);
    addProperty(applyLightAttenuation_);
    addProperty(lightAttenuation_);

    // assign lighting properties to property group
    lightPosition_.setGroupID("lighting");
    lightAmbient_.setGroupID("lighting");
    lightDiffuse_.setGroupID("lighting");
    lightSpecular_.setGroupID("lighting");
    materialShininess_.setGroupID("lighting");
    applyLightAttenuation_.setGroupID("lighting");
    lightAttenuation_.setGroupID("lighting");
    setPropertyGroupGuiName("lighting", "Lighting Parameters");

    // listen to changes of properties that influence the GUI state (i.e. visibility of other props)
    compositingMode_.onChange(CallMemberAction<TNMRaycaster>(this, &TNMRaycaster::adjustPropertyVisibilities));
    applyLightAttenuation_.onChange(CallMemberAction<TNMRaycaster>(this, &TNMRaycaster::adjustPropertyVisibilities));
}

Processor* TNMRaycaster::create() const {
    return new TNMRaycaster();
}

void TNMRaycaster::initialize() throw (tgt::Exception) {
    VolumeRaycaster::initialize();

    raycastPrg_ = ShdrMgr.loadSeparate("passthrough.vert", "rc_raycaster.frag",
        generateHeader(), false);

    adjustPropertyVisibilities();

    if (transferFunc_.get()) {
        transferFunc_.get()->getTexture();
        transferFunc_.get()->invalidateTexture();
    }
}

void TNMRaycaster::deinitialize() throw (tgt::Exception) {
    ShdrMgr.dispose(raycastPrg_);
    raycastPrg_ = 0;
    LGL_ERROR;

    VolumeRaycaster::deinitialize();
}

void TNMRaycaster::compile() {
    raycastPrg_->setHeaders(generateHeader());
    raycastPrg_->rebuild();
}

bool TNMRaycaster::isReady() const {
    //check if all inports are connected
    if(!entryPort_.isReady() || !exitPort_.isReady() || !volumeInport_.isReady())
        return false;

    //check if at least one outport is connected
    if (!outport_.isReady())
        return false;

    return true;
}

void TNMRaycaster::beforeProcess() {
    VolumeRaycaster::beforeProcess();

    // compile program if needed
    if (getInvalidationLevel() >= Processor::INVALID_PROGRAM) {
        PROFILING_BLOCK("compile");
        compile();
    }
    LGL_ERROR;

    transferFunc_.setVolumeHandle(volumeInport_.getData());
}

void TNMRaycaster::process() {
    // bind transfer function
    tgt::TextureUnit transferUnit;
    transferUnit.activate();
    LGL_ERROR;
    if (transferFunc_.get())
        transferFunc_.get()->bind();

    outport_.activateTarget();
    outport_.clearTarget();
    LGL_ERROR;

    // bind entry params
    tgt::TextureUnit entryUnit, entryDepthUnit, exitUnit, exitDepthUnit;
    entryPort_.bindTextures(entryUnit, entryDepthUnit);
    LGL_ERROR;

    // bind exit params
    exitPort_.bindTextures(exitUnit, exitDepthUnit);
    LGL_ERROR;

    // vector containing the volumes to bind; is passed to bindVolumes()
    std::vector<VolumeStruct> volumeTextures;

    // add main volume
    TextureUnit volUnit;
    volumeTextures.push_back(VolumeStruct(
        volumeInport_.getData(),
        &volUnit,
        "volumeStruct_",
        GL_CLAMP,
        tgt::vec4(0.f),
        GL_LINEAR)
    );

    // initialize shader
    raycastPrg_->activate();

    // set common uniforms used by all shaders
    tgt::Camera cam = camera_.get();
    setGlobalShaderParameters(raycastPrg_, &cam);
    // bind the volumes and pass the necessary information to the shader
    bindVolumes(raycastPrg_, volumeTextures, &cam, lightPosition_.get());

    // pass the remaining uniforms to the shader
    raycastPrg_->setUniform("entryPoints_", entryUnit.getUnitNumber());
    raycastPrg_->setUniform("entryPointsDepth_", entryDepthUnit.getUnitNumber());
    entryPort_.setTextureParameters(raycastPrg_, "entryParameters_");
    raycastPrg_->setUniform("exitPoints_", exitUnit.getUnitNumber());
    raycastPrg_->setUniform("exitPointsDepth_", exitDepthUnit.getUnitNumber());
    exitPort_.setTextureParameters(raycastPrg_, "exitParameters_");

    if (classificationMode_.get() == "transfer-function") {
        transferFunc_.get()->setUniform(raycastPrg_, "transferFunc_", transferUnit.getUnitNumber());
    }
    
    {
        PROFILING_BLOCK("raycasting");
        renderQuad();
    }

    raycastPrg_->deactivate();
    outport_.deactivateTarget();

    TextureUnit::setZeroUnit();
    LGL_ERROR;
}

std::string TNMRaycaster::generateHeader() {
    std::string headerSource = VolumeRaycaster::generateHeader();

    headerSource += transferFunc_.get()->getShaderDefines();

    return headerSource;
}

void TNMRaycaster::adjustPropertyVisibilities() {
    bool useLighting = !shadeMode_.isSelected("none");
    setPropertyGroupVisible("lighting", useLighting);

    lightAttenuation_.setVisible(applyLightAttenuation_.get());
}

} // namespace
