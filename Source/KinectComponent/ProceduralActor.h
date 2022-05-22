#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#endif

#include <Kinect.h>
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
	bool Initialize();
	void Shutdown();
	
	void CreateEditorPlaceHolder();
	void InitializeInGameMesh();

	void CreateTerrainMesh();
	void UpdateTerrainMesh();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

	inline float Normalize(const float value);
	inline float Bilinear(const float tx, const float ty, const InterpolatedPoints& points);

	template<typename T>
	void SafeRelease(T& ptr) { if (ptr) { ptr->Release(); ptr = nullptr; } }
	
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

	IDepthFrameReader* m_depthFrameReader = nullptr;
	IKinectSensor* m_sensor = nullptr;
	int depthWidth = 0, depthHeight = 0;

	UINT16* rawImage;
	FILE* filePtr;

	bool tick;
};
