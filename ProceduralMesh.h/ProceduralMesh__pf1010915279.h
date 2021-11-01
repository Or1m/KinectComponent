#pragma once
#include "Blueprint/BlueprintSupport.h"
#include "Runtime/CoreUObject/Public/UObject/NoExportTypes.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"
class UProceduralMeshComponent;
class USceneComponent;
#include "ProceduralMesh__pf1010915279.generated.h"
UCLASS(config=Engine, Blueprintable, BlueprintType, meta=(ReplaceConverted="/Game/ProceduralMesh.ProceduralMesh_C", OverrideNativeName="ProceduralMesh_C"))
class AProceduralMesh_C__pf1010915279 : public AActor
{
public:
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, NonTransactional, meta=(Category="Default", OverrideNativeName="ProceduralMesh"))
	UProceduralMeshComponent* bpv__ProceduralMesh__pf;
	UPROPERTY(BlueprintReadWrite, NonTransactional, meta=(Category="Default", OverrideNativeName="DefaultSceneRoot"))
	USceneComponent* bpv__DefaultSceneRoot__pf;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="Width", Category="Default", MultiLine="true", OverrideNativeName="Width"))
	int32 bpv__Width__pf;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta=(DisplayName="X Index", Category="Default", MultiLine="true", OverrideNativeName="X Index"))
	int32 bpv__XxIndex__pfT;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta=(DisplayName="Y Index", Category="Default", MultiLine="true", OverrideNativeName="Y Index"))
	int32 bpv__YxIndex__pfT;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta=(DisplayName="Vertices", Category="Default", MultiLine="true", OverrideNativeName="Vertices"))
	TArray<FVector> bpv__Vertices__pf;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta=(DisplayName="Triangles", Category="Default", MultiLine="true", OverrideNativeName="Triangles"))
	TArray<int32> bpv__Triangles__pf;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="Length", Category="Default", MultiLine="true", OverrideNativeName="Length"))
	float bpv__Length__pf;
	AProceduralMesh_C__pf1010915279(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void PostLoadSubobjects(FObjectInstancingGraph* OuterInstanceGraph) override;
	static void __CustomDynamicClassInitialization(UDynamicClass* InDynamicClass);
	static void __StaticDependenciesAssets(TArray<FBlueprintDependencyData>& AssetsToLoad);
	static void __StaticDependencies_DirectlyUsedAssets(TArray<FBlueprintDependencyData>& AssetsToLoad);
	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly="true", Comment="/**\t * Construction script, the place to spawn components and do other setup.\t * @note Name used in CreateBlueprint function\t */", DisplayName="Construction Script", ToolTip="Construction script, the place to spawn components and do other setup.@note Name used in CreateBlueprint function", Category, CppFromBpEvent, OverrideNativeName="UserConstructionScript"))
	virtual void bpf__UserConstructionScript__pf();
public:
};
