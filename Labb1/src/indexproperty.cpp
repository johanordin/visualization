#include "modules/tnm093/include/indexproperty.h"
#include "voreen/core/utils/variant.h"

namespace voreen {

IndexProperty::IndexProperty(const std::string& id, const std::string& guiText) 
    : TemplateProperty(id, guiText, std::set<unsigned int>())
{}

IndexProperty::IndexProperty()
    : TemplateProperty()
{}

Property* IndexProperty::create() const {
    return new IndexProperty;
}

std::string IndexProperty::getClassName() const {
    return "IndexProperty";
}

int IndexProperty::getVariantType() const {
    return Variant::VariantTypeUserType + 1;
}

Variant IndexProperty::getVariant(bool normalized) const {
    Variant r;
    r.set<std::set<unsigned int> >(get(), Variant::VariantTypeUserType + 1);
    return r;
}

void IndexProperty::setVariant(const Variant& v, bool normalized) {
    set(v.get<std::set<unsigned int> >());
}

} // namespace voreen
