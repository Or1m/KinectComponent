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

	const FString FileName = "kinectDepthData_0.raw";

	// Kinect constants
	static constexpr unsigned int MIN = 0;
	static constexpr unsigned int MAX = 8192; // 2^13

	static constexpr unsigned int KINECT_DEPTH_WIDTH = 512;
	static constexpr unsigned int KINECT_DEPTH_HEIGHT = 424;

	static constexpr unsigned int KINECT_DEPTH_CAPACITY = KINECT_DEPTH_WIDTH * KINECT_DEPTH_HEIGHT;
	static constexpr unsigned int FRAME = KINECT_DEPTH_CAPACITY * sizeof(UINT16);

	struct InterpolatedPoints
	{
		UINT16 p00;
		UINT16 p10;
		UINT16 p01;
		UINT16 p11;
	};

protected:
	void CreateEditorPlaceHolder();
	void InitializeInGameMesh();

	bool LoadHeightMap();
	bool UnloadHeightMap();

	void CreateTerrainMesh();
	void UpdateTerrainMesh();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

	inline float Normalize(const float value);
	inline float Bilinear(const float tx, const float ty, const InterpolatedPoints& points);
	
private:
	UPROPERTY(EditAnywhere, Category = Attributes)
		int terrainWidth = 480;
	UPROPERTY(EditAnywhere, Category = Attributes)
		int terrainHeight = 640;

	UPROPERTY(EditAnywhere, Category = Attributes)
		int heightMultiplicator = 30;
	UPROPERTY(EditAnywhere, Category = Attributes)
		int lengthMultiplicator = 100;

	UPROPERTY(EditAnywhere, Category = Debug)
		float updateInterval = 1.0f;

	UPROPERTY()
		UProceduralMeshComponent* mesh;
	UPROPERTY()
		UStaticMeshComponent* editorMesh;
	UPROPERTY()
		UMaterial* terrainMaterial;

	UPROPERTY()
		TArray<FVector> terrainVertices;
	UPROPERTY()
		TArray<FVector> normals;
	UPROPERTY()
		TArray<int32> triangles;
	UPROPERTY()
		TArray<FVector2D> uvs;
	UPROPERTY()
		TArray<FColor> vertexColors;
	UPROPERTY()
		TArray<FProcMeshTangent> tangents;


	UINT16* rawImage;
	FILE* filePtr;

	float currentTime = 0.0f;
	unsigned long long count;
	bool tick;
};
