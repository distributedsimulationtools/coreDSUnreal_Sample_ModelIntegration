/* Copyright (C) Consultants 2J's, Inc - operating under the name ds.tools
 * coreDS Unreal sample - Model Integration
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ModelIntegrationDataStructures_V1.h"
#include "ModelIntegrationTank.generated.h"

class UStaticMeshComponent;
class USpotLightComponent;
class UTextRenderComponent;
class UMaterialInstanceDynamic;

DECLARE_LOG_CATEGORY_EXTERN(LogModelIntegration, Log, All);

/*
 * A C++ model made compatible with coreDS Unreal through the Model Integration specification
 * (see "Automatic Object Support / Content provider support" in the coreDS Unreal user guide).
 *
 * Nothing here depends on coreDS Unreal: the model only exposes coreDS_* properties that the
 * plugin discovers by name. The editor's "Search for compatible assets" finds both this class and
 * BP_ModelIntegrationTank, a Blueprint deriving from it that only changes coreDS_EntityType, so the
 * C++ and the Blueprint integrations can be tested side by side.
 *
 * What the model exercises:
 *  - TurretAzimuth : UPROPERTY written directly by coreDS, then OnTurretAzimuthUpdated is called.
 *  - GunElevation  : read and written only through GetGunElevation / SetGunElevation.
 *  - DamagePercent : 0-100 model range converted by coreDS to the 0-3 DIS/RPR DamageState.
 *  - bHeadlightsOn : boolean appearance flag.
 *  - Articulated parts : primary turret #1 azimuth and primary gun #1 elevation.
 *  - Events : weapon fire and detonation.
 */
UCLASS(Blueprintable)
class COREDSMODELINTEGRATION_API AModelIntegrationTank : public AActor
{
	GENERATED_BODY()

public:
	AModelIntegrationTank();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	//////////////////////////////////////////////////////////////////////////
	// coreDS Model Integration variables (found by coreDS Unreal by name)

	// Entity types this model represents, most important first. Wildcards (*) are allowed.
	// For outgoing objects, the first entry is sent with its wildcards replaced by 0.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "coreDS")
	TArray<FString> coreDS_EntityType;

	// Variables coreDS Unreal can read and write on this model.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "coreDS")
	TArray<FModelIntegrationVariable_V1> coreDS_Variables;

	// Articulated and attached parts.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "coreDS")
	TArray<FModelIntegrationPart_V1> coreDS_Parts;

	// Appearance bit fields.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "coreDS")
	TArray<FModelIntegrationAppearance_V1> coreDS_Appearance;

	// When true, coreDS Unreal publishes this actor when it is spawned or placed in the level.
	// False on the class: only the instances set to true in the level are published. The copies
	// coreDS spawns for remote entities are never published back.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "coreDS")
	bool coreDS_Replicate = false;

	// Clamp remote copies of this model to the ground.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "coreDS")
	bool coreDS_SnapToGround = true;

	// Model Integration specification version ("X.Y.Z").
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "coreDS")
	FString coreDS_IntegrationVersion = TEXT("3.0.0");

	// UFUNCTION called on a weapon fire event: (FVector Location).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "coreDS")
	FString coreDS_Event_Fire = TEXT("OnCoreDSFire");

	// UFUNCTION called on a detonation event: (FVector Location, FVector LocationOnEntity).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "coreDS")
	FString coreDS_Event_Detonation = TEXT("OnCoreDSDetonation");

	//////////////////////////////////////////////////////////////////////////
	// Model state

	// Turret azimuth, in radians, relative to the hull.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model", meta = (UIMin = "-3.1416", UIMax = "3.1416"))
	float TurretAzimuth = 0.f;

	// Damage, from 0 (intact) to 100 (destroyed).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model", meta = (ClampMin = "0", ClampMax = "100"))
	int32 DamagePercent = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model")
	bool bHeadlightsOn = false;

	// When the model is not controlled by coreDS, drive in a circle, move the turret and gun,
	// increase the damage and toggle the headlights so every outgoing value changes over time.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model")
	bool bSimulateLocalActivity = true;

	// Radius of the circle driven by a local tank, in cm. The starting point on the circle is
	// random, so two instances of the sample running the same map do not overlap. 0 to stay still.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model", meta = (ClampMin = "0"))
	float DriveRadius = 1500.f;

	//////////////////////////////////////////////////////////////////////////
	// Functions referenced by coreDS_Variables

	UFUNCTION(BlueprintCallable, Category = "Model")
	void OnTurretAzimuthUpdated();

	UFUNCTION(BlueprintCallable, Category = "Model")
	void SetGunElevation(float NewElevation);

	UFUNCTION(BlueprintPure, Category = "Model")
	float GetGunElevation() const;

	UFUNCTION(BlueprintCallable, Category = "Model")
	void OnDamageUpdated();

	UFUNCTION(BlueprintCallable, Category = "Model")
	void OnHeadlightsUpdated();

	//////////////////////////////////////////////////////////////////////////
	// Functions referenced by coreDS_Event_Fire / coreDS_Event_Detonation

	UFUNCTION(BlueprintNativeEvent, Category = "Model")
	void OnCoreDSFire(FVector Location);

	UFUNCTION(BlueprintNativeEvent, Category = "Model")
	void OnCoreDSDetonation(FVector Location, FVector LocationOnEntity);

	// True when coreDS Unreal spawned this actor for a remote entity.
	UFUNCTION(BlueprintPure, Category = "Model")
	bool IsControlledByCoreDS() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> HullMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> TurretPivot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> TurretMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> GunPivot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> GunMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpotLightComponent> LeftHeadlight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpotLightComponent> RightHeadlight;

	// Shows whether this is the C++ class or a Blueprint, and whether coreDS controls it.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UTextRenderComponent> Label;

private:
	void BuildCoreDSTables();
	void UpdateLabel();
	void ApplyArticulations();
	void ApplyDamage();
	void ApplyHeadlights();

	// Gun elevation, in radians. Private: coreDS reaches it only through the getter/setter.
	float GunElevation = 0.f;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> HullMaterial;

	float LocalActivityTime = 0.f;
	FVector DriveCenter = FVector::ZeroVector;
	float DriveStartAngle = 0.f;

	// coreDS can tag the actor after BeginPlay, so the label is refreshed when this changes.
	bool bLabelShowsControlledByCoreDS = false;
};
