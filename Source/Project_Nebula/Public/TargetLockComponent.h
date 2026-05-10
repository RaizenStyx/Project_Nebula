// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TargetLockComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_NEBULA_API UTargetLockComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTargetLockComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void ToggleTargetLock();

protected:

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Target Lock")
	AActor* CurrentTarget;

	UPROPERTY(EditDefaultsOnly, Category = "Target Lock")
	float MaxLockDistance = 2000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Target Lock")
	float LockSweepRadius = 800.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Target Lock")
	float CameraInterpSpeed = 5.0f;

	// The visual reticle widget (optional but recommended)
	UPROPERTY(EditDefaultsOnly, Category = "Target Lock")
	TSubclassOf<UUserWidget> TargetReticleClass;
	class UUserWidget* ReticleWidget;

public:	
	class AProject_NebulaCharacter* OwnerCharacter;
	class APlayerController* PlayerController;

	void FindBestTarget();
	void BreakLock();
	void UpdateCameraLook(float DeltaTime);

	// Delegate binding for when the target dies
	UFUNCTION()
	void OnTargetDestroyed(AActor* DestroyedActor);

	// Pass +1.0 for Right, -1.0 for Left
	void SwitchTarget(float DirectionAxis);
		
};
