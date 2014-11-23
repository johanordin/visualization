#ifndef VRN_INDEXPROPERTY_H
#define VRN_INDEXPROPERTY_H

#include "voreen/core/properties/condition.h"
#include "voreen/core/properties/templateproperty.h"

namespace voreen {

#ifdef DLL_TEMPLATE_INST
template class VRN_CORE_API TemplateProperty<std::set<unsigned int> >;
#endif
class VRN_CORE_API IndexProperty : public TemplateProperty<std::set<unsigned int> > {
public:
    IndexProperty();
    IndexProperty(const std::string& id, const std::string& guiText);
    Property* create() const;
    std::string getClassName() const;

    int getVariantType() const;

    Variant getVariant(bool normalized) const;
    void setVariant(const Variant& v, bool normalized);
};

} // namespace voreen

#endif // VRN_INDEXPROPERTY_H
