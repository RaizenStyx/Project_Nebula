#include "SkillManagerComponent.h"
#include "../Project_NebulaCharacter.h"
#include "NebulaClassTemplate.h"

USkillManagerComponent::USkillManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

bool USkillManagerComponent::EquipActiveSkill(TSubclassOf<UNebulaSkillBase> SkillClass, ENebulaSkillSlot Slot)
{
    if (!SkillClass || Slot == ENebulaSkillSlot::None) return false;

    // 1. Get the default object to read what category this skill is BEFORE we instantiate it
    UNebulaSkillBase* DefaultSkill = SkillClass->GetDefaultObject<UNebulaSkillBase>();
    if (!DefaultSkill) return false;

    ENebulaSkillCategory Category = DefaultSkill->SkillCategory;

    // 2. Instantiate the new skill object
    UNebulaSkillBase* NewSkill = NewObject<UNebulaSkillBase>(this, SkillClass);

    // 3. Route it to the correct pool
    // Note: TMap::Add() automatically replaces the old value if the key (Slot) already exists.
    // If the old skill isn't saved in an "Unlocked" array, Unreal's Garbage Collector deletes it naturally!
    switch (Category)
    {
    case ENebulaSkillCategory::Normal:
        EquippedNormalActives.Add(Slot, NewSkill);
        break;

    case ENebulaSkillCategory::Class:
        EquippedClassActives.Add(Slot, NewSkill);
        break;

    case ENebulaSkillCategory::Essence:
        EquippedEssenceActives.Add(Slot, NewSkill);
        break;
    }

    return true;
}

void USkillManagerComponent::ForgetActiveSkill(ENebulaSkillCategory Category, ENebulaSkillSlot Slot)
{
    switch (Category)
    {
    case ENebulaSkillCategory::Normal:
        EquippedNormalActives.Remove(Slot);
        break;
    case ENebulaSkillCategory::Class:
        EquippedClassActives.Remove(Slot);
        break;
    case ENebulaSkillCategory::Essence:
        EquippedEssenceActives.Remove(Slot);
        break;
    }
}

void USkillManagerComponent::SwapActiveSlots(ENebulaSkillCategory Category, ENebulaSkillSlot SlotA, ENebulaSkillSlot SlotB)
{
    // A helpful lambda function to handle swapping inside any map
    auto SwapInMap = [](TMap<ENebulaSkillSlot, UNebulaSkillBase*>& Map, ENebulaSkillSlot A, ENebulaSkillSlot B)
        {
            UNebulaSkillBase* TempA = Map.Contains(A) ? Map[A] : nullptr;
            UNebulaSkillBase* TempB = Map.Contains(B) ? Map[B] : nullptr;

            if (TempA) Map.Add(B, TempA); else Map.Remove(B);
            if (TempB) Map.Add(A, TempB); else Map.Remove(A);
        };

    switch (Category)
    {
    case ENebulaSkillCategory::Normal:
        SwapInMap(EquippedNormalActives, SlotA, SlotB);
        break;
    case ENebulaSkillCategory::Class:
        SwapInMap(EquippedClassActives, SlotA, SlotB);
        break;
    case ENebulaSkillCategory::Essence:
        SwapInMap(EquippedEssenceActives, SlotA, SlotB);
        break;
    }
}

void USkillManagerComponent::ExecuteSkillInSlot(ENebulaSkillCategory Category, ENebulaSkillSlot Slot, bool bIsHold)
{
    UNebulaSkillBase* SkillToExecute = nullptr;

    // Grab the correct skill based on the active modifier state (Category)
    switch (Category)
    {
    case ENebulaSkillCategory::Normal:
        if (EquippedNormalActives.Contains(Slot)) SkillToExecute = EquippedNormalActives[Slot];
        break;
    case ENebulaSkillCategory::Class:
        if (EquippedClassActives.Contains(Slot)) SkillToExecute = EquippedClassActives[Slot];
        break;
    case ENebulaSkillCategory::Essence:
        if (EquippedEssenceActives.Contains(Slot)) SkillToExecute = EquippedEssenceActives[Slot];
        break;
    }

    if (!SkillToExecute) return; // Slot was empty

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

    UNebulaSkillBase* DefaultSkill = PassiveClass->GetDefaultObject<UNebulaSkillBase>();
    if (!DefaultSkill) return false;

    // Instantiate it
    UNebulaSkillBase* NewPassive = NewObject<UNebulaSkillBase>(this, PassiveClass);

    switch (DefaultSkill->SkillCategory)
    {
    case ENebulaSkillCategory::Normal:
        if (EquippedNormalPassives.Num() >= 10) return false; // Optional cap check
        EquippedNormalPassives.Add(NewPassive);
        break;

    case ENebulaSkillCategory::Class:
        // Could check CurrentClass->MaxPassiveSkillSlots here!
        EquippedClassPassives.Add(NewPassive);
        break;

    case ENebulaSkillCategory::Essence:
        // Essences only have 1 Strong Passive, no arrays needed
        ActiveEssencePassive = NewPassive;
        break;
    }

    return true;
}

void USkillManagerComponent::UnequipPassiveSkill(ENebulaSkillCategory Category, int32 EquippedIndex)
{
    switch (Category)
    {
    case ENebulaSkillCategory::Normal:
        if (EquippedNormalPassives.IsValidIndex(EquippedIndex))
        {
            UnlockedNormalPassives.Add(EquippedNormalPassives[EquippedIndex]);
            EquippedNormalPassives.RemoveAt(EquippedIndex);
        }
        break;

    case ENebulaSkillCategory::Class:
        if (EquippedClassPassives.IsValidIndex(EquippedIndex))
        {
            UnlockedClassPassives.Add(EquippedClassPassives[EquippedIndex]);
            EquippedClassPassives.RemoveAt(EquippedIndex);
        }
        break;

    case ENebulaSkillCategory::Essence:
        // You can't unequip the permanent essence passive
        break;
    }
}

void USkillManagerComponent::EvaluateLevelUpUnlocks()
{
    if (!CurrentClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot evaluate unlocks: No Class Equipped!"));
        return;
    }

    // 1. Evaluate CLASS ACTIVE Skills
    for (const FSkillUnlockNode& Node : CurrentClass->ActiveSkillUnlocks)
    {
        if (CurrentClassLevel >= Node.RequiredLevel && Node.SkillToUnlock)
        {
            if (!UnlockedClassActives.Contains(Node.SkillToUnlock))
            {
                UnlockedClassActives.Add(Node.SkillToUnlock);
                UE_LOG(LogTemp, Log, TEXT("Unlocked new Class Active: %s"), *Node.SkillToUnlock->GetName());
            }
        }
    }

    // 2. Evaluate CLASS PASSIVE Skills
    for (const FSkillUnlockNode& Node : CurrentClass->PassiveSkillUnlocks)
    {
        if (CurrentClassLevel >= Node.RequiredLevel && Node.SkillToUnlock)
        {
            bool bAlreadyUnlocked = false;
            for (UNebulaSkillBase* ExistingPassive : UnlockedClassPassives)
            {
                if (ExistingPassive && ExistingPassive->GetClass() == Node.SkillToUnlock)
                {
                    bAlreadyUnlocked = true;
                    break;
                }
            }

            if (!bAlreadyUnlocked)
            {
                UNebulaSkillBase* NewPassive = NewObject<UNebulaSkillBase>(this, Node.SkillToUnlock);
                UnlockedClassPassives.Add(NewPassive);
                UE_LOG(LogTemp, Log, TEXT("Unlocked new Class Passive: %s"), *Node.SkillToUnlock->GetName());
            }
        }
    }

    // 3. Evaluate EX SKILL (The Ultimate)
    if (CurrentClassLevel >= 100 && CurrentClass->EXSkill)
    {
        if (!UnlockedClassActives.Contains(CurrentClass->EXSkill))
        {
            UnlockedClassActives.Add(CurrentClass->EXSkill);
            UE_LOG(LogTemp, Log, TEXT("Unlocked Pinnacle EX Skill!"));
        }
    }
}

bool USkillManagerComponent::LearnSkillFromItem(TSubclassOf<UNebulaSkillBase> SkillToLearn, const TArray<UNebulaClassTemplate*>& AllowedClasses)
{
    if (!SkillToLearn) return false;

    // 1. Check Class Restrictions
    if (AllowedClasses.Num() > 0)
    {
        if (!CurrentClass || !AllowedClasses.Contains(CurrentClass))
        {
            UE_LOG(LogTemp, Warning, TEXT("Cannot learn skill: Class mismatch."));
            return false;
        }
    }

    // 2. Determine if it is Active or Passive BEFORE instantiating
    UNebulaSkillBase* DefaultSkill = SkillToLearn->GetDefaultObject<UNebulaSkillBase>();
    if (!DefaultSkill) return false;

    // 3. Instantiate the skill object
    UNebulaSkillBase* NewSkill = NewObject<UNebulaSkillBase>(this, SkillToLearn);

    // ---------------------------------------------------------
    // PASSIVE SKILL ROUTING
    // ---------------------------------------------------------
    if (DefaultSkill->bIsPassive)
    {
        switch (NewSkill->SkillCategory)
        {
        case ENebulaSkillCategory::Normal:
            if (EquippedNormalPassives.Num() >= 10) return false; // Fail if full
            EquippedNormalPassives.Add(NewSkill);
            return true;

        case ENebulaSkillCategory::Class:
            if (CurrentClass && EquippedClassPassives.Num() >= CurrentClass->MaxPassiveSkillSlots) return false; // Fail if full
            EquippedClassPassives.Add(NewSkill);
            return true;

        case ENebulaSkillCategory::Essence:
            return false; // Essence passives shouldn't be learned via items
        }
    }
    // ---------------------------------------------------------
    // ACTIVE SKILL ROUTING (Cross-Hotbar)
    // ---------------------------------------------------------
    else
    {
        switch (NewSkill->SkillCategory)
        {
        case ENebulaSkillCategory::Normal:
            return AutoEquipNewSkill(NewSkill, EquippedNormalActives);

        case ENebulaSkillCategory::Class:
            return AutoEquipNewSkill(NewSkill, EquippedClassActives);

        case ENebulaSkillCategory::Essence:
            return AutoEquipNewSkill(NewSkill, EquippedEssenceActives);
        }
    }

    return false;
}

bool USkillManagerComponent::AutoEquipNewSkill(UNebulaSkillBase* NewSkill, TMap<ENebulaSkillSlot, UNebulaSkillBase*>& TargetMap)
{
    // Define the exact order we want the game to fill empty slots
    TArray<ENebulaSkillSlot> PreferredOrder = {
        ENebulaSkillSlot::DPad_Up,
        ENebulaSkillSlot::DPad_Right,
        ENebulaSkillSlot::DPad_Down,
        ENebulaSkillSlot::DPad_Left,
        ENebulaSkillSlot::Face_Top,    // Triangle / Y
        ENebulaSkillSlot::Face_Right,  // Circle / B
        ENebulaSkillSlot::Face_Bottom, // Cross / A
        ENebulaSkillSlot::Face_Left    // Square / X
    };

    // Scan through the preferred slots
    for (ENebulaSkillSlot Slot : PreferredOrder)
    {
        // If the map DOES NOT contain this slot yet, it's empty!
        if (!TargetMap.Contains(Slot))
        {
            TargetMap.Add(Slot, NewSkill);
            UE_LOG(LogTemp, Log, TEXT("Auto-equipped skill %s"), *NewSkill->GetName());
            return true; // Successfully equipped
        }
    }

    // If the loop finishes without returning true, all 4 slots are full.
    UE_LOG(LogTemp, Warning, TEXT("All slots full! Player must manually open menu to replace a skill."));

    // Returning false tells the inventory to cancel the item consumption
    return false;
}

void USkillManagerComponent::EquipNewClass(UNebulaClassTemplate* NewClassTemplate, int32 StartingLevel)
{
    // Safety checks
    if (!NewClassTemplate) return;

    // Don't do anything if they are already this class
    if (CurrentClass == NewClassTemplate) return;

    // Optional: If you have Legendary classes that permanently lock, check that here!
    if (CurrentClass && CurrentClass->bIsPermanentlyLocked)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot change class. Current class is permanently locked!"));
        return;
    }

    // 1. Wipe the old Class skills clean
    // We intentionally leave NORMAL and ESSENCE skills alone!
    EquippedClassActives.Empty();
    EquippedClassPassives.Empty();
    UnlockedClassActives.Empty();
    UnlockedClassPassives.Empty();

    // 2. Assign the new Class and Level
    CurrentClass = NewClassTemplate;
    CurrentClassLevel = StartingLevel; // In the future, you could load their saved level for this specific class

    // 3. Immediately evaluate unlocks!
    // This will run through the new template and automatically grant all the baseline skills 
    // for their starting level, so they aren't left with an empty spellbook.
    EvaluateLevelUpUnlocks();

    UE_LOG(LogTemp, Log, TEXT("Successfully changed class to a new template!"));
}


void USkillManagerComponent::Debug_AddClassLevels(int32 LevelsToAdd)
{
    if (!CurrentClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot level up: No class equipped!"));
        return;
    }

    CurrentClassLevel += LevelsToAdd;
    UE_LOG(LogTemp, Log, TEXT("Class Level is now: %d"), CurrentClassLevel);

    // Force the component to check for new skills!
    EvaluateLevelUpUnlocks();
}