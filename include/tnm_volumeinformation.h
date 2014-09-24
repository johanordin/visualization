#ifndef VRN_TNM_VOLUMEINFORMATION_H
#define VRN_TNM_VOLUMEINFORMATION_H

#include "voreen/core/processors/processor.h"
#include "modules/tnm093/include/tnm_common.h"

namespace voreen {

class TNMVolumeInformation : public Processor {
public:
    TNMVolumeInformation();
    ~TNMVolumeInformation();
    std::string getClassName() const   { return "TNMVolumeInformation";   }
    std::string getCategory() const    { return "tnm093"               ;  }
    CodeState getCodeState() const     { return CODE_STATE_EXPERIMENTAL;  }

    Processor* create() const          { return new TNMVolumeInformation; }

protected:
    void process();

private:
    VolumePort _inport; // The inport that contains the volume for which the information is computed
    DataPort _outport; // The outport containing the computed measures

    Data* _data; // The local copy of the computed data; ownership stays with this object at all times
};

} // namespace

#endif // VRN_TNM_VOLUMEINFORMATION_H
