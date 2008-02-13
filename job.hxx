/**
 ** \file scheduler/job.hxx
 ** \brief Inline implementation of scheduler::Job.
 */

#ifndef SCHEDULER_JOB_HXX
# define SCHEDULER_JOB_HXX

# include <cassert>
# include "scheduler/scheduler.hh"
# include "scheduler/libcoroutine/Coro.h"

namespace scheduler
{

  inline
  Job::Job (Scheduler& scheduler)
    : scheduler_ (&scheduler),
      terminated_ (false),
      self_ (Coro_new ()),
      side_effect_free_ (false),
      pending_exception_ (0)
  {
  }

  inline
  Job::Job (const Job& model)
    : scheduler_ (model.scheduler_),
      terminated_ (false),
      self_ (Coro_new ()),
      side_effect_free_ (false),
      pending_exception_ (0)
  {
  }

  inline
  Job::~Job ()
  {
    scheduler_->unschedule_job (this);
    if (pending_exception_)
      delete pending_exception_;
    Coro_free (self_);
  }

  inline Scheduler&
  Job::scheduler_get () const
  {
    return *scheduler_;
  }

  inline void
  Job::terminate ()
  {
  }

  inline bool
  Job::terminated () const
  {
    return terminated_;
  }

  inline void
  Job::yield ()
  {
    scheduler_->resume_scheduler (this);
  }

  inline void
  Job::yield_front ()
  {
    scheduler_->resume_scheduler_front (this);
  }

  inline void
  Job::yield_until (libport::utime_t deadline)
  {
    scheduler_->resume_scheduler_until (this, deadline);
  }

  inline Coro*
  Job::coro_get () const
  {
    assert (self_);
    return self_;
  }

  inline void
  Job::start_job ()
  {
    scheduler_->add_job (this);
  }

  inline void
  Job::side_effect_free_set (bool s)
  {
    side_effect_free_ = s;
  }

  inline bool
  Job::side_effect_free_get () const
  {
    return side_effect_free_;
  }

  inline void
  Job::async_throw (std::exception* ue)
  {
    if (pending_exception_)
      delete pending_exception_;
    pending_exception_ = ue;
  }

  inline void
  Job::check_for_pending_exception ()
  {
    if (pending_exception_)
      throw *pending_exception_;
  }
} // namespace scheduler

#endif // !SCHEDULER_JOB_HXX
