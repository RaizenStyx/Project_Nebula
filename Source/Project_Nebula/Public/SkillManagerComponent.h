#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../NebulaSkillBase.h"
#include "NebulaClassTemplate.h"
#include "NebulaEssenceTemplate.h"
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
    // 1. NORMAL SKILL POOLS
    // Rule: Actives are forgotten if overwritten. Passives can be swapped.
    // -------------------------------------------------------------------

    // The equipped Normal Actives. Notice there is NO "Unlocked" array for Normal Actives per your rules!
    UPROPERTY(VisibleAnywhere, Instanced, BlueprintReadOnly, Category = "Nebula Skills|Normal")
    TMap<ENebulaSkillSlot, UNebulaSkillBase*> EquippedNormalActives;

    // Normal Passives (can be swapped at will)
    UPROPERTY(VisibleAnywhere, Instanced, BlueprintReadOnly, Category = "Nebula Skills|Normal")
    TArray<UNebulaSkillBase*> EquippedNormalPassives;

    UPROPERTY(VisibleAnywhere, Instanced, BlueprintReadOnly, Category = "Nebula Skills|Normal")
    TArray<UNebulaSkillBase*> UnlockedNormalPassives;

    UFUNCTION(BlueprintCallable, Category = "Nebula Skills|Actives")
    bool EquipActiveSkill(TSubclassOf<UNebulaSkillBase> SkillClass, ENebulaSkillSlot Slot);

    UFUNCTION(BlueprintCallable, Category = "Nebula Skills|Actives")
    void ForgetActiveSkill(ENebulaSkillCategory Category, ENebulaSkillSlot Slot);

    UFUNCTION(BlueprintCallable, Category = "Nebula Skills|Actives")
    void SwapActiveSlots(ENebulaSkillCategory Category, ENebulaSkillSlot SlotA, ENebulaSkillSlot SlotB);

    UFUNCTION(BlueprintCallable, Category = "Nebula Skills|Execution")
    void ExecuteSkillInSlot(ENebulaSkillCategory Category, ENebulaSkillSlot Slot, bool bIsHold);

    // Subject to remove
    UFUNCTION(BlueprintCallable, Category = "Nebula Skills|Passives")
    bool EquipPassiveSkill(TSubclassOf<UNebulaSkillBase> PassiveClass);

    // Subject to remove
    UFUNCTION(BlueprintCallable, Category = "Nebula Skills|Passives")
    void UnequipPassiveSkill(ENebulaSkillCategory Category, int32 EquippedIndex);

    // Safely overwrites a slot without changing the array size
    UFUNCTION(BlueprintCallable, Category = "Nebula Skills|Passives")
    bool OverwritePassiveSlot(ENebulaSkillCategory Category, int32 SlotIndex, TSubclassOf<UNebulaSkillBase> NewPassiveClass);

    // -------------------------------------------------------------------
    // 2. CLASS SKILL POOLS
    // Rule: Actives unlocked via leveling are kept. Item-based Class actives are forgotten if overwritten.
    // -------------------------------------------------------------------

    UPROPERTY(VisibleAnywhere, Instanced, BlueprintReadOnly, Category = "Nebula Skills|Class")
    TMap<ENebulaSkillSlot, UNebulaSkillBase*> EquippedClassActives;

    // Only holds Class Actives unlocked naturally via leveling (so they aren't forgotten)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nebula Skills|Class")
    TArray<TSubclassOf<UNebulaSkillBase>> UnlockedClassActives;

    // Class Passives
    UPROPERTY(VisibleAnywhere, Instanced, BlueprintReadOnly, Category = "Nebula Skills|Class")
    TArray<UNebulaSkillBase*> EquippedClassPassives;

    UPROPERTY(VisibleAnywhere, Instanced, BlueprintReadOnly, Category = "Nebula Skills|Class")
    TArray<UNebulaSkillBase*> UnlockedClassPassives;

    // The currently equipped Class Template
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nebula Progression")
    UNebulaClassTemplate* CurrentClass;

    // Current level of the equipped class
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nebula Progression")
    int32 CurrentClassLevel = 1;

    // Safely swaps the player's class, clears old skills, and grants the new ones
    UFUNCTION(BlueprintCallable, Category = "Nebula Skills|Progression")
    void EquipNewClass(UNebulaClassTemplate* NewClassTemplate, int32 StartingLevel = 1);

    // -------------------------------------------------------------------
    // 3. ESSENCE SKILL POOLS
    // Rule: 4 Actives, 1 Permanent Passive (No slots needed)
    // -------------------------------------------------------------------

    UPROPERTY(VisibleAnywhere, Instanced, BlueprintReadOnly, Category = "Nebula Skills|Essence")
    TMap<ENebulaSkillSlot, UNebulaSkillBase*> EquippedEssenceActives;

    // The single, permanently active strong passive from the Essence
    UPROPERTY(VisibleAnywhere, Instanced, BlueprintReadOnly, Category = "Nebula Skills|Essence")
    UNebulaSkillBase* ActiveEssencePassive;

    // The currently equipped Essence Template
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nebula Progression")
    UNebulaEssenceTemplate* CurrentEssence;

    // Checks the Class Template's unlock schedules against the current level
    UFUNCTION(BlueprintCallable, Category = "Nebula Progression")
    void EvaluateLevelUpUnlocks();

    // Attempts to learn a skill from an item. Returns true if successful, false if restricted or slots are full.
    UFUNCTION(BlueprintCallable, Category = "Nebula Skills|Learning")
    bool LearnSkillFromItem(TSubclassOf<UNebulaSkillBase> SkillToLearn, const TArray<UNebulaClassTemplate*>& AllowedClasses);

    // -------------------------------------------------------------------
    // UI HELPER FUNCTIONS
    // -------------------------------------------------------------------

    UFUNCTION(BlueprintPure, Category = "Nebula Skills|UI")
    TArray<TSubclassOf<UNebulaSkillBase>> GetUnlockedClassActives() const { return UnlockedClassActives; }

    UFUNCTION(BlueprintPure, Category = "Nebula Skills|UI")
    TArray<UNebulaSkillBase*> GetUnlockedClassPassives() const { return UnlockedClassPassives; }

    UFUNCTION(BlueprintPure, Category = "Nebula Skills|UI")
    TMap<ENebulaSkillSlot, UNebulaSkillBase*> GetEquippedClassActives() const { return EquippedClassActives; }

    UFUNCTION(BlueprintPure, Category = "Nebula Skills|UI")
    TArray<UNebulaSkillBase*> GetEquippedClassPassives() const { return EquippedClassPassives; }

    // Checks if an Active Skill is currently equipped in ANY slot
    UFUNCTION(BlueprintPure, Category = "Nebula Skills|UI")
    bool IsClassActiveEquipped(TSubclassOf<UNebulaSkillBase> SkillToCheck) const;

    // Checks if a Passive Skill is currently equipped in ANY slot
    UFUNCTION(BlueprintPure, Category = "Nebula Skills|UI")
    bool IsClassPassiveEquipped(TSubclassOf<UNebulaSkillBase> SkillToCheck) const;

    // -------------------------------------------------------------------
    // DEBUG / TESTING
    // -------------------------------------------------------------------

    // Use this to instantly level up and test if your UI populates!
    UFUNCTION(BlueprintCallable, Category = "Nebula Skills|Debug")
    void Debug_AddClassLevels(int32 LevelsToAdd);

private:
    // Helper to automatically find an empty face-button slot and equip the skill
    bool AutoEquipNewSkill(UNebulaSkillBase* NewSkill, TMap<ENebulaSkillSlot, UNebulaSkillBase*>& TargetMap);
};