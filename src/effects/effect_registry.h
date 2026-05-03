#ifndef APP_CREATOR_SHADER_EFFECTS_EFFECT_REGISTRY_H
#define APP_CREATOR_SHADER_EFFECTS_EFFECT_REGISTRY_H

#include "effects/ieffect.h"

#include <memory>
#include <vector>

namespace acs
{

class EffectRegistry
{
public:
    static EffectRegistry& instance();

    void registerAll();

    [[nodiscard]] std::size_t count() const { return m_effects.size(); }
    [[nodiscard]] const IEffect* effectAt(std::size_t index) const;
    [[nodiscard]] const IEffect* findById(const char* id) const;

    EffectRegistry(const EffectRegistry&) = delete;
    EffectRegistry& operator=(const EffectRegistry&) = delete;

private:
    EffectRegistry() = default;

    std::vector<std::unique_ptr<IEffect>> m_effects;
    bool m_registered{false};
};

} // namespace acs

#endif
