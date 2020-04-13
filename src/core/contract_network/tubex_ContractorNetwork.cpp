/** 
 *  ContractorNetwork class
 * ----------------------------------------------------------------------------
 *  \date       2020
 *  \author     Simon Rohou
 *  \copyright  Copyright 2020 Simon Rohou
 *  \license    This program is distributed under the terms of
 *              the GNU Lesser General Public License (LGPL).
 */

#include <iostream>
#include <fstream>
#include "tubex_ContractorNetwork.h"
#include "tubex_CtcFwdBwd.h"
#include "tubex_CtcEval.h"
#include "tubex_Figure.h" // for add_suffix

#include "ibex_IntervalVector.h"

using namespace std;
using namespace ibex;

namespace tubex
{
  ContractorNetwork::ContractorNetwork()
  {

  }

  ContractorNetwork::~ContractorNetwork()
  {
    for(auto& dom : m_v_domains)
      delete dom;
    for(auto& ctc : m_v_ctc)
      delete ctc;

    if(m_ctc_deriv != NULL)
      delete m_ctc_deriv;
  }

  int ContractorNetwork::nb_ctc() const
  {
    return m_v_ctc.size();
  }

  int ContractorNetwork::nb_dom() const
  {
    return m_v_domains.size();
  }

  int ContractorNetwork::nb_ctc_in_stack() const
  {
    return m_deque.size();
  }

  void ContractorNetwork::set_fixedpoint_ratio(float r)
  {
    assert(Interval(0.,1).contains(r));
    m_fixedpoint_ratio = r;
  }

  void ContractorNetwork::set_all_contractors_active()
  {
    m_deque.clear();

    for(auto& ctc : m_v_ctc)
    {
      ctc->set_active(true);
      add_to_queue(ctc, m_deque);
    }
  }

  double ContractorNetwork::contract(bool verbose)
  {
    clock_t t_start = clock();

    if(verbose)
    {
      cout << "Contractor network has " << m_v_ctc.size()
           << " contractors and " << m_v_domains.size() << " domains" << endl;
      cout << "Computing, " << nb_ctc_in_stack() << " contractors in m_deque";
      if(!isinf(m_contraction_duration_max))
        cout << " during " << m_contraction_duration_max << "s";
      cout << endl;
    }

    while(!m_deque.empty()
      && (double)(clock() - t_start)/CLOCKS_PER_SEC < m_contraction_duration_max)
    {
      Contractor *ctc = m_deque.front();
      m_deque.pop_front();

      ctc->contract();
      ctc->set_active(false);

      for(auto& ctc_dom : ctc->domains()) // for each domain related to this contractor
      {
        // If the domain has "changed" after the contraction
        propagate_ctc_from_domain(ctc_dom, ctc);
      }
    }

    if(verbose)
      cout << endl
           << "  computation time: " << (double)(clock() - t_start)/CLOCKS_PER_SEC << "s" << endl;

    // Emptiness test
    // todo: test only contracted domains?
    //for(const auto& ctc : m_v_ctc)
      for(const auto& dom : m_v_domains)
        if(dom->is_empty())
        {
          cout << "  warning: empty set" << endl;
          exit(1);
        }

    return (double)(clock() - t_start)/CLOCKS_PER_SEC;
  }

  void ContractorNetwork::propagate_ctc_from_domain(Domain *dom, Contractor *ctc_to_avoid)
  {
    double current_volume = dom->compute_volume(); // new volume after contraction

    if(current_volume/dom->get_saved_volume() < 1.-m_fixedpoint_ratio)
    {
      // We activate each contractor related to these domains, according to graph orientation

      // Local deque, for specific order related to this domain
      deque<Contractor*> ctc_deque;

      for(auto& ctc_of_dom : dom->contractors()) 
        if(ctc_of_dom != ctc_to_avoid && !ctc_of_dom->is_active())
        {
          ctc_of_dom->set_active(true);
          add_to_queue(ctc_of_dom, ctc_deque);
        }

      // Merging this local deque in the CN one
      for(auto& c : ctc_deque)
        m_deque.push_front(c);
    }
    
    dom->set_volume(current_volume); // updating old volume
  }

  double ContractorNetwork::contract_during(double dt, bool verbose)
  {
    double prev_dt = m_contraction_duration_max;
    m_contraction_duration_max = dt;
    double contraction_time = contract(verbose);
    m_contraction_duration_max = prev_dt;
    return contraction_time;
  }

  Interval& ContractorNetwork::create_var(const Interval& i)
  {
    return add_domain(new Domain(i))->interval();
  }

  IntervalVector& ContractorNetwork::create_var(const IntervalVector& iv)
  {
    return add_domain(new Domain(iv))->interval_vector();
  }

  Tube& ContractorNetwork::create_var(const Tube& t)
  {
    return add_domain(new Domain(t))->tube();
  }

  TubeVector& ContractorNetwork::create_var(const TubeVector& tv)
  {
    return add_domain(new Domain(tv))->tube_vector();
  }

  void ContractorNetwork::add(ibex::Ctc& ctc, const vector<Domain>& v_domains)
  {
    Contractor *abstract_ctc = new Contractor(ctc);
    for(const auto& dom : v_domains)
      add_domain(new Domain(dom), abstract_ctc);
    add_contractor(abstract_ctc);
  }

  void ContractorNetwork::add(tubex::Ctc& ctc, const vector<Domain>& v_domains)
  {
    bool breakdown_to_slice_level = false;

    if(typeid(ctc) == typeid(CtcEval))
    {
      assert(v_domains.size() == 3 || v_domains.size() == 4);

      if(v_domains.size() == 4) // with derivative information
      {
        static_cast<CtcEval&>(ctc).enable_temporal_propagation(false);

        if(m_ctc_deriv == NULL)
          m_ctc_deriv = new CtcDeriv();
        add(*m_ctc_deriv, {v_domains[2], v_domains[3]});
      }
    }

    if(!ctc.is_intertemporal())
    {
      // todo: check that the related constraint is not intertemporal
      // otherwise, tube level only

      if(v_domains[0].type() == Domain::Type::TUBE || v_domains[0].type() == Domain::Type::TUBE_VECTOR)
        breakdown_to_slice_level = true;
      // todo: improve this ^
      // and check that all domains are either tubes/tubevectors or interval/intervalvectors
    }

    // Towards contractors on slices scale, if possible
    // (in case of non-intertemporal constraints)
    if(breakdown_to_slice_level)
    {
      vector<const Slice*> v_slices;

      // Vector initialization with first slices of each tube
      int nb_slices = -1;
      for(const auto& dom : v_domains)
      {
        add_domain(new Domain(dom));

        switch(dom.type())
        {
          case Domain::Type::TUBE:
          {
            if(nb_slices == -1)
              nb_slices = dom.tube().nb_slices();

            else
              assert(nb_slices == dom.tube().nb_slices());

            v_slices.push_back(dom.tube().first_slice());
          }
          break;

          case Domain::Type::TUBE_VECTOR:
          {            
            for(int j = 0 ; j < dom.tube_vector().size() ; j++)
            {
              if(nb_slices == -1)
                nb_slices = dom.tube_vector()[j].nb_slices();

              else
                assert(nb_slices == dom.tube_vector()[j].nb_slices());

              v_slices.push_back(dom.tube_vector()[j].first_slice());
            }
          }
          break;

          default:
            assert(false && "domain is not a tube or a tube vector");
        }
      }

      // Adding each row of slices
      for(int k = 0 ; k < nb_slices ; k++)
      {
        vector<Domain> v_slices_domains;
        for(size_t i = 0 ; i < v_slices.size() ; i++)
          v_slices_domains.push_back(Domain(const_cast<Slice&>(*v_slices[i])));

        add(ctc, v_slices_domains); 

        for(auto& s : v_slices)
          s = s->next_slice();
      }
    }

    else
    {
      Contractor *abstract_ctc = new Contractor(ctc);
      for(const auto& dom : v_domains)
        add_domain(new Domain(dom), abstract_ctc);
      add_contractor(abstract_ctc);
    }
  }
  
  void ContractorNetwork::add_data(Tube& tube, double t, const Interval& y)
  {
    Domain *ad = add_domain(new Domain(tube));
    assert(ad->type() == Domain::Type::TUBE);
    ad->add_data(t, y, *this);
  }
  
  void ContractorNetwork::add_data(TubeVector& tube, double t, const IntervalVector& y)
  {
    Domain *ad = add_domain(new Domain(tube));
    assert(ad->type() == Domain::Type::TUBE_VECTOR);
    ad->add_data(t, y, *this);
  }

  Domain* ContractorNetwork::add_domain(Domain *ad)
  {
    if(ad->is_empty())
    {
      cout << "warning: adding empty domain" << *ad << endl;
      exit(1);
    }

    // Looking if this domain is not already part of the graph
    for(auto& dom : m_v_domains)
      if(*dom == *ad) // found
      {
        delete ad;
        return dom;
      }
    
    // Else, add this new domain
    
      m_v_domains.push_back(ad);

    // And add possible dependencies

      switch(ad->type())
      {
        case Domain::Type::INTERVAL:
          // nothing to do
          break;

        case Domain::Type::SLICE:
          // nothing to do
          break;

        case Domain::Type::TUBE_VECTOR:
        {
          Contractor *ac_component = new Contractor(ContractorType::COMPONENT);
          add_domain(new Domain(ad->tube_vector()), ac_component); // adding vector
          for(int i = 0 ; i < ad->tube_vector().size() ; i++)
            add_domain(new Domain(ad->tube_vector()[i]), ac_component); // adding its components
          add_contractor(ac_component);
        }
        break;

        case Domain::Type::INTERVAL_VECTOR:
        {
          Contractor *ac_component = new Contractor(ContractorType::COMPONENT);
          add_domain(new Domain(ad->interval_vector()), ac_component); // adding vector
          for(int i = 0 ; i < ad->interval_vector().size() ; i++)
            add_domain(new Domain(ad->interval_vector()[i]), ac_component); // adding its components
          add_contractor(ac_component);
        }
        break;

        case Domain::Type::TUBE:
        {
          // Dependencies tube <-> slice
          Contractor *ac_component = new Contractor(ContractorType::COMPONENT);
          add_domain(new Domain(ad->tube()), ac_component); // adding tube
          for(Slice *s = ad->tube().first_slice() ; s != NULL ; s = s->next_slice())
            add_domain(new Domain(*s), ac_component); // adding one of its slices
          add_contractor(ac_component);

          // Dependencies slice <-> slice
          for(Slice *s = ad->tube().first_slice() ; s->next_slice() != NULL ; s = s->next_slice())
          {
            Contractor *ac_component_slices = new Contractor(ContractorType::COMPONENT);
            add_domain(new Domain(*s), ac_component_slices);
            add_domain(new Domain(*(s->next_slice())), ac_component_slices);
            add_contractor(ac_component_slices);
          }
        }
        break;
      }

    return ad;
  }

  void ContractorNetwork::add_domain(Domain *ad, Contractor *ac)
  {
    // The domain ad may be already in the graph, we are looking for it,
    // or we use the current pointer. The result is pointed by ad_.
    Domain *ad_ = add_domain(ad);

    ac->domains().push_back(ad_);
    ad_->contractors().push_back(ac);
  }

  Contractor* ContractorNetwork::add_contractor(Contractor *&ac)
  {
    // Looking if this contractor is not already part of the graph
    for(auto& ctc : m_v_ctc)
      if(*ctc == *ac) // found
      {
        delete ac;
        return ctc;
      }

    m_v_ctc.push_back(ac);
    add_to_queue(ac, m_deque);
    return ac;
  }

  IntervalVector& ContractorNetwork::subvector(IntervalVector& iv, int start_index, int end_index)
  {
    // todo: assert here on indexes

    IntervalVector *subvector = new IntervalVector(iv.subvector(start_index, end_index));
    //m_intervalvectors_to_delete.push_front(subvector);

    for(int i = start_index ; i <= end_index ; i++)
    {
      Contractor *ac_eq = new Contractor(ContractorType::EQUALITY);
      add_domain(new Domain((*subvector)[i-start_index]), ac_eq); // adding vector
      add_domain(new Domain(iv[i]), ac_eq); // adding vector
      add_contractor(ac_eq);
    }

    return add_domain(new Domain(*subvector))->interval_vector();
  }

  TubeVector& ContractorNetwork::subvector(TubeVector& iv, int start_index, int end_index)
  {
//    // todo: assert here on indexes
//
//    TubeVector *subvector = new TubeVector(iv.subvector(start_index, end_index));
//    m_tubevectors_to_delete.push_front(subvector);
//
//    for(int i = start_index ; i <= end_index ; i++)
//    {
//      assert(Tube::same_slicing(iv[i], (*subvector)[i-start_index]));
//      Contractor *ac_eq = new Contractor(ContractorType::EQUALITY);
//      //add_domain(new Domain((*subvector)[i-start_index]), ac_eq); // adding vector
//      //add_domain(new Domain(iv[i]), ac_eq); // adding vector
//      //add_contractor(ac_eq);
//
//      // todo: add EQUALITY link to slices too? already done?
//      // todo: check non reundant addition of domains
//    }
////add_domain(new Domain(*subvector));
//    return *subvector;
    return iv;
  }

  void ContractorNetwork::add_to_queue(Contractor *ac, deque<Contractor*>& ctc_deque)
  {
    // todo: propagate for EQUALITY contractors even in case of poor contractions?

    if(ac->type() == ContractorType::COMPONENT)
      ctc_deque.push_back(ac);

    else
      ctc_deque.push_front(ac); // priority
  }
  
  void ContractorNetwork::set_name(Domain dom, const string& name)
  {
    Domain *temp_dom = new Domain(dom);
    Domain *added_dom = add_domain(temp_dom);
    assert(added_dom != temp_dom);
    added_dom->set_name(name);
  }
  
  void ContractorNetwork::set_name(Contractor ctc, const string& name)
  {
    bool contractor_found = false;
    for(const auto& added_ctc: m_v_ctc)
      if(added_ctc->comes_from(ctc))
      {
        added_ctc->set_name(name);
        contractor_found = true;
      }

    assert(contractor_found);
  }

  int ContractorNetwork::print_dot_graph(const string& cn_name, const string& prog) const
  {
    ofstream dot_file;
    dot_file.open(cn_name + ".dot");

    dot_file << "graph graphname {" << endl;

    int d = 0;
    dot_file << endl << "  // Domains nodes" << endl;
    for(const auto& dom : m_v_domains)
    {
      d++;
      dot_file << "  " << Figure::add_suffix("dom",d) << " [shape=box,label=\"" << dom->name(m_v_domains) << "\"];" << endl;
    }

    int c = 0;
    dot_file << endl << "  // Contractors nodes" << endl;
    for(const auto& ctc : m_v_ctc)
    {
      c++;
      dot_file << "  " << Figure::add_suffix("ctc",c) << " [shape=circle,label=\"\\mathcal{C}_{" << ctc->name() << "}\"];" << endl;
    }

    dot_file << endl << "  // Relations" << endl;
    c = 0;
    for(const auto& ctc : m_v_ctc)
    {
      c++;
      d = 0;
      for(const auto& dom : m_v_domains)
      {
        d++;
        if(find(dom->contractors().begin(), dom->contractors().end(), ctc) != dom->contractors().end())
          dot_file << "  " << Figure::add_suffix("ctc",c) << " -- " << Figure::add_suffix("dom",d) << ";" << endl;
      }
    }

    dot_file << "}" << endl;
    dot_file.close();

    return system((("dot2tex --crop --texmode=math --autosize --prog=" + prog + " -ftikz ") + cn_name + ".dot > "
                   + cn_name + ".tex ; pdflatex " + cn_name + ".tex").c_str());    
  }
}