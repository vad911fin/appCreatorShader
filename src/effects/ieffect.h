#ifndef APP_CREATOR_SHADER_EFFECTS_IEFFECT_H
#define APP_CREATOR_SHADER_EFFECTS_IEFFECT_H

#include "effects/param_descriptor.h"

#include <string>
#include <vector>

namespace acs
{

class UniformRegistry;

class IEffect
{
public:
    virtual ~IEffect() = default;

    virtual const char* id() const = 0;
    virtual const char* displayName() const = 0;
    virtual EffectCategory category() const = 0;
    virtual PerfTier recommendedMinTier() const = 0;
    virtual bool needsDesktopGl43() const;

    virtual std::string vertexGlsl(bool gles) const;
    virtual std::string fragmentGlsl(bool gles, PerfTier tier) const = 0;
    virtual std::vector<ParamDescriptor> exposedParams() const = 0;
    virtual void applyUniforms(UniformRegistry& reg, const EffectRuntimeParams& values) const = 0;

    int effectiveStepCount(PerfTier tier, int baseSteps) const;
};

} // namespace acs

#endif
