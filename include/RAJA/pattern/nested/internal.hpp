#ifndef RAJA_pattern_nested_internal_HPP
#define RAJA_pattern_nested_internal_HPP

#include "RAJA/config.hpp"
#include "RAJA/policy/cuda.hpp"
#include "RAJA/util/defines.hpp"
#include "RAJA/util/types.hpp"

#include "camp/camp.hpp"
#include "camp/concepts.hpp"
#include "camp/tuple.hpp"

#include <type_traits>

namespace RAJA
{
namespace nested
{

namespace internal
{


template <typename... Stmts>
using StatementList = camp::tuple<Stmts...>;


template <typename... EnclosedStmts>
struct Statement {
  using enclosed_statements_t = StatementList<EnclosedStmts...>;
  enclosed_statements_t enclosed_statements;
};


template <typename T>
using remove_all_t =
    typename std::remove_cv<typename std::remove_reference<T>::type>::type;

// Universal base of all For wrappers for type traits
struct ForList {
};
struct ForBase {
};
struct CollapseBase {
};
template <camp::idx_t ArgumentId, typename Policy>
struct ForTraitBase : public ForBase {
  constexpr static camp::idx_t index_val = ArgumentId;
  using index = camp::num<ArgumentId>;
  using index_type = camp::nil;  // default to invalid type
  using policy_type = Policy;
  using type = ForTraitBase;  // make camp::value compatible
};

using has_for_list = typename camp::bind_front<std::is_base_of, ForList>::type;

template <typename T>
using get_space_list = typename T::as_space_list;


/*
 * Get a camp::list of execution policies from a RAJA::nested::Policy to be
 * used to determine the execution space.
 *
 * This extracts the execution policy from each For<>, and extracts the
 * COLLAPSE policy P from inside of Collapse<P, ...>.  It drops all of the
 * For<>'s inside of a collapse.
 *
 */
template <typename Seq>
using get_space_policies = typename camp::flatten<
    typename camp::transform<get_space_list,
                             typename camp::filter_l<has_for_list,
                                                     Seq>::type>::type>::type;



template <typename Iterator>
struct iterable_value_type_getter {
  using type = typename Iterator::iterator::value_type;
};
template <>
struct iterable_value_type_getter<IndexSet> {
  // TODO: when static indexset drops, specialize properly
  using type = Index_type;
};

template <typename Segments>
using value_type_list_from_segments =
    typename camp::transform<iterable_value_type_getter, Segments>::type;


template <typename Segments>
using index_tuple_from_segments =
    typename camp::apply_l<camp::lambda<camp::tuple>,
                           value_type_list_from_segments<Segments> >::type;


template <typename Policy>
struct StatementExecutor;


template <typename PolicyType, typename SegmentTuple, typename ... Bodies>
struct LoopData {

  constexpr static size_t n_policies = camp::tuple_size<PolicyType>::value;

  using index_tuple_t = index_tuple_from_segments<
  typename SegmentTuple::TList>;


  const PolicyType policy;
  SegmentTuple segment_tuple;

  using BodiesTuple = camp::tuple<typename std::remove_reference<Bodies>::type...> ;
  const BodiesTuple bodies;
  index_tuple_t index_tuple;


  LoopData(PolicyType const &p, SegmentTuple const &s, Bodies const & ... b)
      : policy(p), segment_tuple(s), bodies(b...)
  {
  }

  template <camp::idx_t Idx, typename IndexT>
  RAJA_HOST_DEVICE void assign_index(IndexT const &i)
  {
    camp::get<Idx>(index_tuple) =
        camp::tuple_element_t<Idx, decltype(index_tuple)>{i};
  }
};


template<camp::idx_t LoopIndex, typename Data>
RAJA_INLINE
void invoke_lambda(Data && data){
  camp::invoke(data.index_tuple, camp::get<LoopIndex>(data.bodies));
}



template <camp::idx_t idx, camp::idx_t N>
struct StatementListExecutor;


template<typename PolicyTuple, typename Data>
void execute_statement_list(PolicyTuple && statement_list, Data && data){
  using policy_tuple_type = camp::decay<PolicyTuple>;
  StatementListExecutor<0, camp::tuple_size<policy_tuple_type>::value> launcher;
  launcher(statement_list, data);
}



template <typename StmtList, typename Data>
struct StatementListWrapper {

  using data_type = typename std::remove_reference<Data>::type;

  StmtList const &statement_list;
  Data &data;

  StatementListWrapper(StmtList const &pt, Data &d) : statement_list(pt), data{d} {}

  void operator()() const
  {
    execute_statement_list(statement_list, data);
  }
};



// Create a wrapper for this policy
template<typename PolicyT, typename Data>
auto make_statement_list_wrapper(PolicyT && policy, Data && data) ->
  StatementListWrapper<decltype(policy), camp::decay<Data>>
{
  return StatementListWrapper<decltype(policy), camp::decay<Data>>(
      policy, std::forward<Data>(data));
}


template <camp::idx_t statement_index, camp::idx_t num_statements>
struct StatementListExecutor{

  template<typename StmtList, typename Data>
  RAJA_INLINE
  void operator()(StmtList const &statement_list, Data &data) const {

    // Get the statement we're going to execute
    auto const &statement = camp::get<statement_index>(statement_list);

    // Create a wrapper for enclosed statements
    auto enclosed_wrapper = make_statement_list_wrapper(statement.enclosed_statements, data);

    // Execute this statement
    StatementExecutor<remove_all_t<decltype(statement)>> e{};
    e(statement, enclosed_wrapper);

    // call our next statement
    StatementListExecutor<statement_index+1, num_statements> next_sequential;
    next_sequential(statement_list, data);
  }
};


/*
 * termination case.
 * If any policies were specified, this becomes a NOP.
 * If no policies were specified, this defaults to Lambda<0>
 */

template <camp::idx_t num_statements>
struct StatementListExecutor<num_statements,num_statements> {

  template<typename StmtList, typename Data>
  RAJA_INLINE
  void operator()(StmtList const &, Data &data) const {
    if(num_statements == 0){
      invoke_lambda<0>(data);
    }
  }

};


/*!
 * Convenience object used to create thread-private a LoopData object.
 */
template <typename T>
struct NestedPrivatizer {
  using data_type = typename T::data_type;
  using value_type = camp::decay<T>;
  using reference_type = value_type &;

  data_type privatized_data;
  value_type privatized_wrapper;

  NestedPrivatizer(const T &o) : privatized_data{o.wrapper.data}, privatized_wrapper{value_type{privatized_data}} {}

  reference_type get_priv() { return privatized_wrapper; }
};


}  // end namespace internal
}  // end namespace nested
}  // end namespace RAJA


#endif /* RAJA_pattern_nested_internal_HPP */
