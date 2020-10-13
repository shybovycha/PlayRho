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

#ifndef PLAYRHO_DYNAMICS_JOINTS_DISTANCEJOINTCONF_HPP
#define PLAYRHO_DYNAMICS_JOINTS_DISTANCEJOINTCONF_HPP

#include <PlayRho/Dynamics/Joints/JointConf.hpp>
#include <PlayRho/Common/NonNegative.hpp>
#include <PlayRho/Common/Math.hpp>

namespace playrho {

struct ConstraintSolverConf;
class StepConf;

namespace d2 {

class World;
class BodyConstraint;

/// @brief Distance joint definition.
/// @details This requires defining an anchor point on both bodies and the non-zero
///   length of the distance joint. The definition uses local anchor points so that
///   the initial configuration can violate the constraint slightly. This helps when
///   saving and loading a game.
/// @warning Do not use a zero or short length.
/// @see Joint, World::CreateJoint
/// @ingroup JointsGroup
/// @image html distanceJoint.gif
struct DistanceJointConf : public JointBuilder<DistanceJointConf>
{
    /// @brief Super type.
    using super = JointBuilder<DistanceJointConf>;

    /// @brief Default constructor.
    constexpr DistanceJointConf() = default;

    /// @brief Copy constructor.
    DistanceJointConf(const DistanceJointConf& copy) = default;

    /// @brief Initializing constructor.
    /// @details Initialize the bodies, anchors, and length using the world anchors.
    DistanceJointConf(BodyID bA, BodyID bB,
                      Length2 laA = Length2{}, Length2 laB = Length2{}, Length l = 1_m) noexcept;

    /// @brief Uses the given length.
    /// @note Manipulating the length when the frequency is zero can lead to non-physical behavior.
    constexpr auto& UseLength(Length v) noexcept
    {
        length = v;
        return *this;
    }

    /// @brief Uses the given frequency.
    constexpr auto& UseFrequency(NonNegative<Frequency> v) noexcept
    {
        frequency = v;
        return *this;
    }

    /// @brief Uses the given damping ratio.
    constexpr auto& UseDampingRatio(Real v) noexcept
    {
        dampingRatio = v;
        return *this;
    }

    /// @brief Local anchor point relative to body A's origin.
    Length2 localAnchorA = Length2{};
    
    /// @brief Local anchor point relative to body B's origin.
    Length2 localAnchorB = Length2{};
    
    /// @brief Natural length between the anchor points.
    Length length = 1_m;
    
    /// @brief Mass-spring-damper frequency.
    /// @note 0 disables softness.
    NonNegative<Frequency> frequency = NonNegative<Frequency>{0_Hz};
    
    /// @brief Damping ratio.
    /// @note 0 = no damping, 1 = critical damping.
    Real dampingRatio = 0;

    // Solver shared
    Momentum impulse = 0_Ns; ///< Impulse.

    // Solver temp
    InvMass invGamma = {}; ///< Inverse gamma.
    LinearVelocity bias = {}; ///< Bias.
    Mass mass = 0_kg; ///< Mass.
    UnitVec u; ///< "u" directional.
    Length2 rA = {}; ///< Relative A position.
    Length2 rB = {}; ///< Relative B position.
};

/// @brief Gets the definition data for the given joint.
/// @relatedalso Joint
DistanceJointConf GetDistanceJointConf(const Joint& joint) noexcept;

/// @relatedalso World
DistanceJointConf GetDistanceJointConf(const World& world, BodyID bodyA, BodyID bodyB,
                                       Length2 anchorA = Length2{}, Length2 anchorB = Length2{});

/// @relatedalso DistanceJointConf
constexpr Momentum2 GetLinearReaction(const DistanceJointConf& object) noexcept
{
    return object.impulse * object.u;
}

/// @relatedalso DistanceJointConf
constexpr AngularMomentum GetAngularReaction(const DistanceJointConf&) noexcept
{
    return AngularMomentum{0};
}

/// @relatedalso DistanceJointConf
constexpr bool ShiftOrigin(DistanceJointConf&, Length2) noexcept
{
    return false;
}

/// @brief Initializes velocity constraint data based on the given solver data.
/// @note This MUST be called prior to calling <code>SolveVelocity</code>.
/// @see SolveVelocity.
/// @relatedalso DistanceJointConf
void InitVelocity(DistanceJointConf& object, std::vector<BodyConstraint>& bodies,
                  const StepConf& step,
                  const ConstraintSolverConf& conf);

/// @brief Solves velocity constraint.
/// @pre <code>InitVelocity</code> has been called.
/// @see InitVelocity.
/// @return <code>true</code> if velocity is "solved", <code>false</code> otherwise.
/// @relatedalso DistanceJointConf
bool SolveVelocity(DistanceJointConf& object, std::vector<BodyConstraint>& bodies,
                   const StepConf& step);

/// @brief Solves the position constraint.
/// @return <code>true</code> if the position errors are within tolerance.
/// @relatedalso DistanceJointConf
bool SolvePosition(const DistanceJointConf& object, std::vector<BodyConstraint>& bodies,
                   const ConstraintSolverConf& conf);

/// @relatedalso DistanceJointConf
constexpr void SetFrequency(DistanceJointConf& object, NonNegative<Frequency> value) noexcept
{
    object.UseFrequency(value);
}

/// @relatedalso DistanceJointConf
constexpr void SetDampingRatio(DistanceJointConf& object, Real value) noexcept
{
    object.UseDampingRatio(value);
}

/// @relatedalso DistanceJointConf
constexpr auto GetLength(const DistanceJointConf& object) noexcept
{
    return object.length;
}

/// @relatedalso DistanceJointConf
constexpr auto SetLength(DistanceJointConf& object, Length value) noexcept
{
    object.UseLength(value);
}

} // namespace d2

template <>
struct TypeInfo<d2::DistanceJointConf>
{
    static const char* name() noexcept {
        return "d2::DistanceJointConf";
    }
};

} // namespace playrho

#endif // PLAYRHO_DYNAMICS_JOINTS_DISTANCEJOINTCONF_HPP
