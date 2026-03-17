#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "WeaponDataTypes.generated.h"

UENUM(BlueprintType)
enum class EWeaponArchetype : uint8
{
    None    UMETA(DisplayName = "None"),
    Sword   UMETA(DisplayName = "Sword (Wide/Fighting)"),
    Dagger  UMETA(DisplayName = "Dagger (Quick/Evasive)"),
    Spear   UMETA(DisplayName = "Spear (Long/Survival)")
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
    }
};