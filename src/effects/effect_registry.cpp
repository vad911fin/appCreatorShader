#include "effects/effect_registry.h"

#include "effects/effects_library.h"

#include <cstring>

namespace acs
{

EffectRegistry& EffectRegistry::instance()
{
    static EffectRegistry s;
    return s;
}

void EffectRegistry::registerAll()
{
    if (m_registered)
    {
        return;
    }
    m_registered = true;
    m_effects.clear();
    m_effects.push_back(makeEffectRaymarch());
    m_effects.push_back(makeEffectSdf());
    m_effects.push_back(makeEffectFluid());
    m_effects.push_back(makeEffectBloomStub());
    m_effects.push_back(makeEffectDomainWarpStub());
    m_effects.push_back(makeEffectQuantum());
    m_effects.push_back(makeEffectGravitationalLens());
    m_effects.push_back(makeEffectNeuralStub());
    m_effects.push_back(makeEffectEntropyStub());
    m_effects.push_back(makeEffectPlasmaStub());
}

const IEffect* EffectRegistry::effectAt(std::size_t index) const
{
    if (index >= m_effects.size())
    {
        return nullptr;
    }
    return m_effects[index].get();
}

const IEffect* EffectRegistry::findById(const char* id) const
{
    if (id == nullptr)
    {
        return nullptr;
    }
    for (const std::unique_ptr<IEffect>& e : m_effects)
    {
        if (e && std::strcmp(e->id(), id) == 0)
        {
            return e.get();
        }
    }
    return nullptr;
}

} // namespace acs
