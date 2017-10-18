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

#ifndef PLAYRHO_BULLET_TEST_HPP
#define  PLAYRHO_BULLET_TEST_HPP

#include "../Framework/Test.hpp"

namespace playrho {

class BulletTest : public Test
{
public:

    BulletTest()
    {
        {
            BodyDef bd;
            bd.location = Vec2(0.0f, 0.0f) * Meter;
            Body* body = m_world.CreateBody(bd);

            body->CreateFixture(std::make_shared<EdgeShape>(Vec2(-10.0f, 0.0f) * Meter, Vec2(10.0f, 0.0f) * Meter));

            PolygonShape shape;
            SetAsBox(shape, Real{0.2f} * Meter, Real{1.0f} * Meter, Vec2(0.5f, 1.0f) * Meter, Real{0.0f} * Radian);
            body->CreateFixture(std::make_shared<PolygonShape>(shape));
        }

        {
            BodyDef bd;
            bd.type = BodyType::Dynamic;
            bd.location = Vec2(0.0f, 4.0f) * Meter;

            PolygonShape box;
            box.SetAsBox(Real{2.0f} * Meter, Real{0.1f} * Meter);
            box.SetDensity(Real{1} * KilogramPerSquareMeter);

            m_body = m_world.CreateBody(bd);
            m_body->CreateFixture(std::make_shared<PolygonShape>(box));

            box.SetAsBox(Real{0.25f} * Meter, Real{0.25f} * Meter);
            box.SetDensity(Real{100} * KilogramPerSquareMeter);

            //m_x = RandomFloat(-1.0f, 1.0f);
            m_x = 0.20352793f;
            bd.location = Vec2(m_x, 10.0f) * Meter;
            bd.bullet = true;

            m_bullet = m_world.CreateBody(bd);
            m_bullet->CreateFixture(std::make_shared<PolygonShape>(box));

            m_bullet->SetVelocity(Velocity{Vec2{0.0f, -50.0f} * MeterPerSecond, AngularVelocity{0}});
        }
    }

    void Launch()
    {
        m_body->SetTransform(Vec2(0.0f, 4.0f) * Meter, Real{0.0f} * Radian);
        m_body->SetVelocity(Velocity{LinearVelocity2D{}, AngularVelocity{0}});

        m_x = RandomFloat(-1.0f, 1.0f);
        m_bullet->SetTransform(Vec2(m_x, 10.0f) * Meter, Real{0.0f} * Radian);
        m_bullet->SetVelocity(Velocity{Vec2(0.0f, -50.0f) * MeterPerSecond, AngularVelocity{0}});
    }

    void PostStep(const Settings&, Drawer&) override
    {
        if (GetStepCount() % 60 == 0)
        {
            Launch();
        }
    }

    Body* m_body;
    Body* m_bullet;
    Real m_x;
};

} // namespace playrho

#endif
