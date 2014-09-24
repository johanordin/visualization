#ifndef VRN_TNM_DATAREDUCTION_H
#define VRN_TNM_DATAREDUCTION_H

#include "modules/tnm093/include/tnm_common.h"

namespace voreen {

class TNMDataReduction : public Processor {
public:
    TNMDataReduction();
    Processor* create() const;

    std::string getClassName() const    { return "TNMDataReduction"; }
    std::string getCategory() const     { return "tnm093"; }
    CodeState getCodeState() const      { return CODE_STATE_EXPERIMENTAL; }

protected:
    void process();

    DataPort _inport; // The incoming data
    DataPort _outport; // Outgoing, filtered data

    FloatProperty _percentage; // The percentage of how many values should be filtered away
};


} // namespace voreen

#endif // VRN_TNM_DATAREDUCTION_H
