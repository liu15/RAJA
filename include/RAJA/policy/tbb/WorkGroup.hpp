/*!
 ******************************************************************************
 *
 * \file
 *
 * \brief   Header file containing RAJA WorkGroup templates for
 *          tbb execution.
 *
 *          These methods should work on any platform.
 *
 ******************************************************************************
 */

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2016-20, Lawrence Livermore National Security, LLC
// and RAJA project contributors. See the RAJA/COPYRIGHT file for details.
//
// SPDX-License-Identifier: (BSD-3-Clause)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

#ifndef RAJA_tbb_WorkGroup_HPP
#define RAJA_tbb_WorkGroup_HPP

#include "RAJA/config.hpp"

#include "RAJA/internal/MemUtils_CPU.hpp"

// #include "RAJA/pattern/detail/WorkGroup.hpp"
#include "RAJA/pattern/WorkGroup.hpp"

#include "RAJA/policy/tbb/policy.hpp"

#include "RAJA/util/types.hpp"

namespace RAJA
{

namespace detail
{
// template <typename T, typename Reduce>
// class ReduceSeq
//     : public reduce::detail::BaseCombinable<T, Reduce, ReduceSeq<T, Reduce>>
// {
//   using Base = reduce::detail::BaseCombinable<T, Reduce, ReduceSeq<T, Reduce>>;

// public:
//   //! prohibit compiler-generated default ctor
//   ReduceSeq() = delete;

//   using Base::Base;
// };


}  // namespace detail

// RAJA_DECLARE_ALL_REDUCERS(seq_reduce, detail::ReduceSeq)

}  // namespace RAJA

#endif  // closing endif for header file include guard
