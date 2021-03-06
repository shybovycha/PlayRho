/*
* Original work Copyright (c) 2006-2009 Erin Catto http://www.box2d.org
* Modified work Copyright (c) 2017 Louis Langholtz https://github.com/louis-langholtz/PlayRho
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#ifndef PLAYRHO_CONTINUOUS_TEST_HPP
#define  PLAYRHO_CONTINUOUS_TEST_HPP

#include "../Framework/Test.hpp"

namespace testbed {

class ContinuousTest : public Test
{
public:

    ContinuousTest()
    {
        {
            const auto body = CreateBody(m_world);
            CreateFixture(m_world, body, Shape{EdgeShapeConf{Vec2(-10.0f, 0.0f) * 1_m, Vec2(10.0f, 0.0f) * 1_m}});
            CreateFixture(m_world, body, Shape{PolygonShapeConf{}.SetAsBox(0.2_m, 1_m, Vec2(0.5f, 1.0f) * 1_m, 0_rad)});
        }

        {
            auto bd = BodyConf{};
            bd.type = BodyType::Dynamic;
            bd.location = Vec2(0.0f, 20.0f) * 1_m;
            bd.linearAcceleration = m_gravity;
            //bd.angle = 0.1f;

            m_body = CreateBody(m_world, bd);
            CreateFixture(m_world, m_body, Shape{PolygonShapeConf{}.UseDensity(1_kgpm2).SetAsBox(2_m, 0.1_m)});
            m_angularVelocity = RandomFloat(-50.0f, 50.0f) * 1_rad / 1_s;
            //m_angularVelocity = 46.661274f;
            SetVelocity(m_world, m_body, Velocity{Vec2(0.0f, -100.0f) * 1_mps, m_angularVelocity});
        }
    }

    void Launch()
    {
        SetTransform(m_world, m_body, Vec2(0.0f, 20.0f) * 1_m, 0_rad);
        m_angularVelocity = RandomFloat(-50.0f, 50.0f) * 1_rad / 1_s;
        SetVelocity(m_world, m_body, Velocity{Vec2(0.0f, -100.0f) * 1_mps, m_angularVelocity});
    }

    void PostStep(const Settings&, Drawer&) override
    {
        if (GetStepCount() % 60 == 0)
        {
            Launch();
        }
    }

    BodyID m_body;
    AngularVelocity m_angularVelocity;
};

} // namespace testbed

#endif
