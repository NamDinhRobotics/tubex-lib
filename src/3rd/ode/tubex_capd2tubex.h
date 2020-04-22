/** 
 *  \file
 *  CAPD to Tubex
 * ----------------------------------------------------------------------------
 *  \date       2020
 *  \author     Julien Damers
 *  \copyright  Copyright 2020 Tubex
 *  \license    This program is distributed under the terms of
 *              the GNU Lesser General Public License (LGPL).
 */

#ifndef __TUBEX_CAPD_H__
#define __TUBEX_CAPD_H__

#include "tubex_TubeVector.h"
#include "ibex_IntervalVector.h"

namespace tubex
{
  /** \brief  Returns a std::vector<ibex::IntervalVector> corresponding to the guaranteed curve computed by CAPD

   * \param f function that we would like to integrate
   * \param x0 The initial condition
   * \param timestep time step desired for the integration. If equal to 0 CAPD will calculate the timesep by itself
   * to increase calculation speed
   */

  std::vector<ibex::IntervalVector> capd2ibex(const ibex::Interval domain, const tubex::Function& f, const ibex::IntervalVector& x0,
                                              const double timestep=0);


  /** \brief Convert a std::vector<ibex::IntervalVector> corresponding to the guaranteed curve computed by CAPD into a
   * tubex::TubeVector
   * \param ibex_curve a vector of ibex::IntervalVectors representing the curve computed by CAPD
   * */
  tubex::TubeVector ibex2tubex(std::vector<ibex::IntervalVector> ibex_curve);

  /** \brief Combination of capd2ibex and ibex2tubex, to generate a tube from a curve obtained by
   * the guaranteed integration of CAPD
   * \param result Tube Vector that will store the result of the integration
   * \param domain period of tine on which we would like to perform the integration
   * \param f function that we would like to integrate
   * \param x0 The initial condition
   * \param timestep time step desired for the integration. If equal to 0 CAPD will calculate the timesep by itself
   * to increase calculation speed
   */

  int capd2tubex(tubex::TubeVector& result,const tubex::Function& f,
                 const ibex::IntervalVector& x0, const double timestep=0);
}

#endif