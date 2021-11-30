// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShooterCharacter.h"
#include "ShooterWeapon.generated.h"


UENUM()
enum class EWeaponState
{
	Idle,
	Firing,
	Equipping,
	Reloading
};


UENUM()
enum class EWeaponType
{
	Rifle,
	Pistol,
	Katana,
};


class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;
class AShooterCharacter;
class AShooterWeaponPickup;
class USoundCue;


UCLASS(ABSTRACT, Blueprintable)
class PROTOTYPE_API AShooterWeapon : public AActor
{
	GENERATED_BODY()

	virtual void PostInitializeComponents() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	float GetEquipStartedTime() const;

	float GetEquipDuration() const;

	/** last time when this weapon was switched to */
	float EquipStartedTime;

	/** how much time weapon needs to be equipped */
	float EquipDuration;

	bool bIsEquipped;

	bool bPendingEquip;

	FTimerHandle TimerHandle_HandleFiring;

	FTimerHandle TimerHandle_EquipFinished;

	FTimerHandle TimerHandle_UnEquipFinished;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float ShotsPerMinute;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float CurrentShotsPerMinute;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	EWeaponType WeaponType;

	/* The character socket to store this item at. */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	EInventorySlot StorageSlot;

	/** pawn owner */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_MyPawn)
	AShooterCharacter* MyPawn;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* MeshComp;

	UFUNCTION()
	void OnRep_MyPawn();

	/** detaches weapon mesh from pawn */
	void DetachMeshFromPawn();

	virtual void OnEquipFinished();

	virtual void OnUnEquipFinished();

	bool IsEquipped() const;

	bool IsAttachedToPawn() const;

public:
	// Sets default values for this actor's properties
	AShooterWeapon();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName WeaponName;

	/** get weapon mesh (needs pawn owner to determine variant) */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	USkeletalMeshComponent* GetWeaponMesh() const;

	/* You can assign default values to function parameters, these are then optional to specify/override when calling the function. */
	void AttachMeshToPawn(EInventorySlot Slot = EInventorySlot::Hands);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnUnEquip"), Category = "Weapon")
	void ReceiveOnUnEquip();

	virtual void OnUnEquip();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnEquip"), Category = "Weapon")
	void ReceiveOnEquip();

	void OnEquip(bool bPlayAnimation);

	/* Set the weapon's owning pawn */
	void SetOwningPawn(AShooterCharacter* NewOwner);

	/* Get pawn owner */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	AShooterCharacter* GetPawnOwner() const;

	virtual void OnEnterInventory(AShooterCharacter* NewOwner);

	virtual void OnLeaveInventory();

	FORCEINLINE EInventorySlot GetStorageSlot()
	{
		return StorageSlot;
	}

	FORCEINLINE EWeaponType GetWeaponType()
	{
		return WeaponType;
	}

	/* The class to spawn in the level when dropped */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<class AShooterWeaponPickup> WeaponPickupClass;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<UCameraShakeBase> FireCamShake;


	/************************************************************************/
	/* Fire & Damage Handling                                               */
	/************************************************************************/
public:
	void StartFire();

	void StopFire();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "StartTarget"), Category = "Weapon")
	void ReceiveStartTarget();

	virtual void StartTarget();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "EndTarget"), Category = "Weapon")
	void ReceiveEndTarget();

	virtual void EndTarget();

	EWeaponState GetCurrentState() const;

protected:
	bool CanFire() const;

	FVector GetAdjustedAim() const;

	FVector GetCameraDamageStartLocation(const FVector& AimDir) const;

	FHitResult WeaponTrace(const FVector& TraceFrom, const FVector& TraceTo) const;

	/* With PURE_VIRTUAL we skip implementing the function in SWeapon.cpp and can do this in SWeaponInstant.cpp / SFlashlight.cpp instead */
	virtual void FireWeapon() PURE_VIRTUAL(AShooterWeapon::FireWeapon,);

private:
	void SetWeaponState(EWeaponState NewState);

	void DetermineWeaponState();

	virtual void HandleFiring();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStartFire();
	void ServerStartFire_Implementation();
	bool ServerStartFire_Validate();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStopFire();
	void ServerStopFire_Implementation();
	bool ServerStopFire_Validate();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerHandleFiring();
	void ServerHandleFiring_Implementation();
	bool ServerHandleFiring_Validate();

	void OnBurstStarted();

	void OnBurstFinished();

	bool bWantsToFire;

	EWeaponState CurrentState;

	bool bRefiring;

	float LastFireTime;

	/* Time between shots for repeating fire */
	UPROPERTY(Replicated)
	float TimeBetweenShots;
public:
	void UpdateTimeBetweenShots();

	/************************************************************************/
	/* Simulation & FX                                                      */
	/************************************************************************/

private:
	UFUNCTION()
	void OnRep_BurstCounter();

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundCue* FireSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundCue* EquipSound;

	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* MuzzleFX;

	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* EquipAnim;

	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* FireAnim;

	UPROPERTY(Transient)
	UParticleSystemComponent* MuzzlePSC;

	UPROPERTY(EditDefaultsOnly)
	FName MuzzleSocketName;

	bool bPlayingFireAnim;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_BurstCounter)
	int32 BurstCounter;

protected:
	virtual void SimulateWeaponFire();

	virtual void StopSimulatingWeaponFire();

	FVector GetMuzzleLocation() const;

	FVector GetMuzzleDirection() const;

	UAudioComponent* PlayWeaponSound(USoundCue* SoundToPlay);

	float PlayWeaponAnimation(UAnimMontage* Animation, float InPlayRate = 1.f, FName StartSectionName = NAME_None);

	void StopWeaponAnimation(UAnimMontage* Animation);

	/************************************************************************/
	/* Ammo & Reloading                                                     */
	/************************************************************************/

private:
	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundCue* OutOfAmmoSound;

	FTimerHandle TimerHandle_ReloadWeapon;

	FTimerHandle TimerHandle_StopReload;

protected:
	/* Time to assign on reload when no animation is found */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	float NoAnimReloadDuration;

	/* Time to assign on equip when no animation is found */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	float NoEquipAnimDuration;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_Reload)
	bool bPendingReload;

	void UseAmmo();

	UPROPERTY(Transient, Replicated)
	int32 CurrentAmmo;

	UPROPERTY(Transient, Replicated)
	int32 CurrentAmmoInClip;

	/* Weapon ammo on spawn */
	UPROPERTY(EditDefaultsOnly)
	int32 StartAmmo;

	UPROPERTY(EditDefaultsOnly)
	int32 MaxAmmo;

	UPROPERTY(EditDefaultsOnly)
	int32 MaxAmmoPerClip;

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundCue* ReloadSound;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* ReloadAnim;

	virtual void ReloadWeapon();

	/* Trigger reload from server */
	UFUNCTION(Reliable, Client)
	void ClientStartReload();

	void ClientStartReload_Implementation();

	/* Is weapon and character currently capable of starting a reload */
	bool CanReload();

	UFUNCTION()
	void OnRep_Reload();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStartReload();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStopReload();

public:
	virtual void StartReload(bool bFromReplication = false);

	virtual void StopSimulateReload();

	/* Give ammo to weapon and return the amount that was not 'consumed' beyond the max count */
	int32 GiveAmmo(int32 AddAmount);

	/* Set a new total amount of ammo of weapon */
	void SetAmmoCount(int32 NewTotalAmount);

	UFUNCTION(BlueprintCallable, Category = "Ammo")
	int32 GetCurrentAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "Ammo")
	int32 GetCurrentAmmoInClip() const;

	UFUNCTION(BlueprintCallable, Category = "Ammo")
	int32 GetMaxAmmoPerClip() const;

	UFUNCTION(BlueprintCallable, Category = "Ammo")
	int32 GetMaxAmmo() const;
};
