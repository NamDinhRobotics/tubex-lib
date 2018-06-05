/* ============================================================================
 *  tubex-lib - DomainException class
 * ============================================================================
 *  Copyright : Copyright 2017 Simon Rohou
 *  License   : This program is distributed under the terms of
 *              the GNU Lesser General Public License (LGPL).
 *
 *  Author(s) : Simon Rohou
 *  Bug fixes : -
 *  Created   : 2015
 * ---------------------------------------------------------------------------- */

#include "tubex_StructureException.h"
#include "tubex_DomainException.h"
#include "tubex_Tube.h"
#include "tubex_TubeComponent.h"
#include "tubex_TubeSlice.h"
#include <string>
#include <sstream>

using namespace std;
using namespace ibex;

namespace tubex
{
  StructureException::StructureException(const TubeComponent& x1, const TubeComponent& x2)
  {
    ostringstream os;
    os << "unable to perform an operation over tubes of different structure";

    if(x1.nbSlices() != x2.nbSlices())
      os << endl << "TubeComponents of different slices number: " 
         << "n1=" << x1.nbSlices() << " and n2=" << x2.nbSlices();

    os << endl;
    m_what_msg = os.str();
  }

  void StructureException::check(const TubeComponent& x1, const TubeComponent& x2)
  {
    DomainException::check(x1, x2);
    if(x1.nbSlices() != x2.nbSlices())
      throw StructureException(x1, x2);

    TubeSlice *slice_x1 = x1.getFirstSlice();
    TubeSlice *slice_x2 = x2.getFirstSlice();

    while(slice_x1 != NULL)
    {
      if(slice_x1->domain() != slice_x2->domain())
        throw StructureException(*slice_x1, *slice_x2);

      slice_x1 = slice_x1->nextSlice();
      slice_x2 = slice_x2->nextSlice();
    }
  }

    StructureException::StructureException(const Tube& x1, const Tube& x2)
    {
      ostringstream os;
      os << "unable to perform an operation over tubes of different structure";

      if(x1.nbSlices() != x2.nbSlices())
        os << endl << "TubeComponents of different slices number: " 
           << "n1=" << x1.nbSlices() << " and n2=" << x2.nbSlices();

      os << endl;
      m_what_msg = os.str();
    }

    void StructureException::check(const Tube& x1, const Tube& x2)
    {
      DomainException::check(x1, x2);
      if(x1.nbSlices() != x2.nbSlices())
        throw StructureException(x1, x2);

      TubeSlice *slice_x1 = x1.getFirstSlice();
      TubeSlice *slice_x2 = x2.getFirstSlice();

      while(slice_x1 != NULL)
      {
        if(slice_x1->domain() != slice_x2->domain())
          throw StructureException(*slice_x1, *slice_x2);

        slice_x1 = slice_x1->nextSlice();
        slice_x2 = slice_x2->nextSlice();
      }
    }
}