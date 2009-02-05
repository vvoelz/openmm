/* -------------------------------------------------------------------------- *
 *                                   OpenMM                                   *
 * -------------------------------------------------------------------------- *
 * This is part of the OpenMM molecular simulation toolkit originating from   *
 * Simbios, the NIH National Center for Physics-Based Simulation of           *
 * Biological Structures at Stanford, funded under the NIH Roadmap for        *
 * Medical Research, grant U54 GM072970. See https://simtk.org.               *
 *                                                                            *
 * Portions copyright (c) 2009 Stanford University and the Authors.           *
 * Authors: Scott Le Grand, Peter Eastman                                     *
 * Contributors:                                                              *
 *                                                                            *
 * Permission is hereby granted, free of charge, to any person obtaining a    *
 * copy of this software and associated documentation files (the "Software"), *
 * to deal in the Software without restriction, including without limitation  *
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,   *
 * and/or sell copies of the Software, and to permit persons to whom the      *
 * Software is furnished to do so, subject to the following conditions:       *
 *                                                                            *
 * The above copyright notice and this permission notice shall be included in *
 * all copies or substantial portions of the Software.                        *
 *                                                                            *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   *
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    *
 * THE AUTHORS, CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,    *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR      *
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE  *
 * USE OR OTHER DEALINGS IN THE SOFTWARE.                                     *
 * -------------------------------------------------------------------------- */

#include <stdio.h>
#include <cuda.h>
#include <vector_functions.h>
#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
using namespace std;

#include "gputypes.h"

static __constant__ cudaGmxSimulation cSim;

void SetRandomSim(gpuContext gpu)
{
    cudaError_t status;
    status = cudaMemcpyToSymbol(cSim, &gpu->sim, sizeof(cudaGmxSimulation));     
    RTERROR(status, "cudaMemcpyToSymbol: SetSim copy to cSim failed");
}

void GetRandomSim(gpuContext gpu)
{
    cudaError_t status;
    status = cudaMemcpyFromSymbol(&gpu->sim, cSim, sizeof(cudaGmxSimulation));     
    RTERROR(status, "cudaMemcpyFromSymbol: SetSim copy from cSim failed");
}

extern __shared__ float3 sRand[];


__global__ void kGenerateRandoms_kernel()
{
    unsigned int pos            = blockIdx.x * blockDim.x + threadIdx.x;
    unsigned int increment      = blockDim.x * gridDim.x;
    
    // Read generator state
    uint4 state                 = cSim.pRandomSeed[pos];
    unsigned int carry          = 0;
    
    float4 random4;
    float2 random2;
    while (pos < cSim.totalRandomsTimesTwo)
    {
        
        // Generate 6 randoms in GRF
        unsigned int pos1       = threadIdx.x;
        for (int i = 0; i < 2; i++)
        {
            state.x             = state.x * 69069 + 1;
            state.y            ^= state.y << 13;
            state.y            ^= state.y >> 17;
            state.y            ^= state.y << 5;
            unsigned int k      = (state.z >> 2) + (state.w >> 3) + (carry >> 2);
            unsigned int m      = state.w + state.w + state.z + carry;
            state.z             = state.w;
            state.w             = m;
            carry               = k >> 30;
            float x1            = (float)max(state.x + state.y + state.w, 0x00000001) / (float)0xffffffff;
            state.x             = state.x * 69069 + 1;
            state.y            ^= state.y << 13;
            state.y            ^= state.y >> 17;
            state.y            ^= state.y << 5;
            x1                  = sqrt(-2.0f * log(x1));
            k                   = (state.z >> 2) + (state.w >> 3) + (carry >> 2);
            m                   = state.w + state.w + state.z + carry;
            state.z             = state.w;
            state.w             = m;
            carry               = k >> 30;
            float x2            = (float)(state.x + state.y + state.w) / (float)0xffffffff;
            
            state.x             = state.x * 69069 + 1;
            state.y            ^= state.y << 13;
            state.y            ^= state.y >> 17;
            state.y            ^= state.y << 5;
            sRand[pos1].x       = x1 * cos(2.0f * 3.14159265f * x2);
            k                   = (state.z >> 2) + (state.w >> 3) + (carry >> 2);
            m                   = state.w + state.w + state.z + carry;
            state.z             = state.w;
            state.w             = m;
            carry               = k >> 30;
            float x3            = (float)max(state.x + state.y + state.w, 0x00000001) / (float)0xffffffff;
            state.x             = state.x * 69069 + 1;
            state.y            ^= state.y << 13;
            state.y            ^= state.y >> 17;
            state.y            ^= state.y << 5;
            x3                  = sqrt(-2.0f * log(x3));
            k                   = (state.z >> 2) + (state.w >> 3) + (carry >> 2);
            m                   = state.w + state.w + state.z + carry;
            state.z             = state.w;
            state.w             = m;
            carry               = k >> 30;
            float x4            = (float)(state.x + state.y + state.w) / (float)0xffffffff;
            
            state.x             = state.x * 69069 + 1;
            state.y            ^= state.y << 13;
            state.y            ^= state.y >> 17;
            state.y            ^= state.y << 5;
            sRand[pos1].y       = x3 * cos(2.0f * 3.14159265f * x4);
            k                   = (state.z >> 2) + (state.w >> 3) + (carry >> 2);
            m                   = state.w + state.w + state.z + carry;
            state.z             = state.w;
            state.w             = m;
            carry               = k >> 30;
            float x5            = (float)max(state.x + state.y + state.w, 0x00000001) / (float)0xffffffff;
            state.x             = state.x * 69069 + 1;
            state.y            ^= state.y << 13;
            state.y            ^= state.y >> 17;
            state.y            ^= state.y << 5;
            x5                  = sqrt(-2.0f * log(x5));
            k                   = (state.z >> 2) + (state.w >> 3) + (carry >> 2);
            m                   = state.w + state.w + state.z + carry;
            state.z             = state.w;
            state.w             = m;
            carry               = k >> 30;
            float x6            = (float)(state.x + state.y + state.w) / (float)0xffffffff;
            sRand[pos1].z       = x5 * cos(2.0f * 3.14159265f * x6); 
            pos1               += blockDim.x;
        }
        
        // Output final randoms
        float c1, c2;
        if (pos < cSim.totalRandoms)
        {
            c1                  = cSim.Yv;
            c2                  = cSim.V;
        }
        else
        {
            c1                  = cSim.Yx;
            c2                  = cSim.X;
        }
        random4.x               = c1 * sRand[threadIdx.x].x;
        random4.y               = c1 * sRand[threadIdx.x].y;
        random4.z               = c1 * sRand[threadIdx.x].z;
        random4.w               = c2 * sRand[threadIdx.x + blockDim.x].x;
        cSim.pRandom4a[pos]     = random4;
        random2.x               = c2 * sRand[threadIdx.x + blockDim.x].y;
        random2.y               = c2 * sRand[threadIdx.x + blockDim.x].z;
        cSim.pRandom2a[pos]     = random2;
        
   
        pos += increment;
    }
    
    
    // Write generator state
    pos                     = blockIdx.x * blockDim.x + threadIdx.x;
    cSim.pRandomSeed[pos]   = state;
}

void kGenerateRandoms(gpuContext gpu)
{
    kGenerateRandoms_kernel<<<gpu->sim.blocks, gpu->sim.random_threads_per_block, gpu->sim.random_threads_per_block * 2 * sizeof(float3)>>>();
}