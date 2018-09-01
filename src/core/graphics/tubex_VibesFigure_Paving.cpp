/* ============================================================================
 *  tubex-lib - VibesFigure_Paving class
 * ============================================================================
 *  Copyright : Copyright 2017 Simon Rohou
 *  License   : This program is distributed under the terms of
 *              the GNU Lesser General Public License (LGPL).
 *
 *  Author(s) : Simon Rohou
 *  Bug fixes : -
 *  Created   : 2015
 * ---------------------------------------------------------------------------- */

#include "tubex_VibesFigure_Paving.h"

using namespace std;
using namespace ibex;

namespace tubex
{
  VibesFigure_Paving::VibesFigure_Paving(const string& fig_name, const Paving *paving)
    : VibesFigure(fig_name), m_paving(paving)
  {
    assert(paving != NULL);

    set_properties(100, 100, 500, 500); // default properties
    axis_limits(paving->box());

    // Default color map
    map<int,string> color_map;
    color_map[VALUE_IN] = "#9C9C9C[green]";
    color_map[VALUE_MAYBE] = "#9C9C9C[yellow]";
    color_map[VALUE_OUT] = "#9C9C9C[cyan]";
    set_color_map(color_map);
  }

  void VibesFigure_Paving::show()
  {
    vibes::clearGroup(name(), "val_in");
    vibes::clearGroup(name(), "val_maybe");
    vibes::clearGroup(name(), "val_out");
    draw_paving(m_paving);
  }

  void VibesFigure_Paving::set_color_map(const map<int,string> color_map)
  {
    vibes::newGroup("val_in", color_map.at(VALUE_IN), vibesParams("figure", name()));
    vibes::newGroup("val_maybe", color_map.at(VALUE_MAYBE), vibesParams("figure", name()));
    vibes::newGroup("val_out", color_map.at(VALUE_OUT), vibesParams("figure", name()));
  }

  void VibesFigure_Paving::draw_paving(const Paving *paving)
  {
    assert(paving != NULL);
    
    if(paving->is_leaf())
    {
      string color_group;
      switch(paving->value())
      {
        case VALUE_IN:
          color_group = "val_in";
          break;

        case VALUE_OUT:
          color_group = "val_out";
          break;

        case VALUE_MAYBE:
        default:
          color_group = "val_maybe";
      }

      draw_box(paving->box(), vibesParams("figure", name(), "group", color_group));
    }

    else
    {
      draw_paving(paving->get_first_subpaving());
      draw_paving(paving->get_second_subpaving());
    }
  }
}