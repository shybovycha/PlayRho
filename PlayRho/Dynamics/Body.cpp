/*
 * Original work Copyright (c) 2006-2007 Erin Catto http://www.box2d.org
 * Modified work Copyright (c) 2017 Louis Langholtz https://github.com/louis-langholtz/PlayRho
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include <PlayRho/Dynamics/Body.hpp>

#include <PlayRho/Dynamics/BodyConf.hpp>
#include <PlayRho/Common/WrongState.hpp>

#include <iterator>
#include <type_traits>
#include <utility>

namespace playrho {
namespace d2 {

static_assert(std::is_default_constructible<Body>::value,
              "Body must be default constructible!");
static_assert(std::is_copy_constructible<Body>::value,
              "Body must be copy constructible!");
static_assert(std::is_move_constructible<Body>::value,
              "Body must be move constructible!");
static_assert(std::is_copy_assignable<Body>::value,
              "Body must be copy assignable!");
static_assert(std::is_move_assignable<Body>::value,
              "Body must be move assignable!");
static_assert(std::is_nothrow_destructible<Body>::value,
              "Body must be nothrow destructible!");

Body::FlagsType Body::GetFlags(const BodyConf& bd) noexcept
{
    // @invariant Only bodies that allow sleeping, can be put to sleep.
    // @invariant Only "speedable" bodies can be awake.
    // @invariant Only "speedable" bodies can have non-zero velocities.
    // @invariant Only "accelerable" bodies can have non-zero accelerations.
    // @invariant Only "accelerable" bodies can have non-zero "under-active" times.

    auto flags = GetFlags(bd.type);
    if (bd.bullet)
    {
        flags |= e_impenetrableFlag;
    }
    if (bd.fixedRotation)
    {
        flags |= e_fixedRotationFlag;
    }
    if (bd.allowSleep)
    {
        flags |= e_autoSleepFlag;
    }
    if (bd.awake)
    {
        if ((flags & e_velocityFlag) != 0)
        {
            flags |= e_awakeFlag;
        }
    }
    else
    {
        if (!bd.allowSleep && ((flags & e_velocityFlag) != 0))
        {
            flags |= e_awakeFlag;
        }
    }
    if (bd.enabled)
    {
        flags |= e_enabledFlag;
    }
    return flags;
}

Body::Body(const BodyConf& bd) noexcept:
    m_xf{::playrho::d2::GetTransformation(bd)},
    m_sweep{Position{bd.location, bd.angle}},
    m_flags{GetFlags(bd)},
    m_invMass{(bd.type == playrho::BodyType::Dynamic)? InvMass{Real{1} / Kilogram}: InvMass{0}},
    m_linearDamping{bd.linearDamping},
    m_angularDamping{bd.angularDamping}
{
    assert(IsValid(bd.location));
    assert(IsValid(bd.angle));
    assert(IsValid(bd.linearVelocity));
    assert(IsValid(bd.angularVelocity));
    assert(IsValid(m_xf));

    SetVelocity(Velocity{bd.linearVelocity, bd.angularVelocity});
    SetAcceleration(bd.linearAcceleration, bd.angularAcceleration);
    SetUnderActiveTime(bd.underActiveTime);
}

void Body::SetVelocity(const Velocity& velocity) noexcept
{
    if ((velocity.linear != LinearVelocity2{}) || (velocity.angular != 0_rpm))
    {
        if (!IsSpeedable())
        {
            return;
        }
        SetAwakeFlag();
        ResetUnderActiveTime();
    }
    JustSetVelocity(velocity);
}

void Body::SetAcceleration(LinearAcceleration2 linear, AngularAcceleration angular) noexcept
{
    assert(IsValid(linear));
    assert(IsValid(angular));

    if ((m_linearAcceleration == linear) && (m_angularAcceleration == angular))
    {
        // no change, bail...
        return;
    }
    
    if (!IsAccelerable())
    {
        if ((linear != LinearAcceleration2{}) || (angular != AngularAcceleration{0}))
        {
            // non-accelerable bodies can only be set to zero acceleration, bail...
            return;
        }
    }
    else
    {
        if ((m_angularAcceleration < angular) ||
            (GetMagnitudeSquared(m_linearAcceleration) < GetMagnitudeSquared(linear)) ||
            (playrho::GetAngle(m_linearAcceleration) != playrho::GetAngle(linear)) ||
            (signbit(m_angularAcceleration) != signbit(angular)))
        {
            // Increasing accel or changing direction of accel, awake & reset time.
            SetAwakeFlag();
            ResetUnderActiveTime();
        }
    }
    
    m_linearAcceleration = linear;
    m_angularAcceleration = angular;
}

void Body::SetFixedRotation(bool flag)
{
    if (flag)
    {
        m_flags |= e_fixedRotationFlag;
    }
    else
    {
        m_flags &= ~e_fixedRotationFlag;
    }
    m_angularVelocity = 0_rpm;
}

bool Body::Insert(JointID joint, BodyID other)
{
    m_joints.push_back(std::make_pair(other, joint));
    return true;
}

bool Body::Insert(ContactKey key, ContactID contact)
{
#ifndef NDEBUG
    // Prevent the same contact from being added more than once...
    const auto it = std::find_if(cbegin(m_contacts), cend(m_contacts), [contact](KeyedContactPtr ci) {
        return std::get<ContactID>(ci) == contact;
    });
    assert(it == end(m_contacts));
    if (it != end(m_contacts))
    {
        return false;
    }
#endif
    m_contacts.emplace_back(key, contact);
    return true;
}

bool Body::Erase(JointID joint)
{
    const auto it = std::find_if(begin(m_joints), end(m_joints), [joint](KeyedJointPtr ji) {
        return std::get<JointID>(ji) == joint;
    });
    if (it != end(m_joints))
    {
        m_joints.erase(it);
        return true;
    }
    return false;
}

bool Body::Erase(ContactID contact)
{
    const auto it = std::find_if(begin(m_contacts), end(m_contacts), [contact](KeyedContactPtr ci) {
        return std::get<ContactID>(ci) == contact;
    });
    if (it != end(m_contacts))
    {
        m_contacts.erase(it);
        return true;
    }
    return false;
}

void Body::Erase(const std::function<bool(ContactID)>& callback)
{
    auto last = end(m_contacts);
    auto iter = begin(m_contacts);
    auto index = Body::Contacts::difference_type{0};
    while (iter != last)
    {
        const auto contact = GetContactPtr(*iter);
        if (callback(contact))
        {
            m_contacts.erase(iter);
            iter = begin(m_contacts) + index;
            last = end(m_contacts);
        }
        else
        {
            iter = std::next(iter);
            ++index;
        }
    }
}

// Free functions...

Velocity GetVelocity(const Body& body, Time h) noexcept
{
    // Integrate velocity and apply damping.
    auto velocity = body.GetVelocity();
    if (body.IsAccelerable())
    {
        // Integrate velocities.
        velocity.linear += h * body.GetLinearAcceleration();
        velocity.angular += h * body.GetAngularAcceleration();

        // Apply damping.
        // Ordinary differential equation: dv/dt + c * v = 0
        //                       Solution: v(t) = v0 * exp(-c * t)
        // Time step: v(t + dt) = v0 * exp(-c * (t + dt)) = v0 * exp(-c * t) * exp(-c * dt) = v * exp(-c * dt)
        // v2 = exp(-c * dt) * v1
        // Pade approximation (see https://en.wikipedia.org/wiki/Pad%C3%A9_approximant ):
        // v2 = v1 * 1 / (1 + c * dt)
        velocity.linear  /= Real{1 + h * body.GetLinearDamping()};
        velocity.angular /= Real{1 + h * body.GetAngularDamping()};
    }

    return velocity;
}

Velocity Cap(Velocity velocity, Time h, MovementConf conf) noexcept
{
    const auto translation = h * velocity.linear;
    const auto lsquared = GetMagnitudeSquared(translation);
    if (lsquared > Square(conf.maxTranslation))
    {
        // Scale back linear velocity so max translation not exceeded.
        const auto ratio = conf.maxTranslation / sqrt(lsquared);
        velocity.linear *= ratio;
    }
    
    const auto absRotation = abs(h * velocity.angular);
    if (absRotation > conf.maxRotation)
    {
        // Scale back angular velocity so max rotation not exceeded.
        const auto ratio = conf.maxRotation / absRotation;
        velocity.angular *= ratio;
    }
    
    return velocity;
}

FixtureCounter GetFixtureCount(const Body& body) noexcept
{
    return static_cast<FixtureCounter>(size(body.GetFixtures()));
}

Transformation GetTransformation(const BodyConf& conf)
{
    return {conf.location, UnitVec::Get(conf.angle)};
}

} // namespace d2
} // namespace playrho
