// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "WeaponDataTypes.h"
#include "Public/NebulaItemTypes.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "NebulaProjectile.h"
#include "Project_NebulaCharacter.generated.h"

class USkillManagerComponent;
class UPlayerStatsComponent;
class UInventoryComponent;
class UEquipmentComponent;
class UTargetLockComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UENUM(BlueprintType)
enum class EWeaponStance : uint8
{
	Unarmed     UMETA(DisplayName = "Unarmed"),
	Sword1H     UMETA(DisplayName = "1H Sword (Empty/Focus)"),
	SwordShield UMETA(DisplayName = "Sword & Shield"),
	GreatSword UMETA(DisplayName = "2H GreatSword")
};

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

	// Actor Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	USkillManagerComponent* SkillManager;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	UPlayerStatsComponent* PlayerStats;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	UInventoryComponent* InventoryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	UEquipmentComponent* EquipmentComponent;

	// Lock on inputs and functionality are handled in the TargetLockComponent, but we still need a reference to it here to bind the input and call the toggle function
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	UTargetLockComponent* TargetLockComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* TargetSwitchAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Light Attack Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LightAttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* HeavyAttackAction;

	/** Dodge/Crouch Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DodgeCrouchAction;

	// We track the current weapon stance here so we can pass it to the animation blueprint and use it for conditional logic in our attack functions.
	UPROPERTY(BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	EWeaponStance CurrentStance = EWeaponStance::Unarmed;

	// --- INPUT ACTIONS ---
	// Left bumper
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* HotbarModifierAction;

	// New RB Modifier Action (Class Skills)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ClassHotbarModifierAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SlotFaceTopAction; // Y / Triangle

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SlotFaceLeftAction; // X / Square

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SlotFaceRightAction; // B / Circle

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SlotFaceBottomAction; // A / Cross

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SlotDPadUpAction; // Up on D-Pad

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SlotDPadDownAction; // Down on D-Pad

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SlotDPadLeftAction; // Left on D-Pad

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SlotDPadRightAction; // Right on D-Pad

	// Quick Use Item slots
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DPadUpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DPadDownAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DPadLeftAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DPadRightAction;

	/** Target Lock Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* TargetLockAction;

	void Input_ToggleTargetLock();

	void Input_SwitchTarget(const FInputActionValue& Value);

public:
	AProject_NebulaCharacter();

protected:

	// The physical box that will deal damage
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class UBoxComponent* MeleeHitbox;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Called for jump input **/
	void Jump();

	/** Called for context-sensitive dodge or crouch */
	void StartDodgeOrSlide(const FInputActionValue& Value);
	void EndSlide(const FInputActionValue& Value);

	// Helper function that actually stops the slide
	void StopSliding();

	// The animation montage to play when sliding
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations")
	UAnimMontage* SlideMontage;

	// State tracking
	bool bIsSliding = false;

	// Save original movement settings for the physics slide
	float OriginalGroundFriction;
	float OriginalBrakingDeceleration;

	// The timer handle that tracks our slide duration
	FTimerHandle SlideTimerHandle;

	// Base slide time before Agility is factored in (e.g., 0.5 seconds)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement|Slide")
	float BaseMaxSlideTime = 0.5f;

	// How much extra time each point of Agility grants (e.g., 0.02s per point)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement|Slide")
	float AgilityTimeMultiplier = 0.02f;

	UPROPERTY(EditDefaultsOnly, Category = "Animations|Combat")
	UAnimMontage* SwordComboMontage;

	bool bIsAttacking = false;
	bool bSaveAttack = false;
	int32 ComboStep = 1;

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

	void ConsumeHealthItem();
	void ConsumeStaminaItem();
	void ConsumeManaItem();
	void CycleActiveElement();

	// The Input Functions for Light and Heavy Attacks.
	//void Input_PrimaryAction(const FInputActionValue& Value);


	void Input_PrimaryAction_Tap();
	void Input_PrimaryAction_Hold();
	void Input_SecondaryAction_Tap();
	void Input_SecondaryAction_Hold();

	// The Blueprint Events you will implement
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void PerformPrimaryMelee();

	// We removed BlueprintImplementableEvent so we can define this in C++
	UFUNCTION(BlueprintCallable, Category = "Nebula Combat|Magic")
	void PerformPrimaryMagic();

	// Fire rate tracking
	float LastPrimaryFireTime = 0.f;
	float LastSecondaryFireTime = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Magic")
	float FocusFireRate = 0.5f; // Fires twice a second

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Magic")
	float FocusHeavyFireRate = 0.8f; // Slower fire rate for 2H heavy magic

	// The projectile to spawn for the base magic attack
	UPROPERTY(EditDefaultsOnly, Category = "Nebula Combat|Magic")
	TSubclassOf<ANebulaProjectile> PrimaryMagicProjectileClass;

	// The animation to play when casting
	UPROPERTY(EditDefaultsOnly, Category = "Nebula Combat|Magic")
	UAnimMontage* PrimaryMagicMontage;

	// --- ADD SECONDARY MAGIC PROPERTIES ---
	UPROPERTY(EditDefaultsOnly, Category = "Nebula Combat|Magic")
	TSubclassOf<class ANebulaProjectile> SecondaryMagicProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Nebula Combat|Magic")
	UAnimMontage* SecondaryMagicMontage;

	// --- UPDATE OUR COMBAT FUNCTIONS ---
	// Make sure we have our magic execution function
	UFUNCTION(BlueprintCallable, Category = "Nebula Combat|Magic")
	void PerformSecondaryMagic();

	// Replace PerformSecondaryOffhand with this specifically for Shield/Melee
	UFUNCTION(BlueprintImplementableEvent, Category = "Nebula Combat")
	void PerformSecondaryMelee();

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void PerformSecondaryHeavy(); // For 2H Heavy/Charge attacks

protected:
	virtual void BeginPlay() override;

	virtual void NotifyControllerChanged() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Safely totals up all armor values and applies them to the Stats Component
	void RecalculateArmorStats();

	// The function that runs when the box hits something
	UFUNCTION()
	void OnMeleeOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	// Start light attack combo.
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ExecuteLightAttack();

	// BlueprintCallable so Anim Notifies can trigger these. Combat combos
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ContinueCombo();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ResetCombo();

	// Call this whenever equipment changes to update the animation stance
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void DetermineWeaponStance();

	// Triggered by Animation Notifies
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void EnableHitbox();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void DisableHitbox();

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
	FWeaponInfo CurrentWeaponInfo; // (Right Hand or 2H)

	// --- ADD THE OFFHAND TRACKER ---
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Nebula Combat|Equipment")
	FWeaponInfo OffhandWeaponInfo; // (Left Hand)

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

	// Slot to assign your DT_ArmorList in the Blueprint Editor
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Nebula Combat|Equipment")
	UDataTable* ArmorDataTable;

	// --- Equipped Armor Tracking ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nebula Combat|Equipment")
	FNebulaArmorData HeadArmorInfo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nebula Combat|Equipment")
	FNebulaArmorData ChestArmorInfo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nebula Combat|Equipment")
	FNebulaArmorData ArmRArmorInfo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nebula Combat|Equipment")
	FNebulaArmorData ArmLArmorInfo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nebula Combat|Equipment")
	FNebulaArmorData LegsArmorInfo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nebula Combat|Equipment")
	FNebulaArmorData FeetArmorInfo;

	// --- The Universal Update Function ---
	// Replaces EquipWeaponFromRow
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void UpdateEquipmentVisuals(EEquipmentSlot Slot, FName ItemRowName);

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	// I-frame state flag
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsInvincible = false;

	UPROPERTY(BlueprintReadWrite, Category = "Skills")
	bool bCrossbarIsVisible = false;

	UFUNCTION()
	void UseStudyBook();

	// The currently equipped element
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nebula Combat|Elements")
	EElement ActiveElement = EElement::None;

	// Array of elements the player has unlocked
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Nebula Combat|Elements")
	TArray<EElement> UnlockedElements;
};

