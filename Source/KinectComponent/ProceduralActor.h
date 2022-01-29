// Copyright 2021, Bc. Miroslav Kaèeriak. All Rights Reserved.

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
	// Sets default values for this actor's properties
	AProceduralActor();

	UPROPERTY(EditAnywhere, Category = Attributes)
		int Width = 640;
	UPROPERTY(EditAnywhere, Category = Attributes)
		int Length = 480;
	UPROPERTY(EditAnywhere, Category = Attributes)
		int HeightMultiplicator = 30;
	UPROPERTY(EditAnywhere, Category = Attributes)
		int LengthMultiplicator = 100;
	UPROPERTY(EditAnywhere, Category = Debug)
		float updateInterval = 1.0f;

protected:
	bool LoadHeightMap();
	bool UnloadHeightMap();
	void InitializeInGameMesh();
	void CreateMesh();
	void UpdateMesh();

	void CreateEditorPlaceHolder();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Called when the game starts or when spawned
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY()
		UProceduralMeshComponent* mesh;
	UPROPERTY()
		UStaticMeshComponent* editorMash;

	UPROPERTY()
		TArray<FVector> vertices;
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

	float currentTime = 0.0f;
	bool tick;
};
