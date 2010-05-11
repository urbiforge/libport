/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/containers.hh>
#include <libport/preproc.hh>
#include <libport/unit-test.hh>

using libport::test_suite;

#define LIST LIBPORT_LIST(1, 2, 3, 4, 5, 6, 7, 8, 9,)

static void
arity()
{
  BOOST_CHECK_EQUAL(LIBPORT_LIST_SIZE(LIBPORT_LIST()), 0);
  BOOST_CHECK_EQUAL(LIBPORT_LIST_SIZE(LIBPORT_LIST(1,)), 1);
  BOOST_CHECK_EQUAL(LIBPORT_LIST_SIZE(LIBPORT_LIST(1, 2,)), 2);
  BOOST_CHECK_EQUAL(LIBPORT_LIST_SIZE(LIBPORT_LIST(1, 2, 3,)), 3);
  BOOST_CHECK_EQUAL(LIBPORT_LIST_SIZE(LIBPORT_LIST(1, 2, 3, 4,)), 4);
  BOOST_CHECK_EQUAL(LIBPORT_LIST_SIZE(LIBPORT_LIST(1, 2, 3, 4, 5,)), 5);
  BOOST_CHECK_EQUAL(LIBPORT_LIST_SIZE(LIBPORT_LIST(1, 2, 3, 4, 5, 6,)), 6);
  BOOST_CHECK_EQUAL(LIBPORT_LIST_SIZE(LIBPORT_LIST(1, 2, 3, 4, 5, 6, 7,)), 7);
  BOOST_CHECK_EQUAL(LIBPORT_LIST_SIZE(LIBPORT_LIST(1, 2, 3, 4, 5, 6, 7, 8,)), 8);
  BOOST_CHECK_EQUAL(LIBPORT_LIST_SIZE(LIBPORT_LIST(1, 2, 3, 4, 5, 6, 7, 8, 9,)), 9);
}

#define HEAD_TAIL_LIST LIBPORT_LIST(42, 51, 69,)
static void
head_tail()
{
  BOOST_CHECK_EQUAL(LIBPORT_LIST_HEAD(HEAD_TAIL_LIST), 42);
  BOOST_CHECK_EQUAL(LIBPORT_LIST_HEAD(LIBPORT_LIST_TAIL(LIBPORT_LIST_TAIL(HEAD_TAIL_LIST))), 69);
}

#define MAP_MACRO(X) (X * X)
#define MAP_LIST LIBPORT_LIST_MAP(MAP_MACRO, LIST)

static void
map()
{
  BOOST_CHECK_EQUAL(LIBPORT_LIST_NTH(0, MAP_LIST), 1);
  BOOST_CHECK_EQUAL(LIBPORT_LIST_NTH(1, MAP_LIST), 4);
  BOOST_CHECK_EQUAL(LIBPORT_LIST_NTH(2, MAP_LIST), 9);
  BOOST_CHECK_EQUAL(LIBPORT_LIST_NTH(3, MAP_LIST), 16);
  BOOST_CHECK_EQUAL(LIBPORT_LIST_NTH(4, MAP_LIST), 25);
  BOOST_CHECK_EQUAL(LIBPORT_LIST_NTH(5, MAP_LIST), 36);
  BOOST_CHECK_EQUAL(LIBPORT_LIST_NTH(6, MAP_LIST), 49);
  BOOST_CHECK_EQUAL(LIBPORT_LIST_NTH(7, MAP_LIST), 64);
  BOOST_CHECK_EQUAL(LIBPORT_LIST_NTH(8, MAP_LIST), 81);
}

static void
nth()
{
  BOOST_CHECK_EQUAL(LIBPORT_LIST_NTH(0, LIST), 1);
  BOOST_CHECK_EQUAL(LIBPORT_LIST_NTH(1, LIST), 2);
  BOOST_CHECK_EQUAL(LIBPORT_LIST_NTH(2, LIST), 3);
  BOOST_CHECK_EQUAL(LIBPORT_LIST_NTH(3, LIST), 4);
  BOOST_CHECK_EQUAL(LIBPORT_LIST_NTH(4, LIST), 5);
  BOOST_CHECK_EQUAL(LIBPORT_LIST_NTH(5, LIST), 6);
  BOOST_CHECK_EQUAL(LIBPORT_LIST_NTH(6, LIST), 7);
  BOOST_CHECK_EQUAL(LIBPORT_LIST_NTH(7, LIST), 8);
  BOOST_CHECK_EQUAL(LIBPORT_LIST_NTH(8, LIST), 9);
}

static void
flatten()
{
  std::vector<int> expected;
  expected.push_back(0);
  expected.push_back(1);
  expected.push_back(2);
  std::vector<int> effective;
  LIBPORT_LIST_FLATTEN(LIBPORT_LIST(effective.push_back(0);, effective.push_back(1);, effective.push_back(2);, ));
  BOOST_CHECK_EQUAL(effective, expected);
}

#define APPLY_MACRO(X) effective.push_back(X);
static void
apply()
{
  std::vector<int> expected;
  expected.push_back(0);
  expected.push_back(1);
  expected.push_back(2);
  std::vector<int> effective;
  LIBPORT_LIST_APPLY(APPLY_MACRO, LIBPORT_LIST(0, 1, 2,));
  BOOST_CHECK_EQUAL(effective, expected);
}

static int arg_f(int a, int b, int c) { return (a + b) * c; };
static void
arg()
{
  BOOST_CHECK_EQUAL(42, LIBPORT_LIST_ARG(arg_f, LIBPORT_LIST(10, 11, 2,)));
}

static void
apply_vaargs()
{
  // Test the backend
  #define CAT(r, data, elem) data elem
  const char* val = LIBPORT_VAARGS_APPLY(CAT, "a", "a", "b", "c", "d");
  // Evaling VAARGS_APPLY inside the CHECK_EQUAL results in "a" "b" "c" "d" != "abdc". Don't ask...
  BOOST_CHECK_EQUAL(val, "aaabacad");

  // Test it the way it will be used
  #define USERMACRO_HELPER(r, data, elem) s= s*10 + data + elem;
  #define USERMACRO(a, ...) LIBPORT_VAARGS_APPLY(USERMACRO_HELPER, a, __VA_ARGS__)
  int s = 0;
  USERMACRO(2, 1, 2, 3, 4);
  BOOST_CHECK_EQUAL(s, 3456);
  #undef USERMACRO
  #undef USERMACRO_HELPER
  #undef CAT
}

test_suite*
init_test_suite()
{
  test_suite* suite = BOOST_TEST_SUITE("libport::preproc test suite");
  suite->add(BOOST_TEST_CASE(arity));
  suite->add(BOOST_TEST_CASE(head_tail));
  suite->add(BOOST_TEST_CASE(map));
  suite->add(BOOST_TEST_CASE(nth));
  suite->add(BOOST_TEST_CASE(flatten));
  suite->add(BOOST_TEST_CASE(apply));
  suite->add(BOOST_TEST_CASE(arg));
  suite->add(BOOST_TEST_CASE(apply_vaargs));
  return suite;
}
