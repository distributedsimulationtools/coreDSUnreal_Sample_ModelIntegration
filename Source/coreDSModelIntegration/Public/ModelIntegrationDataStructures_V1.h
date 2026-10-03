/* Copyright (C) Consultants 2J's, Inc - operating under the name ds.tools
 * coreDS Unreal sample - Model Integration
 */

#pragma once

#include "CoreMinimal.h"
#include "ModelIntegrationDataStructures_V1.generated.h"

/*
 * Local copies of the coreDS Model Integration data structures (specification V1).
 *
 * This is the "loose coupling" approach: the model does not depend on the coreDS Unreal
 * modules, so it can be distributed and used without the plugin. coreDS Unreal finds the
 * tables by property name (coreDS_Variables, coreDS_Parts, coreDS_Appearance) and reads
 * the rows through reflection.
 *
 * Keep the field names, types and order identical to coreDSActorGenericArticulations_datatypes_V1.h:
 * coreDS Unreal reads coreDS_Variables and coreDS_Appearance as arrays of its own
 * FcoreDSVariablesDataStructure_V1 / FcoreDSAppearanceDataStructure_V1, so the memory
 * layout must match, not only the names.
 */

UENUM(BlueprintType)
enum class EModelIntegrationDatatype_V1 : uint8
{
	None			UMETA(DisplayName = "None"),
	Float			UMETA(DisplayName = "Float"),
	Double			UMETA(DisplayName = "Double"),
	MAX				UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EModelIntegrationResolutionMode_V1 : uint8
{
	Round			UMETA(DisplayName = "Round"),
	Floor			UMETA(DisplayName = "Floor"),
	Ceil			UMETA(DisplayName = "Ceil"),
	MAX				UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EModelIntegrationUpdateRate_V1 : uint8
{
	Important		UMETA(DisplayName = "Important"),
	Average			UMETA(DisplayName = "Average"),
	Low				UMETA(DisplayName = "Low"),
	Manual			UMETA(DisplayName = "Manual"),
	MAX				UMETA(Hidden)
};

// One row of coreDS_Variables: a model variable coreDS Unreal can read and write.
USTRUCT(BlueprintType)
struct FModelIntegrationVariable_V1
{
	GENERATED_BODY()

	// Generic name of the variable. coreDS_Parts and coreDS_Appearance refer to the variable by this name.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Variables)
	FString Name;

	// UPROPERTY of the actor read and written by coreDS Unreal. The getter/setter take precedence.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Variables)
	FString VariableName;

	// UFUNCTION without parameter called after the value was set.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Variables)
	FString FunctionCallbackName;

	// UFUNCTION taking a single parameter, used to set the value.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Variables)
	FString FunctionSetterName;

	// UFUNCTION returning a single value, used to read the value.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Variables)
	FString FunctionGetterName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Variables)
	float ValueRangeMin = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Variables)
	float ValueRangeMax = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Variables)
	float DefaultValue = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Variables)
	EModelIntegrationUpdateRate_V1 UpdateRate = EModelIntegrationUpdateRate_V1::Average;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Variables)
	EModelIntegrationResolutionMode_V1 ResolutionMode = EModelIntegrationResolutionMode_V1::Round;
};

// One row of coreDS_Parts: an articulated or attached part driven by a coreDS_Variables entry.
USTRUCT(BlueprintType)
struct FModelIntegrationPart_V1
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Parts, meta = (ToolTip = "Generic name so you know what the part is"))
	FString PartName;

	// Name of the coreDS_Variables row driving this part.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Parts)
	FString VariableName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Parts)
	EModelIntegrationDatatype_V1 Datatype = EModelIntegrationDatatype_V1::None;

	// SISO-REF-010 "Articulated Part Type Class" (e.g. 4096 primary turret #1).
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Parts)
	int32 PartEnumeration = 0;

	// SISO-REF-010 "Articulated Part Type Metric" (e.g. 11 azimuth, 13 elevation).
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Parts)
	int32 PartType = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Parts)
	int32 PartAttachedTo = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Parts)
	int32 PartAttachmentType = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Parts)
	float ValueRangeMin = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Parts)
	float ValueRangeMax = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Parts, meta = (ToolTip = "When set to true, the value sign is reversed"))
	bool ReverseDirection = false;
};

// One row of coreDS_Appearance: an appearance bit field driven by a coreDS_Variables entry.
USTRUCT(BlueprintType)
struct FModelIntegrationAppearance_V1
{
	GENERATED_BODY()

	// RPR-FOM name of the appearance field (e.g. DamageState).
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Appearance)
	FString AppearanceName;

	// Name of the coreDS_Variables row holding the value.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Appearance)
	FString VariableName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Appearance)
	int32 BitIndex = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Appearance)
	int32 BitFieldLength = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Appearance)
	int32 DefaultState = 0;
};
