#pragma once



#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "ProceduralActor.generated.h"

UCLASS()
class KINECTCOMPONENT_API AProceduralActor : public AActor
{
	GENERATED_BODY()
	
public:
	AProceduralActor();

	UPROPERTY(EditAnywhere, Category = Attributes)
		int TerrainWidth = 480;
	UPROPERTY(EditAnywhere, Category = Attributes)
		int TerrainHeight = 640;

	UPROPERTY(EditAnywhere, Category = Attributes)
		int HeightMultiplicator = 30;
	UPROPERTY(EditAnywhere, Category = Attributes)
		int LengthMultiplicator = 100;
	UPROPERTY(EditAnywhere, Category = Debug)
		float updateInterval = 1.0f;

	const FString fileName = "kinectDepthData_0.raw";

	// Kinect constants
	static constexpr unsigned int MIN = 0;
	static constexpr unsigned int MAX = 8192; // 2^13

	static constexpr unsigned int KINECT_DEPTH_WIDTH = 512;
	static constexpr unsigned int KINECT_DEPTH_HEIGHT = 424;

	static constexpr unsigned int KINECT_DEPTH_CAPACITY = KINECT_DEPTH_WIDTH * KINECT_DEPTH_HEIGHT;
	static constexpr unsigned int FRAME = KINECT_DEPTH_CAPACITY * sizeof(UINT16);

protected:
	void CreateEditorPlaceHolder();
	void InitializeInGameMesh();

	bool LoadHeightMap();
	bool UnloadHeightMap();

	void CreateMesh();
	void UpdateMesh();
	void UpdateTerrainHeight();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

	inline float Normalize(const float value);
	inline float Bilinear(const float& tx, const float& ty, const UINT16& c00, const UINT16& c10, const UINT16& c01, const UINT16& c11);
	
private:
	UPROPERTY()
		UProceduralMeshComponent* mesh;
	UPROPERTY()
		UStaticMeshComponent* editorMesh;

	UPROPERTY()
		TArray<FVector> terrainVertices;
	UPROPERTY()
		TArray<FVector> normals;
	UPROPERTY()
		TArray<int32> triangles;
	UPROPERTY()
		TArray<FVector2D> uvs;
	UPROPERTY()
		TArray<FLinearColor> vertexColors;
	UPROPERTY()
		TArray<FProcMeshTangent> tangents;

	UINT16* rawImage;
	FILE* filePtr;

	float currentTime = 0.0f;
	unsigned long long count;

	bool tick;
};
