// Copyright 2021, Bc. Miroslav Kaèeriak. All Rights Reserved.

#include "ProceduralActor.h"
#include <ProceduralMeshComponent/Public/KismetProceduralMeshLibrary.h>
#include <iostream>
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

struct HeightMapType
{
    float x, y, z;
    float nx, ny, nz;
    float r, g, b;
};

constexpr unsigned int KINECT_DEPTH_WIDTH = 512;
constexpr unsigned int KINECT_DEPTH_HEIGHT = 424;
constexpr unsigned int KINECT_DEPTH_CAPACITY = KINECT_DEPTH_WIDTH * KINECT_DEPTH_HEIGHT;
constexpr unsigned int FRAME = KINECT_DEPTH_CAPACITY * sizeof(UINT16);

constexpr unsigned int MAX = 7908;
HeightMapType* HEIGHTMAP;

FString absoluteFilePath = FPaths::ProjectDir() + TEXT("kinectDepthData_0.raw");
std::string stringPath = std::string(TCHAR_TO_UTF8(*absoluteFilePath));

inline float Normalize(float val, int max, int min) { return (val - min) / (max - min); }

// Sets default values
AProceduralActor::AProceduralActor()
{
	//F1 for wireframe in game

	tick = false;
	PrimaryActorTick.bCanEverTick = tick;

	CreateEditorPlaceHolder();
	InitializeInGameMesh();
}


void AProceduralActor::CreateEditorPlaceHolder()
{
	editorMash = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Placeholder"));

	ConstructorHelpers::FObjectFinder<UStaticMesh>	meshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Plane'"));
	ConstructorHelpers::FObjectFinder<UMaterial>	material(TEXT("Material'/Game/Materials/Material'"));

	editorMash->SetStaticMesh(meshAsset.Object);
	editorMash->GetStaticMesh()->SetMaterial(0, material.Object);
	
	RootComponent = editorMash;
}

void AProceduralActor::LoadHeightMap()
{
    int error, i, j, index;
    FILE* filePtr;
    unsigned long long count;
    UINT16* rawImage;

    HEIGHTMAP = new HeightMapType[KINECT_DEPTH_WIDTH * KINECT_DEPTH_HEIGHT];
    if (!HEIGHTMAP)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("!HEIGHTMAP"));
        return;
    }

    // Open the 16 bit raw height map file for reading in binary.
    error = fopen_s(&filePtr, stringPath.c_str(), "rb");
    if (error != 0)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("ERR in opening " + absoluteFilePath));
        return;
    }

    // Allocate memory for the raw image data.
    rawImage = new UINT16[KINECT_DEPTH_CAPACITY];
    if (!rawImage)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("!rawImage"));
        return;
    }

    // Read in the raw image data.
    count = fread(rawImage, sizeof(UINT16), KINECT_DEPTH_CAPACITY, filePtr);
    if (count != KINECT_DEPTH_CAPACITY)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("count != imageSize"));
        return;
    }

    // Close the file.
    error = fclose(filePtr);
    if (error != 0)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("error during closing"));
        return;
    }

    // Copy the image data into the height map array.
    for (j = 0; j < KINECT_DEPTH_HEIGHT; j++)
    {
        for (i = 0; i < KINECT_DEPTH_WIDTH; i++)
        {
            index = (KINECT_DEPTH_WIDTH * j) + i;

            // Store the height at this point in the height map array.
            HEIGHTMAP[index].y = (float)rawImage[index];
        }
    }

    // Release the bitmap image data.
    delete[] rawImage;
    rawImage = 0;
}

void AProceduralActor::InitializeInGameMesh()
{
	mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeneratedMesh"));
	mesh->bUseAsyncCooking = true; // New in UE 4.17, multi-threaded PhysX cooking.
}

void AProceduralActor::CreateMesh()
{
	RootComponent = mesh;
	editorMash->DestroyComponent();

	FVector item;
    for (int j = 0; j < KINECT_DEPTH_HEIGHT; j++)
    {
        for (int i = 0; i < KINECT_DEPTH_WIDTH; i++)
        {
            int index = (KINECT_DEPTH_WIDTH * j) + i;

            item.X = i * LengthMultiplicator;
            item.Y = j * LengthMultiplicator;
            item.Z = Normalize(HEIGHTMAP[index].y, MAX, 0) * HeightMultiplicator; //static_cast<float>((rand() / static_cast<float>(RAND_MAX)) * HeightMultiplicator);

            vertices.Add(item);
        }
    }

 //   int w = 20, h = 30;
	//for (int i = 0; i < w; i++)
	//{
	//	for (int j = 0; j < h; j++)
	//	{
 //           item.X = i * LengthMultiplicator;
 //           item.Y = j * LengthMultiplicator;
	//		//item.Z = static_cast<float>((rand() / static_cast<float>(RAND_MAX)) * HeightMultiplicator);

	//		vertices.Add(item);
	//	}
	//}

	UpdateMesh();
}

void AProceduralActor::UpdateMesh()
{
	UKismetProceduralMeshLibrary::CreateGridMeshTriangles(KINECT_DEPTH_HEIGHT, KINECT_DEPTH_WIDTH, true, triangles);

	mesh->CreateMeshSection_LinearColor(0, vertices, triangles, normals, uvs, vertexColors, tangents, true);
	mesh->ContainsPhysicsTriMeshData(true); // Enable collision data
}


// Called when the game starts or when spawned
void AProceduralActor::BeginPlay()
{
	Super::BeginPlay();
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "BeginPlay");

	PrimaryActorTick.SetTickFunctionEnable(tick);

    LoadHeightMap();
	CreateMesh();
}
// Called every frame
void AProceduralActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	currentTime += DeltaTime;

	if (currentTime >= updateInterval)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Tick"));
		currentTime = 0.0f;

		int length = vertices.Num();

		for (int i = 0; i < length; i++)
		{
			vertices[i].Z = static_cast<float>((rand() / static_cast<float>(RAND_MAX)) * HeightMultiplicator);
		}

		UpdateMesh();
	}
}
