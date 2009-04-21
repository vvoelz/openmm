#ifndef OPENMM_GBVIFORCEFIELDIMPL_H_
#define OPENMM_GBVIFORCEFIELDIMPL_H_

/* -------------------------------------------------------------------------- *
 *                                   OpenMM                                   *
 * -------------------------------------------------------------------------- *
 * This is part of the OpenMM molecular simulation toolkit originating from   *
 * Simbios, the NIH National Center for Physics-Based Simulation of           *
 * Biological Structures at Stanford, funded under the NIH Roadmap for        *
 * Medical Research, grant U54 GM072970. See https://simtk.org.               *
 *                                                                            *
 * Portions copyright (c) 2008 Stanford University and the Authors.           *
 * Authors: Peter Eastman                                                     *
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

#include "ForceImpl.h"
#include "openmm/GBVIForce.h"
#include "openmm/Kernel.h"
#include <string>

namespace OpenMM {

/**
 * This is the internal implementation of GBVIForce.
 */

class GBVIForceImpl : public ForceImpl {
public:
    GBVIForceImpl(GBVIForce& owner);
    void initialize(OpenMMContextImpl& context);
    GBVIForce& getOwner() {
        return owner;
    }
    
    // calculate scaled radii (Eq. 5 of Labute paper [JCC 29 1693-1698 2008])

    void findScaledRadii( int numberOfParticles, const std::vector<std::vector<int> >& bondIndices,
                          const std::vector<double> & bondLengths, std::vector<double> & scaledRadii) const;

    void updateContextState(OpenMMContextImpl& context) {
        // This force field doesn't update the state directly.
    }
    void calcForces(OpenMMContextImpl& context, Stream& forces);
    double calcEnergy(OpenMMContextImpl& context);
    std::map<std::string, double> getDefaultParameters() {
        return std::map<std::string, double>(); // This force field doesn't define any parameters.
    }
    std::vector<std::string> getKernelNames();
private:
    GBVIForce& owner;
    Kernel kernel;
};

} // namespace OpenMM

#endif /*OPENMM_GBVIFORCEFIELDIMPL_H_*/