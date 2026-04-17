/**
 * @file StatusEffectBase.cpp
 * @brief Implementation of duration management and tag lookup for StatusEffectBase.
 */

#include "Core/StatusEffectBase.h"
#include <algorithm>

StatusEffectBase::StatusEffectBase(std::string_view id,
                                   std::string_view name,
                                   std::optional<int> duration,
                                   std::initializer_list<std::string_view> tags)
    : m_id{id}, m_name{name}, m_duration{duration}
{
    for (std::string_view tag : tags)
        m_tags.emplace_back(tag);
}

std::string_view StatusEffectBase::getId() const
{
    return m_id;
}
std::string_view StatusEffectBase::getName() const
{
    return m_name;
}

std::optional<int> StatusEffectBase::getDuration() const
{
    return m_duration;
}

const std::vector<std::string> &StatusEffectBase::getTags() const
{
    return m_tags;
}

bool StatusEffectBase::hasTag(std::string_view tag) const
{
    for (const auto &t : m_tags)
        if (t == tag)
            return true;
    return false;
}

void StatusEffectBase::extendDuration(int turns)
{
    // Permanent effects (nullopt) are unaffected by duration changes.
    if (m_duration.has_value())
        m_duration = std::max(0, *m_duration + turns);
}