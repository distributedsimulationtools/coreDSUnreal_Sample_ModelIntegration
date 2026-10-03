/* Copyright (C) Consultants 2J's, Inc - operating under the name ds.tools
 * coreDS Unreal sample - Model Integration
 */

#include "ModelIntegrationTank.h"

#include "Components/SceneComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

DEFINE_LOG_CATEGORY(LogModelIntegration);

namespace
{
	// Ranges declared in coreDS_Variables, in the model's own units.
	constexpr float TurretAzimuthMin = -UE_PI;
	constexpr float TurretAzimuthMax = UE_PI;
	constexpr float GunElevationMin = -0.17f;	// about -10 degrees
	constexpr float GunElevationMax = 0.35f;	// about 20 degrees

	constexpr float DriveSpeed = 300.f;		// cm/s

	// SISO-REF-010 articulated parts
	constexpr int32 PrimaryTurret1 = 4096;
	constexpr int32 PrimaryGun1 = 4416;
	constexpr int32 MetricAzimuth = 11;
	constexpr int32 MetricElevation = 13;

	// DIS land platform appearance record
	constexpr int32 DamageBitIndex = 3;
	constexpr int32 DamageBitLength = 2;
	constexpr int32 HeadlightsBitIndex = 12;
	constexpr int32 HeadlightsBitLength = 1;

	const FName ColorParameterName(TEXT("Color"));

	FModelIntegrationVariable_V1 MakeVariable(const TCHAR* Name, const TCHAR* VariableName, const TCHAR* Callback,
		const TCHAR* Setter, const TCHAR* Getter, float Min, float Max, EModelIntegrationUpdateRate_V1 UpdateRate)
	{
		FModelIntegrationVariable_V1 Variable;
		Variable.Name = Name;
		Variable.VariableName = VariableName;
		Variable.FunctionCallbackName = Callback;
		Variable.FunctionSetterName = Setter;
		Variable.FunctionGetterName = Getter;
		Variable.ValueRangeMin = Min;
		Variable.ValueRangeMax = Max;
		Variable.UpdateRate = UpdateRate;
		Variable.ResolutionMode = EModelIntegrationResolutionMode_V1::Round;
		return Variable;
	}

	FModelIntegrationPart_V1 MakePart(const TCHAR* PartName, const TCHAR* VariableName, int32 PartEnumeration,
		int32 PartType, float Min, float Max)
	{
		FModelIntegrationPart_V1 Part;
		Part.PartName = PartName;
		Part.VariableName = VariableName;
		Part.Datatype = EModelIntegrationDatatype_V1::Float;
		Part.PartEnumeration = PartEnumeration;
		Part.PartType = PartType;
		Part.ValueRangeMin = Min;
		Part.ValueRangeMax = Max;
		return Part;
	}

	FModelIntegrationAppearance_V1 MakeAppearance(const TCHAR* AppearanceName, const TCHAR* VariableName,
		int32 BitIndex, int32 BitFieldLength)
	{
		FModelIntegrationAppearance_V1 Appearance;
		Appearance.AppearanceName = AppearanceName;
		Appearance.VariableName = VariableName;
		Appearance.BitIndex = BitIndex;
		Appearance.BitFieldLength = BitFieldLength;
		return Appearance;
	}
}

AModelIntegrationTank::AModelIntegrationTank()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));

	// Everything is movable: coreDS moves remote copies, and a local tank drives around.
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Root->SetMobility(EComponentMobility::Movable);
	SetRootComponent(Root);

	// The basic shapes are 100 cm wide and centered on their pivot.
	HullMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Hull"));
	HullMesh->SetupAttachment(Root);
	HullMesh->SetStaticMesh(CubeMesh.Object);
	HullMesh->SetRelativeLocation(FVector(0.f, 0.f, 80.f));
	HullMesh->SetRelativeScale3D(FVector(6.f, 3.4f, 1.2f));
	HullMesh->SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);

	// The pivots are unscaled so the turret and gun rotate around the right point.
	TurretPivot = CreateDefaultSubobject<USceneComponent>(TEXT("TurretPivot"));
	TurretPivot->SetupAttachment(Root);
	TurretPivot->SetRelativeLocation(FVector(-40.f, 0.f, 165.f));

	TurretMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Turret"));
	TurretMesh->SetupAttachment(TurretPivot);
	TurretMesh->SetStaticMesh(CylinderMesh.Object);
	TurretMesh->SetRelativeScale3D(FVector(2.4f, 2.4f, 0.5f));
	TurretMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GunPivot = CreateDefaultSubobject<USceneComponent>(TEXT("GunPivot"));
	GunPivot->SetupAttachment(TurretPivot);
	GunPivot->SetRelativeLocation(FVector(110.f, 0.f, 0.f));

	GunMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Gun"));
	GunMesh->SetupAttachment(GunPivot);
	GunMesh->SetStaticMesh(CylinderMesh.Object);
	GunMesh->SetRelativeLocation(FVector(150.f, 0.f, 0.f));
	GunMesh->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
	GunMesh->SetRelativeScale3D(FVector(0.14f, 0.14f, 3.f));
	GunMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	LeftHeadlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("LeftHeadlight"));
	LeftHeadlight->SetupAttachment(Root);
	LeftHeadlight->SetMobility(EComponentMobility::Movable);
	LeftHeadlight->SetRelativeLocation(FVector(305.f, -120.f, 100.f));
	LeftHeadlight->SetIntensity(20000.f);
	LeftHeadlight->SetOuterConeAngle(30.f);

	RightHeadlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("RightHeadlight"));
	RightHeadlight->SetupAttachment(Root);
	RightHeadlight->SetMobility(EComponentMobility::Movable);
	RightHeadlight->SetRelativeLocation(FVector(305.f, 120.f, 100.f));
	RightHeadlight->SetIntensity(20000.f);
	RightHeadlight->SetOuterConeAngle(30.f);

	// Above the turret, readable from behind the tank.
	Label = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Label"));
	Label->SetupAttachment(Root);
	Label->SetMobility(EComponentMobility::Movable);
	Label->SetRelativeLocation(FVector(0.f, 0.f, 400.f));
	Label->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
	Label->SetHorizontalAlignment(EHTA_Center);
	Label->SetWorldSize(60.f);
	Label->SetTextRenderColor(FColor::White);

	BuildCoreDSTables();
}

void AModelIntegrationTank::BuildCoreDSTables()
{
	// M1A2 Abrams first, then any US tank, then any land platform tank.
	coreDS_EntityType = {
		TEXT("1.1.225.1.1.3.*"),
		TEXT("1.1.225.1.*.*.*"),
		TEXT("1.1.*.1.*.*.*")
	};

	coreDS_Variables = {
		// UPROPERTY written directly, then the callback updates the visuals.
		MakeVariable(TEXT("TurretAzimuth"), TEXT("TurretAzimuth"), TEXT("OnTurretAzimuthUpdated"), TEXT(""), TEXT(""),
			TurretAzimuthMin, TurretAzimuthMax, EModelIntegrationUpdateRate_V1::Important),

		// No UPROPERTY: the getter and setter are the only access.
		MakeVariable(TEXT("GunElevation"), TEXT(""), TEXT(""), TEXT("SetGunElevation"), TEXT("GetGunElevation"),
			GunElevationMin, GunElevationMax, EModelIntegrationUpdateRate_V1::Important),

		// Model range 0-100, converted by coreDS to the 0-3 DamageState of the appearance.
		MakeVariable(TEXT("Damage"), TEXT("DamagePercent"), TEXT("OnDamageUpdated"), TEXT(""), TEXT(""),
			0.f, 100.f, EModelIntegrationUpdateRate_V1::Low),

		MakeVariable(TEXT("Headlights"), TEXT("bHeadlightsOn"), TEXT("OnHeadlightsUpdated"), TEXT(""), TEXT(""),
			0.f, 1.f, EModelIntegrationUpdateRate_V1::Average)
	};

	coreDS_Parts = {
		MakePart(TEXT("PrimaryTurret1_Azimuth"), TEXT("TurretAzimuth"), PrimaryTurret1, MetricAzimuth,
			TurretAzimuthMin, TurretAzimuthMax),
		MakePart(TEXT("PrimaryGun1_Elevation"), TEXT("GunElevation"), PrimaryGun1, MetricElevation,
			GunElevationMin, GunElevationMax)
	};

	coreDS_Appearance = {
		MakeAppearance(TEXT("DamageState"), TEXT("Damage"), DamageBitIndex, DamageBitLength),
		MakeAppearance(TEXT("HeadLightsOn"), TEXT("Headlights"), HeadlightsBitIndex, HeadlightsBitLength)
	};
}

void AModelIntegrationTank::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (!HullMaterial && HullMesh->GetMaterial(0))
	{
		HullMaterial = HullMesh->CreateDynamicMaterialInstance(0);
	}

	ApplyArticulations();
	ApplyDamage();
	ApplyHeadlights();
	UpdateLabel();
}

void AModelIntegrationTank::BeginPlay()
{
	Super::BeginPlay();

	DriveCenter = GetActorLocation();
	DriveStartAngle = FMath::FRandRange(0.f, 2.f * UE_PI);

	UE_LOG(LogModelIntegration, Log, TEXT("%s: BeginPlay, %s"), *GetName(),
		IsControlledByCoreDS() ? TEXT("controlled by coreDS (remote entity)") :
		coreDS_Replicate ? TEXT("local entity published by coreDS") : TEXT("local entity, not published"));
}

void AModelIntegrationTank::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (IsControlledByCoreDS() != bLabelShowsControlledByCoreDS)
	{
		UpdateLabel();
	}

	// A remote entity is driven by coreDS only.
	if (IsControlledByCoreDS() || !bSimulateLocalActivity)
	{
		return;
	}

	LocalActivityTime += DeltaSeconds;

	if (DriveRadius > 0.f)
	{
		// The hull faces the direction of travel.
		const float Angle = DriveStartAngle + LocalActivityTime * DriveSpeed / DriveRadius;
		const FVector Offset(FMath::Cos(Angle) * DriveRadius, FMath::Sin(Angle) * DriveRadius, 0.f);
		SetActorLocationAndRotation(DriveCenter + Offset, FRotator(0.f, FMath::RadiansToDegrees(Angle) + 90.f, 0.f));
	}

	TurretAzimuth = FMath::Sin(LocalActivityTime * 0.3f) * TurretAzimuthMax * 0.75f;
	GunElevation = FMath::Lerp(GunElevationMin, GunElevationMax, 0.5f + 0.5f * FMath::Sin(LocalActivityTime * 0.7f));
	ApplyArticulations();

	// 0 to 100 over 40 seconds, then repairs.
	const int32 NewDamage = FMath::FloorToInt(FMath::Fmod(LocalActivityTime, 40.f) * 2.5f);
	if (NewDamage != DamagePercent)
	{
		DamagePercent = NewDamage;
		ApplyDamage();
	}

	const bool bNewHeadlights = FMath::Fmod(LocalActivityTime, 10.f) >= 5.f;
	if (bNewHeadlights != bHeadlightsOn)
	{
		bHeadlightsOn = bNewHeadlights;
		ApplyHeadlights();
	}
}

void AModelIntegrationTank::OnTurretAzimuthUpdated()
{
	UE_LOG(LogModelIntegration, Verbose, TEXT("%s: TurretAzimuth set by coreDS to %f"), *GetName(), TurretAzimuth);
	ApplyArticulations();
}

void AModelIntegrationTank::SetGunElevation(float NewElevation)
{
	UE_LOG(LogModelIntegration, Verbose, TEXT("%s: GunElevation set by coreDS to %f"), *GetName(), NewElevation);
	GunElevation = FMath::Clamp(NewElevation, GunElevationMin, GunElevationMax);
	ApplyArticulations();
}

float AModelIntegrationTank::GetGunElevation() const
{
	return GunElevation;
}

void AModelIntegrationTank::OnDamageUpdated()
{
	UE_LOG(LogModelIntegration, Verbose, TEXT("%s: DamagePercent set by coreDS to %d"), *GetName(), DamagePercent);
	ApplyDamage();
}

void AModelIntegrationTank::OnHeadlightsUpdated()
{
	UE_LOG(LogModelIntegration, Verbose, TEXT("%s: bHeadlightsOn set by coreDS to %d"), *GetName(), bHeadlightsOn);
	ApplyHeadlights();
}

void AModelIntegrationTank::OnCoreDSFire_Implementation(FVector Location)
{
	UE_LOG(LogModelIntegration, Log, TEXT("%s: fire event at %s"), *GetName(), *Location.ToString());
	DrawDebugSphere(GetWorld(), Location, 50.f, 12, FColor::Orange, false, 2.f);
}

void AModelIntegrationTank::OnCoreDSDetonation_Implementation(FVector Location, FVector LocationOnEntity)
{
	UE_LOG(LogModelIntegration, Log, TEXT("%s: detonation event at %s (on entity %s)"), *GetName(),
		*Location.ToString(), *LocationOnEntity.ToString());
	DrawDebugSphere(GetWorld(), Location, 150.f, 16, FColor::Red, false, 3.f);
}

bool AModelIntegrationTank::IsControlledByCoreDS() const
{
	return ActorHasTag(TEXT("coreDSCreated"));
}

void AModelIntegrationTank::ApplyArticulations()
{
	TurretPivot->SetRelativeRotation(FRotator(0.f, FMath::RadiansToDegrees(TurretAzimuth), 0.f));
	GunPivot->SetRelativeRotation(FRotator(FMath::RadiansToDegrees(GunElevation), 0.f, 0.f));
}

void AModelIntegrationTank::ApplyDamage()
{
	if (!HullMaterial)
	{
		return;
	}

	// Green (intact) to red (heavily damaged), black when destroyed.
	const float Damage = FMath::Clamp(DamagePercent, 0, 100) / 100.f;
	const FLinearColor Color = DamagePercent >= 100
		? FLinearColor(0.02f, 0.02f, 0.02f)
		: FLinearColor::LerpUsingHSV(FLinearColor(0.1f, 0.4f, 0.1f), FLinearColor(0.6f, 0.05f, 0.02f), Damage);
	HullMaterial->SetVectorParameterValue(ColorParameterName, Color);
}

void AModelIntegrationTank::UpdateLabel()
{
	bLabelShowsControlledByCoreDS = IsControlledByCoreDS();

	// A Blueprint deriving from this class has a non-native generated class.
	const bool bIsCppClass = GetClass()->HasAnyClassFlags(CLASS_Native);
	Label->SetText(FText::FromString(FString::Printf(TEXT("%s\n%s"),
		bIsCppClass ? TEXT("C++ class") : TEXT("Blueprint"),
		bLabelShowsControlledByCoreDS ? TEXT("remote (coreDS)") : coreDS_Replicate ? TEXT("local, published") : TEXT("local"))));
}

void AModelIntegrationTank::ApplyHeadlights()
{
	LeftHeadlight->SetVisibility(bHeadlightsOn);
	RightHeadlight->SetVisibility(bHeadlightsOn);
}
