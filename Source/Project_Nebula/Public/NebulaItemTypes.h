#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "../NebulaSkillBase.h" 
#include "NebulaItemTypes.generated.h"

// -------------------------------------------------------------------
// 1. THE ENUM (Item Categories)
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

};

// -------------------------------------------------------------------
// 3. THE INVENTORY SLOT STRUCT (For the Player's Pockets)
// -------------------------------------------------------------------
USTRUCT(BlueprintType)
struct PROJECT_NEBULA_API FNebulaInventorySlot
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory Slot")
    FName ItemID = NAME_None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory Slot")
    int32 Quantity = 0;

    // Helper functions inside the struct make our Component code much cleaner!
    bool IsEmpty() const { return ItemID == NAME_None || Quantity <= 0; }
    void Clear() { ItemID = NAME_None; Quantity = 0; }
};