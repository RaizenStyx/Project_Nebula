// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "WeaponDataTypes.h"
#include "Public/NebulaItemTypes.h"
#include "Components/SphereComponent.h"
#include "Project_NebulaCharacter.generated.h"

class USkillManagerComponent;
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

	// In your class definition:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	USkillManagerComponent* SkillManager;

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


	// --- INPUT ACTIONS ---
	// Left bumper
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* HotbarModifierAction; 

	// New RB Modifier Action (Class Skills)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ClassHotbarModifierAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* SlotFaceTopAction; // Y / Triangle

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* SlotFaceLeftAction; // X / Square

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* SlotFaceRightAction; // B / Circle

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* SlotFaceBottomAction; // A / Cross

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* SlotDPadUpAction; // Up on D-Pad

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* SlotDPadDownAction; // Down on D-Pad

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* SlotDPadLeftAction; // Left on D-Pad

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* SlotDPadRightAction; // Right on D-Pad

	// Tracks which bumper is currently being held
    UPROPERTY(BlueprintReadWrite, Category = "Nebula Skills")
    ENebulaSkillCategory CurrentHotbarCategory = ENebulaSkillCategory::Normal;

    // --- Modifier Inputs ---
    // Bind these to your LB (Normal Skills) Input Action Triggered/Completed
    void Input_LB_Started();
    void Input_LB_Completed();

    // Bind these to your RB (Class Skills) Input Action Triggered/Completed
    void Input_RB_Started();
    void Input_RB_Completed();

    // Make sure your Blueprint event can accept the category so the UI knows which skills to draw!
    UFUNCTION(BlueprintImplementableEvent, Category = "Nebula Skills")
    void OnToggleCrossHotbar(bool bShow, ENebulaSkillCategory Category);

	// We pass in boolean parameters to determine if it was a Tap or a Hold
	void Input_FaceTop_Tap();
	void Input_FaceTop_Hold();

	void Input_FaceLeft_Tap();
	void Input_FaceLeft_Hold();

	void Input_FaceBottom_Tap();
	void Input_FaceBottom_Hold();

	void Input_FaceRight_Tap();
	void Input_FaceRight_Hold();

	// D-Pad Functions
	void Input_DPadUp_Tap(); 
	void Input_DPadUp_Hold();

	void Input_DPadDown_Tap();
	void Input_DPadDown_Hold();

	void Input_DPadLeft_Tap();
	void Input_DPadLeft_Hold();

	void Input_DPadRight_Tap();
	void Input_DPadRight_Hold();


			

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

	// Tracks the ID of the currently equipped weapon (matches the Data Table Row Name)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
	FName EquippedWeaponItemID = NAME_None;

	// --- Equipment Visual Components ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment Visuals")
	TObjectPtr<UStaticMeshComponent> HeadMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment Visuals")
	TObjectPtr<UStaticMeshComponent> ChestMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment Visuals")
	TObjectPtr<UStaticMeshComponent> ArmRMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment Visuals")
	TObjectPtr<UStaticMeshComponent> ArmLMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment Visuals")
	TObjectPtr<UStaticMeshComponent> LegsMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment Visuals")
	TObjectPtr<UStaticMeshComponent> FeetMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment Visuals")
	TObjectPtr<UStaticMeshComponent> WeaponRMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment Visuals")
	TObjectPtr<UStaticMeshComponent> WeaponLMesh;

	// --- The Universal Update Function ---
	// Replaces EquipWeaponFromRow
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void UpdateEquipmentVisuals(EEquipmentSlot Slot, FName ItemRowName);

	// Helper to unequip the current weapon
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void UnequipWeapon();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	// I-frame state flag
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsInvincible = false;

	UFUNCTION()
	void UseStudyBook();
};

