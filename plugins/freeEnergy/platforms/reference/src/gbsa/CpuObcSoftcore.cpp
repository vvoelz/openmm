
/* Portions copyright (c) 2006-2009 Stanford University and Simbios.
 * Contributors: Pande Group
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS, CONTRIBUTORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <string.h>
#include <sstream>

#include "../SimTKUtilities/SimTKOpenMMCommon.h"
#include "../SimTKUtilities/SimTKOpenMMLog.h"
#include "../SimTKUtilities/SimTKOpenMMUtilities.h"
#include "CpuObcSoftcore.h"
#include "../SimTKReference/ReferenceForce.h"
#include <cmath>
#include <cstdio>

/**---------------------------------------------------------------------------------------

   CpuObcSoftcore constructor

   obcSoftcoreParameters      obcSoftcoreParameters object
   
   --------------------------------------------------------------------------------------- */

CpuObcSoftcore::CpuObcSoftcore( ImplicitSolventParameters* obcSoftcoreParameters ) : CpuImplicitSolvent( obcSoftcoreParameters ){

   // ---------------------------------------------------------------------------------------

   // static const char* methodName = "\nCpuObcSoftcore::CpuObcSoftcore";

   // ---------------------------------------------------------------------------------------

   _initializeObcDataMembers( );

   _obcSoftcoreParameters = static_cast<ObcSoftcoreParameters*> (obcSoftcoreParameters);

}

/**---------------------------------------------------------------------------------------

   CpuObcSoftcore destructor

   --------------------------------------------------------------------------------------- */

CpuObcSoftcore::~CpuObcSoftcore( ){

   // ---------------------------------------------------------------------------------------

   // static const char* methodName = "\nCpuObcSoftcore::~CpuObcSoftcore";

   // ---------------------------------------------------------------------------------------

   //if( _obcSoftcoreParameters != NULL ){
     // delete _obcSoftcoreParameters;
   //}

   delete[] _obcChain;
   delete[] _obcChainTemp;
}

/**---------------------------------------------------------------------------------------

   Initialize data members

   --------------------------------------------------------------------------------------- */

void CpuObcSoftcore::_initializeObcDataMembers( void ){

   // ---------------------------------------------------------------------------------------

   // static const char* methodName = "\nCpuObcSoftcore::initializeDataMembers";

   // ---------------------------------------------------------------------------------------

   _obcSoftcoreParameters = NULL;
   _obcChain              = NULL;
   _obcChainTemp          = NULL;
}

/**---------------------------------------------------------------------------------------

   Get ObcSoftcoreParameters reference

   @return ObcSoftcoreParameters reference

   --------------------------------------------------------------------------------------- */

ObcSoftcoreParameters* CpuObcSoftcore::getObcSoftcoreParameters( void ) const {

   // ---------------------------------------------------------------------------------------

   // static const char* methodName = "\nCpuObcSoftcore::getObcSoftcoreParameters";

   // ---------------------------------------------------------------------------------------

   return _obcSoftcoreParameters;
}

/**---------------------------------------------------------------------------------------

   Set ObcSoftcoreParameters reference

   @param ObcSoftcoreParameters reference

   @return SimTKOpenMMCommon::DefaultReturn;

   --------------------------------------------------------------------------------------- */

int CpuObcSoftcore::setObcSoftcoreParameters(  ObcSoftcoreParameters* obcSoftcoreParameters ){

   // ---------------------------------------------------------------------------------------

   // static const char* methodName = "\nCpuObcSoftcore::setObcSoftcoreParameters";

   // ---------------------------------------------------------------------------------------

   _obcSoftcoreParameters = obcSoftcoreParameters;
   return SimTKOpenMMCommon::DefaultReturn;
}

/**---------------------------------------------------------------------------------------

   Return OBC chain derivative: size = _obcSoftcoreParameters->getNumberOfAtoms()
   On first call, memory for array is allocated if not set

   @return array

   --------------------------------------------------------------------------------------- */

RealOpenMM* CpuObcSoftcore::getObcChain( void ){

   // ---------------------------------------------------------------------------------------

   // static const char* methodName = "\nCpuObcSoftcore::getObcChain";

   // ---------------------------------------------------------------------------------------

   if( _obcChain == NULL ){
      _obcChain = new RealOpenMM[_obcSoftcoreParameters->getNumberOfAtoms()];
   }
   return _obcChain;
}

/**---------------------------------------------------------------------------------------

   Return OBC chain derivative: size = _obcSoftcoreParameters->getNumberOfAtoms()

   @return array

   --------------------------------------------------------------------------------------- */

RealOpenMM* CpuObcSoftcore::getObcChainConst( void ) const {

   // ---------------------------------------------------------------------------------------

   // static const char* methodName = "\nCpuObcSoftcore::getObcChain";

   // ---------------------------------------------------------------------------------------

   return _obcChain;
}

/**---------------------------------------------------------------------------------------

   Return OBC chain temp work array of size=_obcSoftcoreParameters->getNumberOfAtoms()
   On first call, memory for array is allocated if not set

   @return array

   --------------------------------------------------------------------------------------- */

RealOpenMM* CpuObcSoftcore::getObcChainTemp( void ){

   // ---------------------------------------------------------------------------------------

   // static const char* methodName = "\nCpuObcSoftcore::getImplicitSolventObcChainTemp";

   // ---------------------------------------------------------------------------------------

   if( _obcChainTemp == NULL ){
      _obcChainTemp = new RealOpenMM[_obcSoftcoreParameters->getNumberOfAtoms()];
   }
   return _obcChainTemp;
}

/**---------------------------------------------------------------------------------------

   Get Born radii based on papers:

      J. Phys. Chem. 1996 100, 19824-19839 (HCT paper)
      Proteins: Structure, Function, and Bioinformatcis 55:383-394 (2004) (OBC paper)

   @param atomCoordinates     atomic coordinates
   @param bornRadii           output array of Born radii

   @return array of Born radii

   --------------------------------------------------------------------------------------- */

int CpuObcSoftcore::computeBornRadii( RealOpenMM** atomCoordinates, RealOpenMM* bornRadii, RealOpenMM* obcChain ){

   // ---------------------------------------------------------------------------------------

   static const RealOpenMM zero    = (RealOpenMM) 0.0;
   static const RealOpenMM one     = (RealOpenMM) 1.0;
   static const RealOpenMM two     = (RealOpenMM) 2.0;
   static const RealOpenMM three   = (RealOpenMM) 3.0;
   static const RealOpenMM half    = (RealOpenMM) 0.5;
   static const RealOpenMM fourth  = (RealOpenMM) 0.25;

   static const char* methodName   = "\nCpuObcSoftcore::computeBornRadii";

   // ---------------------------------------------------------------------------------------

   ObcSoftcoreParameters* obcSoftcoreParameters             = getObcSoftcoreParameters();

   int numberOfAtoms                        = obcSoftcoreParameters->getNumberOfAtoms();
   RealOpenMM* atomicRadii                  = obcSoftcoreParameters->getAtomicRadii();
   const RealOpenMM* scaledRadiusFactor     = obcSoftcoreParameters->getScaledRadiusFactors();
   if( !obcChain ){
      obcChain                              = getObcChain();
   }

   const RealOpenMM* nonPolarScaleFactors   = obcSoftcoreParameters->getNonPolarScaleFactors();
   RealOpenMM dielectricOffset              = obcSoftcoreParameters->getDielectricOffset();
   RealOpenMM alphaObc                      = obcSoftcoreParameters->getAlphaObc();
   RealOpenMM betaObc                       = obcSoftcoreParameters->getBetaObc();
   RealOpenMM gammaObc                      = obcSoftcoreParameters->getGammaObc();

   // ---------------------------------------------------------------------------------------

   // calculate Born radii

//FILE* logFile = SimTKOpenMMLog::getSimTKOpenMMLogFile( );
//FILE* logFile = NULL;
//FILE* logFile = fopen( "bR", "w" );

   for( int atomI = 0; atomI < numberOfAtoms; atomI++ ){
     
      RealOpenMM radiusI         = atomicRadii[atomI];
      RealOpenMM offsetRadiusI   = radiusI - dielectricOffset;

      RealOpenMM radiusIInverse  = one/offsetRadiusI;
      RealOpenMM sum             = zero;

      // HCT code

      for( int atomJ = 0; atomJ < numberOfAtoms; atomJ++ ){

         if( atomJ != atomI ){

            RealOpenMM deltaR[ReferenceForce::LastDeltaRIndex];
            if (_obcSoftcoreParameters->getPeriodic())
                ReferenceForce::getDeltaRPeriodic( atomCoordinates[atomI], atomCoordinates[atomJ], _obcSoftcoreParameters->getPeriodicBox(), deltaR );
            else
                ReferenceForce::getDeltaR( atomCoordinates[atomI], atomCoordinates[atomJ], deltaR );
            RealOpenMM r               = deltaR[ReferenceForce::RIndex];
            if (_obcSoftcoreParameters->getUseCutoff() && r > _obcSoftcoreParameters->getCutoffDistance())
                continue;
            RealOpenMM offsetRadiusJ   = atomicRadii[atomJ] - dielectricOffset; 
            RealOpenMM scaledRadiusJ   = offsetRadiusJ*scaledRadiusFactor[atomJ];
            RealOpenMM rScaledRadiusJ  = r + scaledRadiusJ;

            if( offsetRadiusI < rScaledRadiusJ ){
               RealOpenMM rInverse = one/r;
               RealOpenMM l_ij     = offsetRadiusI > FABS( r - scaledRadiusJ ) ? offsetRadiusI : FABS( r - scaledRadiusJ );
                          l_ij     = one/l_ij;

               RealOpenMM u_ij     = one/rScaledRadiusJ;

               RealOpenMM l_ij2    = l_ij*l_ij;
               RealOpenMM u_ij2    = u_ij*u_ij;
 
               RealOpenMM ratio    = LN( (u_ij/l_ij) );
               RealOpenMM term     = l_ij - u_ij + fourth*r*(u_ij2 - l_ij2)  + ( half*rInverse*ratio) + (fourth*scaledRadiusJ*scaledRadiusJ*rInverse)*(l_ij2 - u_ij2);

               // this case (atom i completely inside atom j) is not considered in the original paper
               // Jay Ponder and the authors of Tinker recognized this and
               // worked out the details

               if( offsetRadiusI < (scaledRadiusJ - r) ){
                  term += two*( radiusIInverse - l_ij);
               }
               sum += nonPolarScaleFactors[atomJ]*term;

/*
if( logFile && atomI == 0 ){
   (void) fprintf( logFile, "\nRR %d %d r=%.4f rads[%.6f %.6f] scl=[%.3f %.3f] sum=%12.6e %12.6e %12.6e %12.6e",
                   atomI, atomJ, r, offsetRadiusI, offsetRadiusJ, scaledRadiusFactor[atomI], scaledRadiusFactor[atomJ], 0.5f*sum,
                   l_ij, u_ij, term );
}
*/

            }
         }
      }
 
      // OBC-specific code (Eqs. 6-8 in paper)

      sum                  *= half*offsetRadiusI;
      RealOpenMM sum2       = sum*sum;
      RealOpenMM sum3       = sum*sum2;
      RealOpenMM tanhSum    = TANH( alphaObc*sum - betaObc*sum2 + gammaObc*sum3 );
      
      bornRadii[atomI]      = one/( one/offsetRadiusI - tanhSum/radiusI ); 
 
      obcChain[atomI]       = offsetRadiusI*( alphaObc - two*betaObc*sum + three*gammaObc*sum2 );
      obcChain[atomI]       = (one - tanhSum*tanhSum)*obcChain[atomI]/radiusI;

#if 0
if( logFile && atomI >= 0 ){
   (void) fprintf( logFile, "\nRRQ %d sum %12.6e tanhS %12.6e radI %.5f %.5f born %18.10e obc %12.6e",
                   atomI, sum, tanhSum, radiusI, offsetRadiusI, bornRadii[atomI], obcChain[atomI] );
}
#endif

   }

#if 0
if( logFile ){
      (void) fclose( logFile );
}
#endif

   return SimTKOpenMMCommon::DefaultReturn;

}

/**---------------------------------------------------------------------------------------

   Get nonpolar solvation force constribution via ACE approximation

   @param obcSoftcoreParameters     parameters
   @param vdwRadii                  Vdw radii
   @param bornRadii                 Born radii
   @param energy                    energy (output): value is incremented from input value 
   @param forces                    forces: values are incremented from input values

   @return SimTKOpenMMCommon::DefaultReturn

   --------------------------------------------------------------------------------------- */

int CpuObcSoftcore::computeAceNonPolarForce( const ObcSoftcoreParameters* obcSoftcoreParameters,
                                             const RealOpenMM* bornRadii, RealOpenMM* energy,
                                             RealOpenMM* forces ) const {

   // ---------------------------------------------------------------------------------------

   // static const char* methodName = "\nCpuImplicitSolvent::computeAceNonPolarForce";

   static const RealOpenMM minusSix = -6.0;

   // ---------------------------------------------------------------------------------------

   // compute the nonpolar solvation via ACE approximation

   const RealOpenMM probeRadius           = obcSoftcoreParameters->getProbeRadius();
   const RealOpenMM surfaceAreaFactor     = obcSoftcoreParameters->getPi4Asolv();

   const RealOpenMM* atomicRadii          = obcSoftcoreParameters->getAtomicRadii();
   const RealOpenMM* nonPolarScaleFactors = obcSoftcoreParameters->getNonPolarScaleFactors();
   int numberOfAtoms                      = obcSoftcoreParameters->getNumberOfAtoms();

   // 1 + 1 + pow + 3 + 1 + 2 FLOP

   // the original ACE equation is based on Eq.2 of

   // M. Schaefer, C. Bartels and M. Karplus, "Solution Conformations
   // and Thermodynamics of Structured Peptides: Molecular Dynamics
   // Simulation with an Implicit Solvation Model", J. Mol. Biol.,
   // 284, 835-848 (1998)  (ACE Method)

   // The original equation includes the factor (atomicRadii[atomI]/bornRadii[atomI]) to the first power,
   // whereas here the ratio is raised to the sixth power: (atomicRadii[atomI]/bornRadii[atomI])**6

   // This modification was made by Jay Ponder who observed it gave better correlations w/
   // observed values. He did not think it was important enough to write up, so there is
   // no paper to cite.

   for( int atomI = 0; atomI < numberOfAtoms; atomI++ ){
      if( bornRadii[atomI] > 0.0 ){
         RealOpenMM r            = atomicRadii[atomI] + probeRadius;
         RealOpenMM ratio6       = POW( atomicRadii[atomI]/bornRadii[atomI], (RealOpenMM) 6.0 );
         RealOpenMM saTerm       = nonPolarScaleFactors[atomI]*surfaceAreaFactor*r*r*ratio6;
         *energy                += saTerm;
         forces[atomI]          += minusSix*saTerm/bornRadii[atomI]; 
      }
   }

   return SimTKOpenMMCommon::DefaultReturn; 

}

/**---------------------------------------------------------------------------------------

   Get Obc Born energy and forces

   @param bornRadii           Born radii -- optional; if NULL, then ObcSoftcoreParameters 
                              entry is used
   @param atomCoordinates     atomic coordinates
   @param partialCharges      partial charges
   @param forces              forces

   @return SimTKOpenMMCommon::DefaultReturn;

   The array bornRadii is also updated and the obcEnergy

   --------------------------------------------------------------------------------------- */

int CpuObcSoftcore::computeBornEnergyForces( RealOpenMM* bornRadii, RealOpenMM** atomCoordinates,
                                             const RealOpenMM* partialCharges, RealOpenMM** inputForces ){

   // ---------------------------------------------------------------------------------------

   // static const char* methodName = "\nCpuObcSoftcore::computeBornEnergyForces";

   static const RealOpenMM zero    = (RealOpenMM) 0.0;
   static const RealOpenMM one     = (RealOpenMM) 1.0;
   static const RealOpenMM two     = (RealOpenMM) 2.0;
   static const RealOpenMM three   = (RealOpenMM) 3.0;
   static const RealOpenMM four    = (RealOpenMM) 4.0;
   static const RealOpenMM half    = (RealOpenMM) 0.5;
   static const RealOpenMM fourth  = (RealOpenMM) 0.25;
   static const RealOpenMM eighth  = (RealOpenMM) 0.125;

   // ---------------------------------------------------------------------------------------

   const ObcSoftcoreParameters* obcSoftcoreParameters = getObcSoftcoreParameters();
   const int numberOfAtoms            = obcSoftcoreParameters->getNumberOfAtoms();

   if( bornRadii == NULL ){
      bornRadii   = getBornRadii();
   }

   // ---------------------------------------------------------------------------------------

   // constants

   const RealOpenMM preFactor           = obcSoftcoreParameters->getPreFactor();
   const RealOpenMM dielectricOffset    = obcSoftcoreParameters->getDielectricOffset();

   // ---------------------------------------------------------------------------------------

#if 0
{
   RealOpenMM* atomicRadii               = obcSoftcoreParameters->getAtomicRadii();
   const RealOpenMM* scaledRadiusFactor  = obcSoftcoreParameters->getScaledRadiusFactors();
   RealOpenMM* obcChain                  = getObcChain();
   FILE* logFile = fopen( "bornParameters", "w" );
   (void) fprintf( logFile, "%5d dielOff=%.4e rad::hct::q::bR::Chain::coords\n", numberOfAtoms, dielectricOffset );
   for( int atomI = 0; atomI < numberOfAtoms; atomI++ ){
      (void) fprintf( logFile, "%5d %10.5f %10.5f %10.5f %14.7e %14.7e %14.7e %14.7e %14.7e\n", atomI,
                      atomicRadii[atomI], scaledRadiusFactor[atomI], partialCharges[atomI], bornRadii[atomI], obcChain[atomI],
                      atomCoordinates[atomI][0], atomCoordinates[atomI][1], atomCoordinates[atomI][2] );
   }
   (void) fclose( logFile );
}
#endif

   // set energy/forces to zero

   RealOpenMM obcEnergy                 = zero;
   const unsigned int arraySzInBytes    = sizeof( RealOpenMM )*numberOfAtoms;

   RealOpenMM** forces  = (RealOpenMM**) malloc( sizeof( RealOpenMM* )*numberOfAtoms );
   RealOpenMM*  block   = (RealOpenMM*)  malloc( sizeof( RealOpenMM )*numberOfAtoms*3 );
	memset( block, 0, sizeof( RealOpenMM )*numberOfAtoms*3 );
	RealOpenMM* blockPtr = block;
   for( int ii = 0; ii < numberOfAtoms; ii++ ){
      forces[ii] = blockPtr;
		blockPtr  += 3;
   }

   RealOpenMM* bornForces = getBornForce();
   memset( bornForces, 0, arraySzInBytes );

   // ---------------------------------------------------------------------------------------

   // N*( 8 + pow) ACE
   // compute the nonpolar solvation via ACE approximation
    
   if( includeAceApproximation() ){
      computeAceNonPolarForce( obcSoftcoreParameters, bornRadii, &obcEnergy, bornForces );
   }
 
   // ---------------------------------------------------------------------------------------

   // first main loop

   for( int atomI = 0; atomI < numberOfAtoms; atomI++ ){
 
      RealOpenMM partialChargeI = preFactor*partialCharges[atomI];
      for( int atomJ = atomI; atomJ < numberOfAtoms; atomJ++ ){

         RealOpenMM deltaR[ReferenceForce::LastDeltaRIndex];
         if (_obcSoftcoreParameters->getPeriodic())
             ReferenceForce::getDeltaRPeriodic( atomCoordinates[atomI], atomCoordinates[atomJ], _obcSoftcoreParameters->getPeriodicBox(), deltaR );
         else
             ReferenceForce::getDeltaR( atomCoordinates[atomI], atomCoordinates[atomJ], deltaR );
         if (_obcSoftcoreParameters->getUseCutoff() && deltaR[ReferenceForce::RIndex] > _obcSoftcoreParameters->getCutoffDistance())
             continue;
         RealOpenMM r2                 = deltaR[ReferenceForce::R2Index];
         RealOpenMM deltaX             = deltaR[ReferenceForce::XIndex];
         RealOpenMM deltaY             = deltaR[ReferenceForce::YIndex];
         RealOpenMM deltaZ             = deltaR[ReferenceForce::ZIndex];

         // 3 FLOP

         RealOpenMM alpha2_ij          = bornRadii[atomI]*bornRadii[atomJ];
         RealOpenMM D_ij               = r2/(four*alpha2_ij);

         // exp + 2 + sqrt FLOP 

         RealOpenMM expTerm            = EXP( -D_ij );
         RealOpenMM denominator2       = r2 + alpha2_ij*expTerm; 
         RealOpenMM denominator        = SQRT( denominator2 ); 
         
         // 6 FLOP

         RealOpenMM Gpol               = (partialChargeI*partialCharges[atomJ])/denominator; 
         RealOpenMM dGpol_dr           = -Gpol*( one - fourth*expTerm )/denominator2;  

         // 5 FLOP

         RealOpenMM dGpol_dalpha2_ij   = -half*Gpol*expTerm*( one + D_ij )/denominator2;

         // 11 FLOP

         if( atomI != atomJ ){

             bornForces[atomJ] += dGpol_dalpha2_ij*bornRadii[atomI];

             deltaX            *= dGpol_dr;
             deltaY            *= dGpol_dr;
             deltaZ            *= dGpol_dr;

             forces[atomI][0]  += deltaX;
             forces[atomI][1]  += deltaY;
             forces[atomI][2]  += deltaZ;

             forces[atomJ][0]  -= deltaX;
             forces[atomJ][1]  -= deltaY;
             forces[atomJ][2]  -= deltaZ;

         } else {
            Gpol *= half;
         }

         // 3 FLOP

         obcEnergy         += Gpol;
         bornForces[atomI] += dGpol_dalpha2_ij*bornRadii[atomJ];

      }
   }

   //obcEnergy *= getEnergyConversionFactor();

   // ---------------------------------------------------------------------------------------

   // second main loop

   // initialize Born radii & ObcChain temp arrays -- contain values
   // used in next iteration

   RealOpenMM* bornRadiiTemp             = getBornRadiiTemp();
   memset( bornRadiiTemp, 0, arraySzInBytes );

   RealOpenMM* obcChainTemp              = getObcChainTemp();
   memset( obcChainTemp, 0, arraySzInBytes );

   RealOpenMM* obcChain                  = getObcChain();
   const RealOpenMM* atomicRadii         = obcSoftcoreParameters->getAtomicRadii();

   const RealOpenMM alphaObc             = obcSoftcoreParameters->getAlphaObc();
   const RealOpenMM betaObc              = obcSoftcoreParameters->getBetaObc();
   const RealOpenMM gammaObc             = obcSoftcoreParameters->getGammaObc();
   const RealOpenMM* scaledRadiusFactor  = obcSoftcoreParameters->getScaledRadiusFactors();

    // compute factor that depends only on the outer loop index

   for( int atomI = 0; atomI < numberOfAtoms; atomI++ ){
      bornForces[atomI] *= bornRadii[atomI]*bornRadii[atomI]*obcChain[atomI];      
   }

   for( int atomI = 0; atomI < numberOfAtoms; atomI++ ){
 
      // radius w/ dielectric offset applied

      RealOpenMM radiusI        = atomicRadii[atomI];
      RealOpenMM offsetRadiusI  = radiusI - dielectricOffset;

      // used to compute Born radius for next iteration

      RealOpenMM bornSum        = zero;

      for( int atomJ = 0; atomJ < numberOfAtoms; atomJ++ ){

         if( atomJ != atomI ){

            RealOpenMM deltaR[ReferenceForce::LastDeltaRIndex];
            if (_obcSoftcoreParameters->getPeriodic())
               ReferenceForce::getDeltaRPeriodic( atomCoordinates[atomI], atomCoordinates[atomJ], _obcSoftcoreParameters->getPeriodicBox(), deltaR );
            else 
               ReferenceForce::getDeltaR( atomCoordinates[atomI], atomCoordinates[atomJ], deltaR );
            if (_obcSoftcoreParameters->getUseCutoff() && deltaR[ReferenceForce::RIndex] > _obcSoftcoreParameters->getCutoffDistance())
                   continue;
   
            RealOpenMM deltaX             = deltaR[ReferenceForce::XIndex];
            RealOpenMM deltaY             = deltaR[ReferenceForce::YIndex];
            RealOpenMM deltaZ             = deltaR[ReferenceForce::ZIndex];
            RealOpenMM r                  = deltaR[ReferenceForce::RIndex];
 
            // radius w/ dielectric offset applied

            RealOpenMM offsetRadiusJ      = atomicRadii[atomJ] - dielectricOffset;

            RealOpenMM scaledRadiusJ      = offsetRadiusJ*scaledRadiusFactor[atomJ];
            RealOpenMM scaledRadiusJ2     = scaledRadiusJ*scaledRadiusJ;
            RealOpenMM rScaledRadiusJ     = r + scaledRadiusJ;

            // dL/dr & dU/dr are zero (this can be shown analytically)
            // removed from calculation

            if( offsetRadiusI < rScaledRadiusJ ){

               RealOpenMM l_ij          = offsetRadiusI > FABS( r - scaledRadiusJ ) ? offsetRadiusI : FABS( r - scaledRadiusJ );
                    l_ij                = one/l_ij;

               RealOpenMM u_ij          = one/rScaledRadiusJ;

               RealOpenMM l_ij2         = l_ij*l_ij;

               RealOpenMM u_ij2         = u_ij*u_ij;
 
               RealOpenMM rInverse      = one/r;
               RealOpenMM r2Inverse     = rInverse*rInverse;

               RealOpenMM t3            = eighth*(one + scaledRadiusJ2*r2Inverse)*(l_ij2 - u_ij2) + fourth*LN( u_ij/l_ij )*r2Inverse;

               RealOpenMM de            = bornForces[atomI]*t3*rInverse;

               deltaX                  *= de;
               deltaY                  *= de;
               deltaZ                  *= de;
   
               forces[atomI][0]        -= deltaX;
               forces[atomI][1]        -= deltaY;
               forces[atomI][2]        -= deltaZ;
  
               forces[atomJ][0]        += deltaX;
               forces[atomJ][1]        += deltaY;
               forces[atomJ][2]        += deltaZ;
 
               // Born radius term

               RealOpenMM term          =  l_ij - u_ij  + fourth*r*(u_ij2 - l_ij2) + (half*rInverse)*LN(u_ij/l_ij)   +
                                           (fourth*scaledRadiusJ*scaledRadiusJ*rInverse)*(l_ij2-u_ij2);

               if( offsetRadiusI < (scaledRadiusJ - r) ){
                  term += two*( (one/offsetRadiusI) - l_ij);
               }
               bornSum += term; 
            }
         }
      }

      // OBC-specific code (Eqs. 6-8 in paper)

      bornSum                   *= half*offsetRadiusI;
      RealOpenMM sum2            = bornSum*bornSum;
      RealOpenMM sum3            = bornSum*sum2;
      RealOpenMM tanhSum         = TANH( alphaObc*bornSum - betaObc*sum2 + gammaObc*sum3 );
      
      bornRadiiTemp[atomI]       = one/( one/offsetRadiusI - tanhSum/radiusI ); 
 
      obcChainTemp[atomI]        = offsetRadiusI*( alphaObc - two*betaObc*bornSum + three*gammaObc*sum2 );
      obcChainTemp[atomI]        = (one - tanhSum*tanhSum)*obcChainTemp[atomI]/radiusI;
   }

   // cal to Joule conversion

   RealOpenMM conversion = (RealOpenMM)0.4184;  
   for( int atomI = 0; atomI < numberOfAtoms; atomI++ ){
      inputForces[atomI][0] += conversion*forces[atomI][0];
      inputForces[atomI][1] += conversion*forces[atomI][1];
      inputForces[atomI][2] += conversion*forces[atomI][2];
   }
   setEnergy( obcEnergy*conversion );

#if 0
{
   RealOpenMM* atomicRadii               = obcSoftcoreParameters->getAtomicRadii();
   const RealOpenMM* scaledRadiusFactor  = obcSoftcoreParameters->getScaledRadiusFactors();
   RealOpenMM* obcChain                  = getObcChain();
   //FILE* logFile = fopen( "bornParameters", "w" );
   FILE* logFile = stderr;
   (void) fprintf( logFile, "%5d dielOff=%.4e rad::hct::q::bR::Chain::bF::f::coords\n", numberOfAtoms, dielectricOffset );
   for( int atomI = 0; atomI < numberOfAtoms; atomI++ ){
      (void) fprintf( logFile, "%5d %10.5f %10.5f q=%10.5f b[%14.7e %14.7e %14.7e] f[%14.7e %14.7e %14.7e] x[%14.7e %14.7e %14.7e]\n", atomI,
                      atomicRadii[atomI], scaledRadiusFactor[atomI], partialCharges[atomI], bornRadii[atomI], obcChain[atomI],
                      conversion*bornForces[atomI], 
                      conversion*forces[atomI][0], conversion*forces[atomI][1], conversion*forces[atomI][2],
                        atomCoordinates[atomI][0],   atomCoordinates[atomI][1],   atomCoordinates[atomI][2] );
   }
   if( logFile != stderr || logFile != stdout ){
      (void) fclose( logFile );
   }
}
#endif

   // copy new Born radii and obcChain values into permanent array

   memcpy( bornRadii, bornRadiiTemp, arraySzInBytes );
   memcpy( obcChain, obcChainTemp, arraySzInBytes );

	free( (char*) block );
	free( (char*) forces );

   return SimTKOpenMMCommon::DefaultReturn;

}

/**---------------------------------------------------------------------------------------
      
   Get string w/ state 
   
   @param title               title (optional)
      
   @return string containing state
      
   --------------------------------------------------------------------------------------- */

std::string CpuObcSoftcore::getStateString( const char* title ) const {

   // ---------------------------------------------------------------------------------------

   // static const char* methodName = "\nCpuImplicitSolvent::getStateString";

   // ---------------------------------------------------------------------------------------

   std::stringstream message;
   message << CpuImplicitSolvent::getStateString( title );

   return message.str();
}

/**---------------------------------------------------------------------------------------

   Write Born energy and forces (Simbios)

   @param atomCoordinates     atomic coordinates
   @param partialCharges      partial charges
   @param forces              forces
   @param resultsFileName     output file name

   @return SimTKOpenMMCommon::DefaultReturn unless
           file cannot be opened
           in which case return SimTKOpenMMCommon::ErrorReturn

   --------------------------------------------------------------------------------------- */

int CpuObcSoftcore::writeBornEnergyForces( RealOpenMM** atomCoordinates,
                                   const RealOpenMM* partialCharges, RealOpenMM** forces,
                                   const std::string& resultsFileName ) const {

   // ---------------------------------------------------------------------------------------

   static const char* methodName  = "\nCpuObcSoftcore::writeBornEnergyForces";

   // ---------------------------------------------------------------------------------------

   ImplicitSolventParameters* implicitSolventParameters = getImplicitSolventParameters();
   const ObcSoftcoreParameters* obcSoftcoreParameters                   = static_cast<const ObcSoftcoreParameters*>(implicitSolventParameters);
   

   int numberOfAtoms                    = obcSoftcoreParameters->getNumberOfAtoms();
   const RealOpenMM* atomicRadii        = obcSoftcoreParameters->getAtomicRadii();
   const RealOpenMM* bornRadii          = getBornRadiiConst();
   const RealOpenMM* scaledRadii        = obcSoftcoreParameters->getScaledRadiusFactors();
   const RealOpenMM* obcChain           = getObcChainConst();
   const RealOpenMM  energy             = getEnergy();

   // ---------------------------------------------------------------------------------------

   // open file -- return if unsuccessful

   FILE* implicitSolventResultsFile = NULL;
#ifdef _MSC_VER
   fopen_s( &implicitSolventResultsFile, resultsFileName.c_str(), "w" );
#else
   implicitSolventResultsFile = fopen( resultsFileName.c_str(), "w" );
#endif

   // diganostics

   std::stringstream message;
   message << methodName;
   if( implicitSolventResultsFile != NULL ){
      std::stringstream message;
      message << methodName;
      message << " Opened file=<" << resultsFileName << ">.";
      SimTKOpenMMLog::printMessage( message );
   } else {
      std::stringstream message;
      message << methodName;
      message << "  could not open file=<" << resultsFileName << "> -- abort output.";
      SimTKOpenMMLog::printMessage( message );
      return SimTKOpenMMCommon::ErrorReturn;
   }

   // header

   (void) fprintf( implicitSolventResultsFile, "# %d atoms E=%.7e   format: coords(3) bornRadii(input) q atomicRadii scaleFactors forces obcChain\n",
                   numberOfAtoms, energy );

   RealOpenMM forceConversion  = (RealOpenMM) 1.0;
   RealOpenMM lengthConversion = (RealOpenMM) 1.0;

   // output

   if( forces != NULL && atomCoordinates != NULL && partialCharges != NULL && atomicRadii != NULL ){
      for( int ii = 0; ii < numberOfAtoms; ii++ ){
            (void) fprintf( implicitSolventResultsFile, "%.7e %.7e %.7e %.7e %.5f %.5f %.5f %.7e %.7e %.7e %.7e\n",
                            lengthConversion*atomCoordinates[ii][0],
                            lengthConversion*atomCoordinates[ii][1], 
                            lengthConversion*atomCoordinates[ii][2],
                           (bornRadii != NULL ? lengthConversion*bornRadii[ii] : 0.0),
                            partialCharges[ii], lengthConversion*atomicRadii[ii], scaledRadii[ii],
                            forceConversion*forces[ii][0],
                            forceConversion*forces[ii][1],
                            forceConversion*forces[ii][2],
                            forceConversion*obcChain[ii]
                          );
      }
   }
   (void) fclose( implicitSolventResultsFile );

   return SimTKOpenMMCommon::DefaultReturn;

}

/**---------------------------------------------------------------------------------------

   Write  results from first loop

   @param numberOfAtoms       number of atoms
   @param forces              forces
   @param bornForce           Born force prefactor
   @param outputFileName      output file name

   @return SimTKOpenMMCommon::DefaultReturn unless
           file cannot be opened
           in which case return SimTKOpenMMCommon::ErrorReturn

   --------------------------------------------------------------------------------------- */

int CpuObcSoftcore::writeForceLoop1( int numberOfAtoms, RealOpenMM** forces, const RealOpenMM* bornForce,
                             const std::string& outputFileName ){

   // ---------------------------------------------------------------------------------------

   // static const char* methodName  = "\nCpuObcSoftcore::writeForceLoop1";

   // ---------------------------------------------------------------------------------------

   int chunkSize;
   if( bornForce ){
      chunkSize = 3;
   } else {
      chunkSize = 4;
   }

   StringVector lineVector;
   std::stringstream header;
   lineVector.push_back( "# bornF F" );
   for( int atomI = 0; atomI < numberOfAtoms; atomI++ ){
      std::stringstream line;
      line << (atomI+1) << " ";
      SimTKOpenMMUtilities::formatRealStringStream( line, forces[atomI], chunkSize );
      if( bornForce ){
         line << " " << bornForce[atomI];
      }
      lineVector.push_back( line.str() );
   }
   return SimTKOpenMMUtilities::writeFile( lineVector, outputFileName );

}

/**---------------------------------------------------------------------------------------

   Write results

   @param numberOfAtoms        number of atoms
   @param chunkSizes           vector of chunk sizes for realRealOpenMMVector
   @param realRealOpenMMVector vector of RealOpenMM**
   @param realVector           vector of RealOpenMM*
   @param outputFileName       output file name

   @return SimTKOpenMMCommon::DefaultReturn unless
           file cannot be opened
           in which case return SimTKOpenMMCommon::ErrorReturn

   --------------------------------------------------------------------------------------- */

int CpuObcSoftcore::writeForceLoop( int numberOfAtoms, const IntVector& chunkSizes,
                            const RealOpenMMPtrPtrVector& realRealOpenMMVector, 
                            const RealOpenMMPtrVector& realVector,
                            const std::string& outputFileName ){

   // ---------------------------------------------------------------------------------------

   // static const char* methodName  = "\nCpuObcSoftcore::writeForceLoop";

   static const int maxChunks = 10;
   int chunks[maxChunks];

   // ---------------------------------------------------------------------------------------

   for( int ii = 0; ii < (int) chunkSizes.size(); ii++ ){
      chunks[ii] = chunkSizes[ii];
   }
   for( int ii = (int) chunkSizes.size(); ii < maxChunks; ii++ ){
      chunks[ii] = 3;
   }

   StringVector lineVector;
   std::stringstream header;
   // lineVector.push_back( "# " );

   for( int atomI = 0; atomI < numberOfAtoms; atomI++ ){

      std::stringstream line;
		char buffer[128];

		(void) sprintf( buffer, "%4d ", atomI );
		line << buffer;

      int index = 0;
      for( RealOpenMMPtrPtrVectorCI ii = realRealOpenMMVector.begin(); ii != realRealOpenMMVector.end(); ii++ ){
         RealOpenMM** forces = *ii;
			(void) sprintf( buffer, "%11.5f %11.5f %11.5f ", forces[atomI][0], forces[atomI][1], forces[atomI][2] );
			line << buffer;
//         SimTKOpenMMUtilities::formatRealStringStream( line, forces[atomI], chunks[index++] );
//         line << " ";
      }

      for( RealOpenMMPtrVectorCI ii = realVector.begin(); ii != realVector.end(); ii++ ){
         RealOpenMM* array = *ii;
			(void) sprintf( buffer, "%11.5f ", array[atomI] );
         line << buffer;
      }

      lineVector.push_back( line.str() );
   }
   return SimTKOpenMMUtilities::writeFile( lineVector, outputFileName );

}

/**---------------------------------------------------------------------------------------

   Get Obc Born energy and forces -- used debugging

   @param bornRadii           Born radii -- optional; if NULL, then ObcSoftcoreParameters 
                              entry is used
   @param atomCoordinates     atomic coordinates
   @param partialCharges      partial charges
   @param forces              forces

   @return SimTKOpenMMCommon::DefaultReturn;

   The array bornRadii is also updated and the obcEnergy

   --------------------------------------------------------------------------------------- */

int CpuObcSoftcore::computeBornEnergyForcesPrint( RealOpenMM* bornRadii, RealOpenMM** atomCoordinates,
                                          const RealOpenMM* partialCharges, RealOpenMM** forces ){
 
   // ---------------------------------------------------------------------------------------

   // static const char* methodName = "\nCpuObcSoftcore::computeBornEnergyForcesPrint";

   static const RealOpenMM zero    = (RealOpenMM) 0.0;
   static const RealOpenMM one     = (RealOpenMM) 1.0;
   static const RealOpenMM two     = (RealOpenMM) 2.0;
   static const RealOpenMM three   = (RealOpenMM) 3.0;
   static const RealOpenMM four    = (RealOpenMM) 4.0;
   static const RealOpenMM half    = (RealOpenMM) 0.5;
   static const RealOpenMM fourth  = (RealOpenMM) 0.25;
   static const RealOpenMM eighth  = (RealOpenMM) 0.125;

   // ---------------------------------------------------------------------------------------

   const ObcSoftcoreParameters* obcSoftcoreParameters = getObcSoftcoreParameters();
   const int numberOfAtoms            = obcSoftcoreParameters->getNumberOfAtoms();

   if( bornRadii == NULL ){
      bornRadii   = getBornRadii();
   }

// suppress warning about fopen in Visual Studio
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4996)
#endif

FILE* logFile = NULL;
//FILE* logFile = SimTKOpenMMLog::getSimTKOpenMMLogFile( );
//FILE* logFile = fopen( "bF", "w" );

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

   // ---------------------------------------------------------------------------------------

   // constants

   const RealOpenMM preFactor           = obcSoftcoreParameters->getPreFactor();
   const RealOpenMM dielectricOffset    = obcSoftcoreParameters->getDielectricOffset();

   // ---------------------------------------------------------------------------------------

   // set energy/forces to zero

   RealOpenMM obcEnergy                 = zero;
   const unsigned int arraySzInBytes    = sizeof( RealOpenMM )*numberOfAtoms;

   for( int ii = 0; ii < numberOfAtoms; ii++ ){
      memset( forces[ii], 0, 3*sizeof( RealOpenMM ) );
   }
	

   RealOpenMM* bornForces = getBornForce();
   memset( bornForces, 0, arraySzInBytes );

   // ---------------------------------------------------------------------------------------

   // N*( 8 + pow) ACE
   // compute the nonpolar solvation via ACE approximation
    
   if( includeAceApproximation() ){
      computeAceNonPolarForce( obcSoftcoreParameters, bornRadii, &obcEnergy, bornForces );

      if( logFile ){
         (void) fprintf( logFile, "\nACE E=%.5e\n", obcEnergy );
         for( int atomI = 0; atomI < numberOfAtoms; atomI++ ){
            (void) fprintf( logFile, "   %d bR=%.6e bF=%.6e\n", atomI, bornRadii[atomI], bornForces[atomI] );
         }
      }

   }

   // ---------------------------------------------------------------------------------------

   // first main loop

   for( int atomI = 0; atomI < numberOfAtoms; atomI++ ){
 
      RealOpenMM partialChargeI = preFactor*partialCharges[atomI];
      for( int atomJ = atomI; atomJ < numberOfAtoms; atomJ++ ){

         RealOpenMM deltaR[ReferenceForce::LastDeltaRIndex];
         if (_obcSoftcoreParameters->getPeriodic())
             ReferenceForce::getDeltaRPeriodic( atomCoordinates[atomI], atomCoordinates[atomJ], _obcSoftcoreParameters->getPeriodicBox(), deltaR );
         else
             ReferenceForce::getDeltaR( atomCoordinates[atomI], atomCoordinates[atomJ], deltaR );
         if (_obcSoftcoreParameters->getUseCutoff() && deltaR[ReferenceForce::RIndex] > _obcSoftcoreParameters->getCutoffDistance())
             continue;

         RealOpenMM r2                 = deltaR[ReferenceForce::R2Index];
         RealOpenMM deltaX             = deltaR[ReferenceForce::XIndex];
         RealOpenMM deltaY             = deltaR[ReferenceForce::YIndex];
         RealOpenMM deltaZ             = deltaR[ReferenceForce::ZIndex];

         // 3 FLOP

         RealOpenMM alpha2_ij          = bornRadii[atomI]*bornRadii[atomJ];
         RealOpenMM D_ij               = r2/(four*alpha2_ij);

         // exp + 2 + sqrt FLOP 

         RealOpenMM expTerm            = EXP( -D_ij );
         RealOpenMM denominator2       = r2 + alpha2_ij*expTerm; 
         RealOpenMM denominator        = SQRT( denominator2 ); 
         
         // 6 FLOP

         RealOpenMM Gpol               = (partialChargeI*partialCharges[atomJ])/denominator; 
  
         // dGpol/dr              = -1/2*(Gpol/denominator2)*(2r - r/2*exp() )
         RealOpenMM dGpol_dr           = -Gpol*( one - fourth*expTerm )/denominator2;  

         // 5 FLOP

         RealOpenMM dGpol_dalpha2_ij   = -half*Gpol*expTerm*( one + D_ij )/denominator2;

         // 11 FLOP

         if( atomI != atomJ ){

             bornForces[atomJ]        += dGpol_dalpha2_ij*bornRadii[atomI];

             deltaX                   *= dGpol_dr;
             deltaY                   *= dGpol_dr;
             deltaZ                   *= dGpol_dr;

             forces[atomI][0]         += deltaX;
             forces[atomI][1]         += deltaY;
             forces[atomI][2]         += deltaZ;

             forces[atomJ][0]         -= deltaX;
             forces[atomJ][1]         -= deltaY;
             forces[atomJ][2]         -= deltaZ;

         } else {
            Gpol *= half;
         }

         // 3 FLOP

         obcEnergy         += Gpol;
         bornForces[atomI] += dGpol_dalpha2_ij*bornRadii[atomJ];

//if( logFile && (atomI == -1 || atomJ == -1) ){
//   (void) fprintf( logFile, "\nWWX %d %d F[%.6e %.6e %.6e] bF=[%.6e %.6e] Gpl[%.6e %.6e %.6e] rb[%6.4f %7.4f] rs[%6.4f %7.4f] ",
//                    atomI, atomJ,
//                    forces[atomI][0],  forces[atomI][1],  forces[atomI][2],
//                    bornForces[atomI], bornForces[atomJ],
//                    Gpol,dGpol_dr,dGpol_dalpha2_ij,
//                    bornRadii[atomI],bornRadii[atomJ],atomicRadii[atomI],atomicRadii[atomJ] );
//
//   (void) fprintf( logFile, "\nWWX %d %d %.1f r2=%.4f q=%.2f bF=[%.6e %.6e] Gpl[%.6e %.6e %.6e] rb[%.5f %.5f] add[%.6e %.6e] ",
//                    atomI, atomJ, preFactor, r2, partialCharges[atomJ],
//                    bornForces[atomI], bornForces[atomJ],
//                    Gpol,dGpol_dr,dGpol_dalpha2_ij,
//                    bornRadii[atomI], bornRadii[atomJ],
//                    dGpol_dalpha2_ij*bornRadii[atomJ], dGpol_dalpha2_ij*bornRadii[atomI] );
//}
      }

   }

if( logFile ){
   (void) fprintf( logFile, "\nWXX bF & F E=%.8e preFactor=%.5f", obcEnergy, preFactor );
   for( int atomI = 0; atomI < numberOfAtoms; atomI++ ){
      (void) fprintf( logFile, "\nWXX %d q=%.4f bR=%.5e bF=%.3f F[%.6e %.6e %.6e] ",
                      atomI, partialCharges[atomI], bornRadii[atomI],  bornForces[atomI], forces[atomI][0],  forces[atomI][1],  forces[atomI][2] );
   }
}


   if( 1 ){
      std::string outputFileName = "Loop1Cpu.txt";
      CpuObcSoftcore::writeForceLoop1( numberOfAtoms, forces, bornForces, outputFileName );
/*
      for( int atomI = 0; atomI < numberOfAtoms; atomI++ ){
         forces[atomI][0] = forces[atomI][1] = forces[atomI][2] = (RealOpenMM) 0.0;
      }
*/
   }

   // ---------------------------------------------------------------------------------------

   // second main loop

   // initialize Born radii & ObcChain temp arrays -- contain values
   // used in next iteration

   RealOpenMM* bornRadiiTemp             = getBornRadiiTemp();
   memset( bornRadiiTemp, 0, arraySzInBytes );

   RealOpenMM* obcChainTemp              = getObcChainTemp();
   memset( obcChainTemp, 0, arraySzInBytes );

   RealOpenMM* obcChain                  = getObcChain();
   const RealOpenMM* atomicRadii         = obcSoftcoreParameters->getAtomicRadii();

   const RealOpenMM alphaObc             = obcSoftcoreParameters->getAlphaObc();
   const RealOpenMM betaObc              = obcSoftcoreParameters->getBetaObc();
   const RealOpenMM gammaObc             = obcSoftcoreParameters->getGammaObc();
   const RealOpenMM* scaledRadiusFactor  = obcSoftcoreParameters->getScaledRadiusFactors();

    // compute factor that depends only on the outer loop index

   for( int atomI = 0; atomI < numberOfAtoms; atomI++ ){
      bornForces[atomI] *= bornRadii[atomI]*bornRadii[atomI]*obcChain[atomI];      
   }

   if( 1 ){

      std::string outputFileName = "PostLoop1Cpu.txt";

      IntVector chunkVector;
      chunkVector.push_back( 3 );

      RealOpenMMPtrPtrVector realPtrPtrVector;
      realPtrPtrVector.push_back( forces );

      RealOpenMMPtrVector realPtrVector;
      realPtrVector.push_back( bornRadii );
      realPtrVector.push_back( bornForces );
      realPtrVector.push_back( obcChain );

      CpuObcSoftcore::writeForceLoop( numberOfAtoms, chunkVector, realPtrPtrVector, realPtrVector, outputFileName );
   }

RealOpenMM* bornSumArray = (RealOpenMM*) malloc( sizeof( RealOpenMM )*numberOfAtoms );
memset( bornSumArray, 0, sizeof( RealOpenMM )*numberOfAtoms );
/*
for( int atomI = 0; atomI < numberOfAtoms; atomI++ ){
   forces[atomI][0]  = 0.0;
   forces[atomI][1]  = 0.0;
   forces[atomI][2]  = 0.0;
} */
   

   for( int atomI = 0; atomI < numberOfAtoms; atomI++ ){
 
      // radius w/ dielectric offset applied

      RealOpenMM radiusI        = atomicRadii[atomI];
      RealOpenMM offsetRadiusI  = radiusI - dielectricOffset;

      // used to compute Born radius for next iteration

      RealOpenMM bornSum        = zero;

      for( int atomJ = 0; atomJ < numberOfAtoms; atomJ++ ){

         if( atomJ != atomI ){

            RealOpenMM deltaR[ReferenceForce::LastDeltaRIndex];
            if (_obcSoftcoreParameters->getPeriodic())
               ReferenceForce::getDeltaRPeriodic( atomCoordinates[atomI], atomCoordinates[atomJ], _obcSoftcoreParameters->getPeriodicBox(), deltaR );
            else 
               ReferenceForce::getDeltaR( atomCoordinates[atomI], atomCoordinates[atomJ], deltaR );
            if (_obcSoftcoreParameters->getUseCutoff() && deltaR[ReferenceForce::RIndex] > _obcSoftcoreParameters->getCutoffDistance())
                   continue;
   
            RealOpenMM deltaX             = deltaR[ReferenceForce::XIndex];
            RealOpenMM deltaY             = deltaR[ReferenceForce::YIndex];
            RealOpenMM deltaZ             = deltaR[ReferenceForce::ZIndex];
            RealOpenMM r                  = deltaR[ReferenceForce::RIndex];

            // radius w/ dielectric offset applied

            RealOpenMM radiusJ            = atomicRadii[atomJ] - dielectricOffset;

            RealOpenMM scaledRadiusJ      = radiusJ*scaledRadiusFactor[atomJ];
            RealOpenMM scaledRadiusJ2     = scaledRadiusJ*scaledRadiusJ;
            RealOpenMM rScaledRadiusJ     = r + scaledRadiusJ;

            // L_ij != 1 && U_ij != 1

            // dL/dr & dU/dr are zero (this can be shown analytically)
            // removed from calculation

            if( offsetRadiusI < rScaledRadiusJ ){

               RealOpenMM l_ij            = offsetRadiusI > FABS( r - scaledRadiusJ ) ? offsetRadiusI : FABS( r - scaledRadiusJ );
                          l_ij            = one/l_ij;

               RealOpenMM l_ij2           = l_ij*l_ij;

               RealOpenMM u_ij            = one/rScaledRadiusJ;
               RealOpenMM u_ij2           = u_ij*u_ij;
 
               RealOpenMM rInverse        = one/r;
               RealOpenMM r2Inverse       = rInverse*rInverse;

               RealOpenMM logRatio        = LN( u_ij/l_ij );
               RealOpenMM t3              = eighth*(one + scaledRadiusJ2*r2Inverse)*(l_ij2 - u_ij2) + fourth*logRatio*r2Inverse;

               RealOpenMM de              = bornForces[atomI]*t3*rInverse;

               deltaX                    *= de;
               deltaY                    *= de;
               deltaZ                    *= de;

               forces[atomI][0]          -= deltaX;
               forces[atomI][1]          -= deltaY;
               forces[atomI][2]          -= deltaZ;
  
               forces[atomJ][0]          += deltaX;
               forces[atomJ][1]          += deltaY;
               forces[atomJ][2]          += deltaZ;
 
               // Born radius term

               RealOpenMM term            =   l_ij - u_ij + fourth*r*(u_ij2 - l_ij2) + (half*rInverse)*logRatio + (fourth*scaledRadiusJ*scaledRadiusJ*rInverse)*(l_ij2-u_ij2);

               if( offsetRadiusI < (scaledRadiusJ - r) ){
                  term += two*( (one/offsetRadiusI) - l_ij);
               }
               bornSum += term; 

if( atomI == -1 || atomJ == -1 ){
   (void) fprintf( logFile, "\nXXY %d %d de=%.6e bF[%.6e %6e] t3=%.6e r=%.6e trm=%.6e bSm=%.6e f[%.6e %.6e %.6e]",
                   atomI, atomJ, de,
                   bornForces[atomI], obcChain[atomI],
                   t3, r, term, bornSum, forces[atomI][0],  forces[atomI][1],  forces[atomI][2] );
}
            }
        }
      }

      bornSumArray[atomI] = bornSum;

      // OBC-specific code (Eqs. 6-8 in paper)

      bornSum             *= half*offsetRadiusI;
      RealOpenMM sum2      = bornSum*bornSum;
      RealOpenMM sum3      = bornSum*sum2;
      RealOpenMM tanhSum   = TANH( alphaObc*bornSum - betaObc*sum2 + gammaObc*sum3 );
      
      bornRadiiTemp[atomI] = one/( one/offsetRadiusI - tanhSum/radiusI); 
 
      obcChainTemp[atomI]  = offsetRadiusI*( alphaObc - two*betaObc*bornSum + three*gammaObc*sum2 );
      obcChainTemp[atomI]  = (one - tanhSum*tanhSum)*obcChainTemp[atomI]/radiusI;

if( logFile && atomI >= 0 ){
   (void) fprintf( logFile, "\nXXX %d bSum[%.6e %.6e %.6e] bRt=[%.6e %6e] obc=%.6e rI=[%.5f %.5f]",
                   atomI, bornSumArray[atomI], bornSum, tanhSum, bornRadii[atomI], bornRadiiTemp[atomI], obcChainTemp[atomI], radiusI, offsetRadiusI );
}

   }

   RealOpenMM conversion = (RealOpenMM)0.4184;  
   for( int atomI = 0; atomI < numberOfAtoms; atomI++ ){
      forces[atomI][0] *= conversion;
      forces[atomI][1] *= conversion;
      forces[atomI][2] *= conversion;
   }
   setEnergy( obcEnergy*conversion );

   if( 1 ){

      std::string outputFileName = "Loop2Cpu.txt";

      IntVector chunkVector;
      chunkVector.push_back( 3 );

      RealOpenMMPtrPtrVector realPtrPtrVector;
      realPtrPtrVector.push_back( forces );

      RealOpenMMPtrVector realPtrVector;
      realPtrVector.push_back( bornSumArray );
      // realPtrVector.push_back( bornRadiiTemp );
      // realPtrVector.push_back( obcChainTemp );

      CpuObcSoftcore::writeForceLoop( numberOfAtoms, chunkVector, realPtrPtrVector, realPtrVector, outputFileName );
   }

   if( bornSumArray ){
      free( (char*) bornSumArray );
   }

   // 6 FLOP

/*
   RealOpenMM forceFactor    = getForceConversionFactor();
   RealOpenMM constantFactor = 1.0f/electricConstant;
   if( fabs(forceFactor - 1.0f) > 1.0e-04 ){
      constantFactor *= forceFactor;
      for( int ii = 0; ii < numberOfAtoms; ii++ ){
         forces[ii][0]  *= forceFactor;
         forces[ii][1]  *= forceFactor;
         forces[ii][2]  *= forceFactor;
      }
   } */

   // copy new Born radii and obcChain values into permanent array

//(void) fprintf( logFile, "\nBorn radii not being updated!!!!" );
   memcpy( bornRadii, bornRadiiTemp, arraySzInBytes );
   memcpy( obcChain, obcChainTemp, arraySzInBytes );

   if( logFile ){
      (void) fclose( logFile );
   }

   return SimTKOpenMMCommon::DefaultReturn;

}