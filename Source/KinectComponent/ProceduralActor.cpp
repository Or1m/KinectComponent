#include "ProceduralActor.h"

#include <ProceduralMeshComponent/Public/KismetProceduralMeshLibrary.h>
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"


AProceduralActor::AProceduralActor()
{
	tick = true;
	PrimaryActorTick.bCanEverTick = tick;

	CreateEditorPlaceHolder(); // Not mandatory
	InitializeInGameMesh(); 
}

void AProceduralActor::CreateEditorPlaceHolder()
{
#if WITH_EDITOR
	editorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Placeholder"));

	ConstructorHelpers::FObjectFinder<UStaticMesh> meshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Plane'"));
	ConstructorHelpers::FObjectFinder<UMaterial> material(TEXT("Material'/Game/Materials/Material'"));
    ConstructorHelpers::FObjectFinder<UMaterial> heightMat(TEXT("Material'/Game/Materials/testMat'"));

	editorMesh->SetStaticMesh(meshAsset.Object);
	editorMesh->GetStaticMesh()->SetMaterial(0, material.Object);
	
    terrainMaterial = heightMat.Object;
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

    if (Initialize())
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Initialized");
        CreateTerrainMesh();
    }
}

void AProceduralActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    Shutdown();
}

void AProceduralActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    HRESULT hr;
    IDepthFrame* depthFrame;

    hr = m_depthFrameReader->AcquireLatestFrame(&depthFrame);
        
    if (FAILED(hr)) {
        //GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Failed to AcquireLatestFrame"));
        return;
    }
            
    hr = depthFrame->CopyFrameDataToArray(depthWidth * depthHeight, rawImage);

    if (FAILED(hr))
    {
        SafeRelease(depthFrame);
        //GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Failed to CopyFrameDataToArray"));
        return;
    }

    SafeRelease(depthFrame);
    UpdateTerrainMesh();
}

// Put initialization stuff here
bool AProceduralActor::Initialize()
{
    HRESULT hr;

    //get the kinect sensor
    hr = GetDefaultKinectSensor(&m_sensor);
    if (FAILED(hr))
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Failed to find the kinect sensor!"));
        return false;
    }

    m_sensor->Open();

    //get the depth frame source
    IDepthFrameSource* depthFrameSource;
    hr = m_sensor->get_DepthFrameSource(&depthFrameSource);
    if (FAILED(hr))
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Failed to get the depth frame source."));
        return false;
    }

    //get depth frame description
    IFrameDescription* frameDesc;
    depthFrameSource->get_FrameDescription(&frameDesc);
    frameDesc->get_Width(&depthWidth);
    frameDesc->get_Height(&depthHeight);

    //get the depth frame reader
    hr = depthFrameSource->OpenReader(&m_depthFrameReader);
    if (FAILED(hr))
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Failed to open the depth frame reader!\n"));
        return false;
    }
    //release depth frame source
    SafeRelease(depthFrameSource);

    //allocate depth buffer
    rawImage = new uint16[depthWidth * depthHeight];

    return true;
}
// Put cleaning up stuff here
void AProceduralActor::Shutdown()
{
    // Release the bitmap image data.
    delete[] rawImage;

    rawImage = nullptr;
    filePtr = nullptr;

    /*delete[] m_colorBuffer;
    SafeRelease(m_colorFrameReader);*/

    //delete[] rawImage;
    SafeRelease(m_depthFrameReader);
    SafeRelease(m_sensor);
}

void AProceduralActor::CreateTerrainMesh()
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

    //Press F1 in game for wireframe view
    UKismetProceduralMeshLibrary::CreateGridMeshTriangles(terrainHeight, terrainWidth, true, triangles);
    mesh->CreateMeshSection(0, terrainVertices, triangles, normals, uvs, vertexColors, tangents, true);

    mesh->SetMaterial(0, terrainMaterial);
}

void AProceduralActor::UpdateTerrainMesh() 
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

    mesh->UpdateMeshSection(0, terrainVertices, normals, uvs, vertexColors, tangents);
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
