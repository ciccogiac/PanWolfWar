#include "Interfaces/CombatInterface.h"

void ICombatInterface::ShortStunned()
{
}

void ICombatInterface::LongStunned()
{
}

bool ICombatInterface::IsStunned()
{
    return false;
}

void ICombatInterface::Block()
{
}

void ICombatInterface::UnBlock()
{
}

bool ICombatInterface::IsBlockingAttackRecently()
{
    return false;
}

bool ICombatInterface::IsBlockingCharged()
{
    return false;
}

void ICombatInterface::FireProjectile()
{
}

void ICombatInterface::AssassinationKilled()
{
}

void ICombatInterface::CancelAttack()
{
}

void ICombatInterface::AttackWarning()
{
}
