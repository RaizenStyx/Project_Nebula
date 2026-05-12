// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GI_Nebula.generated.h"

UENUM(BlueprintType)
enum class ENebulaEssence : uint8
{
    None            UMETA(DisplayName = "None"),
    Fighting        UMETA(DisplayName = "Æsir’s Eye (Fighting)"),
    Evasive         UMETA(DisplayName = "Void’s Spirit (Evasive)"),
    Survivability   UMETA(DisplayName = "Titan’s Heart (Survivability)")
};

UENUM(BlueprintType)
enum class ENebulaOrigin : uint8
{
    None            UMETA(DisplayName = "None"),
    Styx            UMETA(DisplayName = "Styx (Fighting Origin)"),
    Kitsune         UMETA(DisplayName = "Kitsune (Evasive Origin)"),
    Titan           UMETA(DisplayName = "Titan (Survivability Origin)")
};

/**
 * 
 */
UCLASS()
class PROJECT_NEBULA_API UGI_Nebula : public UGameInstance
{
	GENERATED_BODY()

public:

    // The choices stored from the Main Menu UI
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Startup Choices")
    ENebulaEssence SelectedEssence = ENebulaEssence::None;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Startup Choices")
    ENebulaOrigin SelectedOrigin = ENebulaOrigin::None;

    // A simple function to clear choices if the player goes back
    UFUNCTION(BlueprintCallable, Category = "Startup Choices")
    void ClearStartupChoices()
    {
        SelectedEssence = ENebulaEssence::None;
        SelectedOrigin = ENebulaOrigin::None;
    }
	
};
