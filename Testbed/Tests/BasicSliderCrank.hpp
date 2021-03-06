/*
 * Original work Copyright (c) 2006-2014 Erin Catto http://www.box2d.org
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

#ifndef PLAYRHO_BASIC_SLIDER_CRANK_HPP
#define  PLAYRHO_BASIC_SLIDER_CRANK_HPP

#include "../Framework/Test.hpp"

namespace testbed {

// A basic slider crank created for GDC tutorial: Understanding Constraints
class BasicSliderCrank : public Test
{
public:
    BasicSliderCrank()
    {        
        const auto ground = CreateBody(m_world, BodyConf{}.UseLocation(Vec2(0.0f, 17.0f) * 1_m));
        auto prevBody = ground;
        
        // Define crank.
        {
            auto bd = BodyConf{};
            bd.type = BodyType::Dynamic;
            bd.location = Vec2(-8.0f, 20.0f) * 1_m;
            bd.linearAcceleration = m_gravity;
            const auto body = CreateBody(m_world, bd);
            auto conf = PolygonShapeConf{};
            conf.density = 2_kgpm2;
            conf.SetAsBox(4_m, 1_m);
            CreateFixture(m_world, body, Shape(conf));
            m_world.CreateJoint(GetRevoluteJointConf(m_world, prevBody, body, Vec2(-12.0f, 20.0f) * 1_m));
            prevBody = body;
        }
        
        // Define connecting rod
        {
            auto bd = BodyConf{};
            bd.type = BodyType::Dynamic;
            bd.location = Vec2(4.0f, 20.0f) * 1_m;
            bd.linearAcceleration = m_gravity;
            const auto body = CreateBody(m_world, bd);
            auto conf = PolygonShapeConf{};
            conf.density = 2_kgpm2;
            conf.SetAsBox(8_m, 1_m);
            CreateFixture(m_world, body, Shape(conf));
            m_world.CreateJoint(GetRevoluteJointConf(m_world, prevBody, body, Vec2(-4.0f, 20.0f) * 1_m));
            prevBody = body;
        }
        
        // Define piston
        {
            auto bd = BodyConf{};
            bd.type = BodyType::Dynamic;
            bd.fixedRotation = true;
            bd.location = Vec2(12.0f, 20.0f) * 1_m;
            bd.linearAcceleration = m_gravity;
            const auto body = CreateBody(m_world, bd);
            const auto conf = PolygonShapeConf{}.UseDensity(2_kgpm2).SetAsBox(3_m, 3_m);
            CreateFixture(m_world, body, Shape(conf));
            m_world.CreateJoint(GetRevoluteJointConf(m_world, prevBody, body, Vec2(12.0f, 20.0f) * 1_m));
            const PrismaticJointConf pjd = GetPrismaticJointConf(m_world, ground, body, Vec2(12.0f, 17.0f) * 1_m, UnitVec::GetRight());
            m_world.CreateJoint(pjd);
        }
    }
};

} // namespace testbed

#endif
