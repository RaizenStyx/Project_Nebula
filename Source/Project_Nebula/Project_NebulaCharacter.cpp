// Copyright Epic Games, Inc. All Rights Reserved.

#include "Project_NebulaCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "PlayerStatsComponent.h"
#include "Public/SkillManagerComponent.h"
#include "Public/InventoryComponent.h"
#include "Public/EquipmentComponent.h"
#include "Public/TargetLockComponent.h"
#include "InputActionValue.h"
#include "Interactable.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Public/GI_Nebula.h"
#include "Public/PhysicalDamageType.h"
#include "Public/EnemyBase.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AProject_NebulaCharacter

AProject_NebulaCharacter::AProject_NebulaCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm



	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(RootComponent);
	// Adjust this radius for how close they need to be
	InteractionSphere->SetSphereRadius(150.0f); 
	// Only overlaps, doesn't block movement
	InteractionSphere->SetCollisionProfileName(TEXT("Trigger")); 

	// Initialize and attach all equipment meshes to their respective sockets!
	// We set NoCollision so your armor doesn't block your own camera or movement.

	HeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadMesh"));
	HeadMesh->SetupAttachment(GetMesh(), TEXT("Socket_Head"));
	HeadMesh->SetCollisionProfileName(TEXT("NoCollision"));

	ChestMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChestMesh"));
	ChestMesh->SetupAttachment(GetMesh(), TEXT("Socket_Chest"));
	ChestMesh->SetCollisionProfileName(TEXT("NoCollision"));

	WeaponRMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponRMesh"));
	WeaponRMesh->SetupAttachment(GetMesh(), TEXT("Socket_WeaponR"));
	WeaponRMesh->SetCollisionProfileName(TEXT("NoCollision"));

	WeaponLMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponLMesh"));
	WeaponLMesh->SetupAttachment(GetMesh(), TEXT("Socket_WeaponL"));
	WeaponLMesh->SetCollisionProfileName(TEXT("NoCollision"));

	ArmLMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArmLMesh"));
	ArmLMesh->SetupAttachment(GetMesh(), TEXT("Socket_ArmL"));
	ArmLMesh->SetCollisionProfileName(TEXT("NoCollision"));

	ArmRMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArmRMesh"));
	ArmRMesh->SetupAttachment(GetMesh(), TEXT("Socket_ArmR"));
	ArmRMesh->SetCollisionProfileName(TEXT("NoCollision"));

	LegsMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LegsMesh"));
	LegsMesh->SetupAttachment(GetMesh(), TEXT("Socket_Legs"));
	LegsMesh->SetCollisionProfileName(TEXT("NoCollision"));

	FeetMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FeetMesh"));
	FeetMesh->SetupAttachment(GetMesh(), TEXT("Socket_Feet"));
	FeetMesh->SetCollisionProfileName(TEXT("NoCollision"));

	SkillManager = CreateDefaultSubobject<USkillManagerComponent>(TEXT("SkillManager"));
	PlayerStats = CreateDefaultSubobject<UPlayerStatsComponent>(TEXT("PlayerStats"));
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	EquipmentComponent = CreateDefaultSubobject<UEquipmentComponent>(TEXT("EquipmentComponent"));
	TargetLockComponent = CreateDefaultSubobject<UTargetLockComponent>(TEXT("TargetLockComponent"));
}

void AProject_NebulaCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 1. Get the Game Instance and cast it to your custom class
	UGI_Nebula* GI = Cast<UGI_Nebula>(UGameplayStatics::GetGameInstance(this));

	if (GI)
	{
		// 2. Apply Origin Stat Boosts based on the GDD
		switch (GI->SelectedOrigin)
		{
		case ENebulaOrigin::Styx:
			PlayerStats->BasePhysicalProwess += 10;
			PlayerStats->BaseAgility += 8;
			PlayerStats->BaseSynchronization += 5;
			PlayerStats->BaseFortitude += 5;
			PlayerStats->BaseVigor += 5;
			UE_LOG(LogTemp, Warning, TEXT("Applied Styx Origin Stats"));
			break;

		case ENebulaOrigin::Kitsune:
			PlayerStats->BasePhysicalProwess += 5;
			PlayerStats->BaseAgility += 10;
			PlayerStats->BaseSynchronization += 8;
			PlayerStats->BaseFortitude += 5;
			PlayerStats->BaseVigor += 5;
			UE_LOG(LogTemp, Warning, TEXT("Applied Kitsune Origin Stats"));
			break;

		case ENebulaOrigin::Titan:
			PlayerStats->BasePhysicalProwess += 5;
			PlayerStats->BaseAgility += 5;
			PlayerStats->BaseSynchronization += 5;
			PlayerStats->BaseFortitude += 10;
			PlayerStats->BaseVigor += 8;
			UE_LOG(LogTemp, Warning, TEXT("Applied Titan Origin Stats"));
			break;

		default:
			break;
		}

		// 3. Setup the Starting Essence (Skills/Magic)
		// TODO: Create Essence Templates and assign here
		// For now, only stat boosts
		switch (GI->SelectedEssence)
		{
		case ENebulaEssence::Fighting:
			// Assign Fighting Essence starting stats here.
			// TODO: Assign essence template class here
			PlayerStats->BasePhysicalProwess += 20;
			PlayerStats->BaseAgility += 15;
			PlayerStats->BaseSynchronization += 7;
			PlayerStats->BaseFortitude += 7;
			PlayerStats->BaseVigor += 7;
			PlayerStats->PlayerEssence = EEssenceType::Fighting;
			
			// Example of assigning to skill manager when essence templates are built.
			//SkillManager->CurrentEssence = ENebulaEssence::Fighting;
			
			UE_LOG(LogTemp, Warning, TEXT("Applied Fighting Ess Stats"));
			break;

		case ENebulaEssence::Evasive:
			// Assign Evasive Essence starting stats here.
			// TODO: Assign essence template class here
			PlayerStats->BasePhysicalProwess += 7;
			PlayerStats->BaseAgility += 20;
			PlayerStats->BaseSynchronization += 15;
			PlayerStats->BaseFortitude += 7;
			PlayerStats->BaseVigor += 7;
			PlayerStats->PlayerEssence = EEssenceType::Evasive;
			UE_LOG(LogTemp, Warning, TEXT("Applied Evasive Ess Stats"));
			break;
		case ENebulaEssence::Survivability:
			// Assign Survivability Essence starting stats here.
			// TODO: Assign essence template class here
			PlayerStats->BasePhysicalProwess += 7;
			PlayerStats->BaseAgility += 7;
			PlayerStats->BaseSynchronization += 7;
			PlayerStats->BaseFortitude += 20;
			PlayerStats->BaseVigor += 15;
			PlayerStats->PlayerEssence = EEssenceType::Survivability;
			UE_LOG(LogTemp, Warning, TEXT("Applied Survivability Ess Stats"));
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AProject_NebulaCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}


void AProject_NebulaCharacter::Jump()
{
	if (bCrossbarIsVisible) return;
	// You can add custom logic here if needed before calling the base jump functionality
	Super::Jump();

}

void AProject_NebulaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AProject_NebulaCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AProject_NebulaCharacter::Look);

		// Attacking
		EnhancedInputComponent->BindAction(LightAttackAction, ETriggerEvent::Started, this, &AProject_NebulaCharacter::Input_PrimaryAction_Tap);
		EnhancedInputComponent->BindAction(LightAttackAction, ETriggerEvent::Triggered, this, &AProject_NebulaCharacter::Input_PrimaryAction_Hold);
		
		EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Started, this, &AProject_NebulaCharacter::Input_SecondaryAction_Tap);
		EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Triggered, this, &AProject_NebulaCharacter::Input_SecondaryAction_Hold);

		// Dodging & Crouching
		//EnhancedInputComponent->BindAction(DodgeCrouchAction, ETriggerEvent::Started, this, &AProject_NebulaCharacter::DodgeOrCrouch);

		// Triggered when pressed
		EnhancedInputComponent->BindAction(DodgeCrouchAction, ETriggerEvent::Started, this, &AProject_NebulaCharacter::StartDodgeOrSlide);

		// Triggered when released
		EnhancedInputComponent->BindAction(DodgeCrouchAction, ETriggerEvent::Completed, this, &AProject_NebulaCharacter::EndSlide);

		// Target Locking
		EnhancedInputComponent->BindAction(TargetLockAction, ETriggerEvent::Started, this, &AProject_NebulaCharacter::Input_ToggleTargetLock);
		
		EnhancedInputComponent->BindAction(TargetSwitchAction, ETriggerEvent::Triggered, this, &AProject_NebulaCharacter::Input_SwitchTarget);

		// Quick Slot Inputs
		// 1. Define the input action bindings in your SetupPlayerInputComponent
		EnhancedInputComponent->BindAction(SlotDPadDownAction, ETriggerEvent::Started, this, &AProject_NebulaCharacter::ConsumeHealthItem);
		EnhancedInputComponent->BindAction(SlotDPadLeftAction, ETriggerEvent::Started, this, &AProject_NebulaCharacter::ConsumeManaItem);
		EnhancedInputComponent->BindAction(SlotDPadRightAction, ETriggerEvent::Started, this, &AProject_NebulaCharacter::ConsumeStaminaItem);
		EnhancedInputComponent->BindAction(SlotDPadUpAction, ETriggerEvent::Started, this, &AProject_NebulaCharacter::CycleActiveElement);

		// Skill Wheel Inputs
		// The Left Bumper (Normal Skills Modifier)
		EnhancedInputComponent->BindAction(HotbarModifierAction, ETriggerEvent::Started, this, &AProject_NebulaCharacter::Input_LB_Started);
		EnhancedInputComponent->BindAction(HotbarModifierAction, ETriggerEvent::Completed, this, &AProject_NebulaCharacter::Input_LB_Completed);

		// The Right Bumper (Class Skills Modifier)
		EnhancedInputComponent->BindAction(ClassHotbarModifierAction, ETriggerEvent::Started, this, &AProject_NebulaCharacter::Input_RB_Started);
		EnhancedInputComponent->BindAction(ClassHotbarModifierAction, ETriggerEvent::Completed, this, &AProject_NebulaCharacter::Input_RB_Completed);

		// The Face Top Button (Y / Triangle) 
		// ETriggerEvent::Completed acts as a "Tap" (Released before Hold threshold)
		EnhancedInputComponent->BindAction(SlotFaceTopAction, ETriggerEvent::Completed, this, &AProject_NebulaCharacter::Input_FaceTop_Tap);
		// ETriggerEvent::Triggered acts as a "Hold" (Threshold met)
		EnhancedInputComponent->BindAction(SlotFaceTopAction, ETriggerEvent::Triggered, this, &AProject_NebulaCharacter::Input_FaceTop_Hold);


		// The Face Left Button (X / Square)
		// ETriggerEvent::Completed acts as a "Tap" (Released before Hold threshold)
		EnhancedInputComponent->BindAction(SlotFaceLeftAction, ETriggerEvent::Completed, this, &AProject_NebulaCharacter::Input_FaceLeft_Tap);
		// ETriggerEvent::Triggered acts as a "Hold" (Threshold met)
		EnhancedInputComponent->BindAction(SlotFaceLeftAction, ETriggerEvent::Triggered, this, &AProject_NebulaCharacter::Input_FaceLeft_Hold);

		// The Face Left Button (A / X)
		// ETriggerEvent::Completed acts as a "Tap" (Released before Hold threshold)
		EnhancedInputComponent->BindAction(SlotFaceBottomAction, ETriggerEvent::Completed, this, &AProject_NebulaCharacter::Input_FaceBottom_Tap);
		// ETriggerEvent::Triggered acts as a "Hold" (Threshold met)
		EnhancedInputComponent->BindAction(SlotFaceBottomAction, ETriggerEvent::Triggered, this, &AProject_NebulaCharacter::Input_FaceBottom_Hold);

		// The Face Left Button (B / Circle)
		// ETriggerEvent::Completed acts as a "Tap" (Released before Hold threshold)
		EnhancedInputComponent->BindAction(SlotFaceRightAction, ETriggerEvent::Completed, this, &AProject_NebulaCharacter::Input_FaceRight_Tap);
		// ETriggerEvent::Triggered acts as a "Hold" (Threshold met)
		EnhancedInputComponent->BindAction(SlotFaceRightAction, ETriggerEvent::Triggered, this, &AProject_NebulaCharacter::Input_FaceRight_Hold);

		// D-Pad Input buttons will go here
		EnhancedInputComponent->BindAction(SlotDPadUpAction, ETriggerEvent::Completed, this, &AProject_NebulaCharacter::Input_DPadUp_Tap);
		EnhancedInputComponent->BindAction(SlotDPadUpAction, ETriggerEvent::Triggered, this, &AProject_NebulaCharacter::Input_DPadUp_Hold);

		EnhancedInputComponent->BindAction(SlotDPadDownAction, ETriggerEvent::Completed, this, &AProject_NebulaCharacter::Input_DPadDown_Tap);
		EnhancedInputComponent->BindAction(SlotDPadDownAction, ETriggerEvent::Triggered, this, &AProject_NebulaCharacter::Input_DPadDown_Hold);

		EnhancedInputComponent->BindAction(SlotDPadLeftAction, ETriggerEvent::Completed, this, &AProject_NebulaCharacter::Input_DPadLeft_Tap);
		EnhancedInputComponent->BindAction(SlotDPadLeftAction, ETriggerEvent::Triggered, this, &AProject_NebulaCharacter::Input_DPadLeft_Hold);

		EnhancedInputComponent->BindAction(SlotDPadRightAction, ETriggerEvent::Completed, this, &AProject_NebulaCharacter::Input_DPadRight_Tap);
		EnhancedInputComponent->BindAction(SlotDPadRightAction, ETriggerEvent::Triggered, this, &AProject_NebulaCharacter::Input_DPadRight_Hold);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AProject_NebulaCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if ((Controller != nullptr) && !bIsAttacking)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AProject_NebulaCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

// --- PRIMARY TAP (Melee Combos & Single Fire) ---
void AProject_NebulaCharacter::Input_PrimaryAction_Tap()
{
	if (bCrossbarIsVisible) return;

	if (CurrentWeaponInfo.WeaponType == EWeaponArchetype::Focus)
	{
		// Optional: Fire a single shot immediately on tap before the hold logic kicks in
		if (GetWorld()->GetTimeSeconds() - LastPrimaryFireTime >= FocusFireRate)
		{
			PerformPrimaryMagic();
			LastPrimaryFireTime = GetWorld()->GetTimeSeconds();
		}
	}
	else
	{
		// 1H or 2H Melee Sword Tap (Future Combos)
		PerformPrimaryMelee();
	}
}

// --- PRIMARY HOLD (Continuous Magic) ---
void AProject_NebulaCharacter::Input_PrimaryAction_Hold()
{
	if (bCrossbarIsVisible) return;

	// Both 1H and 2H focuses use the same light continuous fire
	if (CurrentWeaponInfo.WeaponType == EWeaponArchetype::Focus)
	{
		if (GetWorld()->GetTimeSeconds() - LastPrimaryFireTime >= FocusFireRate)
		{
			PerformPrimaryMagic();
			LastPrimaryFireTime = GetWorld()->GetTimeSeconds();
		}
	}
}

// --- SECONDARY TAP (Heavy Melee & Blocks) ---
void AProject_NebulaCharacter::Input_SecondaryAction_Tap()
{
	if (bCrossbarIsVisible) return;

	if (CurrentWeaponInfo.bIsTwoHanded)
	{
		if (CurrentWeaponInfo.WeaponType != EWeaponArchetype::Focus)
		{
			// 2H Sword Tap -> Basic Heavy
			PerformSecondaryHeavy();
		}
	}
	else
	{
		if (OffhandWeaponInfo.WeaponType == EWeaponArchetype::Shield)
		{
			// 1H Shield Tap -> Put up shield (or parry)
			PerformSecondaryMelee();
		}
		else if (OffhandWeaponInfo.WeaponType == EWeaponArchetype::Focus)
		{
			// 1H Focus Tap -> Single Offhand Fire
			if (GetWorld()->GetTimeSeconds() - LastSecondaryFireTime >= FocusFireRate)
			{
				PerformSecondaryMagic();
				LastSecondaryFireTime = GetWorld()->GetTimeSeconds();
			}
		}
	}
}

// --- SECONDARY HOLD (Charge Melee, Blocks, & Heavy Magic) ---
void AProject_NebulaCharacter::Input_SecondaryAction_Hold()
{
	if (bCrossbarIsVisible) return;

	if (CurrentWeaponInfo.bIsTwoHanded)
	{
		if (CurrentWeaponInfo.WeaponType == EWeaponArchetype::Focus)
		{
			// 2H Focus Hold -> Slower, Stronger Continuous Fire
			if (GetWorld()->GetTimeSeconds() - LastSecondaryFireTime >= FocusHeavyFireRate)
			{
				// Note: You might want to create a PerformHeavyMagic() function for the stronger projectile
				PerformSecondaryMagic();
				LastSecondaryFireTime = GetWorld()->GetTimeSeconds();
			}
		}
		else
		{
			// 2H Sword Hold -> Charge Attack
			// TODO
			//PerformSecondaryHeavyCharge(); // You'll need to define this function
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Orange, TEXT("2H Sword Charge Attack Triggered! (Hold Logic Executing)"));
		}
	}
	else
	{
		if (OffhandWeaponInfo.WeaponType == EWeaponArchetype::Focus)
		{
			// 1H Focus Offhand Hold -> Continuous Fire
			if (GetWorld()->GetTimeSeconds() - LastSecondaryFireTime >= FocusFireRate)
			{
				PerformSecondaryMagic();
				LastSecondaryFireTime = GetWorld()->GetTimeSeconds();
			}
		}
		else if (OffhandWeaponInfo.WeaponType == EWeaponArchetype::Shield)
		{
			// 1H Shield Hold -> Maintain Block
			PerformSecondaryMelee(); // Assuming this triggers the blocking state
		}
	}
}

void AProject_NebulaCharacter::ExecuteLightAttack()
{
	// 1. Only allow this specific combo if the stance is exactly right
	if (CurrentStance != EWeaponStance::Sword1H) return;

	if (bIsAttacking)
	{
		// If they press the button while already swinging, save the input!
		bSaveAttack = true;
	}
	else
	{
		// Start the combo
		bIsAttacking = true;
		ComboStep = 1;
		PlayAnimMontage(SwordComboMontage, 1.0f, FName("Attack1"));
	}
}

void AProject_NebulaCharacter::ContinueCombo() // Triggered by 'SaveAttack' Notify
{
	if (bSaveAttack)
	{
		bSaveAttack = false;
		ComboStep++;

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && SwordComboMontage)
		{
			// Jump to the next section based on combo step
			FName NextSection = FName(*FString::Printf(TEXT("Attack%d"), ComboStep));
			AnimInstance->Montage_JumpToSection(NextSection, SwordComboMontage);
		}
	}
}

void AProject_NebulaCharacter::ResetCombo() // Triggered by 'ResetAttack' Notify
{
	bIsAttacking = false;
	bSaveAttack = false;
	ComboStep = 1;
}

void AProject_NebulaCharacter::PerformWeaponTrace()
{

	FVector StartLoc = WeaponRMesh->GetSocketLocation(FName("TraceStart"));
	FVector EndLoc = WeaponRMesh->GetSocketLocation(FName("TraceEnd"));

	TArray<FHitResult> HitResults;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	UKismetSystemLibrary::SphereTraceMulti(
		this,
		StartLoc,
		EndLoc,
		20.0f,
		UEngineTypes::ConvertToTraceType(ECC_Pawn),
		false,
		ActorsToIgnore,
		EDrawDebugTrace::ForDuration,
		HitResults,
		true
	);

	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor && !HitActorsToIgnore.Contains(HitActor))
		{
			HitActorsToIgnore.Add(HitActor);

			// 1. Establish a default archetype
			EEnemyArchetype TargetArchetype = EEnemyArchetype::None;

			// 2. Cast to your enemy class to find its specific archetype
			// (Replace 'AEnemyBase' with whatever your actual enemy C++ class is named)
			
			AEnemyBase* HitEnemy = Cast<AEnemyBase>(HitActor);
			if (HitEnemy)
			{
				TargetArchetype = HitEnemy->EnemyArchetype;
			}

			// 3. Calculate the outgoing damage through the Stats Component
			float DamageToApply = 0.0f;
			if (PlayerStats)
			{
				DamageToApply = PlayerStats->CalculateOutgoingPhysicalDamage(
					CurrentWeaponInfo.BaseDamage,
					CurrentWeaponInfo.TechniqueStyle,
					TargetArchetype
				);
			}

			// 4. Apply the damage using your custom Physical Damage Type
			UGameplayStatics::ApplyDamage(
				HitActor,
				DamageToApply,
				GetController(),
				this,
				UPhysicalDamageType::StaticClass() // The flag that tells the enemy this is physical
			);
		}
	}
}

void AProject_NebulaCharacter::OnMeleeOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Ensure we hit a valid actor that isn't ourselves
	if (OtherActor && OtherActor != this)
	{
		// 1. Get your weapon damage from PlayerStats
		float BaseDamage = 15.0f; // Fallback
		if (PlayerStats)
		{
			//BaseDamage = PlayerStats->BaseAttack; // Pull real attack stat
		}

		// 2. THE SENDER: Apply the damage using your custom Physical tag!
		UGameplayStatics::ApplyDamage(
			OtherActor,
			BaseDamage,
			GetController(),
			this,
			UPhysicalDamageType::StaticClass() // <--- The magic tag we created
		);

		// Disable it here so it only turns off AFTER a successful enemy hit
		//DisableMainhandHitbox();
		//DisableOffhandHitbox();
	}
}

void AProject_NebulaCharacter::PerformPrimaryMagic()
{
	// 1. Play the Casting Animation
	if (PrimaryMagicMontage)
	{
		PlayAnimMontage(PrimaryMagicMontage);
	}

	// 2. Spawn the Projectile
	if (PrimaryMagicProjectileClass && GetWorld())
	{
		// Currently spawning slightly in front of the player's center
		FVector SpawnLoc = GetActorLocation() + (GetActorForwardVector() * 100.f);
		FRotator SpawnRot = GetActorRotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Instigator = this; // The character is the instigator
		SpawnParams.Owner = this;      // Good practice to set the owner for damage attribution

		// Spawn it!
		GetWorld()->SpawnActor<ANebulaProjectile>(PrimaryMagicProjectileClass, SpawnLoc, SpawnRot, SpawnParams);

		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan, TEXT("Primary Magic Projectile Spawned via C++!"));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("Failed to spawn: Projectile Class not set in BP_ThirdPersonCharacter!"));
	}
}

void AProject_NebulaCharacter::DetermineWeaponStance()
{
	UEquipmentComponent* EquipComp = FindComponentByClass<UEquipmentComponent>();
	if (!EquipComp) return;

	// 1. Check if we actually have anything in the Right Hand slot
	bool bHasMainHand = EquipComp->EquippedItems.Contains(EEquipmentSlot::WeaponR) && EquipComp->EquippedItems[EEquipmentSlot::WeaponR] != NAME_None;

	if (!bHasMainHand)
	{
		CurrentStance = EWeaponStance::Unarmed;
		return;
	}

	// 2. We DO have a weapon. Check the info that UpdateEquipmentVisuals just saved!
	if (CurrentWeaponInfo.WeaponType == EWeaponArchetype::Sword)
	{
		// 3. Since your current design treats 1H Sword + Empty and 1H Sword + Focus as the exact same stance:
		CurrentStance = EWeaponStance::Sword1H;
	}
	else if (CurrentWeaponInfo.WeaponType == EWeaponArchetype::Focus)
	{
		CurrentStance = EWeaponStance::Unarmed;
	}
	else if (CurrentWeaponInfo.WeaponType == EWeaponArchetype::Sword && OffhandWeaponInfo.WeaponType == EWeaponArchetype::Shield)
	{
		CurrentStance = EWeaponStance::SwordShield;
	}
	else if (CurrentWeaponInfo.WeaponType == EWeaponArchetype::Sword && CurrentWeaponInfo.bIsTwoHanded)
	{
		CurrentStance = EWeaponStance::GreatSword;
	}
	else
	{
		// Fallback for when you add other weapon types later
		CurrentStance = EWeaponStance::Unarmed;
	}

	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan, FString::Printf(TEXT("Weapon Stance Updated to %s!"), *UEnum::GetValueAsString(CurrentStance)));
}

void AProject_NebulaCharacter::PerformSecondaryMagic()
{
	// 1. Play the Left-Hand Casting Animation
	if (SecondaryMagicMontage)
	{
		PlayAnimMontage(SecondaryMagicMontage);
	}

	// 2. Spawn the Secondary Projectile
	if (SecondaryMagicProjectileClass && GetWorld())
	{
		FVector SpawnLoc = GetActorLocation() + (GetActorForwardVector() * 100.f);
		FRotator SpawnRot = GetActorRotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Instigator = this;
		SpawnParams.Owner = this;

		GetWorld()->SpawnActor<ANebulaProjectile>(SecondaryMagicProjectileClass, SpawnLoc, SpawnRot, SpawnParams);

		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Purple, TEXT("Secondary Magic Projectile Spawned via C++!"));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("Failed to spawn: Secondary Projectile Class not set!"));
	}
}

void AProject_NebulaCharacter::StartDodgeOrSlide(const FInputActionValue& Value)
{
	if (bCrossbarIsVisible) return;

	if (GetCharacterMovement()->Velocity.SizeSquared2D() > 100.f)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		if (SlideMontage && AnimInstance && !AnimInstance->Montage_IsPlaying(SlideMontage))
		{
			bIsSliding = true;

			OriginalGroundFriction = GetCharacterMovement()->GroundFriction;
			OriginalBrakingDeceleration = GetCharacterMovement()->BrakingDecelerationWalking;

			GetCharacterMovement()->GroundFriction = 0.0f;
			GetCharacterMovement()->BrakingDecelerationWalking = 200.f;

			// 1. Get Agility
			float EffectiveAgility = PlayerStats ? PlayerStats->GetEffectiveStatValue(PlayerStats->Agility) : 10.0f;

			// 2. Play Animation
			float PlayRate = 1.0f + (EffectiveAgility * 0.002f);
			PlayAnimMontage(SlideMontage, PlayRate);

			// 3. Launch Character
			FVector DodgeDirection = GetCharacterMovement()->Velocity.GetSafeNormal2D();
			float BaseDodgeForce = 1500.0f;
			float TotalDodgeForce = BaseDodgeForce + (EffectiveAgility * 10.0f);
			LaunchCharacter(DodgeDirection * TotalDodgeForce, true, false);

			// 4. Calculate Max Time and Set Timer!
			float MaxSlideTime = BaseMaxSlideTime + (EffectiveAgility * AgilityTimeMultiplier);

			GetWorld()->GetTimerManager().SetTimer(SlideTimerHandle, this, &AProject_NebulaCharacter::StopSliding, MaxSlideTime, false);

			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, FString::Printf(TEXT("Max Slide Time: %f"), MaxSlideTime));
		}
	}
	else
	{
		if (bIsCrouched) UnCrouch();
		else Crouch();
	}
}

// Triggered when the player releases the button early
void AProject_NebulaCharacter::EndSlide(const FInputActionValue& Value)
{
	StopSliding();
}

// The actual logic to halt the slide (called by release OR by the timer)
void AProject_NebulaCharacter::StopSliding()
{
	if (bIsSliding)
	{
		bIsSliding = false;

		// 1. Clear the timer (crucial if they released the button BEFORE the max time ran out)
		GetWorld()->GetTimerManager().ClearTimer(SlideTimerHandle);

		// 2. Restore normal physics
		GetCharacterMovement()->GroundFriction = OriginalGroundFriction;
		GetCharacterMovement()->BrakingDecelerationWalking = OriginalBrakingDeceleration;

		// 3. Force the exit animation
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && SlideMontage && AnimInstance->Montage_IsPlaying(SlideMontage))
		{
			AnimInstance->Montage_JumpToSection(FName("Exit"), SlideMontage);
		}
	}
}

void AProject_NebulaCharacter::UpdateEquipmentVisuals(EEquipmentSlot Slot, FName ItemRowName)
{
	UStaticMeshComponent* TargetMeshComp = nullptr;
	bool bIsWeapon = false; // Flag to route our data table logic

	// 1. Determine mesh and route type
	switch (Slot)
	{
	case EEquipmentSlot::Head: TargetMeshComp = HeadMesh; break;
	case EEquipmentSlot::Chest: TargetMeshComp = ChestMesh; break;
	case EEquipmentSlot::ArmR: TargetMeshComp = ArmRMesh; break;
	case EEquipmentSlot::ArmL: TargetMeshComp = ArmLMesh; break;
	case EEquipmentSlot::Legs: TargetMeshComp = LegsMesh; break;
	case EEquipmentSlot::Feet: TargetMeshComp = FeetMesh; break;
	case EEquipmentSlot::WeaponR: TargetMeshComp = WeaponRMesh; bIsWeapon = true; break;
	case EEquipmentSlot::WeaponL: TargetMeshComp = WeaponLMesh; bIsWeapon = true; break;
	case EEquipmentSlot::None: return;
	}

	if (!TargetMeshComp) return;

	// 2. UNEQUIP LOGIC: If "NAME_None" is passed in
	if (ItemRowName.IsNone())
	{
		TargetMeshComp->SetStaticMesh(nullptr);

		if (bIsWeapon)
		{
			if (Slot == EEquipmentSlot::WeaponR) CurrentWeaponInfo = FWeaponInfo();
			else if (Slot == EEquipmentSlot::WeaponL) OffhandWeaponInfo = FWeaponInfo();
		}
		else
		{
			// Clear the specific armor struct
			switch (Slot)
			{
			case EEquipmentSlot::Head: HeadArmorInfo = FNebulaArmorData(); break;
			case EEquipmentSlot::Chest: ChestArmorInfo = FNebulaArmorData(); break;
			case EEquipmentSlot::ArmR: ArmRArmorInfo = FNebulaArmorData(); break;
			case EEquipmentSlot::ArmL: ArmLArmorInfo = FNebulaArmorData(); break;
			case EEquipmentSlot::Legs: LegsArmorInfo = FNebulaArmorData(); break;
			case EEquipmentSlot::Feet: FeetArmorInfo = FNebulaArmorData(); break;
			default: break;
			}
			RecalculateArmorStats(); // Re-tally the missing stats
		}
		return;
	}

	// 3. EQUIP LOGIC - WEAPONS
	if (bIsWeapon)
	{
		if (WeaponDataTable)
		{
			FWeaponInfo* FoundRow = WeaponDataTable->FindRow<FWeaponInfo>(ItemRowName, TEXT("Weapon Visual Context"));
			if (FoundRow)
			{
				TargetMeshComp->SetStaticMesh(FoundRow->VisualMesh);
				if (Slot == EEquipmentSlot::WeaponR)
				{
					CurrentWeaponInfo = *FoundRow;
					// Will need to adjust the hit boxes based on weapon type/size.
					// 
					if (CurrentWeaponInfo.bIsTwoHanded)
					{
						//MainhandHitbox->SetBoxExtent(FVector(10.f, 2.5f, 125.f));
					}
				}
				else if (Slot == EEquipmentSlot::WeaponL) OffhandWeaponInfo = *FoundRow;
			}
		}
	}
	// 4. EQUIP LOGIC - ARMOR
	else
	{
		if (ArmorDataTable)
		{
			FNebulaArmorData* FoundRow = ArmorDataTable->FindRow<FNebulaArmorData>(ItemRowName, TEXT("Armor Visual Context"));
			if (FoundRow)
			{
				TargetMeshComp->SetStaticMesh(FoundRow->ArmorMesh);

				// Save the new armor info to the correct slot
				switch (Slot)
				{
				case EEquipmentSlot::Head: HeadArmorInfo = *FoundRow; break;
				case EEquipmentSlot::Chest: ChestArmorInfo = *FoundRow; break;
				case EEquipmentSlot::ArmR: ArmRArmorInfo = *FoundRow; break;
				case EEquipmentSlot::ArmL: ArmLArmorInfo = *FoundRow; break;
				case EEquipmentSlot::Legs: LegsArmorInfo = *FoundRow; break;
				case EEquipmentSlot::Feet: FeetArmorInfo = *FoundRow; break;
				default: break;
				}
				RecalculateArmorStats(); // Apply the new stats
			}
		}
	}
}

void AProject_NebulaCharacter::RecalculateArmorStats()
{
	if (PlayerStats)
	{
		PlayerStats->PhysicalArmorDefense = HeadArmorInfo.BasePhysicalDefense +
			ChestArmorInfo.BasePhysicalDefense +
			ArmRArmorInfo.BasePhysicalDefense +
			ArmLArmorInfo.BasePhysicalDefense +
			LegsArmorInfo.BasePhysicalDefense +
			FeetArmorInfo.BasePhysicalDefense;

		PlayerStats->MagicalArmorDefense = HeadArmorInfo.BaseMagicalDefense +
			ChestArmorInfo.BaseMagicalDefense +
			ArmRArmorInfo.BaseMagicalDefense +
			ArmLArmorInfo.BaseMagicalDefense +
			LegsArmorInfo.BaseMagicalDefense +
			FeetArmorInfo.BaseMagicalDefense;
	}
}

float AProject_NebulaCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// 1. Check Invincibility (I-frames from dodging)
	if (bIsInvincible)
	{
		return 0.0f;
	}

	// 2. Fetch the Stats Component
	if (PlayerStats)
	{
		// 3. Apply Fortitude Reduction
		float ActualDamage = PlayerStats->CalculateIncomingPhysicalDamage(DamageAmount);

		// 4. Apply the damage
		PlayerStats->ModifyHealth(-ActualDamage);

		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString::Printf(TEXT("Took %f damage!"), ActualDamage));

		return ActualDamage;
	}

	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

// ---------------------------------------------------------
// MODIFIER BUTTONS (LB / RB)
// ---------------------------------------------------------

void AProject_NebulaCharacter::Input_LB_Started()
{
	// Set the state to Normal Skills
	CurrentHotbarCategory = ENebulaSkillCategory::Normal;

	// Tell the Blueprint UI to appear and show the Normal spellbook
	OnToggleCrossHotbar(true, CurrentHotbarCategory);
}

void AProject_NebulaCharacter::Input_LB_Completed()
{
	OnToggleCrossHotbar(false, CurrentHotbarCategory);
}

void AProject_NebulaCharacter::Input_RB_Started()
{
	// Set the state to Class Skills
	CurrentHotbarCategory = ENebulaSkillCategory::Class;

	// Tell the Blueprint UI to appear and show the Class spellbook
	OnToggleCrossHotbar(true, CurrentHotbarCategory);
}

void AProject_NebulaCharacter::Input_RB_Completed()
{
	OnToggleCrossHotbar(false, CurrentHotbarCategory);
}


// ---------------------------------------------------------
// FACE BUTTONS & D-PAD EXECUTION
// ---------------------------------------------------------

void AProject_NebulaCharacter::Input_FaceTop_Tap()
{
	if (!bCrossbarIsVisible) return;
	if (SkillManager)
	{
		SkillManager->ExecuteSkillInSlot(CurrentHotbarCategory, ENebulaSkillSlot::Face_Top, false);
	}
}

void AProject_NebulaCharacter::Input_FaceTop_Hold()
{
	if (!bCrossbarIsVisible) return;
	if (SkillManager)
	{
		SkillManager->ExecuteSkillInSlot(CurrentHotbarCategory, ENebulaSkillSlot::Face_Top, true);
	}
}

void AProject_NebulaCharacter::Input_FaceBottom_Tap()
{
	if (!bCrossbarIsVisible) return;
	if (SkillManager)
	{
		SkillManager->ExecuteSkillInSlot(CurrentHotbarCategory, ENebulaSkillSlot::Face_Bottom, false);
	}
}

void AProject_NebulaCharacter::Input_FaceBottom_Hold()
{
	if (!bCrossbarIsVisible) return;
	if (SkillManager)
	{
		SkillManager->ExecuteSkillInSlot(CurrentHotbarCategory, ENebulaSkillSlot::Face_Bottom, true);
	}
}

void AProject_NebulaCharacter::Input_FaceLeft_Tap()
{
	if (!bCrossbarIsVisible) return;
	if (SkillManager)
	{
		SkillManager->ExecuteSkillInSlot(CurrentHotbarCategory, ENebulaSkillSlot::Face_Left, false);
	}
}

void AProject_NebulaCharacter::Input_FaceLeft_Hold()
{
	if (SkillManager)
	{
		SkillManager->ExecuteSkillInSlot(CurrentHotbarCategory, ENebulaSkillSlot::Face_Left, true);
	}
}

void AProject_NebulaCharacter::Input_FaceRight_Tap()
{
	if (!bCrossbarIsVisible) return;
	if (SkillManager)
	{
		SkillManager->ExecuteSkillInSlot(CurrentHotbarCategory, ENebulaSkillSlot::Face_Right, false);
	}
}

void AProject_NebulaCharacter::Input_FaceRight_Hold()
{
	if (!bCrossbarIsVisible) return;
	if (SkillManager)
	{
		SkillManager->ExecuteSkillInSlot(CurrentHotbarCategory, ENebulaSkillSlot::Face_Right, true);
	}
}

void AProject_NebulaCharacter::Input_DPadUp_Tap()
{
	if (!bCrossbarIsVisible) return;
	if (SkillManager)
	{
		SkillManager->ExecuteSkillInSlot(CurrentHotbarCategory, ENebulaSkillSlot::DPad_Up, false);
	}
}

void AProject_NebulaCharacter::Input_DPadUp_Hold()
{
	if (!bCrossbarIsVisible) return;
	if (SkillManager)
	{
		SkillManager->ExecuteSkillInSlot(CurrentHotbarCategory, ENebulaSkillSlot::DPad_Up, true);
	}
}

void AProject_NebulaCharacter::Input_DPadDown_Tap()
{
	if (!bCrossbarIsVisible) return;
	if (SkillManager)
	{
		SkillManager->ExecuteSkillInSlot(CurrentHotbarCategory, ENebulaSkillSlot::DPad_Down, false);
	}
}

void AProject_NebulaCharacter::Input_DPadDown_Hold()
{
	if (!bCrossbarIsVisible) return;
	if (SkillManager)
	{
		SkillManager->ExecuteSkillInSlot(CurrentHotbarCategory, ENebulaSkillSlot::DPad_Down, true);
	}
}

void AProject_NebulaCharacter::Input_DPadLeft_Tap()
{
	if (!bCrossbarIsVisible) return;
	if (SkillManager)
	{
		SkillManager->ExecuteSkillInSlot(CurrentHotbarCategory, ENebulaSkillSlot::DPad_Left, false);
	}
}

void AProject_NebulaCharacter::Input_DPadLeft_Hold()
{
	if (!bCrossbarIsVisible) return;
	if (SkillManager)
	{
		SkillManager->ExecuteSkillInSlot(CurrentHotbarCategory, ENebulaSkillSlot::DPad_Left, true);
	}
}

void AProject_NebulaCharacter::Input_DPadRight_Tap()
{
	if (!bCrossbarIsVisible) return;
	if (SkillManager)
	{
		SkillManager->ExecuteSkillInSlot(CurrentHotbarCategory, ENebulaSkillSlot::DPad_Right, false);
	}
}

void AProject_NebulaCharacter::Input_DPadRight_Hold()
{
	if (!bCrossbarIsVisible) return;
	if (SkillManager)
	{
		SkillManager->ExecuteSkillInSlot(CurrentHotbarCategory, ENebulaSkillSlot::DPad_Right, true);
	}
}

void AProject_NebulaCharacter::Input_ToggleTargetLock()
{
	if (!bCrossbarIsVisible) return;
	if (TargetLockComponent)
	{
		TargetLockComponent->ToggleTargetLock();
	}
}

void AProject_NebulaCharacter::Input_SwitchTarget(const FInputActionValue& Value)
{
	// The Value is a 1D float. Positive = Right, Negative = Left.
	float SwitchDirection = Value.Get<float>();

	if (TargetLockComponent)
	{
		TargetLockComponent->SwitchTarget(SwitchDirection);
	}
}

void AProject_NebulaCharacter::ConsumeHealthItem()
{
	if (!InventoryComponent) return;

	// We need to loop through the raw array. Assuming you have a getter like GetInventorySlots()
	const auto& Slots = InventoryComponent->GetInventorySlots();

	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		// Skip empty slots
		if (Slots[i].IsEmpty() || Slots[i].ItemID.IsNone()) continue;

		// Fetch the item rules using your existing database fetcher
		FNebulaItemData* ItemData = InventoryComponent->GetItemData(Slots[i].ItemID);
		if (!ItemData) continue;

		// 1. Is this a consumable that restores health?
		if (ItemData->ItemType == ENebulaItemType::Consumable && ItemData->HealthRestoreAmount > 0)
		{
			// 2. Is it currently on cooldown? 
			if (!ItemData->CooldownTag.IsNone())
			{
				float RemainingTime = InventoryComponent->GetRemainingCooldown(ItemData->CooldownTag);

				// If it is on cooldown, we CONTINUE the loop to look for a DIFFERENT health item
				if (RemainingTime > 0.f)
				{
					continue;
				}
			}

			// 3. We found a valid health item that is NOT on cooldown! 
			// We tell the inventory to use it, which triggers your existing heal, cooldown, and removal logic.
			InventoryComponent->UseItem(i);
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, FString::Printf(TEXT("Consumed Health Item: %s"), *Slots[i].ItemID.ToString()));

			// Stop scanning so we only consume ONE item.
			break;
		}
	}
}

void AProject_NebulaCharacter::ConsumeStaminaItem()
{
	if (!InventoryComponent) return;

	// We need to loop through the raw array. Assuming you have a getter like GetInventorySlots()
	const auto& Slots = InventoryComponent->GetInventorySlots();

	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		// Skip empty slots
		if (Slots[i].IsEmpty() || Slots[i].ItemID.IsNone()) continue;

		// Fetch the item rules using your existing database fetcher
		FNebulaItemData* ItemData = InventoryComponent->GetItemData(Slots[i].ItemID);
		if (!ItemData) continue;

		// 1. Is this a consumable that restores stamina?
		if (ItemData->ItemType == ENebulaItemType::Consumable && ItemData->StaminaRestoreAmount > 0)
		{
			// 2. Is it currently on cooldown? 
			if (!ItemData->CooldownTag.IsNone())
			{
				float RemainingTime = InventoryComponent->GetRemainingCooldown(ItemData->CooldownTag);

				// If it is on cooldown, we CONTINUE the loop to look for a DIFFERENT health item
				if (RemainingTime > 0.f)
				{
					continue;
				}
			}

			// 3. We found a valid health item that is NOT on cooldown! 
			// We tell the inventory to use it, which triggers your existing heal, cooldown, and removal logic.
			InventoryComponent->UseItem(i);

			// Stop scanning so we only consume ONE item.
			break;
		}
	}
}

void AProject_NebulaCharacter::ConsumeManaItem()
{
	if (!InventoryComponent) return;

	// We need to loop through the raw array. Assuming you have a getter like GetInventorySlots()
	const auto& Slots = InventoryComponent->GetInventorySlots();

	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		// Skip empty slots
		if (Slots[i].IsEmpty() || Slots[i].ItemID.IsNone()) continue;

		// Fetch the item rules using your existing database fetcher
		FNebulaItemData* ItemData = InventoryComponent->GetItemData(Slots[i].ItemID);
		if (!ItemData) continue;

		// 1. Is this a consumable that restores mana?
		if (ItemData->ItemType == ENebulaItemType::Consumable && ItemData->ManaRestoreAmount > 0)
		{
			// 2. Is it currently on cooldown? 
			if (!ItemData->CooldownTag.IsNone())
			{
				float RemainingTime = InventoryComponent->GetRemainingCooldown(ItemData->CooldownTag);

				// If it is on cooldown, we CONTINUE the loop to look for a DIFFERENT health item
				if (RemainingTime > 0.f)
				{
					continue;
				}
			}

			// 3. We found a valid health item that is NOT on cooldown! 
			// We tell the inventory to use it, which triggers your existing heal, cooldown, and removal logic.
			InventoryComponent->UseItem(i);

			// Stop scanning so we only consume ONE item.
			break;
		}
	}
}

void AProject_NebulaCharacter::CycleActiveElement()
{
	if (bCrossbarIsVisible) return;
	// 1. Guard check: If no elements are unlocked, they can only be None
	if (UnlockedElements.Num() == 0)
	{
		ActiveElement = EElement::None;
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("No elements unlocked yet."));
		return;
	}

	// 2. If current is None, switch to the first unlocked element
	if (ActiveElement == EElement::None)
	{
		ActiveElement = UnlockedElements[0];
	}
	else
	{
		// 3. Find current element in the array
		int32 CurrentIndex = UnlockedElements.Find(ActiveElement);

		// 4. If it's the last element in the array, wrap around to None
		if (CurrentIndex == UnlockedElements.Num() - 1)
		{
			ActiveElement = EElement::None;
		}
		else if (CurrentIndex != INDEX_NONE)
		{
			// Move to the next element
			ActiveElement = UnlockedElements[CurrentIndex + 1];
		}
		else
		{
			// Fallback just in case something gets desynced
			ActiveElement = EElement::None;
		}
	}

	FString ElementName = UEnum::GetValueAsName(ActiveElement).ToString();
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, FString::Printf(TEXT("Switched Element to: %s"), *ElementName));
}

// Example implementation when the player interacts with the "Study Book"
void AProject_NebulaCharacter::UseStudyBook()
{
	if (UPlayerStatsComponent* StatsComp = FindComponentByClass<UPlayerStatsComponent>())
	{
		// For testing the instant unlock: 
		// Costs 60 minutes of Awake Time, grants 100% progress.
		// Later, change 100.0f to 25.0f to require 4 separate study sessions.
		StatsComp->StudyMagicBook(60.0f, 100.0f);

		if (StatsComp->bIsManaUnlocked)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan, TEXT("Mana System Unlocked!"));
		}
	}
}