/*
 * Original work Copyright (c) 2007-2011 Erin Catto http://www.box2d.org
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

#include <PlayRho/Dynamics/Joints/GearJoint.hpp>

#include <PlayRho/Dynamics/Joints/RevoluteJoint.hpp>
#include <PlayRho/Dynamics/Joints/PrismaticJoint.hpp>
#include <PlayRho/Dynamics/Joints/JointVisitor.hpp>
#include <PlayRho/Dynamics/World.hpp>
#include <PlayRho/Dynamics/StepConf.hpp>
#include <PlayRho/Dynamics/Contacts/ContactSolver.hpp>
#include <PlayRho/Dynamics/Contacts/BodyConstraint.hpp>

namespace playrho {
namespace d2 {

// Gear Joint:
// C0 = (coordinate1 + ratio * coordinate2)_initial
// C = (coordinate1 + ratio * coordinate2) - C0 = 0
// J = [J1 ratio * J2]
// K = J * invM * JT
//   = J1 * invM1 * J1T + ratio * ratio * J2 * invM2 * J2T
//
// Revolute:
// coordinate = rotation
// Cdot = angularVelocity
// J = [0 0 1]
// K = J * invM * JT = invI
//
// Prismatic:
// coordinate = dot(p - pg, ug)
// Cdot = dot(v + cross(w, r), ug)
// J = [ug cross(r, ug)]
// K = J * invM * JT = invMass + invI * cross(r, ug)^2

namespace {

inline bool IsValidType(JointType t) noexcept
{
    return (t == JointType::Revolute) || (t == JointType::Prismatic);
}

} // unnamed namespace

bool GearJoint::IsOkay(const GearJointConf& def) noexcept
{
    if (!IsValidType(def.type1) || !IsValidType(def.type2))
    {
        return false;
    }
    if (!Joint::IsOkay(def))
    {
        return false;
    }
    return true;
}

GearJoint::GearJoint(const GearJointConf& def):
    Joint(def),
    m_type1(def.type1),
    m_type2(def.type2),
    m_bodyC(def.bodyC),
    m_bodyD(def.bodyD),
    m_localAnchorA(def.localAnchorA),
    m_localAnchorC(def.localAnchorC),
    m_localAnchorB(def.localAnchorB),
    m_localAnchorD(def.localAnchorD),
    m_localAxis1(def.localAxis1),
    m_localAxis2(def.localAxis2),
    m_referenceAngle1(def.referenceAngle1),
    m_referenceAngle2(def.referenceAngle2),
    m_ratio(def.ratio),
    m_constant(def.constant)
{
    assert(IsValidType(m_type1));
    assert(IsValidType(m_type2));
}

void GearJoint::Accept(JointVisitor& visitor) const
{
    visitor.Visit(*this);
}

void GearJoint::Accept(JointVisitor& visitor)
{
    visitor.Visit(*this);
}

void GearJoint::InitVelocityConstraints(BodyConstraintsMap& bodies, const StepConf& step,
                                        const ConstraintSolverConf&)
{
    auto& bodyConstraintA = At(bodies, GetBodyA());
    auto& bodyConstraintB = At(bodies, GetBodyB());
    auto& bodyConstraintC = At(bodies, m_bodyC);
    auto& bodyConstraintD = At(bodies, m_bodyD);

    auto velA = bodyConstraintA.GetVelocity();
    const auto aA = bodyConstraintA.GetPosition().angular;

    auto velB = bodyConstraintB.GetVelocity();
    const auto aB = bodyConstraintB.GetPosition().angular;

    auto velC = bodyConstraintC.GetVelocity();
    const auto aC = bodyConstraintC.GetPosition().angular;

    auto velD = bodyConstraintD.GetVelocity();
    const auto aD = bodyConstraintD.GetPosition().angular;

    const auto qA = UnitVec::Get(aA);
    const auto qB = UnitVec::Get(aB);
    const auto qC = UnitVec::Get(aC);
    const auto qD = UnitVec::Get(aD);

    auto invMass = Real{0}; // Unitless to double for either linear mass or angular mass.

    if (m_type1 == JointType::Revolute)
    {
        m_JvAC = Vec2{};
        m_JwA = 1_m;
        m_JwC = 1_m;
        const auto invAngMass = bodyConstraintA.GetInvRotInertia() + bodyConstraintC.GetInvRotInertia();
        invMass += StripUnit(invAngMass);
    }
    else
    {
        const auto u = Rotate(m_localAxis1, qC);
        const auto rC = Length2{Rotate(m_localAnchorC - bodyConstraintC.GetLocalCenter(), qC)};
        const auto rA = Length2{Rotate(m_localAnchorA - bodyConstraintA.GetLocalCenter(), qA)};
        m_JvAC = Real{1} * u;
        m_JwC = Cross(rC, u);
        m_JwA = Cross(rA, u);
        const auto invRotMassC = InvMass{bodyConstraintC.GetInvRotInertia() * Square(m_JwC) / SquareRadian};
        const auto invRotMassA = InvMass{bodyConstraintA.GetInvRotInertia() * Square(m_JwA) / SquareRadian};
        const auto invLinMass = InvMass{bodyConstraintC.GetInvMass() + bodyConstraintA.GetInvMass() + invRotMassC + invRotMassA};
        invMass += StripUnit(invLinMass);
    }

    if (m_type2 == JointType::Revolute)
    {
        m_JvBD = Vec2{};
        m_JwB = m_ratio * Meter;
        m_JwD = m_ratio * Meter;
        const auto invAngMass = InvRotInertia{Square(m_ratio) * (bodyConstraintB.GetInvRotInertia() + bodyConstraintD.GetInvRotInertia())};
        invMass += StripUnit(invAngMass);
    }
    else
    {
        const auto u = Rotate(m_localAxis2, qD);
        const auto rD = Rotate(m_localAnchorD - bodyConstraintD.GetLocalCenter(), qD);
        const auto rB = Rotate(m_localAnchorB - bodyConstraintB.GetLocalCenter(), qB);
        m_JvBD = m_ratio * u;
        m_JwD = m_ratio * Cross(rD, u);
        m_JwB = m_ratio * Cross(rB, u);
        const auto invRotMassD = InvMass{bodyConstraintD.GetInvRotInertia() * Square(m_JwD) / SquareRadian};
        const auto invRotMassB = InvMass{bodyConstraintB.GetInvRotInertia() * Square(m_JwB) / SquareRadian};
        const auto invLinMass = InvMass{
            Square(m_ratio) * (bodyConstraintD.GetInvMass() + bodyConstraintB.GetInvMass()) +
            invRotMassD + invRotMassB
        };
        invMass += StripUnit(invLinMass);
    }

    // Compute effective mass.
    m_mass = (invMass > Real{0})? Real{1} / invMass: Real{0};

    if (step.doWarmStart)
    {
        velA += Velocity{
            (bodyConstraintA.GetInvMass() * m_impulse) * m_JvAC,
            bodyConstraintA.GetInvRotInertia() * m_impulse * m_JwA / Radian
        };
        velB += Velocity{
            (bodyConstraintB.GetInvMass() * m_impulse) * m_JvBD,
            bodyConstraintB.GetInvRotInertia() * m_impulse * m_JwB / Radian
        };
        velC -= Velocity{
            (bodyConstraintC.GetInvMass() * m_impulse) * m_JvAC,
            bodyConstraintC.GetInvRotInertia() * m_impulse * m_JwC / Radian
        };
        velD -= Velocity{
            (bodyConstraintD.GetInvMass() * m_impulse) * m_JvBD,
            bodyConstraintD.GetInvRotInertia() * m_impulse * m_JwD / Radian
        };
    }
    else
    {
        m_impulse = 0_Ns;
    }

    bodyConstraintA.SetVelocity(velA);
    bodyConstraintB.SetVelocity(velB);
    bodyConstraintC.SetVelocity(velC);
    bodyConstraintD.SetVelocity(velD);
}

bool GearJoint::SolveVelocityConstraints(BodyConstraintsMap& bodies, const StepConf&)
{
    auto& bodyConstraintA = At(bodies, GetBodyA());
    auto& bodyConstraintB = At(bodies, GetBodyB());
    auto& bodyConstraintC = At(bodies, m_bodyC);
    auto& bodyConstraintD = At(bodies, m_bodyD);

    auto velA = bodyConstraintA.GetVelocity();
    auto velB = bodyConstraintB.GetVelocity();
    auto velC = bodyConstraintC.GetVelocity();
    auto velD = bodyConstraintD.GetVelocity();

    const auto acDot = LinearVelocity{Dot(m_JvAC, velA.linear - velC.linear)};
    const auto bdDot = LinearVelocity{Dot(m_JvBD, velB.linear - velD.linear)};
    const auto Cdot = acDot + bdDot
        + (m_JwA * velA.angular - m_JwC * velC.angular) / Radian
        + (m_JwB * velB.angular - m_JwD * velD.angular) / Radian;

    const auto impulse = Momentum{-m_mass * Kilogram * Cdot};
    m_impulse += impulse;

    velA += Velocity{
        (bodyConstraintA.GetInvMass() * impulse) * m_JvAC,
        bodyConstraintA.GetInvRotInertia() * impulse * m_JwA / Radian
    };
    velB += Velocity{
        (bodyConstraintB.GetInvMass() * impulse) * m_JvBD,
        bodyConstraintB.GetInvRotInertia() * impulse * m_JwB / Radian
    };
    velC -= Velocity{
        (bodyConstraintC.GetInvMass() * impulse) * m_JvAC,
        bodyConstraintC.GetInvRotInertia() * impulse * m_JwC / Radian
    };
    velD -= Velocity{
        (bodyConstraintD.GetInvMass() * impulse) * m_JvBD,
        bodyConstraintD.GetInvRotInertia() * impulse * m_JwD / Radian
    };

    bodyConstraintA.SetVelocity(velA);
    bodyConstraintB.SetVelocity(velB);
    bodyConstraintC.SetVelocity(velC);
    bodyConstraintD.SetVelocity(velD);
    
    return impulse == 0_Ns;
}

bool GearJoint::SolvePositionConstraints(BodyConstraintsMap& bodies, const ConstraintSolverConf& conf) const
{
    auto& bodyConstraintA = At(bodies, GetBodyA());
    auto& bodyConstraintB = At(bodies, GetBodyB());
    auto& bodyConstraintC = At(bodies, m_bodyC);
    auto& bodyConstraintD = At(bodies, m_bodyD);

    auto posA = bodyConstraintA.GetPosition();
    auto posB = bodyConstraintB.GetPosition();
    auto posC = bodyConstraintC.GetPosition();
    auto posD = bodyConstraintD.GetPosition();

    const auto qA = UnitVec::Get(posA.angular);
    const auto qB = UnitVec::Get(posB.angular);
    const auto qC = UnitVec::Get(posC.angular);
    const auto qD = UnitVec::Get(posD.angular);

    const auto linearError = 0_m;

    Vec2 JvAC, JvBD;
    Real JwA, JwB, JwC, JwD;

    auto coordinateA = Real{0}; // Angle or length.
    auto coordinateB = Real{0};
    auto invMass = Real{0}; // Inverse linear mass or inverse angular mass.

    if (m_type1 == JointType::Revolute)
    {
        JvAC = Vec2{};
        JwA = 1;
        JwC = 1;
        const auto invAngMass = bodyConstraintA.GetInvRotInertia() + bodyConstraintC.GetInvRotInertia();
        invMass += StripUnit(invAngMass);
        coordinateA = (posA.angular - posC.angular - m_referenceAngle1) / Radian;
    }
    else
    {
        const auto u = Rotate(m_localAxis1, qC);
        const auto rC = Rotate(m_localAnchorC - bodyConstraintC.GetLocalCenter(), qC);
        const auto rA = Rotate(m_localAnchorA - bodyConstraintA.GetLocalCenter(), qA);
        JvAC = u * Real{1};
        JwC = StripUnit(Length{Cross(rC, u)});
        JwA = StripUnit(Length{Cross(rA, u)});
        const auto invLinMass = InvMass{bodyConstraintC.GetInvMass() + bodyConstraintA.GetInvMass()};
        const auto invRotMassC = InvMass{bodyConstraintC.GetInvRotInertia() * Square(JwC * Meter / Radian)};
        const auto invRotMassA = InvMass{bodyConstraintA.GetInvRotInertia() * Square(JwA * Meter / Radian)};
        invMass += StripUnit(invLinMass + invRotMassC + invRotMassA);
        const auto pC = m_localAnchorC - bodyConstraintC.GetLocalCenter();
        const auto pA = InverseRotate(rA + (posA.linear - posC.linear), qC);
        coordinateA = Dot(pA - pC, m_localAxis1) / Meter;
    }

    if (m_type2 == JointType::Revolute)
    {
        JvBD = Vec2{};
        JwB = m_ratio;
        JwD = m_ratio;
        const auto invAngMass = InvRotInertia{
            Square(m_ratio) * (bodyConstraintB.GetInvRotInertia() + bodyConstraintD.GetInvRotInertia())
        };
        invMass += StripUnit(invAngMass);
        coordinateB = (posB.angular - posD.angular - m_referenceAngle2) / Radian;
    }
    else
    {
        const auto u = Rotate(m_localAxis2, qD);
        const auto rD = Rotate(m_localAnchorD - bodyConstraintD.GetLocalCenter(), qD);
        const auto rB = Rotate(m_localAnchorB - bodyConstraintB.GetLocalCenter(), qB);
        JvBD = m_ratio * u;
        JwD = m_ratio * StripUnit(Length{Cross(rD, u)});
        JwB = m_ratio * StripUnit(Length{Cross(rB, u)});
        const auto invLinMass = InvMass{Square(m_ratio) * (bodyConstraintD.GetInvMass() + bodyConstraintB.GetInvMass())};
        const auto invRotMassD = InvMass{bodyConstraintD.GetInvRotInertia() * Square(JwD * Meter / Radian)};
        const auto invRotMassB = InvMass{bodyConstraintB.GetInvRotInertia() * Square(JwB * Meter / Radian)};
        invMass += StripUnit(invLinMass + invRotMassD + invRotMassB);
        const auto pD = m_localAnchorD - bodyConstraintD.GetLocalCenter();
        const auto pB = InverseRotate(rB + (posB.linear - posD.linear), qD);
        coordinateB = Dot(pB - pD, m_localAxis2) / Meter;
    }

    const auto C = ((coordinateA + m_ratio * coordinateB) - m_constant);

    const auto impulse = ((invMass > 0)? -C / invMass: 0) * Kilogram * Meter;

    posA += Position{
        bodyConstraintA.GetInvMass() * impulse * JvAC,
        bodyConstraintA.GetInvRotInertia() * impulse * JwA * Meter / Radian
    };
    posB += Position{
        bodyConstraintB.GetInvMass() * impulse * JvBD,
        bodyConstraintB.GetInvRotInertia() * impulse * JwB * Meter / Radian
    };
    posC -= Position{
        bodyConstraintC.GetInvMass() * impulse * JvAC,
        bodyConstraintC.GetInvRotInertia() * impulse * JwC * Meter / Radian
    };
    posD -= Position{
        bodyConstraintD.GetInvMass() * impulse * JvBD,
        bodyConstraintD.GetInvRotInertia() * impulse * JwD * Meter / Radian
    };

    bodyConstraintA.SetPosition(posA);
    bodyConstraintB.SetPosition(posB);
    bodyConstraintC.SetPosition(posC);
    bodyConstraintD.SetPosition(posD);

    // TODO_ERIN not implemented
    return linearError < conf.linearSlop;
}

Momentum2 GearJoint::GetLinearReaction() const
{
    return m_impulse * m_JvAC;
}

AngularMomentum GearJoint::GetAngularReaction() const
{
    return m_impulse * m_JwA / Radian;
}

void GearJoint::SetRatio(Real ratio)
{
    assert(IsValid(ratio));
    m_ratio = ratio;
}

} // namespace d2
} // namespace playrho
