/*
// *  Copyright (c) 2016, Nagoya University
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither the name of Autoware nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ray_tracing.h"
#include <vector>
#include <iostream>
#include <ctime>
#include <algorithm>
#include <chrono>
#include <omp.h>
#include <signal.h>

#define TEST_PERFORMANCE 0

using namespace ground_field;

template class RayTracing<float>;
int main(int argc, char* argv[])
{
    enum {
#if (TEST_PERFORMANCE)
        NUM_RAYS = 230400, //Velodyne HDL-64 number of points in 1 scan
#else
        NUM_RAYS = 230400,
#endif
    }; 
    RayTracing<float> ray_tracing(Point3D<float>(-50,-50,-50),
                                  Point3D<float>(50,50,50),
                                  Point3D<float>(0.1,0.1,0.1));
    Ray<float> ray_aux(Point3D<float>(0.f, 0.f, 0.f),
                       Point3D<float>(-60.f, -10.f, -60.f));
    std::vector< Ray<float> > rays(NUM_RAYS, ray_aux);

#if (TEST_PERFORMANCE)
    //Add all the rays
    for (auto r : rays) {
        ray_tracing.addRay(r);
    }
    //make the gridmap
    ray_tracing.makeGridMapSize();
    //Perform several parallel tests
    while (1) {
        auto start = std::chrono::high_resolution_clock::now();
        #pragma omp parallel for default(shared)
        for (int i=0; i < NUM_RAYS; i++) {
            ray_tracing.computeRayIntersectionsZFilter2DGridEFLA(i);
        }
        auto elapsed = std::chrono::high_resolution_clock::now() - start;
        std::cout << "Generated gridmap of " << ray_tracing.getGridMap().rows() << " rows, " << ray_tracing.getGridMap().cols() << " cols, " << ray_tracing.getGridMap().size() * sizeof(ray_tracing.getGridMap()(0,0)) << " bytes" << std::endl;
        long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
        std::cout << "Computed in " <<(double)microseconds/1000000.0 << " seconds" <<  std::endl;

        //clear all grid map's data
        ray_tracing.clearGridMap();
    }
#else
    //Add all the rays
    for (auto r : rays) {
        ray_tracing.addRay(r);
    }
    //make the gridmap
    ray_tracing.makeGridMapSize();
    unsigned int idx = ray_tracing.raysCount() - 1;
    auto start = std::chrono::high_resolution_clock::now();
    //Perform sequential ray tracing
	#pragma omp parallel for default(shared)
    for (int i=0; i < NUM_RAYS; i++) {
        ray_tracing.computeRayIntersectionsZFilter2DGridEFLA(i);
    }
    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    
    //std::cout << "grimap: " << ray_tracing.getGridMap() << std::endl;
    std::cout << "Generated gridmap of " << ray_tracing.getGridMap().rows() << " rows, " << ray_tracing.getGridMap().cols() << " cols, " << ray_tracing.getGridMap().size() * sizeof(ray_tracing.getGridMap()(0,0)) << " bytes" << std::endl;
    long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    std::cout << "Computed in " <<(double)microseconds/1000000.0 << " seconds" <<  std::endl;
#endif

    return 1;
}
