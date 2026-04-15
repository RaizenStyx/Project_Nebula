#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../NebulaSkillBase.h"
#include "SkillManagerComponent.generated.h"

// Maps directly to your GDD Section 3 Action Palette
UENUM(BlueprintType)
enum class ENebulaSkillSlot : uint8
{
    None        UMETA(DisplayName = "None"),
    DPad_Up     UMETA(DisplayName = "D-Pad Up (Set A)"),
    DPad_Down   UMETA(DisplayName = "D-Pad Down (Set A)"),
    DPad_Left   UMETA(DisplayName = "D-Pad Left (Set A)"),
    DPad_Right  UMETA(DisplayName = "D-Pad Right (Set A)"),
    Face_Top    UMETA(DisplayName = "Face Top (Set B)"),
    Face_Bottom UMETA(DisplayName = "Face Bottom (Set B)"),
    Face_Left   UMETA(DisplayName = "Face Left (Set B)"),
    Face_Right  UMETA(DisplayName = "Face Right (Set B)"),
    Class_L2    UMETA(DisplayName = "Class Active L2"),
    Class_R2    UMETA(DisplayName = "Class Active R2")
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT_NEBULA_API USkillManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USkillManagerComponent();

    // -------------------------------------------------------------------
    // ACTIVE SKILLS (The Cross-Hotbar)
    // -------------------------------------------------------------------

    // Stores instantiated skill objects mapped to their inputs
    UPROPERTY(VisibleAnywhere, Instanced, BlueprintReadOnly, Category = "Nebula Skills|Actives")
    TMap<ENebulaSkillSlot, UNebulaSkillBase*> ActiveSkillSlots;

    // Equips a skill to a slot. Returns false if the slot is already full!
    UFUNCTION(BlueprintCallable, Category = "Nebula Skills|Actives")
    bool EquipActiveSkill(TSubclassOf<UNebulaSkillBase> SkillClass, ENebulaSkillSlot Slot);

    // Clears a slot so the player can equip a new one
    UFUNCTION(BlueprintCallable, Category = "Nebula Skills|Actives")
    void ForgetActiveSkill(ENebulaSkillSlot Slot);

    // Swaps two slots for free UI arrangement
    UFUNCTION(BlueprintCallable, Category = "Nebula Skills|Actives")
    void SwapActiveSlots(ENebulaSkillSlot SlotA, ENebulaSkillSlot SlotB);

    // The function your Character Input will call to fire the spell
    UFUNCTION(BlueprintCallable, Category = "Nebula Skills|Execution")
    void ExecuteSkillInSlot(ENebulaSkillSlot Slot, bool bIsHold);

    // -------------------------------------------------------------------
    // PASSIVE SKILLS (The Unified Pool)
    // -------------------------------------------------------------------

    // Hard cap of 10 equipped passives per GDD
    UPROPERTY(VisibleAnywhere, Instanced, BlueprintReadOnly, Category = "Nebula Skills|Passives")
    TArray<UNebulaSkillBase*> EquippedPassives;

    // The library of passives the player has found but isn't using
    UPROPERTY(VisibleAnywhere, Instanced, BlueprintReadOnly, Category = "Nebula Skills|Passives")
    TArray<UNebulaSkillBase*> UnlockedPassives;

    UFUNCTION(BlueprintCallable, Category = "Nebula Skills|Passives")
    bool EquipPassiveSkill(TSubclassOf<UNebulaSkillBase> PassiveClass);

    UFUNCTION(BlueprintCallable, Category = "Nebula Skills|Passives")
    void UnequipPassiveSkill(int32 EquippedIndex);
};