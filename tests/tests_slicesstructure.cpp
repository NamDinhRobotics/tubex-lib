#include "tests.h"

using namespace Catch;
using namespace Detail;
using namespace std;
using namespace ibex;
using namespace tubex;

TEST_CASE("operator=")
{
  SECTION("test")
  {
    TubeVector tube_one_slice(Interval(0.,10.), 0.2);
    tube_one_slice.sample(5.);

    TubeVector tube1(Interval(0.,10.), 0.001, 4);
    TubeVector tube2(tube1);
    CHECK(tube1 == tube2);
    CHECK(tube1.nb_slices() != 1);
    CHECK(tube1.nb_slices() == tube2.nb_slices());

    Tube tube1_scalar(Interval(0.,10.), 0.001);
    Tube tube2_scalar(tube1_scalar);
    CHECK(tube1_scalar == tube2_scalar);
    CHECK(tube1_scalar.nb_slices() != 1);
    CHECK(tube1_scalar.nb_slices() == tube2_scalar.nb_slices());
  }
}

TEST_CASE("input2index")
{
  SECTION("input2index")
  {
    Tube tube = tube_test_1();
    CHECK(tube.input2index(0.0) == 0);
    CHECK(tube.input2index(0.1) == 0);
    CHECK(tube.input2index(0.5) == 0);
    CHECK(tube.input2index(0.6) == 0);
    CHECK(tube.input2index(0.9) == 0);
    CHECK(tube.input2index(ibex::previous_float(1.0)) == 0);
    CHECK(tube.input2index(1.0) == 1);
    CHECK(tube.input2index(46.0) == 45);
    CHECK(tube.input2index(tube.domain().ub()) == 45);
    CHECK(tube.nb_slices() == 46);
  }

  SECTION("index")
  {
    Tube tube = tube_test_1();
    // todo: CHECK_THROWS(tube.index(tube.get_slice(-1)) == 3);
    CHECK(tube.index(tube.get_slice(0)) == 0);
    CHECK(tube.index(tube.get_slice(3)) == 3);
    CHECK(tube.index(tube.get_slice(45)) == 45);
    CHECK(tube.index(tube.get_first_slice()) == 0);
    CHECK(tube.index(tube.get_last_slice()) == 45);
  }
}

TEST_CASE("Tube slices structure")
{
  SECTION("Tube class (one slice)")
  {
    Tube tube_a(Interval(0.,1.), Interval(-1.,1.));
    CHECK(tube_a.domain() == Interval(0.,1.));
    CHECK(tube_a.codomain() == Interval(-1.,1.));
    CHECK(tube_a.nb_slices() == 1);
    // todo: CHECK(tube_a.get_slice(0)->tube_reference() == &tube_a);

    Tube tube_b(tube_a);
    CHECK(tube_b.domain() == Interval(0.,1.));
    CHECK(tube_b.codomain() == Interval(-1.,1.));
    CHECK(tube_b.nb_slices() == 1);
    CHECK(tube_b.get_slice(0) != NULL);
    // todo: CHECK(tube_b.get_slice(0)->tube_reference() == &tube_b);

    Tube tube_c = tube_a;
    CHECK(tube_c.domain() == Interval(0.,1.));
    CHECK(tube_c.codomain() == Interval(-1.,1.));
    CHECK(tube_c.nb_slices() == 1);
    // todo: CHECK(tube_c.get_slice(0)->tube_reference() == &tube_c);
  }

  SECTION("Tube class (several slices)")
  {
    Tube tube_a(Interval(0.,1.), Interval(-1.,1.));
    CHECK(tube_a.nb_slices() == 1);
    CHECK(tube_a.domain() == Interval(0.,1.));
    // todo: CHECK(tube_a.get_slice(0)->tube_reference() == &tube_a);

    CHECK_THROWS(tube_a.sample(-1.0););

    tube_a.sample(0.); // the following has no effect
    CHECK(tube_a.nb_slices() == 1);
    tube_a.sample(1.); // the following has no effect
    CHECK(tube_a.nb_slices() == 1);

    tube_a.sample(0.6);

    CHECK(tube_a.nb_slices() == 2);
    CHECK(tube_a.domain() == Interval(0.,1.));

    tube_a.sample(0.7);

    CHECK(tube_a.nb_slices() == 3);

    tube_a.sample(0.7); // the following has no more effect
    tube_a.sample(0.7);
    CHECK(tube_a.nb_slices() == 3);
    tube_a.sample(0.62);

    CHECK(tube_a.nb_slices() == 4);

    tube_a.sample(0.1);
    tube_a.sample(0.1); // the following has no more effect

    CHECK(tube_a.nb_slices() == 5);

    tube_a.sample(0.); // the following has no effect
    CHECK(tube_a.nb_slices() == 5);
    tube_a.sample(1.); // the following has no effect
    CHECK(tube_a.nb_slices() == 5);
  }

  SECTION("Sample method (bis)")
  {
    Tube tube(Interval(0.,6.), 2.);
    CHECK(tube.nb_slices() == 3);

    tube.set(Interval(-3.,2.), 0);
    tube.set(Interval(-1.,3.), 1);
    tube.set(Interval(-3.,2.), 2);
    CHECK(tube.nb_slices() == 3);
    tube.set(Interval(-3.,1.), 0.); // gate
    CHECK(tube.nb_slices() == 3);
    tube.set(Interval(0.,1.), 2.); // gate
    CHECK(tube.nb_slices() == 3);
    tube.set(Interval(-1.,1.), 4.); // gate
    CHECK(tube.nb_slices() == 3);
    tube.set(Interval(-1.,0.), 6.); // gate
    CHECK(tube.nb_slices() == 3);
    CHECK(tube(0) == Interval(-3.,2.));
    CHECK(tube(1) == Interval(-1.,3.));
    CHECK(tube(2) == Interval(-3.,2.));
    CHECK(tube(0.) == Interval(-3.,1.)); // gate
    CHECK(tube(2.) == Interval(0.,1.)); // gate
    CHECK(tube(4.) == Interval(-1.,1.)); // gate
    CHECK(tube(6.) == Interval(-1.,0.)); // gate

    tube.sample(1.);
    tube.sample(5.);

    CHECK(tube.nb_slices() == 5);
    CHECK(tube(0) == Interval(-3.,2.));
    CHECK(tube(1) == Interval(-3.,2.));
    CHECK(tube(2) == Interval(-1.,3.));
    CHECK(tube(3) == Interval(-3.,2.));
    CHECK(tube(4) == Interval(-3.,2.));
    CHECK(tube(0.) == Interval(-3.,1.)); // gate
    CHECK(tube(1.) == Interval(-3.,2.)); // gate
    CHECK(tube(2.) == Interval(0.,1.)); // gate
    CHECK(tube(3.) == Interval(-1.,3.));
    CHECK(tube(4.) == Interval(-1.,1.)); // gate
    CHECK(tube(5.) == Interval(-3.,2.)); // gate
    CHECK(tube(6.) == Interval(-1.,0.)); // gate
  }

  SECTION("input2index and reverse operation")
  {
    Tube tube(Interval(0.,1.), Interval(-1.,1.));
    tube.sample(0.6);
    tube.sample(0.7);
    tube.sample(0.62);
    tube.sample(0.1);

    // input2index
    CHECK(tube.nb_slices() == 5);
    CHECK_THROWS(tube.input2index(-0.1));
    CHECK(tube.input2index(0.) == 0);
    CHECK(tube.input2index(0.01) == 0);
    CHECK(tube.input2index(0.6) == 2);
    CHECK(tube.input2index(0.61) == 2);
    CHECK(tube.input2index(0.62) == 3);
    CHECK(tube.input2index(1.0) == 4);
    CHECK_THROWS(tube.input2index(1.01));
  }

  SECTION("Getting slices")
  {
    Tube tube(Interval(0.,1.), Interval(-1.,1.));
    // todo: CHECK_THROWS(tube.get_slice(1));
    tube.sample(0.6);
    tube.sample(0.7);
    tube.sample(0.62);
    tube.sample(0.1);
    CHECK(tube.nb_slices() == 5);

    // By indexes
    // todo: CHECK_THROWS(tube.get_slice(-1));
    CHECK(tube.get_slice(0)->domain() == Interval(0.0,0.1));
    CHECK(tube.get_slice(1)->domain() == Interval(0.1,0.6));
    CHECK(tube.get_slice(2)->domain() == Interval(0.6,0.62));
    CHECK(tube.get_slice(3)->domain() == Interval(0.62,0.7));
    CHECK(tube.get_slice(4)->domain() == Interval(0.7,1.0));

    // By times
    // todo: CHECK_THROWS(tube.get_slice(-0.001));
    // todo: CHECK_THROWS(tube.get_slice(1.001));
    CHECK(tube.get_slice(0.)->domain() == Interval(0.0,0.1));
    CHECK(tube.get_slice(0.03)->domain() == Interval(0.0,0.1));
    CHECK(tube.get_slice(0.2)->domain() == Interval(0.1,0.6));
    CHECK(tube.get_slice(0.6)->domain() == Interval(0.6,0.62));
    CHECK(tube.get_slice(0.62)->domain() == Interval(0.62,0.7));
    CHECK(tube.get_slice(1.0)->domain() == Interval(0.7,1.0));

    // Copy: constructor
    Tube tube_b(tube);
    CHECK(tube_b.get_slice(0.)->domain() == Interval(0.0,0.1));
    CHECK(tube_b.get_slice(0.03)->domain() == Interval(0.0,0.1));
    CHECK(tube_b.get_slice(0.2)->domain() == Interval(0.1,0.6));
    CHECK(tube_b.get_slice(0.6)->domain() == Interval(0.6,0.62));
    CHECK(tube_b.get_slice(0.62)->domain() == Interval(0.62,0.7));
    CHECK(tube_b.get_slice(1.0)->domain() == Interval(0.7,1.0));
    
    // Copy: operator
    Tube tube_c = tube;
    CHECK(tube_c.get_slice(0.)->domain() == Interval(0.0,0.1));
    CHECK(tube_c.get_slice(0.03)->domain() == Interval(0.0,0.1));
    CHECK(tube_c.get_slice(0.2)->domain() == Interval(0.1,0.6));
    CHECK(tube_c.get_slice(0.6)->domain() == Interval(0.6,0.62));
    CHECK(tube_c.get_slice(0.62)->domain() == Interval(0.62,0.7));
    CHECK(tube_c.get_slice(1.0)->domain() == Interval(0.7,1.0));

    // First and last slices
    CHECK(tube.get_slice(0) == tube.get_first_slice());
    CHECK(tube.get_slice(4) == tube.get_last_slice());

    // Checking root references
    // todo: CHECK(tube.get_slice(0)->tube_reference() == &tube);
    // todo: CHECK(tube.get_slice(1)->tube_reference() == &tube);
    // todo: CHECK(tube.get_slice(2)->tube_reference() == &tube);
    // todo: CHECK(tube.get_slice(3)->tube_reference() == &tube);
    // todo: CHECK(tube.get_slice(4)->tube_reference() == &tube);

    // get_slices
    CHECK(tube.get_slice(0.1)->box()[0] == Interval(0.1,0.6));
    CHECK(tube.get_slice(0.1)->box()[1] == Interval(-1.,1.));
    CHECK(tube.get_slice(0.1)->domain() == Interval(0.1,0.6));
    CHECK(tube.get_slice(1)->box()[0] == Interval(0.1,0.6));
    CHECK(tube.get_slice(1)->box()[1] == Interval(-1.,1.));
    CHECK(tube.get_slice(1)->domain() == Interval(0.1,0.6));
    CHECK(tube.get_slice(4)->box()[0] == Interval(0.7,1.0));
    CHECK(tube.get_slice(4)->box()[1] == Interval(-1.,1.));
    CHECK(tube.get_slice(4)->domain() == Interval(0.7,1.0));
    // todo: CHECK_THROWS(tube.get_slice(5)->box());

    // prev/next slices
    CHECK(tube.get_slice(0)->prev_slice() == NULL);
    for(int i = 0 ; i < tube.nb_slices()-1 ; i++)
    {      
      CHECK(tube.get_slice(i)->next_slice() == tube.get_slice(i+1));
      CHECK(tube.get_slice(i+1)->prev_slice() == tube.get_slice(i));
    }
    CHECK(tube.get_slice(4)->next_slice() == NULL);

    Tube tube_simple(Interval(0.,1.), Interval(-1.,1.));
    CHECK(tube_simple.get_slice(0)->prev_slice() == NULL);
    CHECK(tube_simple.get_slice(0)->next_slice() == NULL);
    tube_simple.sample(0.3);
    CHECK(tube_simple.get_slice(0)->next_slice() == tube_simple.get_slice(1));
    CHECK(tube_simple.get_slice(1)->prev_slice() == tube_simple.get_slice(0));
  }

  SECTION("Gates")
  {
    Tube tube_a(Interval(0.,1.), Interval(-1.,1.));
    tube_a.set(Interval(0.), 0.);
    tube_a.set(Interval(0.2,0.3), 1.);
    tube_a.sample(0.6, Interval(0.0,0.5));

    CHECK(tube_a.get_slice(0)->input_gate() == 0.);
    CHECK(tube_a.get_slice(0)->output_gate() == Interval(0.,0.5));
    CHECK(tube_a.get_slice(1)->input_gate() == Interval(0.,0.5));
    CHECK(tube_a.get_slice(1)->output_gate() == Interval(0.2,0.3));

    tube_a.sample(0.7, Interval(0.2,0.3));
    tube_a.sample(0.62, Interval(-6.,6.)); // too large gate
    tube_a.sample(0.1, Interval(-1.0,-0.8));

    CHECK(tube_a.get_slice(0)->input_gate() == 0.);
    CHECK(tube_a.get_slice(0)->output_gate() == Interval(-1.,-0.8));
    CHECK(tube_a.get_slice(1)->input_gate() == Interval(-1.,-0.8));
    CHECK(tube_a.get_slice(1)->output_gate() == Interval(0.,0.5));
    CHECK(tube_a.get_slice(2)->input_gate() == Interval(0.,0.5));
    CHECK(tube_a.get_slice(2)->output_gate() == Interval(-1.,1.));
    CHECK(tube_a.get_slice(3)->input_gate() == Interval(-1.,1.));
    CHECK(tube_a.get_slice(3)->output_gate() == Interval(0.2,0.3));
    CHECK(tube_a.get_slice(4)->input_gate() == Interval(0.2,0.3));
    CHECK(tube_a.get_slice(4)->output_gate() == Interval(0.2,0.3));

    Tube tube_b(tube_a);
    CHECK(tube_b.get_slice(0)->input_gate() == 0.);
    CHECK(tube_b.get_slice(0)->output_gate() == Interval(-1.,-0.8));
    CHECK(tube_b.get_slice(1)->input_gate() == Interval(-1.,-0.8));
    CHECK(tube_b.get_slice(1)->output_gate() == Interval(0.,0.5));
    CHECK(tube_b.get_slice(2)->input_gate() == Interval(0.,0.5));
    CHECK(tube_b.get_slice(2)->output_gate() == Interval(-1.,1.));
    CHECK(tube_b.get_slice(3)->input_gate() == Interval(-1.,1.));
    CHECK(tube_b.get_slice(3)->output_gate() == Interval(0.2,0.3));
    CHECK(tube_b.get_slice(4)->input_gate() == Interval(0.2,0.3));
    CHECK(tube_b.get_slice(4)->output_gate() == Interval(0.2,0.3));

    Tube tube_c = tube_a;
    CHECK(tube_c.get_slice(0)->input_gate() == 0.);
    CHECK(tube_c.get_slice(0)->output_gate() == Interval(-1.,-0.8));
    CHECK(tube_c.get_slice(1)->input_gate() == Interval(-1.,-0.8));
    CHECK(tube_c.get_slice(1)->output_gate() == Interval(0.,0.5));
    CHECK(tube_c.get_slice(2)->input_gate() == Interval(0.,0.5));
    CHECK(tube_c.get_slice(2)->output_gate() == Interval(-1.,1.));
    CHECK(tube_c.get_slice(3)->input_gate() == Interval(-1.,1.));
    CHECK(tube_c.get_slice(3)->output_gate() == Interval(0.2,0.3));
    CHECK(tube_c.get_slice(4)->input_gate() == Interval(0.2,0.3));
    CHECK(tube_c.get_slice(4)->output_gate() == Interval(0.2,0.3));

    tube_a.sample(0.67); // no gate specification
    CHECK(tube_a.get_slice(0.67)->input_gate() == Interval(-1.,1));
    tube_a.sample(0.68, Interval::ALL_REALS);
    CHECK(tube_a.get_slice(0.68)->input_gate() == Interval(-1.,1));
    tube_a.sample(0.69, Interval::EMPTY_SET);
    CHECK(tube_a.get_slice(0.69)->input_gate() == Interval::EMPTY_SET);

    tube_a.get_slice(0.69)->set_input_gate(Interval(5.));
    CHECK(tube_a.get_slice(0.69)->input_gate().is_empty());
    CHECK(tube_a(0.69).is_empty());

    tube_a.set(Interval(0.1), 0.);
    CHECK(tube_a.get_slice(0)->input_gate() == Interval(0.1));
    CHECK(tube_a(0.) == Interval(0.1));

    tube_a.set(Interval(0.3), 1.);
    CHECK(tube_a.get_slice(tube_a.nb_slices()-1)->output_gate() == Interval(0.3));
    CHECK(tube_a(1.) == Interval(0.3));
  }
}
