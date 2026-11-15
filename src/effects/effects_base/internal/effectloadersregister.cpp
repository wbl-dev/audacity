/*
 * Audacity: A Digital Audio Editor
 */
#include "effectloadersregister.h"

using namespace agu::effects;

void EffectLoadersRegister::registerLoader(IEffectLoaderPtr loader)
{
    m_loaders.push_back(std::move(loader));
}

IEffectLoaderPtr EffectLoadersRegister::loader(EffectFamily family) const
{
    for (const auto& l : m_loaders) {
        if (l->family() == family) {
            return l;
        }
    }
    return nullptr;
}
