// Fill out your copyright notice in the Description page of Project Settings.

#include "ProceduralActor.h"
#include <ProceduralMeshComponent/Public/KismetProceduralMeshLibrary.h>
#include "Components/SphereComponent.h"

// Sets default values
AProceduralActor::AProceduralActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	USphereComponent* SphereComponent = CreateEditorOnlyDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	SphereComponent->InitSphereRadius(30);
	RootComponent = SphereComponent;

	mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeneratedMesh"));
	// New in UE 4.17, multi-threaded PhysX cooking.
	mesh->bUseAsyncCooking = true;
}

// This is called when actor is spawned (at runtime or when you drop it into the world in editor)
void AProceduralActor::PostActorCreated()
{
	Super::PostActorCreated();
	//CreateMesh();
}

// This is called when actor is already in level and map is opened
void AProceduralActor::PostLoad()
{
	Super::PostLoad();
	//CreateMesh();
}

void AProceduralActor::CreateMesh()
{
	RootComponent = mesh;

	FVector item;
	for (int i = -Width; i <= Width; i++) 
	{
		for (int j = -Length; j <= Length; j++) 
		{
			item.X = i * LengthMultiplicator;
			item.Y = j * LengthMultiplicator;
			item.Z = static_cast<float>((rand() / static_cast<float>(RAND_MAX)) * HeightMultiplicator);

			vertices.Add(item);
		}
	}

	UpdateMesh();
}

void AProceduralActor::UpdateMesh()
{
	int numXY = (Width * 2) + 1;
	UKismetProceduralMeshLibrary::CreateGridMeshTriangles(numXY, numXY, false, triangles);

	mesh->CreateMeshSection_LinearColor(0, vertices, triangles, normals, uvs, vertexColors, tangents, true);

	// Enable collision data
	mesh->ContainsPhysicsTriMeshData(true);
}


// Called when the game starts or when spawned
void AProceduralActor::BeginPlay()
{
	Super::BeginPlay();
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "BeginPlay");

	CreateMesh();
}

// Called every frame
void AProceduralActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	currentTime += DeltaTime;

	if (currentTime >= interval) 
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Tick"));
		currentTime = 0.0f;

		int length = vertices.Num();

		for (int i = 0; i < length; i++)
		{
			vertices[i].Z = static_cast<float>((rand() / static_cast<float>(RAND_MAX)) * HeightMultiplicator);
		}

		UpdateMesh();
	}
}
