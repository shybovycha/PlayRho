/*
 * Copyright (c) 2017 Louis Langholtz https://github.com/louis-langholtz/Box2D
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

#include "gtest/gtest.h"
#include <Box2D/Dynamics/Body.hpp>
#include <Box2D/Dynamics/BodyDef.hpp>
#include <Box2D/Dynamics/World.hpp>
#include <Box2D/Dynamics/Fixture.hpp>
#include <Box2D/Dynamics/Joints/Joint.hpp>
#include <Box2D/Collision/Shapes/CircleShape.hpp>

#include <chrono>

using namespace box2d;

TEST(Body, ContactsByteSize)
{
    // Size is C++ library dependent.
    // Some platforms it's 24-bytes. Others 16.
    EXPECT_TRUE(sizeof(Body::Contacts) == size_t(24) || sizeof(Body::Contacts) == size_t(16));
}

TEST(Body, JointsByteSize)
{
    // Size is C++ library dependent.
    // Some platforms it's 40-bytes. Others 56.
#ifdef __APPLE__
    // using std::list<std::pair<JointKey, Joint*>>
    EXPECT_EQ(sizeof(Body::Joints), size_t(24));
    // using std::unordered_set:
    // EXPECT_EQ(sizeof(Body::Joints), size_t(40));
#endif
#ifdef __linux__
    EXPECT_EQ(sizeof(Body::Joints), size_t(56));
#endif
}

TEST(Body, FixturesByteSize)
{
    // Size is arch-dependent (on size of pointer/address)
    EXPECT_EQ(sizeof(Body::Fixtures), size_t(8));
}

TEST(Body, ByteSize)
{
    const auto contactsSize = sizeof(Body::Contacts);
    const auto jointsSize = sizeof(Body::Joints);
    const auto fixturesSize = sizeof(Body::Fixtures);
    const auto allSize = contactsSize + jointsSize + fixturesSize;

    // architecture dependent...
    switch (sizeof(RealNum))
    {
        case  4: EXPECT_EQ(sizeof(Body), size_t(120 + allSize)); break;
        case  8: EXPECT_EQ(sizeof(Body), size_t(216 + allSize)); break;
        case 16: EXPECT_EQ(sizeof(Body), size_t(496)); break;
        default: FAIL(); break;
    }
}

TEST(Body, WorldCreated)
{
    World world;
    
    auto body = world.CreateBody();
    ASSERT_NE(body, nullptr);

    EXPECT_EQ(body->GetWorld(), &world);
    EXPECT_EQ(body->GetUserData(), nullptr);
    EXPECT_TRUE(body->IsEnabled());
    EXPECT_FALSE(body->IsAwake());
    EXPECT_FALSE(body->IsSpeedable());
    EXPECT_FALSE(body->IsAccelerable());
    
    EXPECT_TRUE(body->GetFixtures().empty());
    {
        int i = 0;
        for (auto&& fixture: body->GetFixtures())
        {
            EXPECT_EQ(fixture->GetBody(), body);
            ++i;
        }
        EXPECT_EQ(i, 0);
    }

    EXPECT_TRUE(body->GetJoints().empty());
    {
        int i = 0;
        for (auto&& joint: body->GetJoints())
        {
            NOT_USED(joint);
            ++i;
        }
        EXPECT_EQ(i, 0);        
    }
    
    EXPECT_TRUE(body->GetContacts().empty());
    {
        int i = 0;
        for (auto&& ce: body->GetContacts())
        {
            NOT_USED(ce);
            ++i;
        }
        EXPECT_EQ(i, 0);        
    }
}

TEST(Body, CreateFixture)
{
    World world;
    const auto body = world.CreateBody();

    const auto valid_shape = std::make_shared<CircleShape>(RealNum{1} * Meter);

    const auto invalid_friction_shape = std::make_shared<CircleShape>(RealNum{1} * Meter);
    invalid_friction_shape->SetFriction(-0.1f);
    
    const auto invalid_density_shape = std::make_shared<CircleShape>(RealNum{1} * Meter);
    invalid_density_shape->SetDensity(std::numeric_limits<RealNum>::quiet_NaN() * KilogramPerSquareMeter);
    
    const auto invalid_restitution_shape = std::make_shared<CircleShape>(RealNum{1} * Meter);
    invalid_restitution_shape->SetRestitution(std::numeric_limits<RealNum>::quiet_NaN());

    // Check default settings
    EXPECT_NE(body->CreateFixture(valid_shape, FixtureDef{}), nullptr);
    
    // Check friction settings
    EXPECT_EQ(body->CreateFixture(invalid_friction_shape), nullptr);
    
    // Check density settings
    EXPECT_EQ(body->CreateFixture(invalid_density_shape), nullptr);
    
    // Check restitution settings
    EXPECT_EQ(body->CreateFixture(invalid_restitution_shape), nullptr);
}

TEST(Body, CreateAndDestroyFixture)
{
    World world;

    auto body = world.CreateBody();
    ASSERT_NE(body, nullptr);
    EXPECT_TRUE(body->GetFixtures().empty());
    EXPECT_FALSE(body->IsMassDataDirty());

    auto conf = CircleShape::Conf{};
    conf.vertexRadius = RealNum{2.871f} * Meter;
    conf.location = Vec2{1.912f, -77.31f} * Meter;
    conf.density = RealNum{1} * KilogramPerSquareMeter;
    const auto shape = std::make_shared<CircleShape>(conf);
    
    auto fixture = body->CreateFixture(shape, FixtureDef{}, false);
    const auto fshape = fixture->GetShape();
    ASSERT_NE(fshape, nullptr);
    EXPECT_EQ(typeid(*fshape), typeid(CircleShape));
    EXPECT_EQ(GetVertexRadius(*fshape), GetVertexRadius(*shape));
    EXPECT_EQ(static_cast<const CircleShape*>(fshape)->GetLocation().x, shape->GetLocation().x);
    EXPECT_EQ(static_cast<const CircleShape*>(fshape)->GetLocation().y, shape->GetLocation().y);
    EXPECT_FALSE(body->GetFixtures().empty());
    {
        int i = 0;
        for (auto&& f: body->GetFixtures())
        {
            EXPECT_EQ(f, fixture);
            ++i;
        }
        EXPECT_EQ(i, 1);
    }
    EXPECT_TRUE(body->IsMassDataDirty());
    body->ResetMassData();
    EXPECT_FALSE(body->IsMassDataDirty());

    body->DestroyFixture(fixture, false);
    EXPECT_TRUE(body->GetFixtures().empty());
    EXPECT_TRUE(body->IsMassDataDirty());
    
    body->ResetMassData();
    EXPECT_FALSE(body->IsMassDataDirty());
}

TEST(Body, CreateLotsOfFixtures)
{
    BodyDef bd;
    bd.type = BodyType::Dynamic;
    auto conf = CircleShape::Conf{};
    conf.vertexRadius = RealNum{2.871f} * Meter;
    conf.location = Vec2{1.912f, -77.31f} * Meter;
    conf.density = RealNum{1.3f} * KilogramPerSquareMeter;
    const auto shape = std::make_shared<CircleShape>(conf);
    const auto num = 5000;
    std::chrono::time_point<std::chrono::system_clock> start, end;
    
    start = std::chrono::system_clock::now();
    {
        World world;

        auto body = world.CreateBody(bd);
        ASSERT_NE(body, nullptr);
        EXPECT_TRUE(body->GetFixtures().empty());
        
        for (auto i = decltype(num){0}; i < num; ++i)
        {
            auto fixture = body->CreateFixture(shape, FixtureDef{}, false);
            ASSERT_NE(fixture, nullptr);
        }
        body->ResetMassData();
        
        EXPECT_FALSE(body->GetFixtures().empty());
        {
            int i = decltype(num){0};
            for (auto&& f: body->GetFixtures())
            {
                NOT_USED(f);
                ++i;
            }
            EXPECT_EQ(i, num);
        }
    }
    end = std::chrono::system_clock::now();
    const std::chrono::duration<double> elapsed_secs_resetting_at_end = end - start;

    start = std::chrono::system_clock::now();
    {
        World world;
        
        auto body = world.CreateBody(bd);
        ASSERT_NE(body, nullptr);
        EXPECT_TRUE(body->GetFixtures().empty());
        
        for (auto i = decltype(num){0}; i < num; ++i)
        {
            auto fixture = body->CreateFixture(shape, FixtureDef{}, true);
            ASSERT_NE(fixture, nullptr);
        }
        
        EXPECT_FALSE(body->GetFixtures().empty());
        {
            int i = decltype(num){0};
            for (auto&& f: body->GetFixtures())
            {
                NOT_USED(f);
                ++i;
            }
            EXPECT_EQ(i, num);
        }
    }
    end = std::chrono::system_clock::now();
    const std::chrono::duration<double> elapsed_secs_resetting_in_create = end - start;

    EXPECT_LT(elapsed_secs_resetting_at_end.count(), elapsed_secs_resetting_in_create.count());
}