// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "WeaponDataTypes.h"
#include "Components/SphereComponent.h"
#include "Project_NebulaCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AProject_NebulaCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Light Attack Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LightAttackAction;

	/** Dodge/Crouch Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DodgeCrouchAction;

public:
	AProject_NebulaCharacter();
	

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Called for light attack input */
	void LightAttack(const FInputActionValue& Value);

	/** Called for context-sensitive dodge or crouch */
	void DodgeOrCrouch(const FInputActionValue& Value);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations")
	UAnimMontage* DodgeMontage;
			

protected:

	virtual void NotifyControllerChanged() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	// -------------------------------------------------------------------
	// WEAPON SYSTEM
	// -------------------------------------------------------------------

	// The physical 3D model attached to your hand
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nebula Combat|Equipment")
	UStaticMeshComponent* EquippedWeaponMesh;

	// Slot to slot in your DT_WeaponList in the Blueprint Editor
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Nebula Combat|Equipment")
	UDataTable* WeaponDataTable;

	// Stores the currently active stats (Damage, Technique, etc.)
	UPROPERTY(BlueprintReadOnly, Category = "Nebula Combat|Equipment")
	FWeaponInfo CurrentWeaponInfo;

	// The invisible bubble that detects nearby interactables
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nebula Interaction")
	USphereComponent* InteractionSphere;

	// Stores the object we are currently standing next to
	UPROPERTY(BlueprintReadWrite, Category = "Nebula Interaction")
	AActor* CurrentInteractable;

	// The function your UI will call to swap weapons
	UFUNCTION(BlueprintCallable, Category = "Nebula Combat|Methods")
	void EquipWeaponFromRow(FName WeaponRowName);



	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	// I-frame state flag
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsInvincible = false;
};

