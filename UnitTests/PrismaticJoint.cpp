/*
 * Copyright (c) 2017 Louis Langholtz https://github.com/louis-langholtz/PlayRho
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

#include "UnitTests.hpp"

#include <PlayRho/Dynamics/Joints/PrismaticJointConf.hpp>

#include <PlayRho/Dynamics/Joints/Joint.hpp>
#include <PlayRho/Dynamics/World.hpp>
#include <PlayRho/Dynamics/WorldJoint.hpp>
#include <PlayRho/Dynamics/WorldBody.hpp>
#include <PlayRho/Dynamics/WorldMisc.hpp> // for Step
#include <PlayRho/Collision/Shapes/DiskShapeConf.hpp>

using namespace playrho;
using namespace playrho::d2;

TEST(PrismaticJointConf, ByteSize)
{
    switch (sizeof(Real))
    {
        case  4:
#if defined(_WIN64)
            EXPECT_EQ(sizeof(PrismaticJointConf), std::size_t(160));
#elif defined(_WIN32)
            EXPECT_EQ(sizeof(PrismaticJointConf), std::size_t(160));
#else
            EXPECT_EQ(sizeof(PrismaticJointConf), std::size_t(160));
#endif
            break;
        case  8: EXPECT_EQ(sizeof(PrismaticJointConf), std::size_t(312)); break;
        case 16: EXPECT_EQ(sizeof(PrismaticJointConf), std::size_t(624)); break;
        default: FAIL(); break;
    }
}

TEST(PrismaticJoint, Construction)
{
    auto world = World{};
    const auto b0 = world.CreateBody();
    const auto b1 = world.CreateBody();
    
    auto jd = PrismaticJointConf{};
    jd.bodyA = b0;
    jd.bodyB = b1;
    jd.localAnchorA = Length2(4_m, 5_m);
    jd.localAnchorB = Length2(6_m, 7_m);
    
    auto joint = Joint{jd};
    EXPECT_EQ(GetBodyA(joint), b0);
    EXPECT_EQ(GetBodyB(joint), b1);
    EXPECT_EQ(GetLocalAnchorA(joint), jd.localAnchorA);
    EXPECT_EQ(GetLocalAnchorB(joint), jd.localAnchorB);
    EXPECT_EQ(GetLinearReaction(joint), Momentum2{});
    EXPECT_EQ(GetAngularReaction(joint), AngularMomentum{0});
}

TEST(PrismaticJoint, EnableLimit)
{
    auto world = World{};
    const auto b0 = world.CreateBody();
    const auto b1 = world.CreateBody();
    
    auto jd = PrismaticJointConf{};
    jd.bodyA = b0;
    jd.bodyB = b1;
    jd.localAnchorA = Length2(4_m, 5_m);
    jd.localAnchorB = Length2(6_m, 7_m);
    
    auto joint = Joint{jd};
    EXPECT_FALSE(IsLimitEnabled(joint));
    EnableLimit(joint, false);
    EXPECT_FALSE(IsLimitEnabled(joint));
    EnableLimit(joint, true);
    EXPECT_TRUE(IsLimitEnabled(joint));
    EXPECT_EQ(GetLinearMotorImpulse(joint), 0_Ns);

    const auto id = world.CreateJoint(joint);
    EXPECT_EQ(GetMotorForce(world, id, 1_Hz), 0 * Newton);
}

TEST(PrismaticJoint, ShiftOrigin)
{
    auto world = World{};
    const auto b0 = world.CreateBody();
    const auto b1 = world.CreateBody();
    
    auto jd = PrismaticJointConf{};
    jd.bodyA = b0;
    jd.bodyB = b1;
    jd.localAnchorA = Length2(4_m, 5_m);
    jd.localAnchorB = Length2(6_m, 7_m);
    
    auto joint = Joint{jd};

    const auto newOrigin = Length2{1_m, 1_m};
    EXPECT_FALSE(ShiftOrigin(joint, newOrigin));
}

TEST(PrismaticJoint, EnableMotor)
{
    World world;
    const auto b0 = world.CreateBody();
    const auto b1 = world.CreateBody();
    
    auto jd = PrismaticJointConf{};
    jd.bodyA = b0;
    jd.bodyB = b1;
    jd.localAnchorA = Length2(4_m, 5_m);
    jd.localAnchorB = Length2(6_m, 7_m);
    
    auto joint = Joint{jd};
    EXPECT_FALSE(IsMotorEnabled(joint));
    EnableMotor(joint, false);
    EXPECT_FALSE(IsMotorEnabled(joint));
    EnableMotor(joint, true);
    EXPECT_TRUE(IsMotorEnabled(joint));
}

TEST(PrismaticJoint, SetMaxMotorForce)
{
    World world;
    const auto b0 = world.CreateBody();
    const auto b1 = world.CreateBody();
    
    auto jd = PrismaticJointConf{};
    jd.bodyA = b0;
    jd.bodyB = b1;
    jd.localAnchorA = Length2(4_m, 5_m);
    jd.localAnchorB = Length2(6_m, 7_m);
    
    auto joint = Joint{jd};
    ASSERT_EQ(GetMaxMotorForce(joint), 0_N);
    SetMaxMotorForce(joint, 2_N);
    EXPECT_EQ(GetMaxMotorForce(joint), 2_N);
}

TEST(PrismaticJoint, MotorSpeed)
{
    World world;
    const auto b0 = world.CreateBody();
    const auto b1 = world.CreateBody();
    
    auto jd = PrismaticJointConf{};
    jd.bodyA = b0;
    jd.bodyB = b1;
    jd.localAnchorA = Length2(4_m, 5_m);
    jd.localAnchorB = Length2(6_m, 7_m);
    
    const auto newValue = Real(5) * RadianPerSecond;
    auto joint = Joint{jd};
    ASSERT_NE(GetMotorSpeed(joint), newValue);
    EXPECT_EQ(GetMotorSpeed(joint), jd.motorSpeed);
    SetMotorSpeed(joint, newValue);
    EXPECT_EQ(GetMotorSpeed(joint), newValue);
}

TEST(PrismaticJoint, SetLinearLimits)
{
    World world;
    const auto b0 = world.CreateBody();
    const auto b1 = world.CreateBody();
    
    auto jd = PrismaticJointConf{};
    jd.bodyA = b0;
    jd.bodyB = b1;
    jd.localAnchorA = Length2(4_m, 5_m);
    jd.localAnchorB = Length2(6_m, 7_m);
    
    const auto upperValue = +5_m;
    const auto lowerValue = -8_m;
    auto joint = Joint{jd};
    ASSERT_NE(GetLinearUpperLimit(joint), upperValue);
    ASSERT_NE(GetLinearLowerLimit(joint), lowerValue);
    SetLinearLimits(joint, lowerValue, upperValue);
    EXPECT_EQ(GetLinearUpperLimit(joint), upperValue);
    EXPECT_EQ(GetLinearLowerLimit(joint), lowerValue);
}

TEST(PrismaticJoint, GetAnchorAandB)
{
    World world;
    
    const auto loc0 = Length2{+1_m, -3_m};
    const auto loc1 = Length2{-2_m, Real(+1.2f) * Meter};

    const auto b0 = world.CreateBody(BodyConf{}.UseLocation(loc0));
    const auto b1 = world.CreateBody(BodyConf{}.UseLocation(loc1));
    
    auto jd = PrismaticJointConf{};
    jd.bodyA = b0;
    jd.bodyB = b1;
    jd.localAnchorA = Length2(4_m, 5_m);
    jd.localAnchorB = Length2(6_m, 7_m);
    
    auto joint = world.CreateJoint(Joint{jd});
    ASSERT_EQ(GetLocalAnchorA(world, joint), jd.localAnchorA);
    ASSERT_EQ(GetLocalAnchorB(world, joint), jd.localAnchorB);
    EXPECT_EQ(GetAnchorA(world, joint), loc0 + jd.localAnchorA);
    EXPECT_EQ(GetAnchorB(world, joint), loc1 + jd.localAnchorB);
}

TEST(PrismaticJoint, GetJointTranslation)
{
    World world;
    
    const auto loc0 = Length2{+1_m, -3_m};
    const auto loc1 = Length2{+1_m, +3_m};
    
    const auto b0 = world.CreateBody(BodyConf{}.UseLocation(loc0));
    const auto b1 = world.CreateBody(BodyConf{}.UseLocation(loc1));
    
    auto jd = PrismaticJointConf{};
    jd.bodyA = b0;
    jd.bodyB = b1;
    jd.localAnchorA = Length2(-1_m, 5_m);
    jd.localAnchorB = Length2(+1_m, 5_m);
    
    auto joint = world.CreateJoint(Joint{jd});
    EXPECT_EQ(GetJointTranslation(world, joint), Length(2_m));
}

TEST(PrismaticJoint, GetLinearVelocity)
{
    World world;
    
    const auto loc0 = Length2{+1_m, -3_m};
    const auto loc1 = Length2{+1_m, +3_m};
    
    const auto b0 = world.CreateBody(BodyConf{}.UseLocation(loc0));
    const auto b1 = world.CreateBody(BodyConf{}.UseLocation(loc1));
    
    auto jd = PrismaticJointConf{};
    jd.bodyA = b0;
    jd.bodyB = b1;
    jd.localAnchorA = Length2(-1_m, 5_m);
    jd.localAnchorB = Length2(+1_m, 5_m);
    
    EXPECT_EQ(GetLinearVelocity(world, jd), LinearVelocity(0));
}

TEST(PrismaticJoint, WithDynamicCirclesAndLimitEnabled)
{
    const auto circle = DiskShapeConf{}.UseRadius(0.2_m);
    auto world = World{};
    const auto p1 = Length2{-1_m, 0_m};
    const auto p2 = Length2{+1_m, 0_m};
    const auto b1 = world.CreateBody(BodyConf{}.UseType(BodyType::Dynamic).UseLocation(p1));
    const auto b2 = world.CreateBody(BodyConf{}.UseType(BodyType::Dynamic).UseLocation(p2));
    world.CreateFixture(b1, Shape{circle});
    world.CreateFixture(b2, Shape{circle});
    const auto anchor = Length2(2_m, 1_m);
    const auto jd = GetPrismaticJointConf(world, b1, b2, anchor, UnitVec::GetRight()).UseEnableLimit(true);
    const auto joint = world.CreateJoint(Joint{jd});
    ASSERT_NE(joint, InvalidJointID);
    {
        const auto conf = TypeCast<PrismaticJointConf>(GetJoint(world, joint));
        ASSERT_EQ(GetLimitState(conf), LimitState::e_inactiveLimit);
        ASSERT_EQ(GetLinearLowerLimit(conf), 0_m);
        ASSERT_EQ(GetLinearUpperLimit(conf), 0_m);
    }

    Step(world, 1_s);
    EXPECT_NEAR(double(Real{GetX(GetLocation(world, b1)) / Meter}), -1.0, 0.001);
    EXPECT_NEAR(double(Real{GetY(GetLocation(world, b1)) / Meter}), 0.0, 0.001);
    EXPECT_NEAR(double(Real{GetX(GetLocation(world, b2)) / Meter}), +1.0, 0.01);
    EXPECT_NEAR(double(Real{GetY(GetLocation(world, b2)) / Meter}), 0.0, 0.01);
    EXPECT_EQ(GetAngle(world, b1), 0_deg);
    EXPECT_EQ(GetAngle(world, b2), 0_deg);
    {
        auto conf = TypeCast<PrismaticJointConf>(GetJoint(world, joint));
        EXPECT_EQ(GetLinearLowerLimit(conf), 0_m);
        EXPECT_EQ(GetLinearUpperLimit(conf), 0_m);
        EXPECT_EQ(GetLimitState(conf), LimitState::e_equalLimits);
        EXPECT_NO_THROW(SetLinearLimits(conf, 0_m, 2_m));
        EXPECT_NO_THROW(SetJoint(world, joint, conf));
    }
    EXPECT_NO_THROW(Step(world, 1_s));
    {
        auto conf = TypeCast<PrismaticJointConf>(GetJoint(world, joint));
        EXPECT_EQ(GetLinearLowerLimit(conf), 0_m);
        EXPECT_EQ(GetLinearUpperLimit(conf), 2_m);
        EXPECT_EQ(GetLimitState(conf), LimitState::e_atLowerLimit);
        EXPECT_NO_THROW(SetLinearLimits(conf, -2_m, 0_m));
        EXPECT_NO_THROW(SetJoint(world, joint, conf));
    }
    EXPECT_NO_THROW(Step(world, 1_s));
    {
        auto conf = TypeCast<PrismaticJointConf>(GetJoint(world, joint));
        EXPECT_EQ(GetLinearLowerLimit(conf), -2_m);
        EXPECT_EQ(GetLinearUpperLimit(conf), 0_m);
        EXPECT_EQ(GetLimitState(conf), LimitState::e_atUpperLimit);
    }
    EXPECT_NO_THROW(EnableMotor(world, joint, true));
    EXPECT_NO_THROW(Step(world, 1_s));
    EXPECT_EQ(GetLinearMotorImpulse(world, joint), Momentum(0));
}
