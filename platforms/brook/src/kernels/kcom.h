/* -------------------------------------------------------------------------- *
 *                                   OpenMM                                   *
 * -------------------------------------------------------------------------- *
 * This is part of the OpenMM molecular simulation toolkit originating from   *
 * Simbios, the NIH National Center for Physics-Based Simulation of           *
 * Biological Structures at Stanford, funded under the NIH Roadmap for        *
 * Medical Research, grant U54 GM072970. See https://simtk.org.               *
 *                                                                            *
 * Portions copyright (c) 2008 Stanford University and the Authors.           *
 * Authors: Peter Eastman, Mark Friedrichs, Chris Bruns                       *
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

void  kCalculateLinearMomentum( ::brook::stream mass, ::brook::stream velocities, ::brook::stream linearMomentum );
void  kReduceLinearMomentum( ::brook::stream momentum, ::brook::stream linearMomentum );
//void  kSumLinearMomentum( ::brook::stream momentum, float3* linearMomentum );
void  kScale( float scale, ::brook::stream linearMomentumIn, ::brook::stream linearMomentumOut );
void  kRemoveLinearMomentum( ::brook::stream linearMomentum, ::brook::stream velocitiesIn, ::brook::stream velocitiesOut );
void kSum( ::brook::stream array, ::brook::stream sum );

/**---------------------------------------------------------------------------------------

   This kernel calculates the total linear momentum via a reduction

   @param atomStrWidth   atom stream width
   @param numberOfAtoms  number of atoms
   @param scale          sum of inverse masses
   @param momentum       momentum
   @param linearMomentum total momentum

   --------------------------------------------------------------------------------------- */

void kSumLinearMomentum( float atomStrWidth, float numberOfAtoms, float scale, ::brook::stream momentum, ::brook::stream linearMomentum );

