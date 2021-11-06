// Fill out your copyright notice in the Description page of Project Settings.

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

protected:
	void PostActorCreated();
	void PostLoad();

	void CreateMesh();
	void UpdateMesh();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY()
		UProceduralMeshComponent* mesh;

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
	float interval = 1.0f;
};
