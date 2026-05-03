#ifndef APP_CREATOR_SHADER_EFFECTS_EFFECTS_LIBRARY_H
#define APP_CREATOR_SHADER_EFFECTS_EFFECTS_LIBRARY_H

#include "effects/ieffect.h"

#include <memory>

namespace acs
{

std::unique_ptr<IEffect> makeEffectRaymarch();
std::unique_ptr<IEffect> makeEffectSdf();
std::unique_ptr<IEffect> makeEffectFluid();
std::unique_ptr<IEffect> makeEffectQuantum();
std::unique_ptr<IEffect> makeEffectGravitationalLens();
std::unique_ptr<IEffect> makeEffectBloomStub();
std::unique_ptr<IEffect> makeEffectDomainWarpStub();
std::unique_ptr<IEffect> makeEffectNeuralStub();
std::unique_ptr<IEffect> makeEffectEntropyStub();
std::unique_ptr<IEffect> makeEffectPlasmaStub();

} // namespace acs

#endif
