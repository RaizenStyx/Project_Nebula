#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "../NebulaSkillBase.h" 
#include "NebulaItemTypes.generated.h"

// -------------------------------------------------------------------
// 1. THE ENUMS (Item Categories, Equipment Slots)
// -------------------------------------------------------------------
UENUM(BlueprintType)
enum class ENebulaItemType : uint8
{
    None        UMETA(DisplayName = "None"),
    Resource    UMETA(DisplayName = "Resource"),
    Consumable  UMETA(DisplayName = "Consumable"),
    Deployable  UMETA(DisplayName = "Deployable"),
    Equipment   UMETA(DisplayName = "Equipment"),
    Quest       UMETA(DisplayName = "Quest Item")
};

UENUM(BlueprintType)
enum class EEquipmentSlot : uint8
{
    None        UMETA(DisplayName = "None"),
    Head        UMETA(DisplayName = "Head"),
    Chest       UMETA(DisplayName = "Chest"),
    ArmR        UMETA(DisplayName = "Right Arm/Shoulder"),
    ArmL        UMETA(DisplayName = "Left Arm/Shoulder"),
    Legs        UMETA(DisplayName = "Legs"),
    Feet        UMETA(DisplayName = "Feet"),
    WeaponR     UMETA(DisplayName = "Right Hand (Main Weapon)"),
    WeaponL     UMETA(DisplayName = "Left Hand (Shield/Focus)")
};


// -------------------------------------------------------------------
// 1.1 THE ENUMS For class system
// -------------------------------------------------------------------
UENUM(BlueprintType)
enum class ENebulaStarRank : uint8
{
    OneStar     UMETA(DisplayName = "1-Star"),
    TwoStar     UMETA(DisplayName = "2-Star"),
    ThreeStar   UMETA(DisplayName = "3-Star Pinnacle")
};

UENUM(BlueprintType)
enum class ENebulaResourceType : uint8
{
    PurePhysical UMETA(DisplayName = "Pure Physical"),
    PureMana     UMETA(DisplayName = "Pure Mana"),
    Hybrid       UMETA(DisplayName = "Hybrid")
};

// -------------------------------------------------------------------
// 2. THE MASTER DATA STRUCT (For the Data Table)
// -------------------------------------------------------------------
USTRUCT(BlueprintType)
struct PROJECT_NEBULA_API FNebulaItemData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    FName ItemID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    ENebulaItemType ItemType = ENebulaItemType::None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|General")
    TObjectPtr<UTexture2D> ItemIcon; 

    // --- Inventory Management ---
    // How many can fit in a single slot?
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Inventory", meta = (ClampMin = "1"))
    int32 MaxStackSize = 99;

    // --- Usage: Deployables ---
    // The pre-fab Actor to spawn (e.g., a Tent or House)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Usage")
    TSubclassOf<AActor> DeployablePrefab;

    // --- Usage: Skill Orbs ---
    // If set, consuming this item injects this skill into the USkillManagerComponent
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Usage")
    TSubclassOf<UNebulaSkillBase> GrantedSkill;

    // --- Usage: Consumable Stats ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Usage")
    float HealthRestoreAmount = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Usage")
    float StaminaRestoreAmount = 0.f;

    // --- Usage: Cooldowns ---
    // e.g., "Potion_Health", "Food", "Potion_Stamina". Leave as "None" for no cooldown.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Usage")
    FName CooldownTag = NAME_None;

    // How long (in seconds) the cooldown lasts
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Usage")
    float CooldownDuration = 0.f;

    // --- Usage: Equipment ---
    // If this item is Equipment, which body part does it attach to?
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Usage")
    EEquipmentSlot EquipSlot = EEquipmentSlot::None;

    // If empty, ANY class can learn it (Normal Skill). 
    // If populated, ONLY the classes in this list can learn it (Class Skill).
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Skills")
    TArray<class UNebulaClassTemplate*> AllowedClasses;

    // For items that grant classes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Skills")
    UNebulaClassTemplate* GrantedClass;

};

// -------------------------------------------------------------------
// 3. THE INVENTORY SLOT STRUCT (For the Player's Pockets)
// -------------------------------------------------------------------
USTRUCT(BlueprintType)
struct PROJECT_NEBULA_API FNebulaInventorySlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Slot")
    FName ItemID = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Slot")
    int32 Quantity = 0;

    // Helper functions inside the struct make our Component code much cleaner!
    bool IsEmpty() const { return ItemID == NAME_None || Quantity <= 0; }
    void Clear() { ItemID = NAME_None; Quantity = 0; }
};

// -------------------------------------------------------------------
// 4. THE CRAFTING RECIPE STRUCT
// -------------------------------------------------------------------
USTRUCT(BlueprintType)
struct PROJECT_NEBULA_API FCraftingRecipe : public FTableRowBase
{
    GENERATED_BODY()

    // The item you actually get when you craft this (e.g., "Tent")
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    FName ResultItemID = NAME_None;

    // How many you get (usually 1, but maybe 5 if crafting Arrows)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe", meta = (ClampMin = "1"))
    int32 ResultQuantity = 1;

    // The list of required items. We reuse our trusty Inventory Slot struct here!
    // Example: [0] ItemID: BasicWood, Quantity: 10
    //          [1] ItemID: Cloth, Quantity: 5
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    TArray<FNebulaInventorySlot> RequiredIngredients;
};

// -------------------------------------------------------------------
// 5. THE SKILL UNLOCK STRUCT
// -------------------------------------------------------------------

USTRUCT(BlueprintType)
struct PROJECT_NEBULA_API FSkillUnlockNode
{
    GENERATED_BODY()

    // The class level required to unlock this skill
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlock Schedule")
    int32 RequiredLevel = 1;

    // The specific skill granted at this level
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlock Schedule")
    TSubclassOf<UNebulaSkillBase> SkillToUnlock;
};