#include "SkillManagerComponent.h"
#include "../Project_NebulaCharacter.h"

USkillManagerComponent::USkillManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

bool USkillManagerComponent::EquipActiveSkill(TSubclassOf<UNebulaSkillBase> SkillClass, ENebulaSkillSlot Slot)
{
    if (!SkillClass || Slot == ENebulaSkillSlot::None) return false;

    // Per your design, they must forget a skill before overwriting it.
    if (ActiveSkillSlots.Contains(Slot))
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("Slot is full! You must forget the old skill first."));
        return false;
    }

    // Instantiate the skill object so it can hold its own variables/cooldowns
    UNebulaSkillBase* NewSkill = NewObject<UNebulaSkillBase>(this, SkillClass);
    ActiveSkillSlots.Add(Slot, NewSkill);

    return true;
}

void USkillManagerComponent::ForgetActiveSkill(ENebulaSkillSlot Slot)
{
    if (ActiveSkillSlots.Contains(Slot))
    {
        ActiveSkillSlots.Remove(Slot);
        // Unreal's Garbage Collector will automatically clean up the removed skill object
    }
}

void USkillManagerComponent::SwapActiveSlots(ENebulaSkillSlot SlotA, ENebulaSkillSlot SlotB)
{
    // Grab the skills (if they exist)
    UNebulaSkillBase* TempA = ActiveSkillSlots.Contains(SlotA) ? ActiveSkillSlots[SlotA] : nullptr;
    UNebulaSkillBase* TempB = ActiveSkillSlots.Contains(SlotB) ? ActiveSkillSlots[SlotB] : nullptr;

    // Swap A -> B
    if (TempA) ActiveSkillSlots.Add(SlotB, TempA);
    else ActiveSkillSlots.Remove(SlotB);

    // Swap B -> A
    if (TempB) ActiveSkillSlots.Add(SlotA, TempB);
    else ActiveSkillSlots.Remove(SlotA);
}

void USkillManagerComponent::ExecuteSkillInSlot(ENebulaSkillSlot Slot, bool bIsHold)
{
    if (!ActiveSkillSlots.Contains(Slot)) return; // Empty slot

    UNebulaSkillBase* SkillToExecute = ActiveSkillSlots[Slot];
    if (!SkillToExecute) return;

    AProject_NebulaCharacter* Caster = Cast<AProject_NebulaCharacter>(GetOwner());
    if (Caster)
    {
        if (bIsHold)
        {
            SkillToExecute->ExecuteHoldSkill(Caster);
        }
        else
        {
            SkillToExecute->ExecuteSkill(Caster);
        }
    }
}

bool USkillManagerComponent::EquipPassiveSkill(TSubclassOf<UNebulaSkillBase> PassiveClass)
{
    if (!PassiveClass) return false;

    // Hard cap of 10 equipped passives
    if (EquippedPassives.Num() >= 10) return false;

    UNebulaSkillBase* NewPassive = NewObject<UNebulaSkillBase>(this, PassiveClass);
    EquippedPassives.Add(NewPassive);
    return true;
}

void USkillManagerComponent::UnequipPassiveSkill(int32 EquippedIndex)
{
    if (EquippedPassives.IsValidIndex(EquippedIndex))
    {
        // Add to unlocked inventory, then remove from equipped
        UnlockedPassives.Add(EquippedPassives[EquippedIndex]);
        EquippedPassives.RemoveAt(EquippedIndex);
    }
}