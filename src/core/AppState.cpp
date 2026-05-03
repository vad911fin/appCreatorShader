#include "core/AppState.h"

namespace acs
{

void AppState::syncSceneDimsFromConstructor()
{
    m_skyWidthPx = m_constructor.m_bgWidth;
    m_skyHeightPx = m_constructor.m_bgHeight;
    m_squareWidthPx = m_constructor.m_objWidth;
    m_squareHeightPx = m_constructor.m_objHeight;
    m_moveSpeed = m_constructor.m_animSpeed;
}

void AppState::syncConstructorDimsFromScene()
{
    m_constructor.m_bgWidth = m_skyWidthPx;
    m_constructor.m_bgHeight = m_skyHeightPx;
    m_constructor.m_objWidth = m_squareWidthPx;
    m_constructor.m_objHeight = m_squareHeightPx;
    m_constructor.m_animSpeed = m_moveSpeed;
}

} // namespace acs
