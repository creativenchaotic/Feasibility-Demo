static const int NumThreads = 128;

RWStructuredBuffer<int> particleOffsets : register(u0); //Data we pass to and from the compute shader

[numthreads(NumThreads, 1, 1)]
void main(uint3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
}