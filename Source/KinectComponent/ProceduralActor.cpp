#include "ProceduralActor.h"

#include <ProceduralMeshComponent/Public/KismetProceduralMeshLibrary.h>
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

//Press F1 in game for wireframe view
AProceduralActor::AProceduralActor()
{
	tick = true;
	PrimaryActorTick.bCanEverTick = tick;

	CreateEditorPlaceHolder();
	InitializeInGameMesh();
}


void AProceduralActor::CreateEditorPlaceHolder()
{
#if WITH_EDITOR
	editorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Placeholder"));

	ConstructorHelpers::FObjectFinder<UStaticMesh> meshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Plane'"));
	ConstructorHelpers::FObjectFinder<UMaterial> material(TEXT("Material'/Game/Materials/Material'"));

	editorMesh->SetStaticMesh(meshAsset.Object);
	editorMesh->GetStaticMesh()->SetMaterial(0, material.Object);
	
	RootComponent = editorMesh;
#endif
}

void AProceduralActor::InitializeInGameMesh()
{
    mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeneratedMesh"));
    mesh->bUseAsyncCooking = true; // New in UE 4.17, multi-threaded PhysX cooking.
}


void AProceduralActor::BeginPlay()
{
    Super::BeginPlay();

    PrimaryActorTick.SetTickFunctionEnable(tick);

    if (LoadHeightMap()) 
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Loaded");
        CreateMesh();
    }
}

void AProceduralActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    UnloadHeightMap();
}

void AProceduralActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    currentTime += DeltaTime;

    if (currentTime >= updateInterval)
    {
        currentTime = 0.0f;
        int length = terrainVertices.Num();

        if (!feof(filePtr)) 
        {
            count = fread(rawImage, sizeof(UINT16), KINECT_DEPTH_CAPACITY, filePtr);

            if (count != KINECT_DEPTH_CAPACITY)
            {
                GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("tick: count != imageSize"));
                return;
            }

            UpdateTerrainHeight();
            UpdateMesh();
        }
    }
}


bool AProceduralActor::LoadHeightMap()
{
    const char* filePath = TCHAR_TO_UTF8(*(FPaths::ProjectDir() + FileName));

    // Open the 16 bit raw height map file for reading in binary.
    int error = fopen_s(&filePtr, filePath, "rb");
    if (error != 0)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("ERR in opening file"));
        return false;
    }

    // Allocate memory for the raw image data.
    rawImage = new UINT16[KINECT_DEPTH_CAPACITY];
    if (!rawImage)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("!rawImage"));
        return false;
    }

    // Read in the raw image data.
    count = fread(rawImage, sizeof(UINT16), KINECT_DEPTH_CAPACITY, filePtr);
    if (count != KINECT_DEPTH_CAPACITY)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("count != imageSize"));
        return false;
    }

    return true;
}

bool AProceduralActor::UnloadHeightMap() 
{
    // Close the file.
    int error = fclose(filePtr);
    if (error != 0)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("error during closing"));
        return false;
    }

    // Release the bitmap image data.
    delete[] rawImage;

    rawImage = nullptr;
    filePtr = nullptr;

    return true;
}


void AProceduralActor::CreateMesh()
{
	RootComponent = mesh;

#if WITH_EDITOR
	editorMesh->DestroyComponent();
#endif

    FVector terrain3DPoint;
    for (int j = 0; j < terrainHeight; j++)
    {
        for (int i = 0; i < terrainWidth; i++)
        {
            terrain3DPoint.X = i * lengthMultiplicator;
            terrain3DPoint.Y = j * lengthMultiplicator;
            terrain3DPoint.Z = 0;

            terrainVertices.Add(terrain3DPoint);
        }
    }

	UpdateMesh();
}

void AProceduralActor::UpdateMesh()
{
	UKismetProceduralMeshLibrary::CreateGridMeshTriangles(terrainHeight, terrainWidth, true, triangles);

	mesh->CreateMeshSection_LinearColor(0, terrainVertices, triangles, normals, uvs, vertexColors, tangents, true);
	mesh->ContainsPhysicsTriMeshData(true); // Enable collision data
}

void AProceduralActor::UpdateTerrainHeight()
{
    int k = 0;
    for (int j = 0; j < terrainHeight; j++)
    {
        for (int i = 0; i < terrainWidth; i++)
        {
            // Coordinates of the desired point transformed from terrain coordinates to heightmap coordinates
            float x = i / float(terrainWidth) * KINECT_DEPTH_WIDTH;
            float y = j / float(terrainHeight) * KINECT_DEPTH_HEIGHT;
            // Exact pixel coordinates in the heightmap
            int hx = int(x);
            int hy = int(y);

            // Height values of 4 nearest points on heightmap to interpolate
            UINT16 p00 = rawImage[hy * KINECT_DEPTH_WIDTH + hx];
            UINT16 p10 = rawImage[hy * KINECT_DEPTH_WIDTH + (hx + 1)];
            UINT16 p01 = rawImage[(hy + 1) * KINECT_DEPTH_WIDTH + hx];
            UINT16 p11 = rawImage[(hy + 1) * KINECT_DEPTH_WIDTH + (hx + 1)];

            terrainVertices[k++].Z = Normalize(Bilinear(x - hx, y - hy, { p00, p10, p01, p11 })) * heightMultiplicator;
        }
    }
}


inline float AProceduralActor::Normalize(const float value) 
{ 
    return (value - MIN) / (MAX - MIN); 
}

inline float AProceduralActor::Bilinear(const float tx, const float ty, const InterpolatedPoints& points)
{
    float a = points.p00 * (1.f - tx) + points.p10 * tx;
    float b = points.p01 * (1.f - tx) + points.p11 * tx;

    return a * (1.f - ty) + b * ty;
}
