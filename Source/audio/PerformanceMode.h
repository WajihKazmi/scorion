#pragma once

/** CPU / RAM tiers for weak hosts (e.g. 6th-gen laptops). */
enum class PerformanceMode
{
    Eco = 0,       // potato-friendly default
    Balanced = 1,
    Quality = 2
};

inline int maxVoicesForMode (PerformanceMode m) noexcept
{
    switch (m)
    {
        case PerformanceMode::Eco:      return 6;
        case PerformanceMode::Balanced: return 10;
        case PerformanceMode::Quality:  return 16;
    }
    return 6;
}

inline int uiHzForMode (PerformanceMode m) noexcept
{
    switch (m)
    {
        case PerformanceMode::Eco:      return 12;
        case PerformanceMode::Balanced: return 24;
        case PerformanceMode::Quality:  return 30;
    }
    return 12;
}

inline const char* performanceModeName (PerformanceMode m) noexcept
{
    switch (m)
    {
        case PerformanceMode::Eco:      return "Eco (low CPU / RAM)";
        case PerformanceMode::Balanced: return "Balanced";
        case PerformanceMode::Quality:  return "Quality";
    }
    return "Eco";
}
