// Horizontal compute blurr, based on Frank Luna's example.

// Blur weightings
cbuffer cbSettings
{
    static float gWeights[11] =
    {
        0.05f, 0.05f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.05f, 0.05f
    };
};

cbuffer cbFixed
{
    static const int gBlurRadius = 5;
};

Texture2D gInput : register(t0);
RWTexture2D<float4> gOutput : register(u0);

#define N 256
#define CacheSize (N + 2*gBlurRadius)
groupshared float4 gCache[CacheSize];

[numthreads(1, 1, 1)]
void main(uint3 groupThreadID : SV_DispatchThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{

}