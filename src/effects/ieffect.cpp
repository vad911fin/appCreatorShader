#include "effects/ieffect.h"

#include <algorithm>

namespace acs
{

bool IEffect::needsDesktopGl43() const
{
    return false;
}

std::string IEffect::vertexGlsl(bool /*gles*/) const
{
    return {};
}

int IEffect::effectiveStepCount(PerfTier tier, int baseSteps) const
{
    int s = baseSteps;
    if (tier == PerfTier::Low)
    {
        s = (std::min)(s, 12);
    }
    else if (tier == PerfTier::Medium)
    {
        s = (std::min)(s, 28);
    }
    return (std::max)(4, s);
}

} // namespace acs
