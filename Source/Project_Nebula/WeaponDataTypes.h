#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "WeaponDataTypes.generated.h"

UENUM(BlueprintType)
enum class EWeaponArchetype : uint8
{
    None    UMETA(DisplayName = "None"),
    Sword   UMETA(DisplayName = "Sword"),
    Dagger  UMETA(DisplayName = "Dagger"),
    Spear   UMETA(DisplayName = "Spear"),
    Focus   UMETA(DisplayName = "Magic Focus"),
    Shield  UMETA(DisplayName = "Shield")
};

UENUM(BlueprintType)
enum class ETechniqueStyle : uint8
{
    None            UMETA(DisplayName = "None"),
    RiverStyx       UMETA(DisplayName = "River-Styx"),
    StoneBreaker    UMETA(DisplayName = "Stone-Breaker"),
    GaleWind        UMETA(DisplayName = "Gale-Wind"),
    IronForm        UMETA(DisplayName = "Iron-Form")
};

USTRUCT(BlueprintType)
struct PROJECT_NEBULA_API FWeaponInfo : public FTableRowBase
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data")
    FText ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data")
    EWeaponArchetype WeaponType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data")
    float BaseDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data")
    ETechniqueStyle TechniqueStyle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data")
    float MultiplierCondition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data")
    UStaticMesh* VisualMesh;

    // If true, equipping this forces the off-hand to be empty
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data")
    bool bIsTwoHanded;

    FWeaponInfo()
    {
        ItemName = FText::FromString("Empty");
        WeaponType = EWeaponArchetype::None;
        BaseDamage = 0.0f;
        TechniqueStyle = ETechniqueStyle::None;
        // Can remove this Mulipier Condition I think.
        MultiplierCondition = 1.0f;
        Description = FText::FromString("Empty");
		VisualMesh = nullptr;
        bIsTwoHanded = false;
    }
};