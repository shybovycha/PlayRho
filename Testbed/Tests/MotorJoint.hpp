/*
* Original work Copyright (c) 2006-2012 Erin Catto http://www.box2d.org
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

#ifndef PLAYRHO_TESTS_MOTOR_JOINT_HPP
#define PLAYRHO_TESTS_MOTOR_JOINT_HPP

#include "../Framework/Test.hpp"

namespace playrho {

/// This test shows how to use a motor joint. A motor joint
/// can be used to animate a dynamic body. With finite motor forces
/// the body can be blocked by collision with other bodies.
class MotorJointTest : public Test
{
public:
    static Test::Conf GetTestConf()
    {
        auto conf = Test::Conf{};
        conf.description =
            "A motor joint forces two bodies to have a given linear and/or angular"
            " offset(s) from each other.";
        return conf;
    }
    
    MotorJointTest(): Test(GetTestConf())
    {
        const auto ground = m_world.CreateBody();
        ground->CreateFixture(std::make_shared<EdgeShape>(Vec2(-20.0f, 0.0f) * Meter, Vec2(20.0f, 0.0f) * Meter));

        // Define motorized body
        const auto body = m_world.CreateBody(BodyDef{}
                                             .UseType(BodyType::Dynamic)
                                             .UseLocation(Vec2(0.0f, 8.0f) * Meter));

        const auto conf = PolygonShape::Conf{}
            .UseFriction(0.6f).UseDensity(2 * KilogramPerSquareMeter);
        body->CreateFixture(std::make_shared<PolygonShape>(2.0f * Meter, 0.5f * Meter, conf));

        auto mjd = MotorJointDef{ground, body};
        mjd.maxForce = Real{1000.0f} * Newton;
        mjd.maxTorque = Real{1000.0f} * NewtonMeter;
        m_joint = (MotorJoint*)m_world.CreateJoint(mjd);
        
        RegisterForKey(GLFW_KEY_S, GLFW_PRESS, 0, "Pause Motor", [&](KeyActionMods) {
            m_go = !m_go;
        });
    }

    void PreStep(const Settings& settings, Drawer& drawer) override
    {
        m_status = m_go? "Motor going.": "Motor paused.";

        if (m_go && settings.dt > 0)
        {
            m_time += settings.dt;
        }

        const auto linearOffset = Vec2{
            Real{6} * std::sin(Real{2} * m_time),
            Real{8} + Real{4} * std::sin(Real{1} * m_time)
        } * Meter;

        m_joint->SetLinearOffset(linearOffset);
        m_joint->SetAngularOffset(Real{4} * Radian * m_time);

        drawer.DrawPoint(linearOffset, 4.0f, Color(0.9f, 0.9f, 0.9f));
    }

    MotorJoint* m_joint;
    Real m_time = 0;
    bool m_go = true;
};

} // namespace playrho

#endif /* PLAYRHO_TESTS_MOTOR_JOINT_HPP */
